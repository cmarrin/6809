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

void BOSS9Base::getCommand()
{
    bool haveCmd = false;
    
    while (true) {
        int c = getc();
        if (c <= 0) {
            break;
        }
        if (_cursor >= CmdBufSize - 1) {
            printf("*** too many chars in cmdbuf\n");
            _cursor = 0;
            haveCmd = true;
            break;
        }

        if (c == '\n') {
            haveCmd = true;
            _cmdBuf[_cursor] = '\0';
            break;
        }
        
        _cmdBuf[_cursor++] = char(c);
    }
    
    if (haveCmd) {
        if (_cursor > 0) {
            processCommand();
        }
        prompt();
    }
}

void BOSS9Base::processCommand()
{
    if (_loading) {
        bool finished;
        if (!loadLine(_cmdBuf, finished)) {
            _loading = false;
            printf("Error loading\n");
        }
        if (finished) {
            _startAddr = loadFinish();
            _loading = false;
            printf("Load complete, start addr = 0x%04x\n", _startAddr);
        }
    } else if (_cmdBuf[0] == 'l') {
        printf("Ready to start loading. ESC to abort\n");
        _loading = true;
        loadStart();
    } else {
        printf("'%s': no such command\n", _cmdBuf);
    }
    _cursor = 0;
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
    
    if (_inMonitor) {
        // Do initial prompt
        prompt();
        _cursor = 0;
    }
    return true;
}

bool BOSS9Base::continueExecution()
{
    if (_inMonitor) {
        getCommand();
        return true;
    }
    return _emu.execute();
}
