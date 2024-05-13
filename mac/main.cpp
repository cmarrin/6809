//
//  main.cpp
//  emulator
//
//  Created by Chris Marrin on 4/23/24.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>

#include "MC6809.h"

// Test data (see test/simple.asm)
//
//                          (       simple.asm):00001         *  simple test file
//                          (       simple.asm):00002
//                          (       simple.asm):00003                 org $200
//                          (       simple.asm):00004
//    0200                  (       simple.asm):00005         loop
//    0200 CC0208           (       simple.asm):00006                 ldd #text3
//    0203 FD020F           (       simple.asm):00007                 std store
//    0206 20F8             (       simple.asm):00008                 bra loop
//                          (       simple.asm):00009
//    0208 48656C6C6F5C6E   (       simple.asm):00010         text3   fcc "Hello\n"
//    020F                  (       simple.asm):00011         store   rmb     2
//                          (       simple.asm):00012
//                          (       simple.asm):00013                 end $200

char simpleTest[ ] =    "S01C00005B6C77746F6F6C7320342E32325D2073696D706C652E61736D18\n"
                        "S1120200CC0208FD020F20F848656C6C6F5C6E31\n"
                        "S5030001FB\n"
                        "S9030200FA\n"

;

//
// Usage: emulator -m [filename]
//
//          -m:         stop in monitor on entry
//          filename:   s19 file to load. If none given a simple test progam is loaded
int main(int argc, char * const argv[])
{
    // For now we're going to assume 64KB of RAM and that there will
    // Be some sort of system functions at $E000, which is where the
    // ACIA is located in some SBC systems. So we'll out the stack
    // at $E000
    //
    // We'll figure out the rest later.
    
    mc6809::Emulator emu(65536);
    emu.setStack(0xe000);
    
    uint16_t startAddr = 0;
    bool startInMonitor = false;
    int c;
        
    while ((c = getopt(argc, argv, "m")) != -1) {
        switch (c) {
            case 'm':
                startInMonitor = true;
                break;
            default: /* '?' */
                fprintf(stderr, "Usage: %s [-m] [filename]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    
    if (optind >= argc) {
        // use sample
        std::stringstream stream(simpleTest);
        startAddr = emu.load(stream);
    } else {
        const char* filename = argv[optind];
        std::ifstream f(filename);
        if (f.is_open()) {
            startAddr = emu.load(f);
            f.close();
        } else {
            std::cout << "Unable to open file";
            return -1;
        }
    }
    
    emu.execute(startAddr, startInMonitor);
    return 0;
}
