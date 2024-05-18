/*-------------------------------------------------------------------------
    This source file is a part of the MC6809 Simulator
    For the latest info, see http:www.marrin.org/
    Copyright (c) 2018-2024, Chris Marrin
    All rights reserved.
    Use of this source code is governed by the MIT license that can be
    found in the LICENSE file.
-------------------------------------------------------------------------*/
//
//  BOSS9.cpp
//  Basic Operating System Services for the 6809
//
//  Created by Chris Marrin on 5/4/24.
//

#include "BOSS9.h"
#include "MC6809.h"

using namespace mc6809;

void BOSS9Base::enterMonitor()
{
    prompt();
    handleCommand();
}

void BOSS9Base::handleCommand()
{
    m8r::string cmd;
    cmd.reserve(CmdBufSize);
    bool haveCmd = false;
    
    while (true) {
        int c = getc();
        if (c == 0) {
            break;
        }
        if (c < 0) {
            printf("*** getc error: c='%02x'\n", c);
            break;
        }
        if (cmd.size() > CmdBufSize) {
            printf("*** too many chars in cmdbuf\n");
            break;
        }
        
        putc(c);
        
        if (c == '\n') {
            haveCmd = true;
            break;
        }
        cmd += char(c);
    }
    
    if (haveCmd) {
        printf("Got Command \"%s\"\n", cmd.c_str());
        prompt();
    }
}

bool BOSS9Base::call(Emulator* engine, uint16_t ea)
{
    switch (Func(ea)) {
        case Func::putc:
            putc(engine->getA());
            break;
        case Func::puts: {
            const char* s = reinterpret_cast<const char*>(engine->getAddr(engine->getX()));
            puts(s);
            break;
        }
        case Func::exit:
            exit(0);
        
        default: return false;
    }
    return false;
}

bool BOSS9Base::startExecution(uint16_t addr, bool startInMonitor)
{
    _inMonitor = startInMonitor;
    _emu.setPC(addr);
    return true;
}

bool BOSS9Base::continueExecution()
{
    if (_inMonitor) {
        enterMonitor();
        return true;
    }
    return _emu.execute();
}
