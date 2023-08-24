	.ORG 	0100h

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
;
	OUT	(1),A		; Output char to terminal
	CALL printchar
	INC	HL 	   	; Next char
	JP	printline	; Loop

;
; ---------------------------------
; Routine to print out a char in (a) to terminal
; --------------------------------
printchar:
	PUSH bc
	PUSH ix
	PUSH a
	LD ix, 0
	LD bc, 40
	LD a, (curs_y)
	CP a
printchar_2:
	JR Z, printchar_1
	ADD ix, bc
	DEC a
	JP printchar_2
printchar_1:
	POP a
	LD bc, (curs_x)
	ADD ix, bc
	INC bc
	LD (curs_x), bc
	LD bc, $8000
	ADD ix, bc
	LD (ix), A
	; TODO if curs_x >= 40 curs_x = 0; ++cursy
	POP ix
	POP bc
	RET

;-------------------
; Data
; ------------------

; 13 is newline character we mark end of line
; with $ character

msg:	.DB	"Hello World", 13, 10, 13, 10, 0

curs_x:	.DB 0
curs_y:	.DB 0

	.END
