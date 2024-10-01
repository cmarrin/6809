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
    ;     int8_t a = 5;
    ;     int8_t b = 6;
    LDA #5
    STA -1,U
    ;     
    LDA #6
    STA -2,U
    ;     if (!(a == 5) || !(b == 5))
    ;         core.printf("Passed\n");
    LDA -1,U
    CMPA #5
    BNE L4
    LDA -2,U
    CMPA #5
    BEQ L1
L4
    ;     else
    LDD #String+$0
    PSHS D
    JSR printf
    BRA L5
L1
    ;         core.printf("Failed\n");
    ;     
    LDD #String+$8
    PSHS D
    JSR printf
L5
    ;     return 0;
    ; }
    LDD #0
    TFR U,S
    PULS U
    RTS

Constants

String
    FCC "Passed"
    FCB $0a
    FCB $00
    FCC "Failed"
    FCB $0a
    FCB $00

    end $200
