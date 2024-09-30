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
    ;     if (!(a == 5) || !(b == 5))
    ;         core.printf("Passed\n");
    LDA #$05
    PSHS A
    LDA -1,U
    CMPA 0,S
    LEAS 1,S
    BNE L4
    LDA #$05
    PSHS A
    LDA -2,U
    CMPA 0,S
    LEAS 1,S
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
