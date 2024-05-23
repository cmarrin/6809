*  simple test file

	org $200

    include BOSS9.inc

loop
        ldx #text3
        jsr puts
        lda #newline
        jsr putc
        nop
        jsr sub1
        nop
        bra loop
        
sub1    nop
        bsr sub2
        nop
        rts
        
sub2    nop
        nop
        rts
	
text3   fcc "Hello"
store	rmb	2

	end $200

