; jxx equivalents to bxx
.macpack longbranch
.define jge jcs
.define jlt jcc

.define wdm .byte $42,$EA

; Puts data in another segment
.macro seg_data seg,data
	.pushseg
	.segment seg
	data
	.popseg
.endmacro

; Reserves size bytes in zeropage/bss for name.
; If size is omitted, reserves one byte.
.macro zp_res name,size
	.ifblank size
		zp_res name,1
	.else
		seg_data "ZEROPAGE",{name: .res size}
	.endif
.endmacro

.macro bss_res name,size
	.ifblank size
		bss_res name,1
	.else
		seg_data "BSS",{name: .res size}
	.endif
.endmacro

.macro nv_res name,size
	.ifblank size
		nv_res name,1
	.else
		seg_data "NVRAM",{name: .res size}
	.endif
.endmacro

; Reserves one byte in zeropage for name (very common)
.macro zp_byte name
	seg_data "ZEROPAGE",{name: .res 1}
.endmacro

; Passes constant data to routine in addr
; Preserved: A, X, Y
.macro jsr_with_addr routine,data
	.local Addr
	phx
	ldx #Addr
	stx ptr
	plx
	jsr routine
	seg_data ROSEG,{Addr: data}
.endmacro

; Calls routine multiple times, with A having the
; value 'start' the first time, 'start+step' the
; second time, up to 'end' for the last time.
.macro for_loop routine,start,end,step
	a16
	lda #(start)-(step)
:   clc
	adc #step
	pha
	a8
	jsr routine
	a16
	pla
	cmp #end
	bne :-
	a8
.endmacro

; Calls routine n times. The value of A in the routine
; counts from 0 to n-1.
.macro loop_n_times routine,n
	for_loop routine,0,n-1,+1
.endmacro

; Makes A 16 bits
.MACRO a16
	.A16
	rep #$20
.ENDMACRO

; Makes A 8 bits
.MACRO a8
	.A8
	sep #$20
.ENDMACRO

; Makes X and Y 16 bits
.MACRO i16
	.I16
	rep #$10
.ENDMACRO

; Makes X and Y 8 bits
.MACRO i8
	.I8
	sep #$10
.ENDMACRO

; Makes A, X, and Y 16 bits
.MACRO ai16
	.A16
	.I16
	rep #$30
.ENDMACRO

; Makes A, X, and Y 8 bits
.MACRO ai8
	.A8
	.I8
	sep #$30
.ENDMACRO

; Copies byte from in to out
; Preserved: X, Y
.macro mov out, in
	lda in
	sta out
.endmacro

; Stores byte at addr
; Preserved: X, Y
.macro setb addr, byte
	lda #byte
	sta addr
.endmacro

; Stores word at addr
; Preserved: X, Y
.macro setw addr, word
	lda #<(word)
	sta addr
	lda #>(word)
	sta addr+1
.endmacro

; Initializes CPU registers to reasonable values
.macro init_cpu_regs
	sei
	cld
	clc
	xce
	a8
	i16
	ldx #$1FFF
	txs
	pea 0
	pld
	pea 0
	plb
	plb
.endmacro

