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
#include "MC6809.h"

using namespace mc6809;

void BOSS9Base::getCommand()
{
    bool haveCmd = false;
    promptIfNeeded();
    
    while (true) {
        int c = getc();
        if (c <= 0) {
            break;
        }
        if (_cursor >= CmdBufSize - 1) {
            printf("*** too many chars in cmdbuf\n");
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
            printf("*** unrecognized control char '0x%02x'\n", uint8_t(c));
            _cursor = 0;
            haveCmd = true;
            break;
        }
        
        _cmdBuf[_cursor++] = char(c);
    }
        
    if (haveCmd) {
        _needPrompt = true;
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
        printf("...ABORT...\n");
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
            printf("Error loading\n");
        }
        if (finished) {
            _startAddr = emulator().loadEnd();
            _runState = RunState::Cmd;
            printf("Load complete, start addr = 0x%04x\n", _startAddr);
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
        printf("%s is not a valid number\n", s.c_str());
        return false;
    }
    return true;
}

void BOSS9Base::showBreakpoint(uint8_t i) const
{
    BreakpointEntry entry;
    if (emulator().breakpoint(i, entry)) {
        printf("    Breakpoint[%d] -> $%04x (%sabled)\n", i, entry.addr, (entry.status == BPStatus::Enabled) ? "en" : "dis");
    }
}

bool BOSS9Base::executeCommand(m8r::string cmdElements[3])
{
    assert(_runState == RunState::Cmd);
    
    if (cmdElements[0] == "help" || cmdElements[0] == "h" || cmdElements[0] == "?") {
        if (cmdElements[1].empty()) {
            printf("BOSS9 Monitor Commands:\n");
            printf("\n");
            printf("\tld      - Load SRecords from serial\n");
            printf("\tr       - run at start addr\n");
            printf("\tr a     - run at addr a, set start to a\n");
            printf("\tc       - cont at current addr\n");
            printf("\tc a     - cont at addr a\n");
            printf("\tb       - show cur brkpts\n");
            printf("\tb a     - set brkpt at addr a\n");
            printf("\tbc      - clear all brkpts\n");
            printf("\tbc n    - clear brkpt <n>\n");
            printf("\tbe      - enable all brkpts\n");
            printf("\tbe n    - enable brkpt <n>\n");
            printf("\tbd      - disable all brkpts\n");
            printf("\tbd n    - disable brkpt <n>\n");
            printf("\tn       - next, if at func step over\n");
            printf("\ts       - step, if at func step in\n");
            printf("\to       - step out of cur func\n");
            printf("\tl       - list next 5 insts at cur addr\n");
            printf("\tl n     - list next n insts at cur addr\n");
            printf("\tla a    - list next 5 insts at addr a\n");
            printf("\tla a n  - list next n insts at addr a\n");
            printf("\ta       - show inst at cur addr\n");
            printf("\ta a     - set cur addr to a and show inst\n");
            printf("\tregs    - show all regs\n");
            printf("\treg r   - show reg r\n");
            printf("\treg r v - set reg r to v\n");

            return true;
        }
        printf("Individual command help not yet available\n");
        return false;
    }
    
    // Load file
    if (cmdElements[0] == "ld") {
        if (!cmdElements[1].empty() || !cmdElements[2].empty()) {
            return false;
        }
        
        // Load s19 file
        printf("Ready to start loading. ESC to abort\n");
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
        printf("Running at address $%04x\n", _startAddr);
        _emu.setPC(_startAddr);
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
        printf("Continuing at address $%04x\n", _emu.getPC());
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
                printf("    No breakpoints\n");
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
            printf("too many breakpoints\n");
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
            printf("invalid breakpoint index\n");
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
            printf("invalid breakpoint index\n");
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
            printf("invalid breakpoint index\n");
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
        _emu.printInstructions(_emu.getPC(), num);
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
            if (!toNum(cmdElements[1], num)) {
                return false;
            }
        }
        emulator().printInstructions(addr, num);
        return true;
    }

    // show current addr or set it to <addr>
    if(cmdElements[0] == "a") {
        if (cmdElements[1].empty()) {
            _emu.printInstructions(_emu.getPC(), 1);
            return true;
        }

        uint32_t addr;
        if (!toNum(cmdElements[1], addr)) {
            return false;
        }
        
        if (addr > 65535) {
            return false;
        }
        _emu.setPC(addr);
        _emu.printInstructions(_emu.getPC(), 1);
        return true;
    }

    // show all regs
    if(cmdElements[0] == "regs") {
        if (!cmdElements[1].empty()) {
            return true;
        }
        
        printf("    A:%02x B:%02x D:%04x X:%04x Y:%04x\n",
            _emu.getA(), _emu.getB(), _emu.getD(), _emu.getX(), _emu.getY());
        printf("    U:%04x S:%04x PC:%04x CC:%02x DP:%02x\n",
            _emu.getU(), _emu.getS(), _emu.getPC(), _emu.getCC(), _emu.getDP());
        return true;
    }

    // show or set reg
    if(cmdElements[0] == "reg") {
        if (cmdElements[1].empty()) {
            return false;
        }
        
        const m8r::string& reg = cmdElements[1].tolower();
        
        bool setReg = false;
        uint32_t v = 0;
        if (!cmdElements[2].empty()) {
           if (!toNum(cmdElements[2], v)) {
                return false;
            }
            return false;
        }
        
        if (reg == "a") {
            if (setReg) {
                _emu.setA(v);
            }
            printf("    A:%02x\n", _emu.getA());
        }
        if (reg == "b") {
            if (setReg) {
                _emu.setB(v);
            }
            printf("    B:%02x\n", _emu.getB());
        }
        if (reg == "d") {
            if (setReg) {
                _emu.setD(v);
            }
            printf("    D:%02x\n", _emu.getD());
        }
        if (reg == "x") {
            if (setReg) {
                _emu.setX(v);
            }
            printf("    X:%02x\n", _emu.getX());
        }
        if (reg == "y") {
            if (setReg) {
                _emu.setY(v);
            }
            printf("    Y:%02x\n", _emu.getY());
        }
        if (reg == "u") {
            if (setReg) {
                _emu.setU(v);
            }
            printf("    U:%02x\n", _emu.getU());
        }
        if (reg == "s") {
            if (setReg) {
                _emu.setS(v);
            }
            printf("    S:%02x\n", _emu.getS());
        }
        if (reg == "cc") {
            if (setReg) {
                _emu.setCC(v);
            }
            printf("    CC:%02x\n", _emu.getCC());
        }
        if (reg == "pc") {
            if (setReg) {
                _emu.setPC(v);
            }
            printf("    PC:%02x\n", _emu.getPC());
        }
        if (reg == "dp") {
            if (setReg) {
                _emu.setDP(v);
            }
            printf("    DP:%02x\n", _emu.getDP());
        }
        return true;
    }

    if (_runState != RunState::Cmd) {
        return cmdElements[1].empty() && cmdElements[2].empty();
    }
    
    printf("Invalid command\n");
    return false;
}

bool BOSS9Base::call(Func func)
{
    switch (func) {
        case Func::putc:
            putc(emulator().getReg(Reg::A));
            return true;
        case Func::puts: {
            const char* s = reinterpret_cast<const char*>(emulator().getAddr(emulator().getReg(Reg::X)));
            puts(s);
            return true;
        }
        case Func::getc: {
            emulator().setReg(Reg::A, getc());
            break;
        }
        case Func::exit:
            printf("Program exited with code %d\n", int32_t(emulator().getReg(Reg::A)));
            enterMonitor();
            _emu.setPC(_startAddr);
            return false;
        default: return false;
    }
    return false;
}

bool BOSS9Base::startExecution(uint16_t addr, bool startInMonitor)
{
    if (startInMonitor) {
        enterMonitor();
    } else {
        leaveMonitor();
    }
    _emu.setPC(addr);
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
        printf("*** Stopped at $%04x\n", _emu.getPC());
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
