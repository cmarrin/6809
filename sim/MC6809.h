/*-------------------------------------------------------------------------
    This source file is a part of the MC6809 Simulator
    For the latest info, see http:www.marrin.org/
    Copyright (c) 2018-2024, Chris Marrin
    All rights reserved.
    Use of this source code is governed by the MIT license that can be
    found in the LICENSE file.
-------------------------------------------------------------------------*/
//
//  MC6809.h
//  6809 simulator
//
//  Created by Chris Marrin on 4/22/24.
//

#pragma once

#include <cstdint>

namespace mc6809 {

// Opcode table

// The 6809 has 2 extended opcodess Page2 (0x10) and Page3 (0x11). These
// typically indicate a change the opcode in the following byte. For instance
// for branches it indicates a long (16 bit) branch.

enum class Op : uint8_t {
    ILL, Page2, Page3, ABX, ADC, ADD8, ADD16, AND,
    ANDCC, ASR, BCC, BCS, BEQ, BGE, BGT, BHI,
    BHS, BIT, BLE, BLO, BLS, BLT, BMI, BNE,
    BPL, BRA, BRN, BSR, BVC, BVS, CLR, CMP8,
    CMP16, COM, CWAI, DAA, DEC, EOR, EXG, INC,
    JMP, JSR, LD8, LD16, LEA, LSL, LSR, MUL,
    NEG, NOP, OR, ORCC, PSH, PUL, ROL, ROR,
    RTI, RTS, SBC, SEX, ST8, ST16, SUB8, SUB16,
    SWI, SWI2, SWI3, SYNC, TFR, TST, FIRQ, IRQ,
    NMI, RESTART
};

// The Special mode has info in the next byte which is not data. for instance
// the PSH opcode has bits for which registers to push and tbe EXG and TFR
// opcodes have the from and to registers
enum class Adr : uint8_t { None, Direct, Inherent, Rel, RelL, Immed8, Immed16, Special, Indexed, Extended };
enum class Reg : uint8_t { None, A, B, D, X, Y, U, S, PC, DP, CC };

struct Opcode
{
    Op op : 7;
    bool loadLeft : 1;
    Adr adr : 4;
    Reg reg : 4;
    uint8_t cycles : 5;
    uint8_t bytes : 2;
};

class sim
{
public:
    sim(uint32_t size)
    {
        ram = new uint8_t[size];
    }
    
    ~sim() { delete [ ] ram; }
    
    bool execute(uint16_t addr);
    
private:
    uint8_t* ram = nullptr;
    
    union {
        struct { uint8_t a; uint8_t b; };
        uint16_t d;
    };
    
    uint16_t x, y, u, s, pc;
    uint8_t dp;
};

}
