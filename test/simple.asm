* 6809 assembly generated from Clover source

    include BOSS9.inc
    org $200

    LEAS -1,S
    TFR S,Y
    LEAS -2,S
    JSR Simple_ctor
    LEAS 2,S
    LEAS -2,S
    JSR Simple_main
    LEAS 2,S
    JMP exit

Simple_main
    PSHS U
    TFR S,U
    LEAS -6,S
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
    ; //struct A
    ; //{
    ; //    int8 b = 12;
    ; //    int16 c = 24;
    ; //};
    ; //
    ; //const A arr[ ] = { 8, 9, 10, 11, 12, 13 };
    ; //
    ; int8 xxx = 9;
    ; //
    ; 
    ; const uint16 array[4] = { 2, 4, 6, 8 };
    ; 
    ; function int16 main()
    ; {
    ; //    uint16* p = &array[0];
    ; //    for (uint8 i = 0; i < 4; i++) {
    ; //        core.printf("array[%d] = %d\n", i, *p);
    ; //        p++;
    ; //    }
    ; //
    ;     int16 a = 5;
    ;     uint16 b = 6;
    LDD #$0005
    PSHS D
    PULS D
    STD -2,U
    ;     int8 c = 8;
    LDD #$0006
    PSHS D
    PULS D
    STD -4,U
    ;     int8 d = 7;
    LDA #$08
    PSHS A
    PULS A
    STA -5,U
    ;  
    LDA #$07
    PSHS A
    PULS A
    STA -6,U
    ; //    bool x = a < b;
    ;     
    ;     if (a < b && c > d) {
    CLR ,-S
    LDD -2,U
    PSHS D
    LDD -4,U
    PSHS D
    LDA 1,S
    CMPA 0,S
    LEAS 2,S
    BGE L1
    INC 0,S
L1
    PULS A
    BEQ L2
    CLR ,-S
    LDA -5,U
    PSHS A
    LDA -6,U
    PSHS A
    LDA 1,S
    CMPA 0,S
    LEAS 2,S
    BLE L4
    INC 0,S
L4
    BRA L3
L2
    LDA #0
    PSHS A
L3
    PULS A
    BEQ L5
    ;         core.printf("Hello\n\n");
    ;     }
    LDX #String+$0
    PSHS X
    JSR printf
L5
    ;     
    ; //    A a;
    ; //    
    ; //    core.printf(" v1=%d, v2=%d\n", arr[1].b, arr[2].c);
    ; //    core.printf(" a.b=%d, a.c=%d\n", a.b, a.c);
    ; //    core.printf(" xxx=%d\n", xxx);
    ;     return 0;
    ; }
    LDD #$0000
    PSHS D
    TFR U,S
    PULS U
    RTS

Simple_ctor
    PSHS U
    TFR S,U
    LDA #$09
    PSHS A
    PULS A
    STA 0,Y
    TFR U,S
    PULS U
    RTS

Constants
    FCB $00,$02,$00,$04,$00,$06,$00,$08

String
    FCC "Hello"
    FCB $0a
    FCB $0a
    FCB $00

    end $200
