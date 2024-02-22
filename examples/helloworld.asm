; TODO Use https://github.com/gp48k/zmac with --dotfiles extension

	.ORG 	0100h

stackpop macro size
	LD IX, size
	ADD IX, SP
	LD SP, IX
endm

stackpush macro size
	stackpop -size
endm

	LD HL, msg
	CALL printline

test:
	stackpush 10

	LD A, (IX + 0)
	LD A, (IX + 1)

	stackpop 10

loop:
	CALL editline
	CALL newline
	JP loop

    LD BC, 10
    LD DE, 40
    CALL multiply
	LD HL, msg		; Address of line in HL
	CALL printline	; Print hello world
;
	HALT			; Stop the program
;
; ---------------------------------
; Print out a line in (hl)
; --------------------------------
printline:
	LD A, (HL)		; Get char to print
	CP 0			; Check terminator
	RET Z
	CALL printchar
	INC	HL			; Next char
	JP	printline	; Loop

SCREEN_REF		equ		$8000
SCREEN_WIDTH	equ		80
SCREEN_HEIGHT	equ		25
SCREEN_CURSOR_X	equ		$F101
SCREEN_CURSOR_Y	equ		$F102

KEYB_READ		equ		$FF20
KEYB_WRITE		equ		$FF21
KEYB_BUFFER		equ		$FF00

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

; ---------------------------------
; Return
;  B - length
editline:
	LD B, 0
.next:
	CALL readchar
	CMP 13	; return
	RET Z
	CMP 8	; backspace
	JZ .backspace
	INC B
	PUSH B
	CALL printchar
	POP B
	JMP .next
.backspace:
	LD A, B
	CP 0
	JZ .next
	DEC B
	PUSH B
	CALL backspace
	POP B
	JMP .next


; ---------------------------------
backspace:
; Uses HL
; Uses BC, DE
	CALL dec_cursor
	CALL get_cursor_ref
	LD (HL), 32
	RET

; ---------------------------------

newline:
; Uses A
	LD A, (SCREEN_CURSOR_Y)
	INC A
	LD (SCREEN_CURSOR_Y), A
	LD A, 0
	LD (SCREEN_CURSOR_X), A
	RET

; ---------------------------------
; Waits until character is ready
; Return
; A - character read
; Uses
; HL
; Saves
; BC
readchar:
	LD A, (KEYB_READ)
.loop:
	LD HL, KEYB_WRITE
	CP (HL)
	JZ .loop
	LD HL, KEYB_BUFFER
	add8to16 HL, A
	INC A
	AND A, $0F
	LD (KEYB_READ), A
	LD A, (HL)
	RET

;
; ---------------------------------
; Routine to print out a char in (a) to terminal
; --------------------------------
; Uses BC, DE
; Saves HL
printchar:
	CMP 13	; return
	JZ newline
	PUSH HL
	CALL get_cursor_ref
	LD (HL), A
	CALL inc_cursor
	POP HL
	RET

; Load HL with cursor pos as a memory location
; HL = SCREEN_REF + SCREEN_CURSOR_Y * SCREEN_WIDTH + SCREEN_CURSOR_X
; Uses BC, DE
; return HL
get_cursor_ref:
	PUSH A
	LD A, (SCREEN_CURSOR_Y)
	LD B, 0
	LD C, A
	LD DE, SCREEN_WIDTH
	CALL multiply
	LD A, (SCREEN_CURSOR_X)
	LD B, 0
	LD C, A
	ADD HL, BC
	LD BC, SCREEN_REF
	ADD HL, BC
	POP A
	RET

inc_cursor:
; uses A
; ++SCREEN_CURSOR_X
; if (SCREEN_CURSOR_X == SCREEN_WIDTH)
; {
;   ++SCREEN_CURSOR_Y
;   SCREEN_CURSOR_X = 0
; }
	LD A, (SCREEN_CURSOR_X)
	INC A
	CP SCREEN_WIDTH
	JNZ .store
	LD A, (SCREEN_CURSOR_Y)
	INC A
	LD (SCREEN_CURSOR_Y), A
	LD A, 0
.store:
	LD (SCREEN_CURSOR_X), A
	RET

dec_cursor:
; uses A
; if (SCREEN_CURSOR_X == 0)
; {
;   --SCREEN_CURSOR_Y
;   SCREEN_CURSOR_X = SCREEN_WIDTH
; }
; --SCREEN_CURSOR_X
	LD A, (SCREEN_CURSOR_X)
	CP 0
	JNZ .store
	LD A, (SCREEN_CURSOR_Y)
	DEC A
	LD (SCREEN_CURSOR_Y), A
	LD A, SCREEN_WIDTH
.store:
	DEC A
	LD (SCREEN_CURSOR_X), A
	RET

; -------------------
; Multiply HL = BC * DE
; Uses HL, BC
; Overflow in carry flag
; -------------------
multiply:
	LD	hl, 0			; clear the result register
.loop:
	LD	a, b			; is BC == 0?
	OR	c				;  (also resets carry flag)
	RET z				; then we're done!
	SRL	b				; logical right shift of BC
	RR	c				; bit 0 goes to carry flag
	JR	nc, .zerobit	; unless bit 0 was 0
	ADD	hl, de			; add multiplier to result
	RET	c				; return on overflow
.zerobit:
	SLA	e				; shift multiplier to the left
	RL	d				; topmost bit goes to carry flag
	RET	c				; return on overflow
	JR	.loop			; next iteration

; -------------------
; Data
; ------------------

; 13 is newline character we mark end of line
; with $ character

msg:	.DB	"Hello World", 13, 10, 13, 10, 0

	.END
