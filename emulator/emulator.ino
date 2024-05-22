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
    "S01C00005B6C77746F6F6C7320342E32325D2073696D706C652E61736D18\n"
    "S11302008E020FBDFC02860ABDFC00121220F148CA\n"
    "S1070210656C6C6F3A\n"
    "S5030002FA\n"
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
    ESPBOSS9() : BOSS9()
    {
        _echoBS = true;
    }
    
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
