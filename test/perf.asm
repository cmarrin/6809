*  simple test file

    include BOSS9.inc

NumPrints equ 50000

	org $200

main    ldd #NumPrints
        
loop
      ;  ldx #text3
      ;  jsr puts
      ;  lda #newline
      ;  jsr putc
        
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        subd #1
        bra loop
        clra
        jsr exit
	
text3   fcc "Hello"
store	rmb	2

	end $200

