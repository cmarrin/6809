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

* Misc equates

newline equ     $0a
                        

