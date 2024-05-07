/*-------------------------------------------------------------------------
    This source file is a part of the MC6809 Simulator
    For the latest info, see http:www.marrin.org/
    Copyright (c) 2018-2024, Chris Marrin
    All rights reserved.
    Use of this source code is governed by the MIT license that can be
    found in the LICENSE file.
-------------------------------------------------------------------------*/
//
//  BOSS9/core.h
//  Basic Operating System Services for the 6809
//
//  Created by Chris Marrin on 5/4/24.
//

#pragma once

#include <functional>
#include "MString.h"

namespace mc6809 {

using ConsoleCB = std::function<void(const char*)>;

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
};

class BOSSCore
{
  public:
    BOSSCore()
    {
        // FIXME: Need to make this cross platform
        _consoleCB = [](const char* s) { ::printf("%s", s); };
    }
    ~BOSSCore() { }
    
    bool call(Emulator*, uint16_t ea);
    
    void putc(char c) { char s[2] = " "; s[0] = c; print(s); }
    void puts(const char* s) { print(s); }
    
    void print(const char* s) const { _consoleCB(s); }
    
    void printf(const char* fmt, ...) const
    {
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
    }

    void vprintf(const char* fmt, va_list args) const
    {
        print(String::vformat(fmt, args).c_str());
    }
    
    void setStartInMonitor(bool b) { _startInMonitor = b; }
    
  protected:
    
  private:
      ConsoleCB _consoleCB;
      bool _startInMonitor = false;
};

}
