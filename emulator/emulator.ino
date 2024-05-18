//
//  emulator.ino
//
//  Created by Chris Marrin on 5/17/24.
//

#include <Arduino.h>

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
static constexpr uint32_t MemorySize = 32768;

class ESPBOSS9 : public mc6809::BOSS9<MemorySize>
{
  public:
    ESPBOSS9() : BOSS9() { }
    
    virtual ~ESPBOSS9() { }

    void setup()
    {
        Serial.begin(115200);
        delay(500);
        Serial.println("\n\n++++++++++++++ Emulator ************************\n\n");

        uint16_t startAddr = 0;
        setStack(0x6000);
        startAddr = load(simpleTest);
        startExecution(startAddr, false);
    }

    void loop()
    {
        continueExecution();
    }

  protected:
    virtual void putc(char c) override
    {
        Serial.write(c);
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
            delay(1000);
        }
    }

  private:
    char _console[ConsoleWidth * ConsoleHeight];
    uint32_t _cursor = 0;
};

ESPBOSS9 boss9;

void setup()
{
	boss9.setup();
}

void loop()
{
	boss9.loop();
}
