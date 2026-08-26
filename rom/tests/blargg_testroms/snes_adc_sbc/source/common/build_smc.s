.define ROSEG "RODATA"
.define CODESEG "CODE1"

reloc_addr = $0800

.segment "HEADER"
	.res $15,0
	.byte $31 ; 
	.byte 0,0,0,0,0,0
	.word $AAAA,$5555 ; checksum

.segment "VECTORS"
	.res $1C
	;.word (reset-reloc_addr)+$8000
	.word reset

.segment "CODE1"

.res $8000,0

.ifndef NEED_CONSOLE
	NEED_CONSOLE = 1
.endif

.include "shell.s"

std_reset:
.if 0
	init_cpu_regs
	; Copy code
	ldx #$1000 - 1
:       lda $8000,x
	sta reloc_addr,x
	dex
	bpl :-
	
	jmp next        ; magic
next:
.endif
	jmp run_shell

init_runtime:
	rts


post_exit:
	jsr clear_nvram
	bra forever
