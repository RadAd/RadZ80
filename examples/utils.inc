save macro l
	irp r, <l>
		push r
	endm
endm

restore macro l
	irp r, <l>
		pop r
	endm
endm

stackpop macro reg, size
	LD reg, size
	ADD reg, SP
	LD SP, reg
endm

stackpush macro reg, size
	stackpop reg, -size
endm

; unsigned add 8bit (A) to 16 bit
; https://plutiedev.com/z80-add-8bit-to-16bit
; HL += A
uadd8 macro rh, rl
	add   a, rl    ; A = A+L
	ld    rl, a    ; L = A+L
	adc   a, rh    ; A = A+L+H+carry
	sub   rl       ; A = H+carry
	ld    rh, a    ; H = H+carry
endm

; Uses
; BC
add8to16q macro r16, r8
	LD B, 0
	LD C, r8
	ADD r16, BC
endm

; Saves
; BC
add8to16 macro r16, r8
	PUSH BC
	add8to16q r16, r8
	POP BC
endm
