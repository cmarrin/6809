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
#include "DisplayInst.h"

namespace mc6809 {

static constexpr const char* MainPromptString = "BOSS9> ";
static constexpr const char* LoadingPromptString = "Loading> ";
static constexpr uint16_t CmdBufSize = 100;

class Emulator;

// These must match BOSS9.inc
enum class Func : uint16_t {
    putc        = 0xFC00,   // output char in A to console
    puts        = 0xFC02,   // output string pointed to by X (null terminated)
    putsn       = 0xFC04,   // Output string pointed to by X for length in Y
    getc        = 0xFC06,   // Get char from console, return it in A
    peekc       = 0xFC08,   // Return in A a 1 if a char is available and 0 otherwise
    getsn       = 0xFC0A,   // Get a line terminated by \n, place in buffer
                            // pointed to by X, with max length in Y
    peeksn      = 0xFC0C,   // Return in A a 1 if a line is available and 0 otherwise.
                            // If available return length of line in Y
    exit        = 0xFC0E,   // Exit program. A contains exit code
    mon         = 0xFC10,   // Enter monitor
    ldStart     = 0xFC12,   // Start loading s-records
    ldLine      = 0xFC14,   // Load an s-record line
    ldEnd       = 0xFC16,   // End loading s-records

    printf      = 0xFC20,   // Formatted print: TOS=fmt, (varargs)
    format      = 0xFC22,   // Format string
    memset      = 0xFC24,   // Set memory: TOS=
    irand       = 0xFC26,   //
    imin        = 0xFC28,   //
    imax        = 0xFC2a,   //
    initargs    = 0xFC2c,   //
    argint8     = 0xFC2e,   //
    argint16    = 0xFC30,   //

    switch1     = 0xFC40,   // TOS -> N, Table, Value
    switch2     = 0xFC42,   // Table is a list of N value/addr pairs
                            //Binary search table looking for value
                            //when found return addr in X. if not
                            //found return Table + N * (<1/2> + 2)
    idiv8       = 0xFC44,
    idiv16      = 0xFC46,
    udiv8       = 0xFC48,
    udiv16      = 0xFC4a,
};

class BOSS9Base
{
  public:
    BOSS9Base(uint8_t* ram) : _emu(ram, this) { }
    
    virtual ~BOSS9Base() { }
        
    bool call(Func);
    
    bool startExecution(uint16_t addr, bool startInMonitor = false);
    bool continueExecution();
    
    void enterMonitor()
    {
        _runState = RunState::Cmd;
        _needPrompt = true;
        _needInstPrint = true;
    }
    
    Emulator& emulator() { return _emu; }
    const Emulator& emulator() const { return _emu; }
    
    virtual void putc(char c) const = 0;

    void puts(const char* s) const
    {
        while (*s) {
            putc(*s++);
        }
    }

    void printF(const char* fmt, ...) const
    {
        va_list args;
        va_start(args, fmt);
        vprintF(fmt, args);
    }

    void vprintF(const char* fmt, va_list args) const
    {
        puts(m8r::string::vformat(fmt, args).c_str());
    }

  protected:
    // Methods to override
    virtual int getc() = 0;
    virtual bool handleRunLoop() = 0;

    bool _echoBS = false; // If true when backspace received, sends <space><backspace> to erase char
    
  private:
    void promptIfNeeded()
    {
        if (_needPrompt) {
            if (_needInstPrint) {
                m8r::string s;
                DisplayInst::instToString(emulator(), s, emulator().getReg(Reg::PC));
                printF("%s", s.c_str());
            }
            puts((_runState == RunState::Loading) ? LoadingPromptString : MainPromptString);
            _cursor = 0;
            _needPrompt = false;
            _needInstPrint = false;

        }
    }
    
    void leaveMonitor()
    {
        _runState = RunState::Running;
        _needPrompt = false;
        _needInstPrint = false;
    }
    
    void getCommand();
    void processCommand();
    bool executeCommand(m8r::string _cmdElements[3]);

    void showBreakpoint(uint8_t i) const;
    
    bool checkEscape(int c);
    
    bool toNum(m8r::string& s, uint32_t& num);

    bool _needPrompt = false;
    bool _needInstPrint = false;
    
    char _cmdBuf[CmdBufSize];
    uint32_t _cursor = 0;
    
    uint16_t _startAddr = 0;
    
    RunState _runState = RunState::Cmd;
    
    Emulator _emu;
    
    float _startTime = 0;
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
