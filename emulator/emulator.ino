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

//char simpleTest[ ] =
//    "S02000005B6C77746F6F6C7320342E32325D2048656C6C6F576F726C642E61736DA2\n"
//    "S1130200860AB702298E0219BDFC02860ABDFC00CB\n"
//    "S11302107A02292EF04FBDFC0E48656C6C6F206687\n"
//    "S10C0220726F6D2036383039008C\n"
//    "S5030003F9\n"
//    "S9030200FA\n"
//;

char simpleTest[ ] =
    "S02300005B6C77746F6F6C7320342E32335D20436C6F7665722F73696D706C652E61736D76\n"
    "S11302001F42327EBD020C32627EFC0E34401F431C\n"
    "S11302103275CC00C8ED5ECC0028ED5CCC0000ED5E\n"
    "S113022058EC5810830064102400EECC0000ED5606\n"
    "S1130230EC5610830064102400D18E03238603C679\n"
    "S1130240013D308BA600A755A6558105102D000849\n"
    "S1130250A6558106102F0055EC5C3406EC5E34067E\n"
    "S11302606FE26D63102A00096060CC0000A363EDA7\n"
    "S1130270636D61102A00096060CC0000A361ED6128\n"
    "S1130280A664E6623D3406A665E6643DEB61E7617B\n"
    "S1130290A666E6633DEB61E7616D62102A0005CC5A\n"
    "S11302A00000A36035063265ED5A160052EC5E3448\n"
    "S11302B006EC5C34066FE26D63102A00096060CCC2\n"
    "S11302C00000A363ED636D61102A00096060CC0037\n"
    "S11302D000A361ED61A664E6623D3406A665E664AA\n"
    "S11302E03DEB61E761A666E6633DEB61E7616D6244\n"
    "S11302F0102A0005CC0000A36035063265ED5A30A3\n"
    "S113030056EC00C30001ED0016FF2534103058EC04\n"
    "S113031000C30001ED0016FF083406CC00001F34B2\n"
    "S10A03203540390102030717\n"
    "S5030013E9\n"
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
        emulator().setStack(0x6000);

        char* fileString = simpleTest;
        
        emulator().loadStart();
        while (true) {
            bool finished;
            if (!emulator().loadLine(fileString, finished)) {
                Serial.println("Unable to load file\n");
                break;
            }

            fileString = findNextLine(fileString);
            if (*fileString == '\0') {
                break;
            }
        }
    
        startAddr = emulator().loadEnd();

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
