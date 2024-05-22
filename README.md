# MC6809 Tools

This is a machine code emulator for the MC6809, along with assemblers, tests and other support tools. My goal is to run it on Mac and ESP8266/ESP32. It will not only simulate the 6809, but an entire SBC, including RAM, a ROM monitor and various simulated peripherals.

## Emulated Single Board Computer

There are several 6809 single board computers (SBC) available and plenty of emulators, assemblers and other tools available. I'm writing an emulator completely from scratch both to enjoy the process and to make it easy to run it on a variety of platforms. Running on a Mac will allow me to test and debug the emulator as well as 6809 code written for it.

I will create what is the equivalent of a 6809 SBC running on an ESP8266 or ESP32 board. These boards are tiny, typically under $5 and generally have 4MB of flash. Some of this can hold program code while the rest can act as a solid state drive. For instance, 1MB of code space is probably plenty for the emulator and system software, leaving 3MB for disk. I will write a custom operating system which will give access to a console (through the USB port used to program the ESP modules), the on-board WiFi (in the form of TCP/IP access) and disk I/O.

## BOSS9

I'm implementing my own system software for the emulator. There's lots of system software out there for the 6809, from monitors written in the distant past, to more recent efforts for modern 6809 SBC system. But all of these are designed for actual 6809 hardware and everything on this system will be emulated. So I'm creating BOSS9 (Basic Operating System Services for the 6809). I want to have system calls to handle console input and output, making TCP/IP connection (e.g. remote console, accessing weather data, etc.), and disk I/O. All of these will be handled by making indirect JSR calls to specific locations in high memory. Using indirect calls would allow actual addresses to 6809 functions to be stored at these locations. But for this system, the emulator will intercept these calls and route them to native ESP functions.

### API

The API is implemented as addresses in high memory. They are called with JSR (or JMP in the case of exit). Params and return values can be in registers or on the stack according to the description of the function call.

    *
    * Console functions
    *
    [x] putc    equ     $FC00   ; output char in A to console
    [x] puts    equ     $FC02   ; output string pointed to by X (null terminated)
    [ ] putsn   equ     $FC04   ; Output string pointed to by X for length in Y
    [ ] getc    equ     $FC06   ; Get char from console, return it in A
    [ ] peekc   equ     $FC08   ; Return in A a 1 if a char is available and 0 otherwise
    [ ] gets    equ     $FC0A   ; Get a line terminated by \n, place in buffer
                                ; pointed to by X, with max length in Y
    [ ] peeks   equ     $FC0C   ; Return in A a 1 if a line is available and 0 otherwise.
                                ; If available return length of line in Y

    [x] exit    equ     $FC0E   ; Exit program. A contains exit code. If active, enter monitor
                                ; and show prompt
    *
    * Misc equates
    *
    [x] newline equ     $0a  

### Commands

     All commands are case insensitive. [x] incicates that the command is implemented

      [x] <ESC>           - Abort loading or running. Go back to monitor

      [x] L               - Load s19 file. Set _startAddress if successful

      [x] R [<addr>]      - Run from startAddr or passed addr

      [x] C               - Continue running from the current PC

      [x] B [<addr]       - If no addr view current breakpoints, otherwise set a breakpoint at addr

      [x] BC [<num>]      - Clear breakpoint <num> (0-3) or all breakpoints

      [x] BD [<num>]      - Disable breakpoint <num> or all breakpoints

      [x] BE [<num>]      - Enable breakpoint <num> or all breakpoints

      [ ] N [<num>]       - Execute the next 1 or <num> instructions, stepping over BSR and JSR

      [ ] S [<num>]       - Execute the next 1 or <num> instructions, stepping into BSR and JSR

      [ ] O               - Step out of current function, stepping over any JSR or BSR instructions

      [ ] M [<addr>]      - Show 16 bytes at <addr> or at current addr. Set current addr to <addr> + 16

      [ ] RX [<reg>]      - Show <reg> or all regs. <reg> is A | B | D | DP | X | Y | U | S | PC

      [ ] RS <reg> <val>  - Set <reg> to <val>

     <addr> and <val> can be decimal or hex is preceded by '$'. If value is too large it will
     be truncated. There can be 4 breakpoints and each is assigned a number from 0 to 3. when
     a breakpoint is deleted the others are moved up in the list. 'B' lists the breakpoints
     with their assigned number. If a breakpoint is enabled it will be prededed by a '+' sign.
     If disabled it will be preceded by a '-' sign.

## External Code/Docs Used

I've used several packages from other sources:

- LWTools: This is a package of 6809 tools at http://www.lwtools.ca. I'm using lwasm to assemble all my .asm files. I built the assembler with no issues and then included the executable in this repos. The only issue is that it runs fine on a Monterey system but not on Sonoma. So I need to deal with that. This is the best assembler I've used, even better than the original Motorola assembler.

- MC6809-MC6809E 8-Bit Microprocessor Programming Manual: This is an HTML version of the original Motorola manual, circa 1981. It was converted (no doubt with great pain and suffering) by Matthias Bücher (https://www.maddes.net Tach Auch! [Howdy!]). He has put a copy of the docs into a repo (https://github.com/M6809-Docs/m6809pm) to allow collaboration. This will allow me to submit some minor issues I've found in the doc.

- SRecord Parsing Routines: These are from Dave Hylands (http://www.davehylands.com/Software/SRec/). I've included srec.h and srec.cpp in this repo. They comprise a C++ class to parse an SRecord file and make callbacks with the data.
