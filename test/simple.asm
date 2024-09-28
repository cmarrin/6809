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
    ; function int16 main()
    ; {
    ;     int8 a = 5;
    ;     int8 b = 6;
    LDA #$05
    STA -1,U
    ;     
    LDA #$06
    STA -2,U
    ;     if (a < 6 && b >= 6)
    ;         core.printf("Passed\n");
    LDA #$06
    PSHS A
    LDA -1,U
    CMPA 0,S
    BGE L1
    LDA #1
    BRA L2
L1
    CLRA
L2
    LEAS 1,S
    BEQ L3
    LDA #$06
    PSHS A
    LDA -2,U
    CMPA 0,S
    BLT L5
    LDA #1
    BRA L6
L5
    CLRA
L6
    LEAS 1,S
    BRA L4
L3
    LDA #0
L4
    BEQ L7
    ;     else
    LDD #String+$0
    PSHS D
    JSR printf
    BRA L8
L7
    ;         core.printf("Failed\n");
    ;     
    LDD #String+$8
    PSHS D
    JSR printf
L8
    ;     return 0;
    ; }
    LDD #$0000
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
