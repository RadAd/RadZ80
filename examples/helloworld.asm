; TODO Use https://github.com/gp48k/zmac with --dotfiles extension

	.ORG 	0100h

.include "utils.inc"

	LD HL, msg
	CALL printline

test:
	stackpush IX, 10

	LD A, (IX + 0)
	LD A, (IX + 1)
	INC (IX + 0)

	stackpop IX, 10

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
SCREEN_REG		equ		$F100
OFFSET_CURSOR_X	equ		$01
OFFSET_CURSOR_Y	equ		$02
SCREEN_CURSOR_X	equ		SCREEN_REG + OFFSET_CURSOR_X
SCREEN_CURSOR_Y	equ		SCREEN_REG + OFFSET_CURSOR_Y


KEYB_READ		equ		$FF20
KEYB_WRITE		equ		$FF21
KEYB_BUFFER		equ		$FF00

; ---------------------------------
; Return
;  A - length
editline:
.stacksize equ 10
.count equ 0
	stackpush IX, .stacksize
	LD (IX[.count]), 0
.next:
	CALL readchar
	CMP 13	; return
	JZ .return
	CMP 8	; backspace
	JZ .backspace
	INC (IX[.count])
	CALL printchar
	JMP .next
.backspace:
	LD A, (IX[.count])
	CP 0
	JZ .next
	DEC (IX[.count])
	CALL backspace
	JMP .next
.return:
	LD A, (IX[.count])
	stackpop IX, .stacksize
	RET

; ---------------------------------
backspace:
; Uses HL
	CALL dec_cursor
	CALL get_cursor_ref
	LD (HL), 32 ; space
	RET

; ---------------------------------

newline:
if 1
	save IX
	LD IX, SCREEN_REG
	INC (IX[OFFSET_CURSOR_Y])
	LD (IX[OFFSET_CURSOR_X]), 0
	restore IX
	RET
else
	PUSH A
	LD A, (SCREEN_CURSOR_Y)
	INC A
	LD (SCREEN_CURSOR_Y), A
	LD A, 0
	LD (SCREEN_CURSOR_X), A
	POP A
	RET
endif

; ---------------------------------
; Waits until character is ready
; Return
; A - character read
readchar:
	PUSH HL
	LD A, (KEYB_READ)
	LD HL, KEYB_WRITE
.wait:	; Wait for key
	CP (HL)
	JZ .wait
	LD HL, KEYB_BUFFER
	add8to16 HL, A
	INC A
	AND A, $0F	; Wrap - buffer is only 15 characters long
	LD (KEYB_READ), A
	LD A, (HL)
	POP HL
	RET

;
; ---------------------------------
; Routine to print out a char in (a) to terminal
; --------------------------------
printchar:
	CMP 13	; return
	JZ newline
	PUSH HL
	CALL get_cursor_ref
	LD (HL), A
	CALL inc_cursor
	POP HL
	RET

; load a 8bit value into a 16bit register
ld8 macro rh, rl, s
	LD rh, 0
	LD rl, s
endm

; Load HL with cursor pos as a memory location
; HL = SCREEN_REF + SCREEN_CURSOR_Y * SCREEN_WIDTH + SCREEN_CURSOR_X
; return HL
get_cursor_ref:
	save <A, BC, DE, IX>
	LD IX, SCREEN_REG
	ld8 B, C, (IX[OFFSET_CURSOR_Y])
	LD DE, SCREEN_WIDTH
	CALL multiply	; HL = SCREEN_WIDTH * IX[OFFSET_CURSOR_Y]
	ld8 B, C, (IX[OFFSET_CURSOR_X])
	ADD HL, BC		; HL += IX[OFFSET_CURSOR_X]
	LD BC, SCREEN_REF
	ADD HL, BC		; HL += SCREEN_REF
	restore <IX, DE, BC, A>
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
if 1
	save IX
	LD IX, SCREEN_REG
	LD A, 0
	CP (IX[OFFSET_CURSOR_X])
	JNZ .store
	DEC (IX[OFFSET_CURSOR_Y])
	LD (IX[OFFSET_CURSOR_X]), SCREEN_WIDTH
.store:
	DEC (IX[OFFSET_CURSOR_X])
	restore IX
	RET
else
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
endif

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
