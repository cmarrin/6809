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

#include <unistd.h>

#include "BOSS9.h"
#include "MC6809.h"

using namespace mc6809;

bool BOSSCore::call(Emulator* engine, uint16_t ea)
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
            // Enter the monitor eventually. For now just loop
            while(1) {
                sleep(1);
            }
        
        default: return false;
    }
    return false;
}
