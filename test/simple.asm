*  simple test file

	org $200

    include BOSS9.inc

loop
        ldx #text3
        jsr puts
        lda #newline
        jsr putc
        nop
        nop
        bra loop
	
text3   fcc "Hello"
store	rmb	2

	end $200

