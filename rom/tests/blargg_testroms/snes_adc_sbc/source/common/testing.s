; Utilities for writing test ROMs

; In NVRAM so these can be used before initializing runtime,
; then runtime initialized without clearing them
nv_res test_code ; code of current test
nv_res test_name,2 ; address of name of current test, or 0 of none


; Sets current test code and optional name. Also resets
; checksum.
; Preserved: A, X, Y
.macro set_test code,name
	pha
	lda #code
	jsr set_test_
	.ifblank name
		setb test_name+1,0
	.else
		.local Addr
		setw test_name,Addr
		seg_data ROSEG,{Addr: .byte name,0}
	.endif
	pla
.endmacro

set_test_:
	sta test_code
	jmp reset_crc


; Initializes testing module
init_testing:
	jmp init_crc


; Reports that all tests passed
tests_passed:
	init_cpu_regs
	
.ifndef BUILD_MULTI
	jsr print_filename
	print_str newline,"Passed"
.endif
	lda #0
	jmp exit


; Reports "Done" if set_test has never been used,
; "Passed" if set_test 0 was last used, or
; failure if set_test n was last used.
tests_done:
	init_cpu_regs
	lda test_code
	jeq tests_passed
	inc
	bne test_failed
.ifndef BUILD_MULTI
	jsr print_filename
	print_str newline,"Done"
.endif
	lda #0
	jmp exit


; Reports that the current test failed
test_failed:
	init_cpu_regs
	
	lda test_code
	
	; Treat $FF as 1, in case it wasn't ever set
	inc
	bne :+
	inc
	sta test_code
:       
	; If code >= 2, print name
	cmp #2-1        ; -1 due to inc above
	blt :+
	ldx test_name
	beq :+
	jsr print_newline
	jsr print_str
	jsr print_newline
:       
.ifndef BUILD_MULTI
	jsr print_filename
.endif
	; End program
	lda test_code
	jmp exit


; If checksum doesn't match expected, reports failed test.
; Clears checksum afterwards.
; Preserved: A, X, Y
.macro check_crc expected
	jsr_with_addr check_crc_,{.dword expected}
.endmacro

check_crc_:
	pha
	phy
	
	; Compare with complemented checksum
	ldy #3
:       lda (ptr),y
	sec
	adc checksum,y
	bne @wrong
	dey
	bpl :-
	
	jsr reset_crc
	ply
	pla
	rts
	
@wrong: jsr print_newline
	jsr print_crc
	jsr print_newline
	jmp test_failed
