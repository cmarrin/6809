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

//#define COMPUTE_CYCLES

namespace mc6809 {

// Opcode table

class Opcodes
{
  public:

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
        M8 = 0xd, M16 = 0xe, None = 0xf,
        DDU = 0x10, XYS = 0x11, XY = 0x12, US = 0x13,
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
        Reg reg : 5;
        Left left : 3;
        Right right : 3;
        Adr adr : 4;
    };

    const Opcode& opcode(uint8_t opcode) const;
    
    const char* opToString(Op op) const;

    void printInstructions(uint16_t addr, uint16_t n);
    
    const char* regToString(Reg, Op prevOp = Op::NOP);
    uint8_t regSizeInBytes(Reg reg)
    {
        return (reg == Reg::A || reg == Reg::B || reg == Reg::CC || reg == Reg::DP) ? 1 : 2;
    }
};

}
