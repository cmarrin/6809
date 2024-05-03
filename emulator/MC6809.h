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
#include <iostream>

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

// Indexed mode
//
// See doc/m6809pm/sections.htm#sec2 for info about indexed mode
//

enum class RR { X = 0, Y = 0b00100000, U = 0b01000000, S = 0b01100000 };
enum class IdxMode {
    ConstRegNoOff       = 0b00000100,
    ConstReg8Off        = 0b00001000,
    ConstReg16Off       = 0b00001001,
    AccAOffReg          = 0b00000110,
    AccBOffReg          = 0b00000101,
    AccDOffReg          = 0b00001011,
    Inc1Reg             = 0b00000000,
    Inc2Reg             = 0b00000001,
    Dec1Reg             = 0b00000010,
    Dec2Reg             = 0b00000011,
    ConstPC8Off         = 0b00001100,
    ConstPC16Off        = 0b00001101,
    Extended            = 0b00001111,
};

// postbyte determines which indexed mode is used. If the MSB is 0
// then this is the 5 bit constant offset direct mode. Bits 6 and 5
// are the register number and bits 4-0 are the signed 5 bit offset

static constexpr uint8_t IdxModeMask = 0b00001111;
static constexpr uint8_t IndexedIndMask = 0b00010000;

enum class Adr : uint8_t { None, Direct, Inherent, Rel, RelL, RelP, Immed8, Immed16, Indexed, Extended };

// Register enums match the register numbers used by EXG and TFR
// These are used to load and store of regs. The Reg::M enum is
// used to load or store the mem at ea
enum class Reg : uint8_t {
    D = 0x0, X = 0x1, Y = 0x2, U = 0x3, S = 0x4, PC = 0X5,
    A = 0x8, B = 0x9, CC = 0xa, DP = 0xb,
    M8 = 0xd, M16 = 0xe, None = 0xf
};

// Determines what type of load and/or store is done with reg
enum class Left : uint8_t { None, Ld, St, LdSt };
enum class Right : uint8_t { None, Ld8, Ld16 };

enum class CCOp : uint8_t { None, HNZVC };

struct Opcode
{
    Op op : 7;
    Left left : 2;
    Right right : 2;
    Adr adr : 4;
    Reg reg : 4;
    CCOp c : 3;
    uint8_t cycles : 5;
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

class Emulator
{
public:
    Emulator(uint32_t size)
    {
        ram = new uint8_t[size];
    }
    
    // Assumes data is in s19 format
    void load(std::istream& stream);
    
    ~Emulator() { delete [ ] ram; }
    
    bool execute(uint16_t addr);
    
private:
    uint16_t getReg(Reg r)
    {
        switch(r) {
            default: return 0;
            case Reg::A:    return a;
            case Reg::B:    return b;
            case Reg::D:    return d;
            case Reg::X:    return x;
            case Reg::Y:    return y;
            case Reg::U:    return u;
            case Reg::S:    return s;
            case Reg::CC:   return ccByte;
            case Reg::PC:   return pc;
            case Reg::DP:   return dp;
        }
    }

    void setReg(Reg r, uint16_t v)
    {
        switch(r) {
            default: break;
            case Reg::A:    a = v; break;
            case Reg::B:    b = v; break;
            case Reg::D:    d = v; break;
            case Reg::X:    x = v; break;
            case Reg::Y:    y = v; break;
            case Reg::U:    u = v; break;
            case Reg::S:    s = v; break;
            case Reg::CC:   ccByte = v; break;
            case Reg::PC:   pc = v; break;
            case Reg::DP:   dp = v; break;
        }
    }

    void push8(uint16_t& s, uint8_t v)
    {
        ram[--s] = v;
    }
    
    void push16(uint16_t& s, uint16_t v)
    {
        ram[--s] = v;
        ram[--s] = v >> 8;
    }
    
    uint8_t pop8(uint16_t& s)
    {
        return ram[s++];
    }
    
    uint16_t pop16(uint16_t& s)
    {
        uint16_t r = ram[s++];
        r <<= 8;
        r |= ram[s++];
        return r;
    }
    
    uint8_t next8()
    {
        uint8_t v = ram[pc];
        pc += 1;
        return v;
    }
    
    uint16_t next16()
    {
        uint16_t v = (uint16_t(ram[pc]) << 8) | uint16_t(ram[pc + 1]);
        pc += 2;
        return v;
    }
    
    uint8_t load8(uint16_t ea)
    {
        return ram[ea];
    }
    
    uint16_t load16(uint16_t ea)
    {
        return (uint16_t(ram[ea]) << 8) | uint16_t(ram[ea + 1]);
    }
    
    void store8(uint16_t ea, uint8_t v)
    {
        ram[ea] = v;
    }
    
    void store16(uint16_t ea, uint16_t v)
    {
        ram[ea] = v >> 8;
        ram[ea + 1] = v;
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
