* 6809 assembly generated from Clover source

    include BOSS9.inc
    org $200
    PSHS U
    TFR S,U
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
    ; struct A
    ; {
    ;     int8 b = 12;
    ;     int16 c = 24;
    LDA #$0c
    PSHS A
    PULS A
    STA 0,Y
    ; };
    LDD #$0018
    PSHS D
    PULS D
    STD 1,Y
    RTS
    PSHS U
    TFR S,U
    LEAS -3,S
    ; 
    ; const A arr[ ] = { 8, 9, 10, 11, 12, 13 };
    ; 
    ; int8 xxx = 9;
    ; 
    ; function int16 main()
    ; {
    ;     A a;
    ;     
    PSHS Y
    LEAX -3,U
    PSHS X
    PULS Y
    JSR 
    PULS Y
    LEAS 0,S
    ;     core.printf(" v1=%d, v2=%d\n", arr[1].b, arr[2].c);
    ;     core.printf(" a.b=%d, a.c=%d\n", a.b, a.c);
    LEAX Constants+0
    PSHS X
    LDA #$02
    PSHS A
    PULS B
    LDA #3
    MUL
    ADDD 0,S
    STD 0,S
    PULS D
    ADDD #1
    TFR D,X
    LDD 0,X
    PSHS D
    LEAX Constants+0
    PSHS X
    LDA #$01
    PSHS A
    PULS B
    LDA #3
    MUL
    ADDD 0,S
    STD 0,S
    PULS X
    LDA 0,X
    PSHS A
    PULS B
    CLRA
    PSHS D
    LDX #String+$0
    PSHS X
    JSR printf
    ;     core.printf(" xxx=%d\n", xxx);
    LDD -2,U
    PSHS D
    LDA -3,U
    PSHS A
    PULS B
    CLRA
    PSHS D
    LDX #String+$f
    PSHS X
    JSR printf
    ;     return 0;
    LDA 0,Y
    PSHS A
    PULS B
    CLRA
    PSHS D
    LDX #String+$20
    PSHS X
    JSR printf
    ; }
    LDD #$0000
    PSHS D
    RTS
    PSHS U
    TFR S,U
    LDA #$09
    PSHS A
    PULS A
    STA 0,Y
    RTS

Constants
    FCB $08,$00,$09,$0a,$00,$0b,$0c,$00
    FCB $0d

String
    FCC " v1=%d, v2=%d"
    FCB $0a
    FCB $00
    FCC " a.b=%d, a.c=%d"
    FCB $0a
    FCB $00
    FCC " xxx=%d"
    FCB $0a
    FCB $00


    end $200
