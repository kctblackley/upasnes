.include "shell.inc"

.macro check_oam Addr, Correct
	.local Table
	.local Size
	ldx #Addr
	ldy #Table
	lda #Size
	jsr check_oam_
	.pushseg
	.rodata
Table:  .byte Correct
Size = * - Table
	.popseg
.endmacro

clear_oam:
	movw OAMADDL,#0
	ldx #544
:       stz OAMDATA
	dex
	bne :-
	movw OAMADDL,#0
	rts

fill_oam:
	movw OAMADDL,#0
	ldx #0
:       txa
	sta OAMDATA
	inx
	cpx #544
	bne :-
	stz OAMDATA ; clear buffer
	movw OAMADDL,#0
	rts

zp_res buffer,16
zp_res table_addr,2
zp_res table_size,2

check_oam_:
	sty table_addr
	sta table_size
	
	; Save OAM
	stx OAMADDL
	ldy #0
:       lda OAMDATAREAD
	sta buffer,y
	iny
	cpy table_size
	bne :-
	
	; Verify
	ldy #0
:       lda buffer,y
	cmp (table_addr),y
	bne @different
	iny
	cpy table_size
	bne :-
	
	rts

@different:
	print_str "Wrong: "
	ldy #0
:       lda buffer,y
	jsr print_a
	iny
	cpy table_size
	bne :-

	print_str newline,"Right: "
	ldy #0
:       lda (table_addr),y
	jsr print_a
	iny
	cpy table_size
	bne :-

	jmp test_failed
