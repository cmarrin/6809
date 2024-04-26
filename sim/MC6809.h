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
    ANDCC, ASL, ASR, BCC, BCS, BEQ, BGE, BGT,
    BHI, BHS, BIT, BLE, BLO, BLS, BLT, BMI,
    BNE, BPL, BRA, BRN, BSR, BVC, BVS, CLR,
    CMP8, CMP16, COM, CWAI, DAA, DEC, EOR, EXG,
    INC, JMP, JSR, LD8, LD16, LEA, LSR, MUL,
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

enum class CCOp : uint8_t { None, HNZVC };

struct Opcode
{
    Op op : 7;
    bool loadLeft : 1;
    Adr adr : 4;
    Reg reg : 4;
    CCOp c : 3;
    uint8_t cycles : 5;
    uint8_t bytes : 2;
};

struct CC
{
    bool E : 1; // Entire       : All registers stacked from last interrupt
    bool F : 1; // FIRQ Mask    : Disable fast interrupt request (FIRQ)
    bool H : 1; // Half Carry   : Carry from bit 3 (for decimal arith)
    bool I : 1; // IRQ Mask     : Disable interrupt request (IRQ)
    bool N : 1; // Negative     : MSB of previous operation was set
    bool Z : 1; // Zero         : Result of previous operation was zero
    bool V : 1; // Overflow     : Previous operation caused signed arith overflow
    bool C : 1; // Carry        : Carry or borrow from bit 7 of previous operation
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
    void push8(uint16_t& s, uint8_t v)
    {
        ram[--s] = v;
    }
    
    void push16(uint16_t& s, uint16_t v)
    {
        ram[--s] = v;
        ram[--s] = v >> 8;
    }
    
    // Update the HNZVC condition codes
    void updateH(uint8_t a, uint8_t b, uint16_t r)
    {
        cc.H = ((a ^ b ^ r) & 0x10) != 0;
    }
    
    void updateNZ8(uint16_t r)
    {
        cc.Z = uint8_t(r) == 0;
        cc.N = (r & 0x80) != 0;
    }
    
    void updateNZ16(uint32_t r)
    {
        cc.Z = uint16_t(r) == 0;
        cc.N = (r & 0x8000) != 0;
    }
    
    void updateC8(uint16_t r)
    {
        cc.C = (r & 0x100) != 0;
    }
    
    void updateC16(uint32_t r)
    {
        cc.C = (r & 0x10000) != 0;
    }
    
    void updateV8(uint8_t a, uint8_t b, uint16_t r)
    {
        cc.V = ((a ^ b ^ r ^ (r >> 1)) & 0x80) != 0;
    }
    
    void updateV16(uint16_t a, uint16_t b, uint32_t r)
    {
        cc.V = ((a ^ b ^ r ^ (r >> 1)) & 0x8000) != 0;
    }
    
    uint8_t* ram = nullptr;
    
    union {
        struct { uint8_t a; uint8_t b; };
        uint16_t d = 0;
    };
    
    uint16_t x = 0;
    uint16_t y = 0;
    uint16_t u = 0;
    uint16_t s = 0;
    uint16_t pc = 0;
    uint8_t dp = 0;
    
    union {
        CC cc;
        uint8_t ccByte = 0;
    };
};

}
