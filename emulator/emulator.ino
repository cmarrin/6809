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

static constexpr bool StartInMonitor = true;
static constexpr uint32_t MemorySize = 32768;

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

        char* fileString = simpleTest;
        
        loadStart();
        while (true) {
            bool finished;
            if (!loadLine(fileString, finished)) {
                Serial.println("Unable to load file\n");
                break;
            }

            fileString = findNextLine(fileString);
            if (*fileString == '\0') {
                break;
            }
        }
    
        startAddr = loadFinish();

        startExecution(startAddr, StartInMonitor);
    }

    void loop()
    {
        continueExecution();
    }

  protected:
    virtual void putc(char c) const override
    {
        Serial.write(c);
    }

    virtual int getc() override
    {
        int c = Serial.read();
        if (c == 0x0d) {
            c = '\n';
        }
//        if (c > 0) {
//            printf("*** get char %02x\n", c);
//        }
        return c;
    }

    virtual bool handleRunLoop() override
    {
        return true;
    }
    
  private:
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
