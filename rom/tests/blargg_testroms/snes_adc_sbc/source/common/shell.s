; Common routines and runtime

; Detect inclusion loops (otherwise ca65 goes crazy)
.ifdef SHELL_INCLUDED
	.error "shell.s included twice"
	.end
.endif
SHELL_INCLUDED = 1

;**** Special globals ****

; Temporary variables that ANY routine might modify, so
; only use them between routine calls.
temp   = <$A
temp2  = <$B
temp3  = <$C
ptr    = <$D

.segment "NVRAM"
	; Beginning of variables not cleared at startup
	nvram_begin:

.segment CODESEG

nop ; first byte gets corrupt in SRAM
.a8
.i16

;**** Common routines ****

.include "macros.inc"
.include "sneshw.inc"
.include "print.s"
.include "delay.s"
.include "crc.s"
.include "testing.s"

.ifdef NEED_CONSOLE
	.include "console.s"
.else
	; Stubs so code doesn't have to care whether
	; console exists
	console_init:
	console_show:
	console_hide:
	console_print:
	console_flush:
		rts
.endif

.ifndef CUSTOM_PRINT
	print_char_:
		jmp console_print
.endif

;**** Shell core ****

.ifndef CUSTOM_RESET
	reset:
		sei
		jmp std_reset
.endif

; Sets up hardware then runs main
run_shell:
	init_cpu_regs
	jsr init_shell
	set_test $FF
	jmp run_main


; Initializes shell
init_shell:
	jsr clear_ram
	jsr init_text_out
	jsr init_testing
	jsr init_runtime
	jsr console_init
	rts


; Runs main in consistent PPU/APU environment, then exits
; with code 0
run_main:
	jsr pre_main
	jsr main
	a8
	lda #0
	jmp exit


; Sets up environment for main to run in
pre_main:
	lda #$24
	pha
	lda #0
	ldx #0
	ldy #0
	plp
	rts


.ifndef CUSTOM_EXIT
	exit:
.endif

; Reports result and ends program
std_exit:
	init_cpu_regs
	jsr report_result
	jmp post_exit


; Reports final result code in A
; Preserved: A
report_result:
	pha
	jsr :+
	jsr play_byte
	pla
	jmp set_final_result

:       jsr print_newline
	jsr console_show
	
	; 0: ""
	cmp #1
	bge :+
	rts
:
	; 1: "Failed"
	bne :+
	print_str {"Failed",newline}
	rts
	
	; n: "Failed #n"
:       print_str "Failed #"
	jsr print_dec
	jsr print_newline
	rts


;**** Other routines ****

; Reports internal error and exits program
internal_error:
	init_cpu_regs
	print_str newline,"Internal error"
	lda #255
	jmp exit


.import __NVRAM_LOAD__, __NVRAM_SIZE__

; Clears $0-$FF and nv_ram_end-$7FF
clear_ram:
	; Main pages
	ldx #$FF
:       stz 0,x
	dex
	bpl :-
	
	; BSS except nvram
	ldx #$7FF
:       stz 0,x
	dex
	cpx #__NVRAM_LOAD__+__NVRAM_SIZE__
	bge :-
	
	rts


; Clears nvram
clear_nvram:
	ldx #__NVRAM_SIZE__
	beq @empty
:       dex
	stz __NVRAM_LOAD__,x
	bne :-
@empty:
	rts


; Prints filename and newline, if available, otherwise nothing.
; Preserved: A, X, Y
print_filename:
	.ifdef FILENAME_KNOWN
		phx
		jsr print_newline
		ldx #filename
		jsr print_str
		jsr print_newline
		plx
	.endif
	rts
	
.pushseg
.segment ROSEG
	; Filename terminated with zero byte.
	filename:
		.ifdef FILENAME_KNOWN
			.incbin "ram:rom.snes"
		.endif
		.byte 0
.popseg


play_byte:
	rts


; Disables interrupts and loops forever. Exits back to
; loader on devcart.
.ifndef CUSTOM_FOREVER
forever:
	; Disable IRQ and NMI
	sei
	setb NMITIMEN,$00
:       bra :-
.endif


; TODO: implement
init_text_out:
set_final_result:
	rts

