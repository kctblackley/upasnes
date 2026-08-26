; Prints values in various ways to output,
; including numbers and strings.

newline = 10

zp_res  print_temp_,2


; The following print_r routines print the
; values of the register, in hex, using the
; current width of that register (8 or 16 bits).
; The checksum is also updated, MSB first when
; printing 16-bit values.
;
; Unlike almost all other routines, A, X, and Y
; can be any width and these will work correctly.
; Preserved: A, X, Y, flags

print_x:
	php
	stx print_temp_
	a8
	pha
	lda #$10
	bra print_reg_

print_y:
	php
	sty print_temp_
	a8
	pha
	lda #$10
	bra print_reg_

print_a:
	php
	sta print_temp_
	a8
	pha
	lda #$20
print_reg_:
	i16
	and 2,s
	bne :+
	lda print_temp_+1
	jsr print_hex
:       lda print_temp_
print_a8_restore_:
	jsr print_a8_
	pla
	plp
	rts

; Prints flags
print_p:
	php
	a8
	i16
	pha
	lda 2,s
	bra print_a8_restore_

; Prints 16-bit stack pointer
print_s:
	php
	i16
	phx
	tsx
	inx
	inx
	inx
	inx
	inx
	jsr print_x
	plx
	plp
	rts

print_a8_:
	jsr print_hex
	lda #' '
	jmp print_char_no_crc


; Prints A as two hex characters, NO space after
; Preserved: A, X, Y
print_hex:
	jsr update_crc
	
	pha
	lsr a
	lsr a
	lsr a
	lsr a
	jsr @nibble
	pla
	
	pha
	and #$0F
	jsr @nibble
	pla
	rts
	
@nibble:
	cmp #10
	blt @digit
	adc #6;+1 since carry is set
@digit: adc #'0'
	jmp print_char_


; Prints character and updates checksum UNLESS
; it's a newline.
; Preserved: A, X, Y
print_char:
	cmp #newline
	beq print_char_no_crc
	jsr update_crc
print_char_no_crc:
	pha
	jsr print_char_
	pla
	rts


; Prints space. Doesn't affect checksum.
; Preserved: A, X, Y
print_space:
	pha
	lda #' '
	jsr print_char_
	pla
	rts


; Advances to next line. Doesn't affect checksum.
; Preserved: A, X, Y
print_newline:
	pha
	lda #newline
	jsr print_char_
	pla
	rts


; Prints zero-terminated string from X.
; On return, X points to zero byte.
; Preserved: A, Y
print_str:
	pha
	bra :+
@loop:  jsr print_char
	inx
:       lda 0,x
	bne @loop
	pla
	rts


; Prints string
; Preserved: A, X, Y
.macro print_str str,str2,str3
	.local Str
	phx
	ldx #Str
	jsr print_str
	plx
	.ifnblank str2
		seg_data ROSEG,{Str: .byte str,str2,0}
	.else
		seg_data ROSEG,{Str: .byte str,0}
	.endif
.endmacro


; Prints A as 1-5 digit decimal value. OK if A is
; 16 bits wide.
; Preserved: A, X, Y
print_dec:
	php
	phx
	ldx #0
	stx print_temp_
	sta print_temp_
	a8
	ldx print_temp_
	jsr print_x_dec
	plx
	plp
	rts


; Prints X as 1-5 digit decimal value
; Preserved: A, X, Y
print_x_dec:
	pha
	phx
	phy
	
	a16
	txa
	ldx #6
	
	; Remove leading zeroes
:       dex
	dex
	bmi @ones
	cmp @places,x
	blt :-
	
@digit: ; Determine digit
	ldy #-1
	sec
:       iny
	sbc @places,x
	bcs :-
	adc @places,x
	
	; Print digit
	pha
	tya
	jsr @print
	a16
	pla
	
	; Next place
	dex
	dex
	bpl @digit
	
@ones:  jsr @print
	
	ply
	plx
	pla
	rts
	
@print: a8
	ora #'0'
	jmp print_char

@places:
	.word 10,100,1000,10000
