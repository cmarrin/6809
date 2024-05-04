# MC6809 Tools

This is a machine code emulator for the MC6809, along with assemblers, tests and other support tools. My goal is to run it on Mac and ESP8266/ESP32. It will not only simulate the 6809, but an entire SBC, including RAM, a ROM monitor and various simulated peripherals.

## Emulated Single Board Computer

There are several 6809 single board computers (SBC) available and plenty of emulators, assemblers and other tools available. I'm writing an emulator completely from scratch both to enjoy the process and to make it easy to run it on a variety of platforms. Running on a Mac will allow me to test and debug the emulator as well as 6809 code written for it.

I will create what is the equivalent of a 6809 SBC These boards are typically under $5 and generally have 4MB of flash. Some of this can hold program code while the rest can act as a solid state drive. For instance, 1MB of code space is probably plenty for the emulator and system software, leaving 3MB for disk.


## BOSS9

I'm implementing my own system software for the emulator. Ultimately my goal is to run the emulator on ESP8266/ESP32. So I want to have system calls to handle console input and output, making TCP/IP connection (e.g. remote console, accessing weather data, etc.), and disk I/O. 
## External Code/Docs Used

I've used several packages from other sources:

- LWTools: This is a package of 6809 tools at http://www.lwtools.ca. I'm using lwasm to assemble all my .asm files. I built the assembler with no issues and then included the executable in this repos. The only issue is that it runs fine on a Monterey system but not on Sonoma. So I need to deal with that. This is the best assembler I've used, even better than the original Motorola assembler.

- MC6809-MC6809E 8-Bit Microprocessor Programming Manual: This is an HTML version of the original Motorola manual, circa 1981. It was converted (no doubt with great pain and suffering) by Matthias Bücher (https://www.maddes.net Tach Auch! [Howdy!]). He has put a copy of the docs into a repo (https://github.com/M6809-Docs/m6809pm) to allow collaboration. This will allow me to submit some minor issues I've found in the doc.

- SRecord Parsing Routines: These are from Dave Hylands (http://www.davehylands.com/Software/SRec/). I've included srec.h and srec.cpp in this repo. They comprise a C++ class to parse an SRecord file and make callbacks with the data.
