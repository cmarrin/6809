*  calling monitor c function
*  display text on terminal using UART

    include BOSS9.inc

NumPrints equ 10
        org $200

main    lda #NumPrints
        sta count
loop
        ldx #text3
        jsr puts
        lda #newline
        jsr putc
        dec count
        bgt loop
        clra
        jsr exit
	
text3   fcn "Hello from 6809"

count   rmb 1

	end main

