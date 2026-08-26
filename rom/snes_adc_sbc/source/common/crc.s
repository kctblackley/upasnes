; CRC-32 checksum calculation

; Extra zero byte after 4-byte checksum so update_crc_fast16
; can do LDA checksum+3 when A is 16 bits wide, and have
; high 8 bits zero.
zp_res  checksum,5
zp_byte checksum_off_

; Turns CRC updating on/off. Allows nesting.
; Preserved: A, X, Y
crc_off:
	dec checksum_off_
	rts

crc_on: inc checksum_off_
	beq :+
	jpl internal_error ; catch unbalanced crc calls
:       rts


; Initializes checksum module. Might initialize tables
; in the future.
init_crc:
	bra reset_crc


; Clears checksum and turns it on
; Preserved: X, Y
reset_crc:
	stz checksum_off_
	stz checksum+4
	lda #$FF
	sta checksum
	sta checksum+1
	sta checksum+2
	sta checksum+3
	rts


; Updates checksum with byte in A (unless disabled via crc_off)
; Preserved: A, X, Y
update_crc:
	bit checksum_off_
	bmi update_crc_off_
update_crc_:
	phx
	a16
	pha
	and #$00FF
	eor checksum
	ldx #8
@bit:   lsr checksum+2
	ror a
	bcc :+
	sta checksum
	lda checksum+2
	eor #$EDB8
	sta checksum+2
	lda checksum
	eor #$8320
:       dex
	bne @bit
	sta checksum
	pla
	plx
	a8
update_crc_off_:
	rts


; Prints CRC-32 checksum as 8-character hex value
print_crc:
	jsr crc_off
	
	; Print complement
	ldx #3
:       lda checksum,x
	eor #$FF
	jsr print_hex
	dex
	bpl :-
	
	jmp crc_on
