/*-------------------------------------------------------------------------
    This source file is a part of the MC6809 Simulator
    For the latest info, see http:www.marrin.org/
    Copyright (c) 2018-2024, Chris Marrin
    All rights reserved.
    Use of this source code is governed by the MIT license that can be
    found in the LICENSE file.
-------------------------------------------------------------------------*/
//
//  DisplayInst.cpp
//  Generate 6809 instruction string
//
//  Created by Chris Marrin on 4/22/24.
//

#include "DisplayInst.h"

using namespace mc6809;

static inline const char* opToString(Op op)
{
    switch (op) {
        default: return "???";
        
        case Op::ILL    : return "ILL";
        case Op::ABX    : return "ABX";
        case Op::ADC    : return "ADC";
        case Op::ADD8   :
        case Op::ADD16  : return "ADD";
        case Op::AND    : return "AND";
        case Op::ANDCC  : return "ANDCC";
        case Op::ASL    : return "ASL";
        case Op::ASR    : return "ASR";
        case Op::BCC    : return "BCC";
        case Op::BCS    : return "BCS"; 
        case Op::BEQ    : return "BEQ";
        case Op::BGE    : return "BGE";
        case Op::BGT    : return "BGT";
        case Op::BHI    : return "BHI";
        case Op::BHS    : return "BHS";
        case Op::BIT    : return "BIT";
        case Op::BLE    : return "BLE";
        case Op::BLO    : return "BLO"; 
        case Op::BLS    : return "BLS";
        case Op::BLT    : return "BLT";
        case Op::BMI    : return "BMI";
        case Op::BNE    : return "BNE";
        case Op::BPL    : return "BPL";
        case Op::BRA    : return "BRA";
        case Op::BRN    : return "BRN";
        case Op::BSR    : return "BSR"; 
        case Op::BVC    : return "BVC";
        case Op::BVS    : return "BVS";
        case Op::CLR    : return "CLR";
        case Op::CMP8   :
        case Op::CMP16  : return "CMP";
        case Op::COM    : return "COM";
        case Op::CWAI   : return "CWAI";
        case Op::DAA    : return "DAA";
        case Op::DEC    : return "DEC"; 
        case Op::EOR    : return "EOR";
        case Op::EXG    : return "EXG";
        case Op::INC    : return "INC";
        case Op::JMP    : return "JMP";
        case Op::JSR    : return "JSR";
        case Op::LD8    :
        case Op::LD16   : return "LD";
        case Op::LEA    : return "LEA"; 
        case Op::LSR    : return "LSR";
        case Op::MUL    : return "MUL";
        case Op::NEG    : return "NEG";
        case Op::NOP    : return "NOP";
        case Op::OR     : return "OR";
        case Op::ORCC   : return "ORCC";
        case Op::PSH    : return "PSH";
        case Op::PUL    : return "PUL";
        case Op::ROL    : return "ROL"; 
        case Op::ROR    : return "ROR";
        case Op::RTI    : return "RTI";
        case Op::RTS    : return "RTS";
        case Op::SBC    : return "SBC";
        case Op::SEX    : return "SEX";
        case Op::ST8    :
        case Op::ST16   : return "ST";
        case Op::SUB8   :
        case Op::SUB16  : return "SUB";
        case Op::SWI    : return "SWI";
        case Op::SYNC   : return "SYNC";
        case Op::TFR    : return "TFR";
        case Op::TST    : return "TST"; 
        case Op::FIRQ   : return "FIRQ"; 
        case Op::IRQ    : return "IRQ"; 
        case Op::NMI    : return "NMI"; 
        case Op::RESTART: return "RESTART";
    }
}

const char*
DisplayInst::regToString(Reg op, Op prevOp)
{
    switch (op) {
        default:        return "";
        case Reg::A:    return "A";
        case Reg::B:    return "B";
        case Reg::D:    return "D";
        case Reg::X:    return "X";
        case Reg::Y:    return "Y";
        case Reg::U:    return "U";
        case Reg::S:    return "S";
        case Reg::CC:   return "CC";
        case Reg::PC:   return "PC";
        case Reg::DP:   return "DP";
        case Reg::DDU:  return (prevOp == Op::Page2) ? "D" : ((prevOp == Op::Page3) ? "U" : "D");
        case Reg::XYS:  return (prevOp == Op::Page2) ? "Y" : ((prevOp == Op::Page3) ? "S" : "X");
        case Reg::XY:   return (prevOp == Op::Page2) ? "Y" : ((prevOp == Op::Page3) ?  "" : "X");
        case Reg::US:   return (prevOp == Op::Page2) ? "S" : ((prevOp == Op::Page3) ?  "" : "U");
    }
}

uint16_t
DisplayInst::instToString(const Emulator& engine, m8r::string& s, uint16_t addr)
{
    uint16_t instAddr = addr;
    const Opcode* opcode = engine.opcode(engine.load8(addr++));
    Op prevOp = Op::NOP;
    Op op = opcode->op;
    
    if (op == Op::Page2 || op == Op::Page3) {
        prevOp = op;
        opcode = engine.opcode(engine.load8(addr++));
        op = opcode->op;
        if (op == Op::SUB16) {
            op = Op::CMP16;
        }
    }
    
    // Do the addr mode
    uint16_t ea = 0;
    int16_t relAddr = 0;
    uint16_t value = 0;
    int16_t offset = 0;
    const char* longBranch = "";
    const char* indexReg = nullptr;
    const char* offsetReg = nullptr;
    bool indirect = false;
    int8_t autoInc = 0;
    Adr addrMode = opcode->adr;
    
    switch(addrMode) {
        case Adr::None:
        case Adr::Inherent:
            break;
        case Adr::Direct:
            ea = engine.load8(addr++);
            break;
        case Adr::Extended:
            ea = engine.load16(addr);
            addr += 2;
            break;
        case Adr::Immed8:
            value = engine.load8(addr++);
            break;
        case Adr::Immed16:
            value = engine.load16(addr);
            addr += 2;
            break;
            
        case Adr::RelL:
            relAddr = int16_t(engine.load16(addr));
            addr += 2;
            longBranch = "l";
            break;
        case Adr::Rel:
            relAddr = int8_t(engine.load8(addr++));
            break;
      case Adr::RelP:
            if (prevOp == Op::Page2) {
                relAddr = int16_t(engine.load16(addr));
                addr += 2;
                longBranch = "l";
                addrMode = Adr::RelL;
            } else {
                relAddr = int8_t(engine.load8(addr++));
                addrMode = Adr::Rel;
            }
            break;
        case Adr::Indexed: {
            uint8_t postbyte = engine.load8(addr++);
            
            // Load value of RR reg in ea
            switch (RR(postbyte & 0b01100000)) {
                case RR::X: indexReg = "X"; break;
                case RR::Y: indexReg = "Y"; break;
                case RR::U: indexReg = "U"; break;
                case RR::S: indexReg = "S"; break;
            }
            
            if ((postbyte & 0x80) == 0) {
                // Constant offset direct (5 bit signed)
                offset = postbyte & 0x1f;
                if (offset & 0x10) {
                    offset |= 0xe0;
                }
            } else {
                switch(IdxMode(postbyte & IdxModeMask)) {
                    case IdxMode::ConstRegNoOff   : offset = 0; break;
                    case IdxMode::ConstReg8Off    : offset = int8_t(engine.load8(addr)); addr += 1; break;
                    case IdxMode::ConstReg16Off   : offset = int16_t(engine.load16(addr)); addr += 2; break;
                    case IdxMode::AccAOffReg      : offsetReg = "A"; break;
                    case IdxMode::AccBOffReg      : offsetReg = "B"; break;
                    case IdxMode::AccDOffReg      : offsetReg = "D"; break;
                    case IdxMode::Inc1Reg         : autoInc = 1; break;
                    case IdxMode::Inc2Reg         : autoInc = 2; break;
                    case IdxMode::Dec1Reg         : autoInc = -1; break;
                    case IdxMode::Dec2Reg         : autoInc = -2; break;
                    case IdxMode::ConstPC8Off     : offset = int8_t(engine.load8(addr)); addr += 1; indexReg = "PC"; break;
                    case IdxMode::ConstPC16Off    : offset = engine.getReg(Reg::PC) + int16_t(engine.load16(addr)); addr += 2; indexReg = "PC"; break;
                    case IdxMode::Extended:
                        offset = engine.load16(addr);
                        addr += 2;
                        indexReg = nullptr;
                        break;
                }
                
                if (postbyte & IndexedIndMask) {
                    // indirect
                    indirect = true;
                }
            }
                
            break;
        }
    }
    
    s = m8r::string::format("[$%04x]    %s%s%s  ", instAddr, longBranch, opToString(op), regToString(opcode->reg, prevOp));

    switch(addrMode) {
        case Adr::None:
        case Adr::Inherent: break;
        case Adr::Direct:   s += m8r::string::format("<$%02x", ea); break;
        case Adr::Extended: s += m8r::string::format("$%04x", ea); break;
        case Adr::Immed16:  s += m8r::string::format("#$%04x", value); break;
        case Adr::Rel:      s += m8r::string::format("%d", relAddr); break;
        case Adr::RelL:     s += m8r::string::format("%d", relAddr); break;
        case Adr::RelP:     break;
        case Adr::Immed8:
            if (op == Op::TFR || op == Op::EXG) {
                s += m8r::string::format("%s,%s", regToString(Reg(uint8_t(value) >> 4), prevOp),
                                          regToString(Reg(uint8_t(value) & 0xf), prevOp));
            } else if (op == Op::PSH || op == Op::PUL) {
                const char* pushRegs[8] = { "CC", "A", "B", "DP", "X", "Y", "S", "PC" };
                bool first = true;
                
                for (int i = 0; i < 8; ++i) {
                    if ((value & (0x01 << i)) != 0) {
                        const char* r = pushRegs[i];
                        if (r[0] == 'S' && opcode->reg == Reg::S) {
                            r = "U";
                        }
                        if (!first) {
                            s += m8r::string::format(",");
                        }
                        s += m8r::string::format("%s", r);
                        first = false;
                    }
                }
            } else {
                s += m8r::string::format("#$%02x", value);
            }
            break;
        case Adr::Indexed:
            if (indexReg) {
                const char* indIn = indirect ? "[" : "";
                const char* indOut = indirect ? "]" : "";
                
                if (offsetReg) {
                    s += m8r::string::format("%s%s,%s%s", indIn, offsetReg, indexReg, indOut); break;
                } else if (autoInc != 0) {
                    if (autoInc > 0) {
                        s += m8r::string::format("%s,%s%s%s", indIn, (autoInc == 1) ? "+" : "++", indexReg, indOut); break;
                    } else {
                        s += m8r::string::format("%s,%s%s%s", indIn, indexReg, (autoInc == -1) ? "-" : "--", indOut); break;
                    }
                } else {
                    s += m8r::string::format("%s%d,%s%s", indIn, offset, indexReg, indOut); break;
                }
            } else {
                // Must be extended indirect
                s += m8r::string::format("[$%04x]", offset); break;
            }
            break;

    }
    
    s += m8r::string::format("\n");
    return addr;
}

