* 6809 assembly generated from Clover source

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
    LEAS -1,S
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
    ;     int8_t a = testIntTable[3];
    ;     core.printf("a=%d\n", a);
    LDX #Constants+0
    LDA #3
    LDB #1
    MUL
    LEAX D,X
    LDA 0,X
    STA -1,U
    ;     return 0;
    LDA -1,U
    TFR A,B
    CLRA
    ADDD #0
    PSHS D
    LDD #String+$0
    PSHS D
    JSR printf
    ; }
    LDD #0
    TFR U,S
    PULS U
    RTS

Constants
    FCB $01,$02,$03,$07

String
    FCC "a=%d"
    FCB $0a
    FCB $00


    end $200
