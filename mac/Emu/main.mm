//
//  main.m
//  Emu
//
//  Created by Chris Marrin on 5/15/24.
//

#import <Cocoa/Cocoa.h>

#import "AppDelegate.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>

#include "string.h"
#include "BOSS9.h"

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

static constexpr uint32_t WindowWidth = 500;
static constexpr uint32_t WindowHeight = 500;
static constexpr uint32_t ConsoleWidth = 80;
static constexpr uint32_t ConsoleHeight = 24;


class MacBOSS9 : public mc6809::BOSS9
{
  public:
    MacBOSS9(uint32_t size) : BOSS9(size)
    {
    }
    
    virtual ~MacBOSS9() { }

  protected:
    virtual void putc(char c) override
    {
    }
    
    virtual int getc() override
    {
        return -1;
    }

    virtual bool handleRunLoop() override
    {
        return true;
    }
    
  private:
    void showConsole()
    {
    }
};

//
// Usage: emulator -m [filename]
//
//          -m:         stop in monitor on entry
//          filename:   s19 file to load. If none given a simple test progam is loaded

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        // Setup code that might create autoreleased objects goes here.
    }
    
    // For now we're going to assume 64KB of RAM and that there will
    // Be some sort of system functions at $E000, which is where the
    // ACIA is located in some SBC systems. So we'll out the stack
    // at $E000
    //
    // We'll figure out the rest later.
    
    MacBOSS9 boss9(65536);
    
    boss9.setStack(0xe000);
    
    uint16_t startAddr = 0;
    bool startInMonitor = false;
    int c;
    
    while ((c = getopt(argc, (char**) argv, "m")) != -1) {
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
        startAddr = boss9.load(stream);
    } else {
        NSString* resourcePath = [[NSBundle mainBundle] resourcePath];
        m8r::string path([resourcePath UTF8String]);
        path += "/";
        path += argv[optind];
        std::ifstream f(path.c_str());
        if (f.is_open()) {
            startAddr = boss9.load(f);
            f.close();
        } else {
            std::cout << "Unable to open file";
            return -1;
        }
    }

    boss9.startExecution(startAddr, startInMonitor);

    MacBOSS9* localBOSS9 = &boss9;
    
    AppDelegate *appDelegate = [[NSApplication sharedApplication] delegate];
    
    appDelegate.observerRef = CFRunLoopObserverCreateWithHandler(NULL, kCFRunLoopBeforeTimers, YES, 0,
        ^(CFRunLoopObserverRef observer, CFRunLoopActivity activity) {
            localBOSS9->continueExecution();
        });

    return NSApplicationMain(argc, argv);
}
