//
//  main.cpp
//  sim
//
//  Created by Chris Marrin on 4/23/24.
//

#include "MC6809.h"

int main(int argc, char * const argv[])
{
    mc6809::Emulator emu(65536);
    
    emu.execute(0);
    return 0;
}
