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

static constexpr const char* PromptString = "BOSS9> ";
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
    
    void enterMonitor();
    
    bool call(Emulator*, uint16_t ea);
    
    void puts(const char* s)
    {
        while (*s) {
            putc(*s++);
        }
    }
        
    void printf(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
    }

    void vprintf(const char* fmt, va_list args)
    {
        puts(m8r::string::vformat(fmt, args).c_str());
    }
    
    void setStartInMonitor(bool b) { _inMonitor = b; }
    
    // Calls to emulator
    uint16_t load(const char* data) { return _emu.load(data); }
    void setStack(uint16_t stack) { _emu.setStack(stack); }
    
    bool startExecution(uint16_t addr, bool startInMonitor = false);
    bool continueExecution();
    
  protected:
    // Methods to override
    virtual void putc(char c) = 0;
    virtual int getc() = 0;
    virtual bool handleRunLoop() = 0;
    virtual void exit(int n) = 0;
    
  private:
    void prompt() { puts(PromptString); }
    void handleCommand();
    
    bool _inMonitor = false;
    
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