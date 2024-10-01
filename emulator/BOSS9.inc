*-------------------------------------------------------------------------
*    This source file is a part of the MC6809 Simulator
*    For the latest info, see http:www.marrin.org/
*    Copyright (c) 2018-2024, Chris Marrin
*    All rights reserved.
*    Use of this source code is governed by the MIT license that can be
*    found in the LICENSE file.
*-------------------------------------------------------------------------
*
*  BOSS9.inc
*  Assembly language function and address includes for BOSS9
*
*  Created by Chris Marrin on 5/4/24.
*

*
* Console functions
*
putc    equ     $FC00   ; output char in A to console
puts    equ     $FC02   ; output string pointed to by X (null terminated)
putsn   equ     $FC04   ; Output string pointed to by X for length in Y
getc    equ     $FC06   ; Get char from console, return it in A
peekc   equ     $FC08   ; Return in A a 1 if a char is available and 0 otherwise
gets    equ     $FC0A   ; Get a line terminated by \n, place in buffer
                        ; pointed to by X, with max length in Y
peeks   equ     $FC0C   ; Return in A a 1 if a line is available and 0 otherwise.
                        ; If available return length of line in Y

exit    equ     $FC0E   ; Exit program. A ccontains exit code
mon     equ     $FC10   ; Enter monitor
ldStart equ     $FC12   ; Start loading s-records
ldLine  equ     $FC14   ; Load an s-record line
ldEnd   equ     $FC16   ; End loading s-records

*
* Core functions
*
printf   equ    $FC20   ; Formatted print: TOS=fmt, (varargs)
format   equ    $FC22   ; Format string
memset   equ    $FC24   ; Set memory: TOS=
irand    equ    $FC26   ;
imin     equ    $FC28   ;
imax     equ    $FC2A   ;
initargs equ    $FC2C   ;
argint8  equ    $FC2E   ;
argint16 equ    $FC30   ;

*
* Helper functions
*
switch1  equ    $FC40   ; TOS -> N, Table, Value
switch2  equ    $FC42   ; Table is a list of N value/addr pairs
                        ; Binary search table looking for value
                        ; when found return addr in X. if not
                        ; found return Table + N * (<1/2> + 2)
idiv8   equ     $FC44
idiv16  equ     $FC46
udiv8   equ     $FC48
udiv16  equ     $FC4a

* Misc equates

newline equ     $0a
                        

