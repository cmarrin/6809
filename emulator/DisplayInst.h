/*-------------------------------------------------------------------------
    This source file is a part of the MC6809 Simulator
    For the latest info, see http:www.marrin.org/
    Copyright (c) 2018-2024, Chris Marrin
    All rights reserved.
    Use of this source code is governed by the MIT license that can be
    found in the LICENSE file.
-------------------------------------------------------------------------*/
//
//  DisplayInst.h
//  Generate 6809 instruction string
//
//  Created by Chris Marrin on 4/22/24.
//

#pragma once

#include "MC6809.h"
#include "string.h"

namespace mc6809 {

class DisplayInst
{
public:
    // Place inst string in s and return next inst addr
    static uint16_t instToString(const Emulator& engine, m8r::string& s, uint16_t addr);
    
    static const char* regToString(Reg, Op prevOp = Op::NOP);
};

}
