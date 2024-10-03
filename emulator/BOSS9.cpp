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

#include <cerrno>
#include <cstdlib>
#include <cctype>

#include "BOSS9.h"
#include "Format.h"
#include "MC6809.h"

using namespace mc6809;

static inline uint8_t typeToBytes(fmt::Type t)
{
    // flt and i32 are not supported on 6809. Return 1 for them
    if (t == fmt::Type::i8 || t == fmt::Type::i32 || t == fmt::Type::flt) {
        return 1;
    }
    return 2;
}

// Args start at U+6 (3 pointers:self, retaddr, prevU)
// _nextAddr and _initialAddr are relative to arg start
class VarArg
{
  public:
    VarArg(BOSS9Base* boss9, uint16_t lastArgOffset, fmt::Type lastArgType)
        : _boss9(boss9)
    {
        initialize(lastArgOffset, lastArgType);
    }
    
    VarArg(BOSS9Base* boss9)
        : _boss9(boss9)
    {
        initialize();
    }
    
    // Type returned is always ArgUNativeType. Use reinterpret_cast to convert to the proper type
    uint16_t arg(uint8_t bytes)
    {
        uint16_t argAddr = _nextAddr;
        _nextAddr += bytes;
        return _boss9->emulator().getArg(argAddr, bytes);
    }

    void initialize()
    {
        _nextAddr = 0;
        _initialAddr = _nextAddr;
    }

    void initialize(uint16_t lastArgOffset, fmt::Type lastArgType)
    {
        _nextAddr = lastArgOffset + typeToBytes(lastArgType);
        _initialAddr = _nextAddr;
    }
    
    void reset() { _nextAddr = _initialAddr; }
    
    void putChar(uint16_t addr, uint8_t c) { _boss9->emulator().store8(addr, c); }
    
    uint16_t getAbs(uint16_t addr, uint8_t size)
    {
        if (size == 1) {
            return _boss9->emulator().load8(addr);
        }
        return _boss9->emulator().load16(addr);
    }
    
    BOSS9Base* boss9() const { return _boss9; }
    
  private:
    uint16_t _nextAddr = 0;
    uint16_t _initialAddr = 0;
    BOSS9Base* _boss9 = nullptr;
};
class InterpPrintArgs : public fmt::FormatterArgs
{
  public:
    InterpPrintArgs(uint16_t fmt, VarArg& args)
        : _fmt(fmt)
        , _args(&args)
    { }
        
    virtual ~InterpPrintArgs() { }
    virtual uint8_t getChar(uint32_t i) const override { return getStringChar(uintptr_t(_fmt + i)); }
    virtual void putChar(uint8_t c) override { _args->boss9()->putc(c); }
    virtual uintptr_t getArg(fmt::Type type) override
    {
        // varargs are always the same size
        return _args->arg(2);
    }

    // The interpreter keeps strings in ROM. The p pointer is actually an offset in the rom
    virtual uint8_t getStringChar(uintptr_t p) const override
    {
        return _args->boss9()->emulator().load8(p);
    }
    
    VarArg* args() { return _args; }

  private:
    uint16_t _fmt;
    VarArg* _args;
};

class InterpFormatArgs : public InterpPrintArgs
{
  public:
    InterpFormatArgs(uint16_t s, uint16_t n, uint16_t fmt, VarArg& args)
        : InterpPrintArgs(fmt, args)
        , _buf(s)
        , _size(n)
        , _index(0)
    { }
        
    virtual ~InterpFormatArgs() { }

    virtual void putChar(uint8_t c) override
    {
        if (_index < _size - 1) {
            args()->putChar(_buf + _index++, c);
        }
    }

    virtual void end() override { putChar('\0'); }

  private:
    uint16_t _buf;
    uint16_t _size;
    uint16_t _index;
};

static inline int32_t
printf(uint16_t fmt, VarArg& args)
{
    InterpPrintArgs f(fmt, args);
    return fmt::doprintf(&f);
}

static inline int32_t
format(uint16_t s, uint16_t n, uint16_t fmt, VarArg& args)
{
    InterpFormatArgs f(s, n, fmt, args);
    return fmt::doprintf(&f);
}

bool BOSS9Base::call(Func func)
{
    switch (func) {
        case Func::putc:
            putc(emulator().getReg(Reg::A));
            break;
        case Func::puts: {
            const char* s = reinterpret_cast<const char*>(emulator().getAddr(emulator().getReg(Reg::X)));
            puts(s);
            break;
        }
        case Func::getc: {
            emulator().setReg(Reg::A, getc());
            break;
        }
        case Func::exit:
            printF("Program exited with code %d\n", int32_t(emulator().getReg(Reg::A)));
            enterMonitor();
            emulator().setReg(Reg::PC, _startAddr);
            return false;
        case Func::mon:
            enterMonitor();
            return false;
        case Func::ldStart:
            emulator().loadStart();
            break;
        case Func::ldLine: {
            // X has pointer to data.
            // Return bool success in A, bool finished in B
            const char* s = reinterpret_cast<const char*>(emulator().getAddr(emulator().getReg(Reg::X)));
            bool finished;
            bool result = emulator().loadLine(s, finished);
            emulator().setReg(Reg::A, result);
            emulator().setReg(Reg::B, finished);
            break;
        }
        case Func::ldEnd:
            emulator().loadEnd();
            break;
        case Func::printf: {
            VarArg va(this, 0, fmt::Type::str);
            uint16_t fmt = emulator().getArg(0, 2);
            printf(fmt, va);
            break;
        }
        case Func::format: {
            VarArg va(this, 4, fmt::Type::str);
            uint16_t s = emulator().getArg(0, 2);
            uint16_t n = emulator().getArg(2, 2);
            uint16_t fmt = emulator().getArg(4, 2);
            format(s, n, fmt, va);
            break;
        }
        case Func::switch1:
        case Func::switch2:
            emulator().setReg(Reg::X, 0xFC0E); // This is exit for now
            break;
        case Func::idiv8 :
        case Func::idiv16:
        case Func::udiv8 :
        case Func::udiv16: {
            // TOS: dividend, divisor. Return quotient on A/D
            bool is16Bit = func == Func::idiv16 || func == Func::udiv16;
            uint8_t bytes = is16Bit ? 2 : 1;
            
            uint16_t dividend = emulator().getArg(0, bytes);
            uint16_t divisor = emulator().getArg(bytes, bytes);
            
            uint16_t quotient = 0;
            switch (func) {
                default: break;
                case Func::idiv8 : quotient = uint16_t(int8_t(dividend) / int8_t(divisor)); break;
                case Func::idiv16: quotient = uint16_t(int16_t(dividend) / int16_t(divisor)); break;
                case Func::udiv8 : quotient = uint16_t(uint8_t(dividend) / uint8_t(divisor)); break;
                case Func::udiv16: quotient = uint16_t(uint16_t(dividend) / uint16_t(divisor)); break;
            }
            emulator().setReg(is16Bit ? Reg::D : Reg::A, quotient);
            break;
        }
        default: return false;
    }
    return true;
}

static Reg regsToPrint[ ] = { Reg::A, Reg::B, Reg::D, Reg::X, Reg::Y,
                              Reg::U, Reg::S, Reg::PC, Reg::CC, Reg::DP };
                                            
void BOSS9Base::getCommand()
{
    if (emulator().error() != Emulator::Error::None) {
        // FIXME: One day we will have more errors
        printF("*** Illegal Instruction error at addr: $%04x\n", emulator().getReg(Reg::PC));
        emulator().resetError();
    }
        
    bool haveCmd = false;
    promptIfNeeded();
    
    while (true) {
        int c = getc();
        if (c <= 0) {
            break;
        }
        if (_cursor >= CmdBufSize - 1) {
            printF("*** too many chars in cmdbuf\n");
            _cursor = 0;
            haveCmd = true;
            break;
        }

        if (c == '\n' || c == '\r') {
            haveCmd = true;
            _cmdBuf[_cursor] = '\0';
            break;
        }
        
        if (checkEscape(c)) {
            break;
        }
        
        if (c == 0x08 || c == 0x7f) {
            // backspace
            if (_echoBS) {
                putc(' ');
                putc(0x08);
            }
            if (_cursor != 0) {
                _cursor -= 1;
            }
            continue;
        }
        
        if (c < 0x20 || c > 0x7f) {
            // Some other control character, for now error
            printF("*** unrecognized control char '0x%02x'\n", uint8_t(c));
            _cursor = 0;
            haveCmd = true;
            break;
        }
        
        _cmdBuf[_cursor++] = char(c);
    }
        
    if (haveCmd) {
        _needPrompt = true;
        _needInstPrint = true;
        if (_cursor > 0) {
            processCommand();
        }
    }
}

bool BOSS9Base::checkEscape(int c)
{
    if (c == 0x1b) {
        // Escape - go back  to command mode
        _runState = RunState::Cmd;
        enterMonitor();
        printF("...ABORT...\n");
        return true;
    }
    return false;
}

// Parse cmd buffer, splitting words by space. Space before first non-space char is ignored.
// All characters are lowercased. If there are more than 3 words, return false.
bool parseCmd(const char* cmdbuf, m8r::string cmdElements[3])
{
    int elementIndex = 0;
    int i = 0;
    
    while (cmdbuf[i] != '\0') {
        // Get past spaces
        while (true) {
            if (!isspace(cmdbuf[i])) {
                break;
            }
            i += 1;
        }
        
        // Get the next word
        while (true) {
            char c = tolower(cmdbuf[i]);
            
            if (c == '\0') {
                break;
            }
            
            if (isspace(c)) {
                // Finished a word
                elementIndex += 1;
                if (elementIndex > 2) {
                    return false;
                }
                break;
            }
        
            cmdElements[elementIndex] += c;
            i += 1;
        }
    }
    
    return true;
}

void BOSS9Base::processCommand()
{
    if (_runState == RunState::Cmd) {
        // Parse Command
        m8r::string cmdElements[3];
        if (parseCmd(_cmdBuf, cmdElements)) {
            executeCommand(cmdElements);
        }
        return;
    }
        
    if (_runState == RunState::Loading) {
        bool finished;
        if (!emulator().loadLine(_cmdBuf, finished)) {
            _runState = RunState::Cmd;
            printF("Error loading\n");
        }
        if (finished) {
            _startAddr = emulator().loadEnd();
            _runState = RunState::Cmd;
            printF("Load complete, start addr = 0x%04x\n", _startAddr);
        }
    }
    _cursor = 0;
}

bool BOSS9Base::toNum(m8r::string& s, uint32_t& num)
{
    bool ishex = false;
    int i = 0;
    if (s[0] == '$') {
        ishex = true;
        i = 1;
    }
    char* strend = nullptr;
    num = uint32_t(strtol(s.c_str() + i, &strend, ishex ? 16 : 10));
    if ((strend == s.c_str() + i) || (errno == ERANGE)) {
        printF("%s is not a valid number\n", s.c_str());
        return false;
    }
    return true;
}

void BOSS9Base::showBreakpoint(uint8_t i) const
{
    BreakpointEntry entry;
    if (emulator().breakpoint(i, entry)) {
        printF("    Breakpoint[%d] -> $%04x (%sabled)\n", i, entry.addr, (entry.status == BPStatus::Enabled) ? "en" : "dis");
    }
}

bool BOSS9Base::executeCommand(m8r::string cmdElements[3])
{
    assert(_runState == RunState::Cmd);
    
    if (cmdElements[0] == "help" || cmdElements[0] == "h" || cmdElements[0] == "?") {
        if (cmdElements[1].empty()) {
            printF("BOSS9 Monitor Commands:\n");
            printF("\n");
            printF("\tld      - Load SRecords from serial\n");
            printF("\tr       - run at start addr\n");
            printF("\tr a     - run at addr a, set start to a\n");
            printF("\tc       - cont at current addr\n");
            printF("\tc a     - cont at addr a\n");
            printF("\tb       - show cur brkpts\n");
            printF("\tb a     - set brkpt at addr a\n");
            printF("\tbc      - clear all brkpts\n");
            printF("\tbc n    - clear brkpt <n>\n");
            printF("\tbe      - enable all brkpts\n");
            printF("\tbe n    - enable brkpt <n>\n");
            printF("\tbd      - disable all brkpts\n");
            printF("\tbd n    - disable brkpt <n>\n");
            printF("\tn       - next, if at func step over\n");
            printF("\ts       - step, if at func step in\n");
            printF("\to       - step out of cur func\n");
            printF("\tl       - list next 5 insts at cur addr\n");
            printF("\tl n     - list next n insts at cur addr\n");
            printF("\tla a    - list next 5 insts at addr a\n");
            printF("\tla a n  - list next n insts at addr a\n");
            printF("\ta       - show inst at cur addr\n");
            printF("\ta a     - set cur addr to a and show inst\n");
            printF("\tregs    - show all regs\n");
            printF("\treg r   - show reg r\n");
            printF("\treg r v - set reg r to v\n");

            return true;
        }
        printF("Individual command help not yet available\n");
        return false;
    }
    
    // Load file
    if (cmdElements[0] == "ld") {
        if (!cmdElements[1].empty() || !cmdElements[2].empty()) {
            return false;
        }
        
        // Load s19 file
        printF("Ready to start loading. ESC to abort\n");
        _runState = RunState::Loading;
        emulator().loadStart();
        return true;
    }

    // Run at <addr> or at current addr
    if (cmdElements[0] == "r") {
        if (!cmdElements[2].empty()) {
            return false;
        }
        
        if (!cmdElements[1].empty()) {
            uint32_t num;
            if (!toNum(cmdElements[1], num)) {
                return false;
            }
            _startAddr = num;
        }
        
        // run
        leaveMonitor();
        printF("Running at address $%04x\n", _startAddr);
        emulator().setReg(Reg::PC, _startAddr);
        return true;
    }
    
    // Continue running at PC
    if (cmdElements[0] == "c") {
        if (!cmdElements[1].empty() || !cmdElements[2].empty()) {
            return false;
        }
        
        // run from current PC
        leaveMonitor();
        _runState = RunState::Continuing;
        printF("Continuing at address $%04x\n", emulator().getReg(Reg::PC));
        return true;
    }
    
    // Show or set breakpoint
    if (cmdElements[0] == "b") {
        if (!cmdElements[2].empty()) {
            return false;
        }

        if (cmdElements[1].empty()) {
            // Show breakpoints
            bool haveBreakpoints = false;
        
            for (auto i = 0; i < NumBreakpoints; ++i) {
                BreakpointEntry entry;
                if (emulator().breakpoint(i, entry)) {
                    showBreakpoint(i);
                    haveBreakpoints = true;
                }
            }
            if (!haveBreakpoints) {
                printF("    No breakpoints\n");
            }
            return true;
        }
        
        // Set breakpoint
        uint32_t num;
        if (!toNum(cmdElements[1], num)) {
            return false;
        }
        
        uint8_t breakpointNum;
        if (!emulator().setBreakpoint(num, breakpointNum)) {
            printF("too many breakpoints\n");
            return false;
        }
        
        showBreakpoint(breakpointNum);
        return true;
    }
    
    // Clear all or one breakpoint
    if(cmdElements[0] == "bc") {
        if (!cmdElements[2].empty()) {
            return false;
        }

        if (cmdElements[1].empty()) {
            emulator().clearAllBreakpoints();
            return true;
        }
        
        uint32_t num;
        if (!toNum(cmdElements[1], num)) {
            return false;
        }

        if (!emulator().clearBreakpoint(num)) {
            printF("invalid breakpoint index\n");
            return false;
        }
        return true;
    }

    // Enable all or one breakpoint
    if(cmdElements[0] == "be") {
        if (!cmdElements[2].empty()) {
            return false;
        }

        if (cmdElements[1].empty()) {
            emulator().enableAllBreakpoints();
            return true;
        }
        
        uint32_t num;
        if (!toNum(cmdElements[1], num)) {
            return false;
        }

        if (!emulator().enableBreakpoint(num)) {
            printF("invalid breakpoint index\n");
            return false;
        }
        return true;
    }

    // Disable all or one breakpoint
    if(cmdElements[0] == "bd") {
        if (!cmdElements[2].empty()) {
            return false;
        }

        if (cmdElements[1].empty()) {
            emulator().disableAllBreakpoints();
            return true;
        }
        
        uint32_t num;
        if (!toNum(cmdElements[1], num)) {
            return false;
        }

        if (!emulator().disableBreakpoint(num)) {
            printF("invalid breakpoint index\n");
            return false;
        }
        return true;
    }

    // Step
    if (cmdElements[0] == "s") {
        _runState = RunState::StepIn;
    } else if (cmdElements[0] == "n") {
        _runState = RunState::StepOver;
    } else if (cmdElements[0] == "o") {
        _runState = RunState::StepOut;
    }
    
    // list next 5 or <n> instructions from current addr
    if(cmdElements[0] == "l") {
        if (!cmdElements[2].empty()) {
            return false;
        }

        uint32_t num = 5;
        if (!cmdElements[1].empty()) {
            if (!toNum(cmdElements[1], num)) {
                return false;
            }
        }
        
        m8r::string s;
        uint16_t addr = emulator().getReg(Reg::PC);
        for (uint32_t i = 0; i < num; ++i) {
            addr = DisplayInst::instToString(emulator(), s, addr);
            printF("%s", s.c_str());
        }
        _needInstPrint = false;
        return true;
    }

    // list next 5 or <n> instructions at <addr>
    if(cmdElements[0] == "la") {
        if (cmdElements[1].empty()) {
            return false;
        }

        uint32_t addr;
        if (!toNum(cmdElements[1], addr)) {
            return false;
        }
        
        if (addr > 65535) {
            return false;
        }

        uint32_t num = 5;
        if (!cmdElements[2].empty()) {        
            if (!toNum(cmdElements[2], num)) {
                return false;
            }
        }
        
        m8r::string s;
        uint16_t printAddr = emulator().getReg(Reg::PC);
        for (uint32_t i = 0; i < num; ++i) {
            printAddr = DisplayInst::instToString(emulator(), s, printAddr);
            printF("%s", s.c_str());
        }
        _needInstPrint = false;
        return true;
    }

    // show current addr or set it to <addr>
    if(cmdElements[0] == "a") {
        if (cmdElements[1].empty()) {
            m8r::string s;
            DisplayInst::instToString(emulator(), s, emulator().getReg(Reg::PC));
            printF("%s", s.c_str());
            _needInstPrint = false;
            return true;
        }

        uint32_t addr;
        if (!toNum(cmdElements[1], addr)) {
            return false;
        }
        
        if (addr > 65535) {
            return false;
        }
        emulator().setReg(Reg::PC, addr);
        m8r::string s;
        DisplayInst::instToString(emulator(), s, emulator().getReg(Reg::PC));
        printF("%s", s.c_str());
        _needInstPrint = false;
        return true;
    }

    // show all regs
    if(cmdElements[0] == "regs") {
        if (!cmdElements[1].empty()) {
            return true;
        }
        
        printF("    ");

        for (Reg reg : regsToPrint) {
            if (reg == Reg::U) {
                printF("\n    ");
            }

            if (emulator().regSizeInBytes(reg) == 1) {
                printF("%s:$%02x ", DisplayInst::regToString(reg), emulator().getReg(reg));
            } else {
                printF("%s:$%04x ", DisplayInst::regToString(reg), emulator().getReg(reg));
            }
        }
        printF("\n");
        return true;
    }

    // show or set reg
    if(cmdElements[0] == "reg") {
        if (cmdElements[1].empty()) {
            return false;
        }
        
        const m8r::string& testRegStr = cmdElements[1].tolower();
        
        bool setReg = false;
        uint32_t v = 0;
        if (!cmdElements[2].empty()) {
           if (!toNum(cmdElements[2], v)) {
                return false;
            }
            setReg = true;
        }
        
        for (Reg reg : regsToPrint) {
            m8r::string regStr(DisplayInst::regToString(reg));
            m8r::string regStrLower = regStr.tolower();
            if (testRegStr == regStrLower) {
                if (setReg) {
                    emulator().setReg(reg, v);
                }
                if (emulator().regSizeInBytes(reg) == 1) {
                    printF("    %s:$%02x\n", regStr.c_str(), emulator().getReg(reg));
                } else {
                    printF("    %s:$%04x\n", regStr.c_str(), emulator().getReg(reg));
                }
            }
        }
        
        return true;
    }

    if (_runState != RunState::Cmd) {
        return cmdElements[1].empty() && cmdElements[2].empty();
    }
    
    printF("Invalid command\n");
    return false;
}

bool BOSS9Base::startExecution(uint16_t addr, bool startInMonitor)
{
    if (startInMonitor) {
        enterMonitor();
    } else {
        leaveMonitor();
    }
    emulator().setReg(Reg::PC, addr);
    _startAddr = addr;
    
    promptIfNeeded();
    return true;
}

bool BOSS9Base::continueExecution()
{
    if (_runState == RunState::Cmd || _runState == RunState::Loading) {
        getCommand();
        return true;
    }
    
    // See if we got an ESC
    int c = getc();
    if (checkEscape(c)) {
        printF("*** Stopped at $%04x\n", emulator().getReg(Reg::PC));
        return true;
    }
    
    // At this point the RunState is anything but Cmd. if its Running
    // it either means we executed the Run command from the monitor
    // or we're looping from main. If it's Continuing it means we
    // just executed the Continue command from the monitor and we
    // need to avoid taking a breakpoint at the current PC (or we'd
    // never execute any instructions). If this is the case we need
    // to change the state to Running after execute() so we run normally
    // the next time through. The other states are for stepping through
    // the code which execute() will deal with.
    bool retval = emulator().execute(_runState);
    if (_runState == RunState::Continuing) {
        _runState = RunState::Running;
    }
    return retval;
}
