*  simple test file

	org $200

loop
        ldd #text3
        std store
        bra loop
	
text3   fcc "Hello\n"
store	rmb	2

	end

