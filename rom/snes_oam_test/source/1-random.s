; Verifies basic writing and reading, and
; mirroring of high table

.include "shell.inc"

main:
	mov INIDISP,#$80
	
	jsr fill_oam
	jsr crc_oam
	check_crc $A4BEB060
	jmp tests_passed

fill_oam:
	jsr reset_crc
	movw OAMADDL,#0
	ldx #0
:       lda #$55
	jsr update_crc
	lda checksum
	sta OAMDATA
	lda checksum+1
	sta OAMDATA
	inx
	cpx #512
	bne :-
	rts

crc_oam:
	jsr reset_crc
	movw OAMADDL,#0
	ldx #0
:       lda OAMDATAREAD
	jsr update_crc
	lda OAMDATAREAD
	jsr update_crc
	inx
	cpx #512
	bne :-
	rts
