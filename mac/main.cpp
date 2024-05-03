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

#include "MC6809.h"

// Test data (see test/simple.asm
char simpleTest[ ] =    "S01C00005B6C77746F6F6C7320342E32325D2073696D706C652E61736D18\n"
                        "S1120200CC0208FD020F20F848656C6C6F5C6E31\n"
                        "S5030001FB\n"
                        "S9030200FA\n"

;

int main(int argc, char * const argv[])
{
    mc6809::Emulator emu(65536);
        
    if (argc < 2) {
        // use sample
        std::stringstream stream(simpleTest);
        emu.load(stream);
    } else {
        std::ifstream f(argv[1]);
        if (f.is_open()) {
            emu.load(f);
            f.close();
        } else {
            std::cout << "Unable to open file";
            return -1;
        }
    }
    
    emu.execute(0x200);
    return 0;
}
