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

#include "BOSS9.h"

// Test data (see test/HelloWorld.asm)
//        *  calling monitor c function
//        *  display text on terminal using UART
//
//            include BOSS9.inc
//
//        NumPrints equ 10
//                org $200
//
//        main    lda #NumPrints
//                sta count
//        loop
//                ldx #text3
//                jsr puts
//                lda #newline
//                jsr putc
//                dec count
//                bgt loop
//        done    bra done
//
//        text3   fcn "Hello from 6809 kit"
//
//        count   rmb 1
//
//            end main

char simpleTest[ ] =
    "S02000005B6C77746F6F6C7320342E32325D2048656C6C6F576F726C642E61736DA2\n"
    "S1130200860AB7022B8E0217BDFC02860ABDFC00CB\n"
    "S11302107A022B2EF020FE48656C6C6F2066726F9C\n"
    "S10E02206D2036383039206B69740003\n"
    "S5030003F9\n"
    "S9030200FA\n"
;

static constexpr uint32_t ConsoleWidth = 80;
static constexpr uint32_t ConsoleHeight = 24;
static constexpr uint32_t MemorySize = 65536;

class MacBOSS9 : public mc6809::BOSS9<MemorySize>
{
  public:
    MacBOSS9() : BOSS9()
    {
    }
    
    virtual ~MacBOSS9() { }

  protected:
    virtual void putc(char c) override
    {
        fputc(c, stdout);
        _console[_cursor++] = c;
    }
    
    virtual int getc() override
    {
        return -1;
    }

    virtual bool handleRunLoop() override
    {
        return true;
    }
    
    virtual void exit(int n) override
    {
        while (true) {
            sleep(1);
        }
    }

  private:
    char _console[ConsoleWidth * ConsoleHeight];
    uint32_t _cursor = 0;
};

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
    
    MacBOSS9 boss9;
    
    boss9.setStack(0xe000);
    
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
    
    char* fileString = nullptr;
    bool isFileStringAllocated = false;
    uint32_t size = 0;
    
    if (optind >= argc) {
        // use sample
        fileString = simpleTest;
        size = sizeof(simpleTest);
    } else {
        const char* filename = argv[optind];
        size = uint32_t(std::filesystem::file_size(filename));
        std::ifstream f(filename);
        if (f.is_open()) {
            fileString = new char[size + 1];
            f.read(fileString, size);
            fileString[size] = '\0';
            f.close();
        } else {
            std::cout << "Unable to open file";
            return -1;
        }
    }

        
    startAddr = boss9.load(fileString);
    
    if (isFileStringAllocated) {
        delete [ ] fileString;
    }

    boss9.startExecution(startAddr, startInMonitor);
    
    while (boss9.continueExecution()) { }
    
    return 0;
}
