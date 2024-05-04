# MC6809 Tools

This is a machine code emulator for the MC6809, along with assemblers, tests and other support tools. My goal is to run it on Mac and ESP8266/ESP32. It will not only simulate the 6809, but an entire SBC, including RAM, a ROM monitor and various simulated peripherals.

There are several 6809 single board computers available and plenty of emulators, assemblers and other tools available. I'm writing an emulator completely from scratch both to enjoy the process and to make it easy to run it on a variety of platforms. Running on a Mac will allow me to test and debug the emulator as well as 6809 code written for it.

## External Code/Docs Used

I've used several packages from other sources:

- LWTools: This is a package of 6809 tools at http://www.lwtools.ca. I'm using lwasm to assemble all my .asm files. I built the assembler with no issues and then included the executable in this repos. The only issue is that it runs fine on a Monterey system but not on Sonoma. So I need to deal with that. This is the best assembler I've used, even better than the original Motorola assembler.

- MC6809-MC6809E 8-Bit Microprocessor Programming Manual: This is an HTML version of the original Motorola manual, circa 1981. It was converted (no doubt with great pain and suffering) by Matthias Bücher (https://www.maddes.net). He has put a copy of the docs into a repo (https://github.com/M6809-Docs/m6809pm) to allow collaboration. This will allow me to submit some minor issues I've found in the doc.
