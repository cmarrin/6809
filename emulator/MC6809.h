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

#include "core.h"

namespace mc6809 {

static constexpr uint16_t SystemAddrStart = 0xFC00;

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
// Right::St8 and Right::St16 are used for Store operations.
// In the post process if these enums are present the value
// in left will be stored at ea.
enum class Left : uint8_t { None, Ld, St, LdSt };
enum class Right : uint8_t { None, Ld8, Ld16, St8, St16 };

enum class CCOp : uint8_t { None, HNZVC };

struct Opcode
{
    Op op : 7;
    Left left : 3;
    Right right : 3;
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
        _ram = new uint8_t[size];
    }
    
    ~Emulator() { delete [ ] _ram; }
    
    // Assumes data is in s19 format
    // Returns the start addr of the program
    uint16_t load(std::istream& stream);
    
    void setStack(uint16_t stack) { _s = stack; }
    
    bool execute(uint16_t addr);
    
    uint8_t getA() const { return _a; }
    uint8_t getB() const { return _b; }
    uint8_t getDP() const { return _dp; }
    uint8_t getCC() const { return _ccByte; }
    uint16_t getX() const { return _x; }
    uint16_t getY() const { return _y; }
    uint16_t getU() const { return _u; }
    uint16_t getS() const { return _s; }
    uint16_t getPC() const { return _pc; }
    
    uint8_t* getAddr(uint16_t ea) { return _ram + ea; }
    
private:
    uint16_t getReg(Reg r)
    {
        switch(r) {
            default: return 0;
            case Reg::A:    return _a;
            case Reg::B:    return _b;
            case Reg::D:    return _d;
            case Reg::X:    return _x;
            case Reg::Y:    return _y;
            case Reg::U:    return _u;
            case Reg::S:    return _s;
            case Reg::CC:   return _ccByte;
            case Reg::PC:   return _pc;
            case Reg::DP:   return _dp;
        }
    }

    void setReg(Reg r, uint16_t v)
    {
        switch(r) {
            default: break;
            case Reg::A:    _a = v; break;
            case Reg::B:    _b = v; break;
            case Reg::D:    _d = v; break;
            case Reg::X:    _x = v; break;
            case Reg::Y:    _y = v; break;
            case Reg::U:    _u = v; break;
            case Reg::S:    _s = v; break;
            case Reg::CC:   _ccByte = v; break;
            case Reg::PC:   _pc = v; break;
            case Reg::DP:   _dp = v; break;
        }
    }

    void push8(uint16_t& s, uint8_t v)
    {
        _ram[--s] = v;
    }
    
    void push16(uint16_t& s, uint16_t v)
    {
        _ram[--s] = v;
        _ram[--s] = v >> 8;
    }
    
    uint8_t pop8(uint16_t& s)
    {
        return _ram[s++];
    }
    
    uint16_t pop16(uint16_t& s)
    {
        uint16_t r = _ram[s++];
        r <<= 8;
        r |= _ram[s++];
        return r;
    }
    
    uint8_t next8()
    {
        uint8_t v = _ram[_pc];
        _pc += 1;
        return v;
    }
    
    uint16_t next16()
    {
        uint16_t v = (uint16_t(_ram[_pc]) << 8) | uint16_t(_ram[_pc + 1]);
        _pc += 2;
        return v;
    }
    
    uint8_t load8(uint16_t ea)
    {
        return _ram[ea];
    }
    
    uint16_t load16(uint16_t ea)
    {
        return (uint16_t(_ram[ea]) << 8) | uint16_t(_ram[ea + 1]);
    }
    
    void store8(uint16_t ea, uint8_t v)
    {
        if (ea >= SystemAddrStart) {
            printf("Address %0x4 is read-only\n", ea);
        } else {
            _ram[ea] = v;
        }
    }
    
    void store16(uint16_t ea, uint16_t v)
    {
        if (ea >= SystemAddrStart) {
            printf("Address %0x4 is read-only\n", ea);
        } else {
            _ram[ea] = v >> 8;
            _ram[ea + 1] = v;
        }
    }
    
    // Update the HNZVC condition codes
    void updateH(uint8_t a, uint8_t b, uint16_t r)
    {
        _cc.H = ((a ^ b ^ r) & 0x10) != 0;
    }
    
    void updateNZ8(uint16_t r)
    {
        _cc.Z = uint8_t(r) == 0;
        _cc.N = (r & 0x80) != 0;
    }
    
    void updateNZ16(uint32_t r)
    {
        _cc.Z = uint16_t(r) == 0;
        _cc.N = (r & 0x8000) != 0;
    }
    
    void updateC8(uint16_t r)
    {
        _cc.C = (r & 0x100) != 0;
    }
    
    void updateC16(uint32_t r)
    {
        _cc.C = (r & 0x10000) != 0;
    }
    
    void updateV8(uint8_t a, uint8_t b, uint16_t r)
    {
        _cc.V = ((a ^ b ^ r ^ (r >> 1)) & 0x80) != 0;
    }
    
    void updateV16(uint16_t a, uint16_t b, uint32_t r)
    {
        _cc.V = ((a ^ b ^ r ^ (r >> 1)) & 0x8000) != 0;
    }
    
    uint8_t* _ram = nullptr;
    
    union {
        struct { uint8_t _a; uint8_t _b; };
        uint16_t _d = 0;
    };
    
    uint16_t _x = 0;
    uint16_t _y = 0;
    uint16_t _u = 0;
    uint16_t _s = 0;
    uint16_t _pc = 0;
    uint8_t _dp = 0;
    
    union {
        CC _cc;
        uint8_t _ccByte = 0;
    };
    
    BOSSCore _core;
};

}
