*  calling monitor c function
*  display text on terminal using UART

putchar    equ $dfdf 
puts       equ $e041
initacia   equ $df73  
newline    equ $e0eb
	
	org $200

main    jsr initacia

loop
        ldd #text3
        pshs d

	jsr puts
	
	leas 2,s

	jsr newline

        bra loop
	
text3   fcc "Hello from 6809 kit"
        fcb 0

	end

