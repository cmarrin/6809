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
#include <cstring>

#include "srec.h"

//#define COMPUTE_CYCLES

namespace mc6809 {

static constexpr uint16_t SystemAddrStart = 0xFC00;
static constexpr uint32_t InstructionsToExecutePerContinue = 10;
static constexpr uint8_t NumBreakpoints = 4;

// Opcode table

// The 6809 has 2 extended opcodess Page2 (0x10) and Page3 (0x11). These
// typically indicate a change the opcode in the following byte. For instance
// for branches it indicates a long (16 bit) branch.

#undef DEC // For Arduino

enum class Op : uint8_t {
    ILL, Page2, Page3, ABX, ADC, ADD8, ADD16, AND,
    ANDCC, ASL, ASR, BCC, BCS, BEQ, BGE, BGT,
    BHI, BHS, BIT, BLE, BLO, BLS, BLT, BMI,
    BNE, BPL, BRA, BRN, BSR, BVC, BVS, CLR,
    CMP8, CMP16, COM, CWAI, DAA, DEC, EOR, EXG,
    INC, JMP, JSR, LD8, LD16, LEA, LSR, MUL,
    NEG, NOP, OR, ORCC, PSH, PUL, ROL, ROR,
    RTI, RTS, SBC, SEX, ST8, ST16, SUB8, SUB16,
    SWI, SYNC, TFR, TST, FIRQ, IRQ, NMI, RESTART
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

struct Opcode
{
    Op op : 7;
    Left left : 3;
    Right right : 3;
    Adr adr : 4;
    Reg reg : 4;
    Reg page2Reg : 4;
    Reg page3Reg : 4;
};

struct CC
{
    bool C : 1; // Carry        : Carry or borrow from bit 7 of previous operation
    bool V : 1; // Overflow     : Previous operation caused signed arith overflow
    bool Z : 1; // Zero         : Result of previous operation was zero
    bool N : 1; // Negative     : MSB of previous operation was set
    bool I : 1; // IRQ Mask     : Disable interrupt request (IRQ)
    bool H : 1; // Half Carry   : Carry from bit 3 (for decimal arith)
    bool F : 1; // FIRQ Mask    : Disable fast interrupt request (FIRQ)
    bool E : 1; // Entire       : All registers stacked from last interrupt
};

enum class BPStatus { Empty, Enabled, Disabled };

struct BreakpointEntry
{
    uint16_t addr;
    BPStatus status = BPStatus::Empty;
};

class BOSS9Base;

class SRecordInfo : public SRecordParser
{
  public:
    SRecordInfo(uint8_t* ram, BOSS9Base* boss9) : _ram(ram), _boss9(boss9) { }
    void init()
    {
        SRecordParser::init();
        _startAddr = 0;
        _startAddrSet = false;
    }
    
    virtual  ~SRecordInfo() { }
    
    bool finished() { return _startAddrSet; }
    uint16_t startAddr() const { return _startAddr; }

  protected:
    virtual bool Header(const SRecordHeader *sRecHdr)
    {
        return true;
    }
    
    virtual bool StartAddress(const SRecordData *sRecData)
    {
        _startAddr = sRecData->m_addr;
        _startAddrSet = true;
        return true;
    }
    
    virtual bool Data(const SRecordData *sRecData)
    {
        // FIXME: Need to handle ranges, which means we need to pass in the ram size
        
        // If the start addr has not been set, set it to the start of the first record.
        // The StartAddress function can change this at the end
        if (!_startAddrSet) {
            _startAddr = sRecData->m_addr;
        }
        memcpy(_ram + sRecData->m_addr, sRecData->m_data, sRecData->m_dataLen);
        return true;
    }
    
    virtual void ParseError(unsigned linenum, const char *fmt, va_list args);
    
  private:
    uint8_t* _ram = nullptr;
    uint16_t _startAddr = 0;
    bool _startAddrSet = false;
    
    BOSS9Base* _boss9 = nullptr;
};

class Emulator
{
public:
    Emulator(uint8_t* ram, BOSS9Base* boss9) : sRecInfo(ram, boss9)
    {
        _ram = ram;
        _boss9 = boss9;
        
        // Testing
        _haveBreakpoints = true;
        _breakpoints[0].status = BPStatus::Enabled;
        _breakpoints[0].addr = 0x205;
    }
    
    ~Emulator() { }
    
    // Assumes data is in s19 format
    // Returns the start addr of the program
    void loadStart();
    bool loadLine(const char* data, bool& finished);
    uint16_t loadFinish();
    
    void setStack(uint16_t stack) { _s = stack; }
    
    void setPC(uint16_t addr) { _pc = addr; }
    
    bool execute();
    
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
    
    // Breakpoint support
    bool breakpoint(uint8_t i, BreakpointEntry& entry);
    bool setBreakpoint(uint16_t addr);
    bool clearBreakpoint(uint8_t i);
    bool clearAllBreakpoints();
    bool disableBreakpoint(uint8_t i);
    bool disableAllBreakpoints();
    bool enableBreakpoint(uint8_t i);
    bool enableAllBreakpoints();
    
    bool atBreakpoint(uint16_t addr)
    {
        if (!_haveBreakpoints) {
            return false;
        }
        for (auto it : _breakpoints) {
            if (it.status == BPStatus::Enabled && it.addr == addr) {
                return true;
            }
        }
        return false;
    }
    
private:
    uint16_t getReg(Reg reg)
    {
        switch(reg) {
            default:        return 0;
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

    uint16_t getReg(const Opcode* op)
    {
        Reg reg;
        
        if (_prevOp == Op::Page2) {
            reg = op->page2Reg;
        } else if (_prevOp == Op::Page3) {
            reg = op->page3Reg;
        } else {
            reg = op->reg;
        }
        return getReg(reg);
    }

    void setReg(Reg reg, uint16_t v)
    {
        switch(reg) {
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

    void setReg(const Opcode* op, uint16_t v)
    {
        Reg reg;
        
        if (_prevOp == Op::Page2) {
            reg = op->page2Reg;
        } else if (_prevOp == Op::Page3) {
            reg = op->page3Reg;
        } else {
            reg = op->reg;
        }
        setReg(reg, v);
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
            readOnlyAddr(ea);
        } else {
            _ram[ea] = v;
        }
    }
    
    void store16(uint16_t ea, uint16_t v)
    {
        if (ea >= SystemAddrStart) {
            readOnlyAddr(ea);
        } else {
            _ram[ea] = v >> 8;
            _ram[ea + 1] = v;
        }
    }
    
    // Update the HNZVC condition codes
    void HNZVC8()  { updateH(); xNZVC8(); }
    void xNZVC8()  { updateNZ8(); updateV8(); updateC8(); }
    void xNZVC16() { updateNZ16(); updateV16(); updateC16(); }
    void xNZ018()  { updateNZ8(); _cc.V = false; _cc.C = true; }
    void x0ZxC8()  { updateNZ8(); _cc.N = false; updateC8(); }
    void xNZVx8()  { updateNZ8(); updateV8(); }
    void xNZ0x8()  { updateNZ8(); _cc.V = false; }
    void xNZ0x16() { updateNZ16(); _cc.V = false; }
    void xNZ0C8()  { updateNZ8(); _cc.V = false; updateC8(); }
    void xNZxC8()  { updateNZ8(); updateC8(); }
    void xNZxx8()  { updateNZ8(); }
    
    bool NxorV() { return (_cc.N ^ _cc.V) != 0; }
    
    void updateH()
    {
        _cc.H = ((_left ^ _right ^ _result) & 0x10) != 0;
    }
    
    void updateNZ8()
    {
        _cc.Z = uint8_t(_result) == 0;
        _cc.N = (_result & 0x80) != 0;
    }
    
    void updateNZ16()
    {
        _cc.Z = uint16_t(_result) == 0;
        _cc.N = (_result & 0x8000) != 0;
    }
    
    void updateC8()
    {
        _cc.C = (_result & 0x100) != 0;
    }
    
    void updateC16()
    {
        _cc.C = (_result & 0x10000) != 0;
    }
    
    void updateV8()
    {
        _cc.V = ((_left ^ _right ^ _result ^ (_result >> 1)) & 0x80) != 0;
    }
    
    void updateV16()
    {
        _cc.V = ((_left ^ _right ^ _result ^ (_result >> 1)) & 0x8000) != 0;
    }
    
    void readOnlyAddr(uint16_t addr);
    
    uint8_t* _ram;
    
    union {
        struct { uint8_t _b; uint8_t _a; };
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
    
    // Used by opcodes. Made members for CC calcs
    uint32_t _left = 0;
    uint32_t _right = 0;
    uint32_t _result = 0;
    Op _prevOp = Op::NOP;
    
    BOSS9Base* _boss9 = nullptr;
    
    SRecordInfo sRecInfo;
    unsigned _lineNum = 0;

    // Breakpoint support
    BreakpointEntry _breakpoints[NumBreakpoints];
    bool _haveBreakpoints = false;
};

}
