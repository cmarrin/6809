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
    LEAS -2,S
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
    ; function int16_t main()
    ; {
    ;     int8_t a = -80;
    ;     int8_t b = 4;
    LDA #80
    NEGA
    STA -1,U
    ;     a = a >> b;
    LDA #4
    STA -2,U
    ;     a = a << b;
    LDA -1,U
    PSHS A
    LDA -2,U
L1
    ASR 0,S
    DECA
    BNE L1
    PULS A
    STA -1,U
    ;     core.printf("a=%d\n", a);
    LDA -1,U
    PSHS A
    LDA -2,U
L2
    LSL 0,S
    DECA
    BNE L2
    PULS A
    STA -1,U
    ;     return 0;
    LDA -1,U
    TFR A,B
    SEX
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

String
    FCC "a=%d"
    FCB $0a
    FCB $00

    end $200
