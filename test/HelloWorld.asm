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
        
done    bra done
	
text3   fcc "Hello from 6809 kit"
        fcb 0

count   rmb 1

	end main

