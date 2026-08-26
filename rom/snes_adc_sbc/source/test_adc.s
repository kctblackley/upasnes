.include "shell.inc"
.include "crc_fast.s"

zp_byte acc
zp_byte operand
zp_byte flags

main:
	print_str "Takes 15 seconds",newline
	
	jsr init_crc_fast
	
	; Test ADC/SBC with various initial flags
	
	; Binary mode with carry clear/set
	
	set_test 2,"Binary"
	;     NVMXDIZC
	lda #%00100100
	jsr test
	check_crc $6A50FDAF
	
	set_test 3,"Binary with carry"
	;     NVMXDIZC
	lda #%00100101
	jsr test
	check_crc $A9D324A8
	
	set_test 4,"Decimal"
	; Decimal mode with carry clear/set
	;     NVMXDIZC
	lda #%00101100
	jsr test
	check_crc $C2113611
	
	set_test 5,"Decimal with carry"
	;     NVMXDIZC
	lda #%00101101
	jsr test
	check_crc $696DAA61
	
	; All other flags set, to be sure
	; they don't affect result
	
	set_test 6,"Decimal with other flags"
	;     NVMXDIZC
	lda #%11101110
	jsr test
	check_crc $C2113611
	
	jmp tests_passed

test:
	sta flags
	stz acc
	stz operand
	
@loop:  ; Set up P and A
	lda flags
	pha
	lda acc
	plp
	
	; Execute instr
	adc operand
	
	; Checksum resulting A and P
	php
	cld
	update_crc_fast
	
	pla
	update_crc_fast
	
	inc operand
	bne @loop
	
	inc acc
	bne @loop
	
	rts
