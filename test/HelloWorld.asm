*  calling monitor c function
*  display text on terminal using UART

    include BOSS9.inc
	
	org $200

main    nop

loop
        ldx #text3
        jsr puts
        
        lda #newline
        jsr putc

        bra loop
	
text3   fcc "Hello from 6809 kit"
        fcb 0

	end main

