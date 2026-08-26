.include "shell.inc"
.include "crc_fast.s"

zp_res acc,2
zp_res operand,2
zp_res flags,2

main:
	print_str "Takes 10 seconds",newline
	
	jsr init_crc_fast
	
	; Test ADC/SBC with various initial flags
	
	; Binary mode with carry clear/set
	
	set_test 2,"Binary"
	;     NVMXDIZC
	lda #%00000100
	jsr test
	check_crc $7F629F3B
	
	set_test 3,"Binary with carry"
	;     NVMXDIZC
	lda #%00000101
	jsr test
	check_crc $7F0C621A
	
	set_test 4,"Decimal"
	; Decimal mode with carry clear/set
	;     NVMXDIZC
	lda #%00001100
	jsr test
	check_crc $827FFA3A
	
	set_test 5,"Decimal with carry"
	;     NVMXDIZC
	lda #%00001101
	jsr test
	check_crc $888BB95C
	
	; All other flags set, to be sure
	; they don't affect result
	
	set_test 6,"Decimal with other flags"
	;     NVMXDIZC
	lda #%11001110
	jsr test
	check_crc $827FFA3A
	
	jmp tests_passed

test:
	sta flags
	sta flags+1
	a16
	ldx #values_end - values - 2
	stx acc
	
@acc_loop:
	ldx #values_end - values - 2
	stx operand
	
@loop:
	; Set up P and A
	lda flags
	pha
	ldx acc
	lda values,x
	ldx operand
	plp
	plp
	
	; Execute instr
	sbc values,x
	
	; Checksum resulting A and P
	php
	php
	cld
	tay
	update_crc_fast16
	tya
	xba
	update_crc_fast16
	
	pla
	update_crc_fast16
	
	dec operand
	dec operand
	bpl @loop
	
	dec acc
	dec acc
	bpl @acc_loop
	
	a8
	rts

values:
	.word $0000
	.word $0001,$0009,$000A,$000F,$0099,$009A,$009F,$00A9,$00AA,$00AF,$00F9,$00FA,$00FF
	.word  $0010,$0090,$00A0,$00F0,$0990,$09A0,$09F0,$0A90,$0AA0,$0AF0,$0F90,$0FA0,$0FF0
	.word   $0100,$0900,$0A00,$0F00,$9900,$9A00,$9F00,$A900,$AA00,$AF00,$F900,$FA00,$FF00
	.word    $1000,$9000,$A000,$F000

	.word $FFFF
	.word $FFFE,$FFF6,$FFF5,$FFF0,$FF66,$FF65,$FF60,$FF56,$FF55,$FF50,$FF06,$FF05,$FF00
	.word  $FFEF,$FF6F,$FF5F,$FF0F,$F66F,$F65F,$F60F,$F56F,$F55F,$F50F,$F06F,$F05F,$F00F
	.word   $FEFF,$F6FF,$F5FF,$F0FF,$66FF,$65FF,$60FF,$56FF,$55FF,$50FF,$06FF,$05FF,$00FF
	.word    $EFFF,$6FFF,$5FFF,$0FFF

	.word $0000
	.word $FFFF,$FFF7,$FFF6,$FFF1,$FF67,$FF66,$FF61,$FF57,$FF56,$FF51,$FF07,$FF06,$FF01
	.word  $FFF0,$FF70,$FF60,$FF10,$F670,$F660,$F610,$F570,$F560,$F510,$F070,$F060,$F010
	.word   $FF00,$F700,$F600,$F100,$6700,$6600,$6100,$5700,$5600,$5100,$0700,$0600,$0100
	.word    $F000,$7000,$6000,$1000
values_end:
