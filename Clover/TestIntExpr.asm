* 6809 assembly generated from Clover source

    pragma autobranchlength
    include BOSS9.inc
    org $200

    LEAS -3,S
    TFR S,Y
    LEAS -2,S
    JSR Test_main
    LEAS 2,S
    JMP exit

Test_showIntResults
    PSHS U
    TFR S,U
    ; /*-------------------------------------------------------------------------
    ;     This source file is a part of Clover
    ;     For the latest info, see https://github.com/cmarrin/Clover
    ;     Copyright (c) 2021-2022, Chris Marrin
    ;     All rights reserved.
    ;     Use of this source code is governed by the MIT license that can be
    ;     found in the LICENSE file.
    ; -------------------------------------------------------------------------*/
    ; 
    ; // Test constants, vars, operands and expressions
    ; 
    ; struct Test
    ; {
    ; 
    ; const uint8_t TestSizeDef = 12;
    ; const int16_t TestIntConst = 0xfc;
    ; 
    ; const int8_t testIntTable[ ] = { 1, 2, 3, 7 };
    ; 
    ; int16_t testIntGlobal;
    ; 
    ; uint8_t errors;
    ; 
    ; function showIntResults(uint8_t testNo, int16_t exp, int16_t act)
    ; {
    ;     core.printf("%10s-> %2hhi: ", " ", testNo);
    ;     if (exp != act) {
    LDA 6,U
    TFR A,B
    CLRA
    ADDD #0
    PSHS D
    LDD #String+$0
    PSHS D
    LDD #String+$2
    PSHS D
    JSR printf
    LDD 7,U
    CMPD 9,U
    BEQ L1
    ;         core.printf("     FAIL: exp %i, got %i\n", exp, act);
    ;         errors++;
    LDD 9,U
    PSHS D
    LDD 7,U
    PSHS D
    LDD #String+$11
    PSHS D
    JSR printf
    ;     } else {
    LEAX 2,Y
    LDA 0,X
    PSHS A
    ADDA #1
    STA 0,X
    BRA L2
L1
    ;         core.printf("     Pass\n");
    ;     }
    PSHS X
    LDD #String+$2c
    PSHS D
    JSR printf
L2
    TFR U,S
    PULS U
    RTS

Test_main
    PSHS U
    TFR S,U
    LEAS -4,S
    LDD #0
    STD -2,U
    ; }
    ; 
    ; function int16_t main()
    ; {
    ;     for (uint16_t count = 0; count < 1000; ++count) {
L3
    LDD -2,U
    CMPD #1000
    BHS L4
    ;     errors = 0;
    ;     
    LDA #0
    STA 2,Y
    ;     testIntGlobal = 42;
    ; 
    LDD #42
    STD 0,Y
    ;     core.printf("\nTest Int Exprs\n");
    ; 
    LDD #String+$37
    PSHS D
    JSR printf
    ;     core.printf("\n  Int vals\n");
    ;     showIntResults(1, 12, TestSizeDef);
    LDD #String+$48
    PSHS D
    JSR printf
    ;     showIntResults(2, 0xfc, TestIntConst);
    LDD #12
    PSHS D
    LDD #12
    PSHS D
    LDA #1
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     showIntResults(3, 7, testIntTable[3]);
    LDD #252
    PSHS D
    LDD #252
    PSHS D
    LDA #2
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     
    LDX #Constants+0
    LDA #3
    LDB #1
    MUL
    LEAX D,X
    LDA 0,X
    TFR A,B
    SEX
    PSHS D
    LDD #7
    PSHS D
    LDA #3
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     core.printf("\n  Int ops\n");
    ;     showIntResults(4, 294, TestIntConst + testIntGlobal);
    LDD #String+$55
    PSHS D
    JSR printf
    ;     showIntResults(5, 210, TestIntConst - testIntGlobal);
    LDD #252
    ADDD 0,Y
    PSHS D
    LDD #294
    PSHS D
    LDA #4
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     showIntResults(6, 10584, TestIntConst * testIntGlobal);
    LDD #252
    SUBD 0,Y
    PSHS D
    LDD #210
    PSHS D
    LDA #5
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     showIntResults(7, 6, TestIntConst / testIntGlobal);
    LDD 0,Y
    PSHS D
    LDD #252
    PSHS D
    CLR ,-S
    TST 3,S
    BPL L5
    NEG 0,S
    LDD #0
    SUBD 3,S
    STD 3,S
L5
    TST 1,S
    BPL L6
    NEG 0,S
    LDD #0
    SUBD 1,S
    STD 1,S
L6
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
    BPL L7
    LDD #0
    SUBD 0,S
L7
    PULS D
    LEAS 5,S
    PSHS D
    LDD #10584
    PSHS D
    LDA #6
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     
    LDD 0,Y
    PSHS D
    LDD #252
    PSHS D
    JSR idiv16
    LEAS 4,S
    PSHS D
    LDD #6
    PSHS D
    LDA #7
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     showIntResults(8, -42, -testIntGlobal);
    ;     showIntResults(9, 0, !TestIntConst);
    LDD 0,Y
    PSHS D
    LDD #0
    SUBD 0,S
    LEAS 2,S
    PSHS D
    LDA #42
    NEGA
    TFR A,B
    SEX
    PSHS D
    LDA #8
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     showIntResults(10, 0xff03, ~TestIntConst);
    LDA #252
    BNE L8
    LDA #1
    BRA L9
L8
    CLRA
L9
    TFR A,B
    SEX
    PSHS D
    LDD #0
    PSHS D
    LDA #9
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     
    LDD #252
    COMA
    COMB
    PSHS D
    LDD #65283
    PSHS D
    LDA #10
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     showIntResults(11, 0, TestIntConst < testIntGlobal);
    ;     showIntResults(12, 0, TestIntConst <= testIntGlobal);
    LDD #252
    CMPD 0,Y
    BGE L10
    LDA #1
    BRA L11
L10
    CLRA
L11
    TFR A,B
    SEX
    PSHS D
    LDD #0
    PSHS D
    LDA #11
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     showIntResults(13, 0, TestIntConst == testIntGlobal);
    LDD #252
    CMPD 0,Y
    BGT L12
    LDA #1
    BRA L13
L12
    CLRA
L13
    TFR A,B
    SEX
    PSHS D
    LDD #0
    PSHS D
    LDA #12
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     showIntResults(14, 1, TestIntConst != testIntGlobal);
    LDD #252
    CMPD 0,Y
    BNE L14
    LDA #1
    BRA L15
L14
    CLRA
L15
    TFR A,B
    SEX
    PSHS D
    LDD #0
    PSHS D
    LDA #13
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     showIntResults(15, 1, TestIntConst >= testIntGlobal);
    LDD #252
    CMPD 0,Y
    BEQ L16
    LDA #1
    BRA L17
L16
    CLRA
L17
    TFR A,B
    SEX
    PSHS D
    LDD #1
    PSHS D
    LDA #14
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     showIntResults(16, 1, TestIntConst > testIntGlobal);
    LDD #252
    CMPD 0,Y
    BLT L18
    LDA #1
    BRA L19
L18
    CLRA
L19
    TFR A,B
    SEX
    PSHS D
    LDD #1
    PSHS D
    LDA #15
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     
    LDD #252
    CMPD 0,Y
    BLE L20
    LDA #1
    BRA L21
L20
    CLRA
L21
    TFR A,B
    SEX
    PSHS D
    LDD #1
    PSHS D
    LDA #16
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     int8_t i = 20;
    ;     int8_t j = i++;
    LDA #20
    STA -3,U
    ;     j = ++i;
    LEAX -3,U
    LDA 0,X
    PSHS A
    ADDA #1
    STA 0,X
    PULS A
    STA -4,U
    ; 
    LEAX -3,U
    LDA 0,X
    ADDA #1
    STA 0,X
    STA -4,U
    ;     showIntResults(17, 22, i);
    ;     j = i--;
    LDA -3,U
    TFR A,B
    SEX
    PSHS D
    LDD #22
    PSHS D
    LDA #17
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     j = --i;
    LEAX -3,U
    LDA 0,X
    PSHS A
    ADDA #-1
    STA 0,X
    PULS A
    STA -4,U
    ;     showIntResults(18, 20, i);
    LEAX -3,U
    LDA 0,X
    ADDA #-1
    STA 0,X
    STA -4,U
    ;     showIntResults(19, 20, j);
    LDA -3,U
    TFR A,B
    SEX
    PSHS D
    LDD #20
    PSHS D
    LDA #18
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ; 
    LDA -4,U
    TFR A,B
    SEX
    PSHS D
    LDD #20
    PSHS D
    LDA #19
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     core.printf("\n  op assign\n");
    ;     i += 1;
    LDD #String+$61
    PSHS D
    JSR printf
    ;     showIntResults(20, 21, i);
    LDA -3,U
    ADDA #1
    STA -3,U
    ;     i -= 1;
    LDA -3,U
    TFR A,B
    SEX
    PSHS D
    LDD #21
    PSHS D
    LDA #20
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     showIntResults(21, 20, i);
    LDA -3,U
    SUBA #1
    STA -3,U
    ;     i *= 5;
    LDA -3,U
    TFR A,B
    SEX
    PSHS D
    LDD #20
    PSHS D
    LDA #21
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     showIntResults(22, 100, i);
    CLR ,-S
    LDA -3,U
    BPL L22
    NEG 0,S
    NEGA
L22
    TFR A,B
    LDA #5
    BPL L23
    NEG 0,S
    NEGA
L23
    MUL
    TST 0,S
    BPL L24
    NEGB
L24
    LEAS 1,S
    TFR B,A
    STA -3,U
    ;     i /= 5;
    LDA -3,U
    TFR A,B
    SEX
    PSHS D
    LDD #100
    PSHS D
    LDA #22
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     showIntResults(23, 20, i);
    LDA #5
    PSHS A
    LDA -3,U
    PSHS A
    JSR idiv8
    LEAS 4,S
    STA -3,U
    ;     
    LDA -3,U
    TFR A,B
    SEX
    PSHS D
    LDD #20
    PSHS D
    LDA #23
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     core.printf("\n  Int exprs\n");
    ;     showIntResults(24, 2984, int16_t(testIntTable[1]) + int16_t(TestSizeDef) * TestIntConst - testIntGlobal);
    LDD #String+$6f
    PSHS D
    JSR printf
    ;     showIntResults(25, 2940, int16_t(testIntTable[1] + TestSizeDef) * (TestIntConst - testIntGlobal));
    LDD 0,Y
    PSHS D
    LDD #252
    PSHS D
    LDA #12
    TFR A,B
    SEX
    PSHS D
    CLR ,-S
    TST 3,S
    BPL L25
    NEG 0,S
    LDD #0
    SUBD 3,S
    STD 3,S
L25
    TST 1,S
    BPL L26
    NEG 0,S
    LDD #0
    SUBD 1,S
    STD 1,S
L26
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
    BPL L27
    LDD #0
    SUBD 0,S
L27
    PULS D
    LEAS 5,S
    LDX #Constants+0
    PSHS D
    LDA #1
    LDB #1
    MUL
    LEAX D,X
    LDA 0,X
    TFR A,B
    SEX
    ADDD 0,S
    LEAS 2,S
    SUBD 0,S
    LEAS 2,S
    PSHS D
    LDD #2984
    PSHS D
    LDA #24
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ; 
    LDD #252
    SUBD 0,Y
    PSHS D
    LDA #12
    LDX #Constants+0
    PSHS A
    LDA #1
    LDB #1
    MUL
    LEAX D,X
    LDA 0,X
    ADDA 0,S
    LEAS 1,S
    TFR A,B
    SEX
    PSHS D
    CLR ,-S
    TST 3,S
    BPL L28
    NEG 0,S
    LDD #0
    SUBD 3,S
    STD 3,S
L28
    TST 1,S
    BPL L29
    NEG 0,S
    LDD #0
    SUBD 1,S
    STD 1,S
L29
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
    BPL L30
    LDD #0
    SUBD 0,S
L30
    PULS D
    LEAS 5,S
    PSHS D
    LDD #2940
    PSHS D
    LDA #25
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     core.printf("\nDone.%40s%s\n\n", " ", errors ? "FAILED" : "Passed");
    ;     }
    LDA 2,Y
    BEQ L31
    LDD #String+$7d
    BRA L32
L31
    LDD #String+$84
L32
    PSHS D
    LDD #String+$8b
    PSHS D
    LDD #String+$8d
    PSHS D
    JSR printf
L33
    LEAX -2,U
    LDD 0,X
    ADDD #1
    STD 0,X
    BRA L3
L4
    ;     return errors;
    ; }
    PSHS D
    LDA 2,Y
    TFR A,B
    SEX
    TFR U,S
    PULS U
    RTS

Constants
    FCB $01,$02,$03,$07

String
    FCC " "
    FCB $00
    FCC "%10s-> %2hhi: "
    FCB $00
    FCC "     FAIL: exp %i, got %i"
    FCB $0a
    FCB $00
    FCC "     Pass"
    FCB $0a
    FCB $00
    FCB $0a
    FCC "Test Int Exprs"
    FCB $0a
    FCB $00
    FCB $0a
    FCC "  Int vals"
    FCB $0a
    FCB $00
    FCB $0a
    FCC "  Int ops"
    FCB $0a
    FCB $00
    FCB $0a
    FCC "  op assign"
    FCB $0a
    FCB $00
    FCB $0a
    FCC "  Int exprs"
    FCB $0a
    FCB $00
    FCC "FAILED"
    FCB $00
    FCC "Passed"
    FCB $00
    FCC " "
    FCB $00
    FCB $0a
    FCC "Done.%40s%s"
    FCB $0a
    FCB $0a
    FCB $00


    end $200
