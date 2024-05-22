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
#include <sys/ioctl.h>

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
//                clra
//                jsr exit
//
//        text3   fcn "Hello from 6809"
//
//        count   rmb 1
//
//            end main

char simpleTest[ ] =
    "S02000005B6C77746F6F6C7320342E32325D2048656C6C6F576F726C642E61736DA2\n"
    "S1130200860AB702298E0219BDFC02860ABDFC00CB\n"
    "S11302107A02292EF04FBDFC0E48656C6C6F206687\n"
    "S10C0220726F6D2036383039008C\n"
    "S5030003F9\n"
    "S9030200FA\n"
;

static constexpr uint32_t MemorySize = 65536;

class MacBOSS9 : public mc6809::BOSS9<MemorySize>
{
  public:
    MacBOSS9() : BOSS9()
    {
        _echoBS = false;
    }
    
    virtual ~MacBOSS9() { }

  protected:
    virtual void putc(char c) const override
    {
        // Throttle character output so console doesn't get swamped
        usleep(1000);
        fputc(c, stdout);
    }
    
    virtual int getc() override
    {
        int bytes = 0;
        if (ioctl(0, FIONREAD, &bytes) == -1) {
            return -1;
        }
        if (bytes == 0) {
            return 0;
        }
        return getchar();
    }

    virtual bool handleRunLoop() override
    {
        return true;
    }
    
  private:
    uint32_t _cursor = 0;
};

char* findNextLine(char* s)
{
    while (*s != '\n' && *s != '\0') {
        ++s;
    }
    if (*s != '\0') {
        s++;
    }
    return s;
}

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
    
    system("stty raw");
    
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
            std::cout << "Unable to open file\n";
            return -1;
        }
    }
    
    boss9.loadStart();
    while (true) {
        bool finished;
        if (!boss9.loadLine(fileString, finished)) {
            std::cout << "Unable to load file\n";
            return -1;
        }

        fileString = findNextLine(fileString);
        if (*fileString == '\0') {
            break;
        }
    }
    
    startAddr = boss9.loadFinish();
    
    if (isFileStringAllocated) {
        delete [ ] fileString;
    }

    boss9.startExecution(startAddr, startInMonitor);
    
    while (boss9.continueExecution()) { }
    
    return 0;
}
