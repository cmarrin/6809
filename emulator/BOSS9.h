/*-------------------------------------------------------------------------
    This source file is a part of the MC6809 Simulator
    For the latest info, see http:www.marrin.org/
    Copyright (c) 2018-2024, Chris Marrin
    All rights reserved.
    Use of this source code is governed by the MIT license that can be
    found in the LICENSE file.
-------------------------------------------------------------------------*/
//
//  BOSS9.h
//  Basic Operating System Services for the 6809
//
//  Created by Chris Marrin on 5/4/24.
//

#pragma once

#include "string.h"
#include "MC6809.h"

namespace mc6809 {

using ConsoleCB = std::function<void(const char*)>;

static constexpr const char* MainPromptString = "BOSS9> ";
static constexpr const char* LoadingPromptString = "Loading> ";
static constexpr uint16_t CmdBufSize = 80;

class Emulator;

// These must match BOSS9.inc
enum class Func : uint16_t {
    putc = 0xFC00,    // output char in A to console
    puts = 0xFC02,    // output string pointed to by X (null terminated)
    putsn = 0xFC04,   // Output string pointed to by X for length in Y
    getc = 0xFC06,    // Get char from console, return it in A
    peekc = 0xFC08,   // Return in A a 1 if a char is available and 0 otherwise
    gets = 0xFC0A,    // Get a line terminated by \n, place in buffer
                                            // pointed to by X, with max length in Y
    peeks = 0xFC0C,   // Return in A a 1 if a line is available and 0 otherwise.
                                            // If available return length of line in Y
    exit = 0xFC0E,    // Exit program. A contains exit code
};

class BOSS9Base
{
  public:
    BOSS9Base(uint8_t* ram) : _emu(ram, this) { }
    
    virtual ~BOSS9Base() { }
        
    bool call(Emulator*, uint16_t ea);
    
    void puts(const char* s) const
    {
        while (*s) {
            putc(*s++);
        }
    }
        
    void printf(const char* fmt, ...) const
    {
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
    }

    void vprintf(const char* fmt, va_list args) const
    {
        puts(m8r::string::vformat(fmt, args).c_str());
    }

    // Calls to emulator
    void loadStart() { _emu.loadStart(); }
    bool loadLine(const char* data, bool& finished) { return _emu.loadLine(data, finished); }
    uint16_t loadFinish() { return _emu.loadFinish(); }
    void setStack(uint16_t stack) { _emu.setStack(stack); }
    
    bool startExecution(uint16_t addr, bool startInMonitor = false);
    bool continueExecution();
    
    void enterMonitor()
    {
        _runState = RunState::Cmd;
        _needPrompt = true;
    }
    
  protected:
    // Methods to override
    virtual void putc(char c) const = 0;
    virtual int getc() = 0;
    virtual bool handleRunLoop() = 0;

    bool _echoBS = false; // If true when backspace received, sends <space><backspace> to erase char
    
  private:
    void promptIfNeeded()
    {
        if (_needPrompt) {
            _emu.printInstruction(_emu.getPC());
            puts((_runState == RunState::Loading) ? LoadingPromptString : MainPromptString);
            _cursor = 0;
            _needPrompt = false;
        }
    }
    
    void leaveMonitor()
    {
        _runState = RunState::Running;
        _needPrompt = false;
    }
    
    void getCommand();
    void processCommand();
    bool executeCommand(m8r::string _cmdElements[3]);

    void showBreakpoint(uint8_t i) const;
    
    bool checkEscape(int c);
    
    bool toNum(m8r::string& s, uint32_t& num);

    bool _needPrompt = false;
    
    char _cmdBuf[CmdBufSize];
    uint32_t _cursor = 0;
    
    uint16_t _startAddr = 0;
    
    RunState _runState = RunState::Cmd;
    
    Emulator _emu;
};

template<uint32_t size> class BOSS9 : public BOSS9Base
{
  public:
    BOSS9() : BOSS9Base(_ram) { }
    
    ~BOSS9() { }
    
  private:
    uint8_t _ram[size];
};

}
