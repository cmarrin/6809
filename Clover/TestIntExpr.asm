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
    LEAS -6,S
    ; }
    ; 
    ; function int16_t main()
    ; {
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
    BPL L3
    NEG 0,S
    LDD #0
    SUBD 3,S
    STD 3,S
L3
    TST 1,S
    BPL L4
    NEG 0,S
    LDD #0
    SUBD 1,S
    STD 1,S
L4
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
    BPL L5
    LDD #0
    SUBD 0,S
L5
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
    BNE L6
    LDA #1
    BRA L7
L6
    CLRA
L7
    TFR A,B
    CLRA
    ADDD #0
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
    BGE L8
    LDA #1
    BRA L9
L8
    CLRA
L9
    TFR A,B
    CLRA
    ADDD #0
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
    BGT L10
    LDA #1
    BRA L11
L10
    CLRA
L11
    TFR A,B
    CLRA
    ADDD #0
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
    BNE L12
    LDA #1
    BRA L13
L12
    CLRA
L13
    TFR A,B
    CLRA
    ADDD #0
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
    BEQ L14
    LDA #1
    BRA L15
L14
    CLRA
L15
    TFR A,B
    CLRA
    ADDD #0
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
    BLT L16
    LDA #1
    BRA L17
L16
    CLRA
L17
    TFR A,B
    CLRA
    ADDD #0
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
    BLE L18
    LDA #1
    BRA L19
L18
    CLRA
L19
    TFR A,B
    CLRA
    ADDD #0
    PSHS D
    LDD #1
    PSHS D
    LDA #16
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     core.printf("\n  Shift ops\n");
    ; 
    LDD #String+$61
    PSHS D
    JSR printf
    ;     int8_t a = -80;
    ;     a = a >> 4;
    LDA #80
    NEGA
    STA -1,U
    ;     showIntResults(17, -5, a);
    LDA -1,U
    PSHS A
    LDA #4
L20
    ASR 0,S
    DECA
    BNE L20
    PULS A
    STA -1,U
    ;     a = a << 4;
    LDA -1,U
    TFR A,B
    SEX
    PSHS D
    LDA #5
    NEGA
    TFR A,B
    SEX
    PSHS D
    LDA #17
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     showIntResults(18, -80, a);
    LDA -1,U
    PSHS A
    LDA #4
L21
    LSL 0,S
    DECA
    BNE L21
    PULS A
    STA -1,U
    ;     
    LDA -1,U
    TFR A,B
    SEX
    PSHS D
    LDA #80
    NEGA
    TFR A,B
    SEX
    PSHS D
    LDA #18
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     int8_t b = 4;
    ;     a = a >> b;
    LDA #4
    STA -2,U
    ;     showIntResults(19, -5, a);
    LDA -1,U
    PSHS A
    LDA -2,U
L22
    ASR 0,S
    DECA
    BNE L22
    PULS A
    STA -1,U
    ;     a = a << b;
    LDA -1,U
    TFR A,B
    SEX
    PSHS D
    LDA #5
    NEGA
    TFR A,B
    SEX
    PSHS D
    LDA #19
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     showIntResults(20, -80, a);
    LDA -1,U
    PSHS A
    LDA -2,U
L23
    LSL 0,S
    DECA
    BNE L23
    PULS A
    STA -1,U
    ;     
    LDA -1,U
    TFR A,B
    SEX
    PSHS D
    LDA #80
    NEGA
    TFR A,B
    SEX
    PSHS D
    LDA #20
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     a = 5;
    ;     a = a << b;
    LDA #5
    STA -1,U
    ;     showIntResults(21, 80, a);
    LDA -1,U
    PSHS A
    LDA -2,U
L24
    LSL 0,S
    DECA
    BNE L24
    PULS A
    STA -1,U
    ;     a = a >> b;
    LDA -1,U
    TFR A,B
    SEX
    PSHS D
    LDD #80
    PSHS D
    LDA #21
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     showIntResults(22, 5, a);
    LDA -1,U
    PSHS A
    LDA -2,U
L25
    ASR 0,S
    DECA
    BNE L25
    PULS A
    STA -1,U
    ;     
    LDA -1,U
    TFR A,B
    SEX
    PSHS D
    LDD #5
    PSHS D
    LDA #22
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     int16_t c = 0x1234;
    ;     c = c >> b;
    LDD #4660
    STD -4,U
    ;     showIntResults(23, 291 , c);
    LDD -4,U
    PSHS D
    LDA -2,U
    TFR A,B
    SEX
    TFR B,A
L26
    ASR 0,S
    ROR 1,S
    DECA
    BNE L26
    PULS D
    STD -4,U
    ;     c = c << b;
    LDD -4,U
    PSHS D
    LDD #291
    PSHS D
    LDA #23
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     showIntResults(24, 4656, c);
    LDD -4,U
    PSHS D
    LDA -2,U
    TFR A,B
    SEX
    TFR B,A
L27
    LSL 1,S
    ROL 0,S
    DECA
    BNE L27
    PULS D
    STD -4,U
    ; 
    LDD -4,U
    PSHS D
    LDD #4656
    PSHS D
    LDA #24
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     int8_t i = 20;
    ;     int8_t j = i++;
    LDA #20
    STA -5,U
    ;     j = ++i;
    LEAX -5,U
    LDA 0,X
    PSHS A
    ADDA #1
    STA 0,X
    PULS A
    STA -6,U
    ; 
    LEAX -5,U
    LDA 0,X
    ADDA #1
    STA 0,X
    STA -6,U
    ;     showIntResults(25, 22, i);
    ;     j = i--;
    LDA -5,U
    TFR A,B
    SEX
    PSHS D
    LDD #22
    PSHS D
    LDA #25
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     j = --i;
    LEAX -5,U
    LDA 0,X
    PSHS A
    ADDA #-1
    STA 0,X
    PULS A
    STA -6,U
    ;     showIntResults(26, 20, i);
    LEAX -5,U
    LDA 0,X
    ADDA #-1
    STA 0,X
    STA -6,U
    ;     showIntResults(27, 20, j);
    LDA -5,U
    TFR A,B
    SEX
    PSHS D
    LDD #20
    PSHS D
    LDA #26
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ; 
    LDA -6,U
    TFR A,B
    SEX
    PSHS D
    LDD #20
    PSHS D
    LDA #27
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     core.printf("\n  op assign\n");
    ;     i += 1;
    LDD #String+$6f
    PSHS D
    JSR printf
    ;     showIntResults(28, 21, i);
    LDA -5,U
    ADDA #1
    STA -5,U
    ;     i -= 1;
    LDA -5,U
    TFR A,B
    SEX
    PSHS D
    LDD #21
    PSHS D
    LDA #28
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     showIntResults(29, 20, i);
    LDA -5,U
    SUBA #1
    STA -5,U
    ;     i *= 5;
    LDA -5,U
    TFR A,B
    SEX
    PSHS D
    LDD #20
    PSHS D
    LDA #29
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     showIntResults(30, 100, i);
    CLR ,-S
    LDA -5,U
    BPL L28
    NEG 0,S
    NEGA
L28
    TFR A,B
    LDA #5
    BPL L29
    NEG 0,S
    NEGA
L29
    MUL
    TST 0,S
    BPL L30
    NEGB
L30
    LEAS 1,S
    TFR B,A
    STA -5,U
    ;     i /= 5;
    LDA -5,U
    TFR A,B
    SEX
    PSHS D
    LDD #100
    PSHS D
    LDA #30
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     showIntResults(31, 20, i);
    LDA #5
    PSHS A
    LDA -5,U
    PSHS A
    JSR idiv8
    LEAS 4,S
    STA -5,U
    ;     
    LDA -5,U
    TFR A,B
    SEX
    PSHS D
    LDD #20
    PSHS D
    LDA #31
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     core.printf("\n  Int exprs\n");
    ;     showIntResults(32, 2984, int16_t(testIntTable[1]) + int16_t(TestSizeDef) * TestIntConst - testIntGlobal);
    LDD #String+$7d
    PSHS D
    JSR printf
    ;     showIntResults(33, 2940, int16_t(testIntTable[1] + TestSizeDef) * (TestIntConst - testIntGlobal));
    LDD 0,Y
    PSHS D
    LDD #252
    PSHS D
    LDA #12
    TFR A,B
    CLRA
    ADDD #0
    PSHS D
    CLR ,-S
    TST 3,S
    BPL L31
    NEG 0,S
    LDD #0
    SUBD 3,S
    STD 3,S
L31
    TST 1,S
    BPL L32
    NEG 0,S
    LDD #0
    SUBD 1,S
    STD 1,S
L32
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
    BPL L33
    LDD #0
    SUBD 0,S
L33
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
    LDA #32
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
    BPL L34
    NEG 0,S
    LDD #0
    SUBD 3,S
    STD 3,S
L34
    TST 1,S
    BPL L35
    NEG 0,S
    LDD #0
    SUBD 1,S
    STD 1,S
L35
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
    BPL L36
    LDD #0
    SUBD 0,S
L36
    PULS D
    LEAS 5,S
    PSHS D
    LDD #2940
    PSHS D
    LDA #33
    PSHS A
    LEAS -2,S
    JSR Test_showIntResults
    LEAS 7,S
    ;     core.printf("\nDone.%40s%s\n\n", " ", errors ? "FAILED" : "Passed");
    ;     return errors;
    LDA 2,Y
    BEQ L37
    LDD #String+$8b
    BRA L38
L37
    LDD #String+$92
L38
    PSHS D
    LDD #String+$99
    PSHS D
    LDD #String+$9b
    PSHS D
    JSR printf
    ; }
    LDA 2,Y
    TFR A,B
    CLRA
    ADDD #0
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
    FCC "  Shift ops"
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
