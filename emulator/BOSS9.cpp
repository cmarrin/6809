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
        
        if (c < 0x20 || c > 0x7f) {
            // Some other control character, for now ignore
            break;
        }
        
        _cmdBuf[_cursor++] = char(c);
    }
        
    if (haveCmd) {
        enterMonitor();
        if (_cursor > 0) {
            processCommand();
        }
    }
}

bool BOSS9Base::checkEscape(int c)
{
    if (c == 0x1b) {
        // Escape - go back  to command mode
        _cmdState = CmdState::Cmd;
        enterMonitor();
        printf("...ABORT...\n");
        return true;
    }
    return false;
}

// Commands
//
// All commands are case insensitive
//
//
//  <ESC>           - Abort loading or running. Go back to monitor
//
//  L               - Load s19 file. Set _startAddress if successful
//
//  R [<addr>]      - Run from startAddr or passed addr
//
//  B               - View current breakpoints
//
//  BS <addr>       - Set breakpoint at <addr>, error if breakpoint list is full
//
//  BC <num>        - Clear breakpoint <num> (0-3)
//
//  BD [<num>]      - Disable breakpoint <num> or all breakpoints
//
//  BE [<num>]      - Enable breakpoint <num> or all breakpoints
//
//  N [<num>]       - Execute the next 1 or <num> instructions, stepping over BSR and JSR
//
//  S [<num>]       - Execute the next 1 or <num> instructions, stepping into BSR and JSR
//
//  O               - Step out of current function, stepping over any JSR or BSR instructions
//
//  M [<addr>]      - Show 16 bytes at <addr> or at current addr. Set current addr to <addr> + 16
//
//  RX [<reg>]      - Show <reg> or all regs. <reg> is A | B | D | DP | X | Y | U | S | PC
//
//  RS <reg> <val>  - Set <reg> to <val>
//
// <addr> and <val> can be decimal or hex is preceded by '$'. If value is too large it will
// be truncated. There can be 4 breakpoints and each is assigned a number from 0 to 3. when
// a breakpoint is deleted the others are moved up in the list. 'B' lists the breakpoints
// with their assigned number. If a breakpoint is enabled it will be prededed by a '+' sign.
// If disabled it will be preceded by a '-' sign.

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
    if (_cmdState == CmdState::Cmd) {
        // Parse Command
        m8r::string cmdElements[3];
        if (parseCmd(_cmdBuf, cmdElements)) {
            executeCommand(cmdElements);
        }
        return;
    }
        
    if (_cmdState == CmdState::Loading) {
        bool finished;
        if (!loadLine(_cmdBuf, finished)) {
            _cmdState = CmdState::Cmd;
            printf("Error loading\n");
        }
        if (finished) {
            _startAddr = loadFinish();
            _cmdState = CmdState::Cmd;
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
    if (_emu.breakpoint(i, entry)) {
        printf("    Breakpoint[%d] -> $%04x (%sabled)\n", i, entry.addr, (entry.status == BPStatus::Enabled) ? "en" : "dis");
    }
}

bool BOSS9Base::executeCommand(m8r::string cmdElements[3])
{
    if (cmdElements[0] == "l") {
        if (!cmdElements[1].empty() || !cmdElements[2].empty()) {
            return false;
        }
        
        // Load s19 file
        printf("Ready to start loading. ESC to abort\n");
        _cmdState = CmdState::Loading;
        loadStart();
        return true;
    }

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
    
    if (cmdElements[0] == "b") {
        if (!cmdElements[1].empty() || !cmdElements[2].empty()) {
            return false;
        }
        bool haveBreakpoints = false;
        
        for (auto i = 0; i < NumBreakpoints; ++i) {
            BreakpointEntry entry;
            if (_emu.breakpoint(i, entry)) {
                showBreakpoint(i);
                haveBreakpoints = true;
            }
        }
        if (!haveBreakpoints) {
            printf("    No breakpoints\n");
        }
        return true;
    }
    
    if (cmdElements[0] == "bs") {
        if (!cmdElements[2].empty()) {
            return false;
        }

        uint32_t num;
        if (!toNum(cmdElements[1], num)) {
            return false;
        }
        
        uint8_t breakpointNum;
        if (!_emu.setBreakpoint(num, breakpointNum)) {
            printf("too many breakpoints\n");
            return false;
        }
        
        showBreakpoint(breakpointNum);
        return true;
    }
    
    if(cmdElements[0] == "bc") {
        if (!cmdElements[2].empty()) {
            return false;
        }

        if (cmdElements[1].empty()) {
            _emu.clearAllBreakpoints();
            return true;
        }
        
        uint32_t num;
        if (!toNum(cmdElements[1], num)) {
            return false;
        }

        if (!_emu.clearBreakpoint(num)) {
            printf("invalid breakpoint index\n");
            return false;
        }
        return true;
    }

    if(cmdElements[0] == "be") {
        if (!cmdElements[2].empty()) {
            return false;
        }

        if (cmdElements[1].empty()) {
            _emu.enableAllBreakpoints();
            return true;
        }
        
        uint32_t num;
        if (!toNum(cmdElements[1], num)) {
            return false;
        }

        if (!_emu.enableBreakpoint(num)) {
            printf("invalid breakpoint index\n");
            return false;
        }
        return true;
    }

    if(cmdElements[0] == "bd") {
        if (!cmdElements[2].empty()) {
            return false;
        }

        if (cmdElements[1].empty()) {
            _emu.disableAllBreakpoints();
            return true;
        }
        
        uint32_t num;
        if (!toNum(cmdElements[1], num)) {
            return false;
        }

        if (!_emu.disableBreakpoint(num)) {
            printf("invalid breakpoint index\n");
            return false;
        }
        return true;
    }

    printf("Invalid command\n");
    return false;
}

bool BOSS9Base::call(Emulator* engine, uint16_t ea)
{
    switch (Func(ea)) {
        case Func::putc:
            putc(engine->getA());
            return true;
        case Func::puts: {
            const char* s = reinterpret_cast<const char*>(engine->getAddr(engine->getX()));
            puts(s);
            return true;
        }
        case Func::exit:
            printf("Program exited with code %d\n", int32_t(engine->getA()));
            enterMonitor();
            return false;
        default: return false;
    }
    return false;
}

bool BOSS9Base::startExecution(uint16_t addr, bool startInMonitor)
{
    if (startInMonitor) {
        enterMonitor();
    }
    _emu.setPC(addr);
    _startAddr = addr;
    
    promptIfNeeded();
    return true;
}

bool BOSS9Base::continueExecution()
{
    if (_inMonitor) {
        getCommand();
        return true;
    }
    
    // See if we got an ESC
    int c = getc();
    if (checkEscape(c)) {
        printf("*** Stopped at $%04x\n", _emu.getPC());
        return true;
    }
        
    return _emu.execute();
}
