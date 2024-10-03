* 6809 assembly generated from Clover source

    pragma autobranchlength
    include BOSS9.inc
    org $200

    TFR S,Y
    LEAS -2,S
    JSR Simple_main
    LEAS 2,S
    JMP exit

Simple_main
    PSHS U
    TFR S,U
    LEAS -11,S
    ; //
    ; //  simple.Clover
    ; //  Clover
    ; //
    ; //  Created by Chris Marrin on 5/25/24.
    ; //
    ; 
    ; struct Simple
    ; {
    ; 
    ; const int8_t testIntTable[ ] = { 1, 2, 3, 7 };
    ; 
    ; function int16_t main()
    ; {
    ;     int16_t aa = 200;
    ;     int16_t bb = 40;
    LDD #200
    STD -2,U
    ;     int16_t x;
    LDD #40
    STD -4,U
    LDD #0
    STD -8,U
    ;     
    ;     for (uint16_t i = 0; i < 200; ++i) {
L1
    LDD -8,U
    CMPD #200
    BHS L2
    LDD #0
    STD -10,U
    ;         for (uint16_t j = 0; j < 100; ++j) {
L3
    LDD -10,U
    CMPD #100
    BHS L4
    ;             int8_t a = testIntTable[3];
    ;             if (a < 5 || a > 6) {
    LDX #Constants+0
    LDA #3
    LDB #1
    MUL
    LEAX D,X
    LDA 0,X
    STA -11,U
    LDA -11,U
    CMPA #5
    BLT L8
    LDA -11,U
    CMPA #6
    BLE L5
L8
    ;                 x = aa * bb;
    ;             } else {
    LDD -4,U
    PSHS D
    LDD -2,U
    PSHS D
    CLR ,-S
    TST 3,S
    BPL L9
    NEG 0,S
    LDD #0
    SUBD 3,S
    STD 3,S
L9
    TST 1,S
    BPL L10
    NEG 0,S
    LDD #0
    SUBD 1,S
    STD 1,S
L10
    LDA 4,S
    LDB 2,S
    MUL
    PSHS D
    LDA 5,S
    LDB 4,S
    MUL
    ADDB 1,S
    STB 1,S
    LDA 6,S
    LDB 3,S
    MUL
    ADDB 1,S
    STB 1,S
    TST 2,S
    BPL L11
    LDD #0
    SUBD 0,S
L11
    PULS D
    LEAS 5,S
    STD -6,U
    BRA L12
L5
    ;                 x = bb * aa;
    ;             }
    LDD -2,U
    PSHS D
    LDD -4,U
    PSHS D
    CLR ,-S
    TST 3,S
    BPL L13
    NEG 0,S
    LDD #0
    SUBD 3,S
    STD 3,S
L13
    TST 1,S
    BPL L14
    NEG 0,S
    LDD #0
    SUBD 1,S
    STD 1,S
L14
    LDA 4,S
    LDB 2,S
    MUL
    PSHS D
    LDA 5,S
    LDB 4,S
    MUL
    ADDB 1,S
    STB 1,S
    LDA 6,S
    LDB 3,S
    MUL
    ADDB 1,S
    STB 1,S
    TST 2,S
    BPL L15
    LDD #0
    SUBD 0,S
L15
    PULS D
    LEAS 5,S
    STD -6,U
L12
L16
    LEAX -10,U
    LDD 0,X
    ADDD #1
    STD 0,X
    BRA L3
L4
L17
    PSHS X
    LEAX -8,U
    LDD 0,X
    ADDD #1
    STD 0,X
    BRA L1
L2
    ;         }
    ;     }
    ;     return 0;
    ; }
    PSHS D
    LDD #0
    TFR U,S
    PULS U
    RTS

Constants
    FCB $01,$02,$03,$07

String


    end $200
