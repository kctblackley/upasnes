; Fast table-based CRC-32

; Uses 1K of RAM
checksum_t0 = $7EFB00
checksum_t2 = checksum_t0+$200

; Initializes fast CRC tables and resets checksum.
init_crc_fast:
	ldx #255*2
	ldy #255
:       ; Calculate CRC for this byte
	stz checksum+0
	stz checksum+1
	stz checksum+2
	stz checksum+3
	tya
	jsr update_crc_
	
	; Write in table
	a16
	lda checksum
	sta checksum_t0,x
	lda checksum+2
	sta checksum_t2,x
	a8
	
	dex
	dex
	dey
	bpl :-
	
	jmp reset_crc



; Updates checksum with byte from A
; Preserved: X, Y
update_crc_fast:
	phx


; Updates checksum with byte from A
; Preserved: Y
.macro update_crc_fast
	eor checksum
	a16
	and #$00FF
	asl a
	tax
	lda checksum+1
	eor checksum_t0,x
	sta checksum
	lda checksum+3
	eor checksum_t2,x
	sta checksum+2
	a8
.endmacro

	update_crc_fast
	plx
	rts


; Same as update_crc_fast, except it requires
; that A already be set to 16 bits wide.
; The upper 8 bits of A are ignored and can
; contain garbage.
; X must be 16 bits wide.
; Preserved: Y
.macro update_crc_fast16
	eor checksum
	and #$00FF
	asl a
	tax
	lda checksum+1
	eor checksum_t0,x
	sta checksum
	lda checksum+3
	eor checksum_t2,x
	sta checksum+2
.endmacro
