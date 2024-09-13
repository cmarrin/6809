/*-------------------------------------------------------------------------
    This source file is a part of the MC6809 Simulator
    For the latest info, see http:www.marrin.org/
    Copyright (c) 2018-2024, Chris Marrin
    All rights reserved.
    Use of this source code is governed by the MIT license that can be
    found in the LICENSE file.
-------------------------------------------------------------------------*/
//
//  MC6809.cpp
//  6809 simulator
//
//  Created by Chris Marrin on 4/22/24.
//

#include "MC6809.h"
#include "BOSS9.h"

using namespace mc6809;

static inline uint16_t concat(uint8_t a, uint8_t b)
{
    return (uint16_t(a) << 8) | uint16_t(b);
}

void SRecordInfo::ParseError(unsigned linenum, const char *fmt, va_list args)
{
    if (linenum == 0) {
        _boss9->printF("Error: line %d: ", linenum);
    } else {
        _boss9->printF("Error: ");
    }
    _boss9->vprintF(fmt, args);
    _boss9->printF("\n");
}
  
void Emulator::loadStart()
{
    sRecInfo.init();
    _lineNum = 0;
}

bool Emulator::loadLine(const char* data, bool& finished)
{
    _lineNum++;
    if (!sRecInfo.ParseLine(_lineNum, data)) {
        return false;
    }
    finished = sRecInfo.finished();
    return true;
}

uint16_t Emulator::loadEnd()
{
    sRecInfo.Flush();
    return sRecInfo.startAddr();
}

bool Emulator::execute(RunState runState)
{
    uint32_t instructionsToExecute = InstructionsToExecutePerContinue;
    bool firstTime = true;
    
    while(true) {
        // if runState is not Running we need to ignore a breakpoint at the
        // PC upon entry. Continuing and all the stepping states need to
        // execute the first instruction they encounter
        if (!firstTime || runState == RunState::Running) {
            if (atBreakpoint(_pc)) {
                _boss9->printF("\n*** hit breakpoint at addr $%04x\n\n", _pc);
                _boss9->call(Func::mon);
                return true;
            }
        }
        
        firstTime = false;
        
#ifdef TRACE
        _traceBuffer[_traceBufferIndex++] = _pc;
        if (_traceBufferIndex >= TraceBufferSize) {
            _traceBufferIndex = 0;
        }
#endif
        
        uint16_t ea = 0;
        uint8_t opIndex = next8();
        
        const Opcode* opcode = &(opcodeTable[opIndex]);
                
        // Handle address modes
        // If this is an addressing mode that produces a 16 bit effective address
        // it will be placed in ea. If it's immediate or branch relative then the
        // 8 or 16 bit value is placed in _right
        //
        // NOTE: gcc seems to have a problem with emum class and bitfields. It
        // tries to cast the value to an int, which can't be done implicitly with
        // enum class. Moving the value into a bare variable solves the problem.
        //
        Adr adr = opcode->adr;
        
        switch(adr) {
            case Adr::None:
            case Adr::Inherent:
                break;
            case Adr::Direct:
                ea = concat(_dp, next8());
                break;
            case Adr::Extended:
                ea = next16();
                break;
            case Adr::Immed8:
                _right = next8();
                break;
            case Adr::Immed16:
                _right = next16();
                break;
                
            // All the relative addressing modes need to be sign extended to 32 bits
            case Adr::RelL:
                _right = int16_t(next16());
                break;
            case Adr::Rel:
                _right = int8_t(next8());
                break;
          case Adr::RelP:
                if (_prevOp == Op::Page2) {
                    _right = int16_t(next16());
                } else {
                    _right = int8_t(next8());
                }
                break;
            case Adr::Indexed: {
                uint8_t postbyte = next8();
                uint16_t* reg = nullptr;
                
                // Load value of RR reg in ea
                switch (RR(postbyte & 0b01100000)) {
                    case RR::X: reg = &_x; break;
                    case RR::Y: reg = &_y; break;
                    case RR::U: reg = &_u; break;
                    case RR::S: reg = &_s; break;
                }
                
                if ((postbyte & 0x80) == 0) {
                    // Constant offset direct (5 bit signed)
                    int8_t offset = postbyte & 0x1f;
                    if (offset & 0x10) {
                        offset |= 0xe0;
                    }
                    
                    ea = *reg + offset;
                } else {
                    switch(IdxMode(postbyte & IdxModeMask)) {
                        case IdxMode::ConstRegNoOff   : ea = *reg; break;
                        case IdxMode::ConstReg8Off    : ea = *reg + int8_t(load8(_pc)); _pc += 1; break;
                        case IdxMode::ConstReg16Off   : ea = *reg + int16_t(load16(_pc)); _pc += 2; break;
                        case IdxMode::AccAOffReg      : ea = *reg + int8_t(_a); break;
                        case IdxMode::AccBOffReg      : ea = *reg + int8_t(_b); break;
                        case IdxMode::AccDOffReg      : ea = *reg + int16_t(_d); break;
                        case IdxMode::Inc1Reg         : ea = *reg; (*reg) += 1; break;
                        case IdxMode::Inc2Reg         : ea = *reg; (*reg) += 2; break;
                        case IdxMode::Dec1Reg         : (*reg) -= 1; ea = *reg; break;
                        case IdxMode::Dec2Reg         : (*reg) -= 2; ea = *reg; break;
                        case IdxMode::ConstPC8Off     : ea = _pc + int8_t(load8(_pc)); _pc += 1; break;
                        case IdxMode::ConstPC16Off    : ea = _pc + int16_t(load16(_pc)); _pc += 2; break;
                        case IdxMode::Extended        : ea = next16();
                    }
                    
                    if (postbyte & IndexedIndMask) {
                        // indirect from ea
                        ea = load16(ea);
                    }
                }
                    
                break;
            }
        }
        
        // Get left operand
        if (opcode->left == Left::Ld || opcode->left == Left::LdSt) {
            if (opcode->reg == Reg::M8) {
                _left = load8(ea);
            } else if (opcode->reg == Reg::M16) {
                _left = load16(ea);
            } else {
                _left = getReg(opcode->reg);
            }
        }
        
        // Get right operand
        if (opcode->right == Right::Ld8) {
            _right = load8(ea);
        } else if (opcode->right == Right::Ld16) {
            _right = load16(ea);
        }
                
        // Perform operation
        Op op = opcode->op;
        
        switch(op) {
            case Op::ILL:
                _error = Error::Illegal;
                return false;
            case Op::Page2:
            case Op::Page3:
                // We never want to leave execution after a Page2 or Page3.
                // They are basically just part of the next instruction.
                // Since we do the check for whether or not to leave the
                // loop at the end we can ensure that simply by doing
                // a continue to skip that test
                _prevOp = op;
                continue;

            case Op::BHS:
            case Op::BCC: if (!_cc.C) _pc += _right; break;
            case Op::BLO:
            case Op::BCS: if (_cc.C) _pc += _right; break;
            case Op::BEQ: if (_cc.Z) _pc += _right; break;
            case Op::BGE: if (!NxorV()) _pc += _right; break;
            case Op::BGT: if (!(NxorV() || _cc.Z)) _pc += _right; break;
            case Op::BHI: if (!_cc.C && !_cc.Z) _pc += _right; break;
            case Op::BLE: if (NxorV() || _cc.Z) _pc += _right; break;
            case Op::BLS: if (_cc.C || _cc.Z) _pc += _right; break;
            case Op::BLT: if (NxorV()) _pc += _right; break;
            case Op::BMI: if (_cc.N) _pc += _right; break;
            case Op::BNE: if (!_cc.Z) _pc += _right; break;
            case Op::BPL: if (!_cc.N) _pc += _right; break;
            case Op::BRA: _pc += _right; break;
            case Op::BRN: break;
            case Op::BVC: if (!_cc.V) _pc += _right; break;
            case Op::BVS: if (_cc.V) _pc += _right; break;
            case Op::BSR:
                push16(_s, _pc);
                _pc += _right;
                _subroutineDepth += 1;
                break;

            case Op::ABX:
                _x = _x + uint16_t(_b);
                break;
            case Op::ADC:
                _result = _left + _right + (_cc.C ? 1 : 0);
                HNZVC8();
                break;
            case Op::ADD8:
                _result = _left + _right;
                HNZVC8();
                break;
            case Op::ADD16:
                _result = _left + _right;
                xNZVC16();
                break;
            case Op::AND:
                _result = _left & _right;
                xNZ0x8();
                break;
            case Op::ANDCC:
                _ccByte &= _right;
                break;
            case Op::ASL:
                _result = _left << 1;
                xNZxC8();
                _cc.V = (((_left & 0x40) >> 6) ^ ((_left & 0x80) >> 7)) != 0;
                break;
            case Op::ASR:
                _result = int16_t(_left) >> 1;
                if (_left & 0x80) {
                    _result |= 0x80;
                }
                xNZxC8();
                break;
            case Op::BIT:
                _result = _left ^ _right;
                xNZ0x8();
                break;
            case Op::CLR:
                _result = 0;
                _cc.N = false;
                _cc.Z = true;
                _cc.V = false;
                _cc.C = false;
                break;
            case Op::CMP8:
            case Op::SUB8:
                _result = _left - _right;
                xNZVC8();
                break;
            case Op::CMP16:
            case Op::SUB16:
                _result = _left - _right;
                xNZVC16();
                
                // We need to do setReg here because if we're on an altPage these
                // are actually CMP16 and not SUB16
                if (_prevOp != Op::Page2 && _prevOp != Op::Page3 && opcode->op == Op::SUB16) {
                    setReg(opcode->reg, _result);
                }
                break;
            case Op::COM:
                _result = ~_right;
                xNZ018();
                break;
            case Op::CWAI:
                _ccByte ^= _right;
                _cc.E = true;
                push16(_s, _pc);
                push16(_s, _u);
                push16(_s, _y);
                push16(_s, _x);
                push8(_s, _dp);
                push8(_s, _b);
                push8(_s, _a);
                
                // Now what?
                break;
            case Op::DAA: {
                _result = _a;
                uint8_t LSN = _result & 0x0f;
                uint8_t MSN = (_result & 0xf0) >> 4;
                
                // LSN
                if (_cc.H || LSN > 9) {
                    _result += 6;
                }
                
                // MSN
                if (_cc.C || (MSN > 9) || (MSN > 8 && LSN > 9)) {
                    _result += 0x60;
                }
                xNZ0C8();
                _a = _result;
                break;
            }
            case Op::DEC:
                _result = _left - 1;
                xNZxx8();
                _cc.V = _left == 0x80;
                break;
            case Op::EOR:
                _result = _left ^ _right;
                xNZ0x8();
                break;
            case Op::EXG: {
                uint16_t r1 = getReg(Reg(_right & 0xf));
                uint16_t r2 = getReg(Reg(_right >> 4));
                setReg(Reg(_right & 0xf), r2);
                setReg(Reg(_right >> 4), r1);
                break;
            }
            case Op::INC:
                _result = _left + 1;
                xNZxx8();
                _cc.V = _left == 0x7f;
                break;
            case Op::JMP:
            case Op::JSR:
                if (ea >= SystemAddrStart) {
                    // This is possibly a system call
                    if (!_boss9->call(Func(ea))) {
                        return true;
                    }
                } else {
                    if (op == Op::JSR) {
                        push16(_s, _pc);
                    }
                    _pc = ea;
                    if (op == Op::JSR) {
                        _subroutineDepth += 1;
                    }
                }
                break;
            case Op::LD8:
                _result = _right;
                xNZ0x8();
                break;
            case Op::LD16:
                _result = _right;
                xNZ0x16();
                break;
            case Op::LEA:
                _result = ea;
                if (opcode->reg == Reg::X || opcode->reg == Reg::Y) {
                    _cc.Z = _result == 0;
                }
                break;
            case Op::LSR:
                _result = _left >> 1;
                x0ZxC8();
                break;
            case Op::MUL:
                _d = _a * _b;
                _cc.Z = _d == 0;
                _cc.C = _b & 0x80;
                break;
            case Op::NEG:
                _result = -_left;
                xNZxC8();
                _cc.V = _left == 0x80;
                break;
            case Op::NOP:
                break;
            case Op::OR:
                _result = _left | _right;
                xNZ0x8();
                break;
            case Op::ORCC:
                _ccByte |= _right;
                break;
            case Op::PSH:
            case Op::PUL: {
                // bit pattern to push or pull are in _right
                uint16_t& stack = (opcode->reg == Reg::U) ? _u : _s;
                if (opcode->op == Op::PSH) {
                    if (_right & 0x80) push16(stack, _pc);
                    if (_right & 0x40) push16(stack, (opcode->reg == Reg::U) ? _s : _u);
                    if (_right & 0x20) push16(stack, _y);
                    if (_right & 0x10) push16(stack, _x);
                    if (_right & 0x08) push8(stack, _dp);
                    if (_right & 0x04) push8(stack, _b);
                    if (_right & 0x02) push8(stack, _a);
                    if (_right & 0x01) push8(stack, _ccByte);
                } else {
                    if (_right & 0x01) _ccByte = pop8(stack);
                    if (_right & 0x02) _a = pop8(stack);
                    if (_right & 0x04) _b = pop8(stack);
                    if (_right & 0x08) _dp = pop8(stack);
                    if (_right & 0x10) _x = pop16(stack);
                    if (_right & 0x20) _y = pop16(stack);
                    if (_right & 0x40) {
                        if (opcode->reg == Reg::U) {
                            _s = pop16(stack);
                        } else {
                            _u = pop16(stack);
                        }
                    }
                    if (_right & 0x80) _pc = pop16(stack);
                }
                break;
            }
            case Op::ROL:
                _result = _left << 1;
                if (_cc.C) {
                    _result |= 0x01;
                }
                xNZxC8();
                _cc.V = (((_left & 0x40) >> 6) ^ ((_left & 0x80) >> 7)) != 0;
                break;
            case Op::ROR:
                _result = _left >> 1;
                xNZxx8();
                if (_cc.C) {
                    _result |= 0x80;
                }
                if (_left & 0x01) {
                    _cc.C = true;
                }
                break;
            case Op::RTI:
                if (_cc.E) {
                    _a = pop8(_s);
                    _b = pop8(_s);
                    _dp = pop8(_s);
                    _x = pop16(_s);
                    _y = pop16(_s);
                    _u = pop16(_s);
                }
                _pc = pop16(_s);
                break;
            case Op::RTS:
                _pc = pop16(_s);
                _subroutineDepth -= 1;
                if (_lastRunState != RunState::Running && _subroutineDepth == 0) {
                    _boss9->printF("\n*** step %s, stopped at addr $%04x\n\n",
                            (_lastRunState == RunState::StepOver) ? "over" : "out", _pc);
                    // enter the monitor
                    _boss9->call(Func::mon);
                    return true;
                }
                break;
            case Op::SBC:
                _result = _left - _right - (_cc.C ? 1 : 0);
                xNZVC8();
                break;
            case Op::SEX:
                _a = (_b & 0x80) ? 0xff : 0;
                xNZ0x8();
                break;
            case Op::ST8:
                xNZ0x8();
                break;
            case Op::ST16: // All done in pre and post processing
                xNZ0x16();
                break;
            case Op::SWI:
                _cc.E = true;
                push16(_s, _pc);
                push16(_s, _u);
                push16(_s, _y);
                push16(_s, _x);
                push8(_s, _dp);
                push8(_s, _b);
                push8(_s, _a);
                _cc.I = true;
                _cc.F = true;
                if (_prevOp == Op::Page3) {
                    _pc = load16(0xfff2);
                } else if (_prevOp == Op::Page2) {
                    _pc = load16(0xfff4);
                } else {
                    _pc = load16(0xfffa);
                }
                break;
            case Op::SYNC:
                // Now what?
                break;
            case Op::TFR:
                setReg(Reg(_right & 0xf), getReg(Reg(_right >> 4)));
                break;
            case Op::TST:
                _result = _left - 0;
                xNZ0x8();
                break;
            case Op::FIRQ:
            case Op::IRQ:
            case Op::NMI:
            case Op::RESTART:
                // Now what?
                break;
        }
        
        // Store _result
        if (opcode->right == Right::St8) {
            store8(ea, _left);
        } else if (opcode->right == Right::St16) {
            store16(ea, _left);
        } else if (opcode->left == Left::St || opcode->left == Left::LdSt) {
            if (opcode->right == Right::St8 || opcode->reg == Reg::M8) {
                store8(ea, _result);
            } else if (opcode->right == Right::St16 || opcode->reg == Reg::M16) {
                store16(ea, _result);
            } else {
                setReg(opcode->reg, _result);
            }
        }
        
        _prevOp = opcode->op;
        
        // Step handling
        //
        //  Step In     - Go back to monitor after each instruction executed
        //  Step Over   - Behave like Step In unless current instruction is
        //                BSR or JSR in which case we run until that subroutine
        //                returns.
        //  Step Out    - Run until we return from current subroutine. If none
        //                then we never enter monitor until program exits of ESC.
        //
        // When doing Step Over or Step Out we need to stop when we hit RTS. but
        // there might be nested subroutines so we have to keep track of the
        // depth using _subroutineDepth. When we execute BSR or JSR we increment
        // and when we hit RTS we decrement. When it hits 0 we stop. For Step Over
        // we start with _subroutineDepth = 0. Entering the current BSR or JSR
        // increments to 0 and the matching RTS decrements back to 0 and we
        // enter the monitor. If there are nested subroutines _subroutineDepth
        // keeps track of them so we return on the correct RTS. For Step Out
        // we set _subroutineDepth = 1 so the next RTS we see will stop.
        //
        if (runState != RunState::Running) {
            _lastRunState = runState;
        }
        
        bool handleStepOverLikeStepIn = false;
        
        if (runState == RunState::StepOver) {
            if ((_prevOp != Op::BSR && _prevOp != Op::JSR) ||
                    (_prevOp == Op::JSR && ea >= SystemAddrStart)) {
                handleStepOverLikeStepIn = true;
            } else {
                _subroutineDepth = 1;
            }
        }
        
        if (runState == RunState::StepIn || handleStepOverLikeStepIn) {
            _boss9->printF("\n*** step %s, stopped at addr $%04x\n\n",
                    (_lastRunState == RunState::StepIn) ? "in" : "over", _pc);
            _boss9->call(Func::mon);
            return true;
        }
        
        if (runState == RunState::StepOut) {
            _subroutineDepth = 1;
        }

        runState = RunState::Running;
        
        if (--instructionsToExecute == 0) {
            return true;
        }
    }
}

void Emulator::readOnlyAddr(uint16_t addr)
{
    _boss9->printF("Address $%04x is read-only\n", addr);
}

void Emulator::checkActiveBreakpoints()
{
    _haveBreakpoints = false;
    for (auto it : _breakpoints) {
        if (it.status == BPStatus::Enabled) {
            _haveBreakpoints = true;
            return;
        }
    }
}

bool Emulator::breakpoint(uint8_t i, BreakpointEntry& entry) const
{
    if (i >= NumBreakpoints || _breakpoints[i].status == BPStatus::Empty) {
        return false;
    }
    entry = _breakpoints[i];
    return true;
}

bool Emulator::setBreakpoint(uint16_t addr, uint8_t& i)
{
    i = 0;
    for (auto &it : _breakpoints) {
        if (it.status == BPStatus::Empty) {
            it.status = BPStatus::Enabled;
            it.addr = addr;
            _haveBreakpoints = true;
            return true;
        }
        i += 1;
    }
    return false;
}

bool Emulator::clearBreakpoint(uint8_t i)
{
    // Clear the passed breakpoint and move all the others past it up one
    if (i >= NumBreakpoints || _breakpoints[i].status == BPStatus::Empty) {
        return false;
    }
    
    for ( ; i < NumBreakpoints - 1; ++i) {
        _breakpoints[i] = _breakpoints[i + 1];
    }
    _breakpoints[i].status = BPStatus::Empty;
    
    checkActiveBreakpoints();
    return true;
}

bool Emulator::clearAllBreakpoints()
{
    for (auto &it : _breakpoints) {
        it.status = BPStatus::Empty;
    }
    checkActiveBreakpoints();
    return true;
}

bool Emulator::disableBreakpoint(uint8_t i)
{
    if (i >= NumBreakpoints || _breakpoints[i].status == BPStatus::Empty) {
        return false;
    }
    
    _breakpoints[i].status = BPStatus::Disabled;
    checkActiveBreakpoints();
    return true;
}

bool Emulator::disableAllBreakpoints()
{
    for (auto &it : _breakpoints) {
        if (it.status == BPStatus::Enabled) {
            it.status = BPStatus::Disabled;
        }
    }
    checkActiveBreakpoints();
    return true;
}

bool Emulator::enableBreakpoint(uint8_t i)
{
    if (i >= NumBreakpoints || _breakpoints[i].status == BPStatus::Empty) {
        return false;
    }
    
    _breakpoints[i].status = BPStatus::Enabled;
    checkActiveBreakpoints();
    return true;
}

bool Emulator::enableAllBreakpoints()
{
    for (auto &it : _breakpoints) {
        if (it.status == BPStatus::Disabled) {
            it.status = BPStatus::Enabled;
        }
    }
    checkActiveBreakpoints();
    return true;
}

// TODO:
//
// - Handle SYNC and CWAI waiting for interrupt
// - Handle firing of IRQ, FIRQ, NMI and RESTART
// - Handle CC. Have a post op deal with most of it, based on value in Opcode
// - Op handling has zero or more of left, right and ea set in addr handling and reg pre op sections.
//      Do proper setting of these in all cases. Need more than op.reg to do this. Need leftReg, rightReg, etc.
// - Add a Reg.M which uses ea to load data at ea (8 and 16 bit version)
// - Add store Reg value to opcode to store value in post op

// - Did I get ROR and ROL right WRT C flag?
