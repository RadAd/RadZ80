; TODO Use https://github.com/gp48k/zmac with --dotfiles extension

	.ORG 	0100h

    LD BC,10
    LD DE,40
    CALL multiply
	LD HL,msg		; Address of line in HL
	CALL printline	; Print hello world
;
	HALT			; Stop the program
;
; ---------------------------------
; Print out a line in (hl)
; --------------------------------
printline:
	LD A,(HL)		; Get char to print
	CP 0			; Check terminator
	RET Z
	CALL printchar
	INC	HL			; Next char
	JP	printline	; Loop

SCREEN_REF		equ		$8000
SCREEN_WIDTH	equ		40
SCREEN_HIEGHT	equ		25

;
; ---------------------------------
; Routine to print out a char in (a) to terminal
; --------------------------------
printchar:
	PUSH HL
	CALL get_cursor_ref
	LD (HL), A
	CALL inc_cursor
	POP HL
	RET

; Load HL with cursor pos as a memory location
; Uses BC, DE
; return HL
get_cursor_ref:
	PUSH A
	LD A, (curs_y)
	LD B, 0
	LD C, A
	LD DE, SCREEN_WIDTH
	CALL multiply
	LD A, (curs_x)
	LD B, 0
	LD C, A
	ADD HL, BC
	LD BC, SCREEN_REF
	ADD HL, BC
	POP A
	RET

inc_cursor:
; uses A
	LD A, (curs_x)
	INC A
	CP SCREEN_WIDTH
	JNZ .store
	LD A, (curs_y)
	INC A
	LD (curs_y), A
	LD A, 0
.store:
	LD (curs_x), A
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

curs_x:	.DB 0
curs_y:	.DB 0

	.END
