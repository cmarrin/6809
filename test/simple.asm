* 6809 assembly generated from Clover source

    include BOSS9.inc
    org $200

    LEAS -0,S
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
    ;     int8 b = a + 1;
    LDA #$05
    STA -1,U
    ;     
    LDA -1,U
    PSHS A
    LDA #$01
    ADDA 0,S
    LEAS 1,S
    STA -2,U
    ;     switch (b) {
    ;         case 4: core.printf("Failed\n");
    LDA -2,U
    PSHS A
    LDD #L3
    PSHS D
    LDD #2
    PSHS D
    JSR switch1
    LEAS 5,S
    JMP 0,X
L3
    FCB 4
    FDB L4
    FCB 6
    FDB L5
    ;         case 6: core.printf("Passed\n");
    ;         default: core.printf("Huh?\n");
    ;     }
    LDD #String+$0
    PSHS D
    JSR printf
    LBRA L6
L4
    LDD #String+$6
    PSHS D
    JSR printf
    LBRA L6
L5
    LDD #String+$e
    PSHS D
    JSR printf
L6
    ;     
    ;     return 0;
    ; }
    LDD #$0000
    TFR U,S
    PULS U
    RTS

Constants

String
    FCC "Huh?"
    FCB $0a
    FCB $00
    FCC "Failed"
    FCB $0a
    FCB $00
    FCC "Passed"
    FCB $0a
    FCB $00

    end $200
