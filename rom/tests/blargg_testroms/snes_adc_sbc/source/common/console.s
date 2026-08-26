; Scrolling text console with line wrapping, 30x30 characters.
; Buffers lines for speed. Will work even if PPU doesn't
; support scrolling.

; Number of characters of margin on left and right, to avoid
; text getting cut off by common TVs
console_margin = 1

console_buf_size = 32
console_width = console_buf_size - (console_margin*2)

; Which background to use (0-3), and where to put nametable
bgsel = 1 ; 0 is burned out on one of my SNES consoles :(
nametable = $0800

zp_res  console_pos,2
zp_res  console_scroll
bss_res console_buf,console_buf_size


; Waits for beginning of VBL
; Preserved: A, X, Y
console_wait_vbl:
:       bit HVBJOY
	bmi :-
:       bit HVBJOY
	bpl :-
	rts


; Initializes console and clears screen
console_init:
	jsr console_hide
	jsr load_chr
	
	; Clear screen
	ldy #' '
	ldx #nametable
	stx VMADDL
	ldx #$400
:       sty VMDATAL
	dex
	bne :-
	
	lda #0
	sta console_scroll
	jsr console_scroll_up_
	; FALL THROUGH
	
; Shows console
; Preserved: A, X, Y
console_show:
	pha
	jsr console_wait_vbl
	lda #$0F        ; screen on
	sta INIDISP
	lda #>nametable
	sta BG1SC+bgsel
	lda #$01<<bgsel
	sta TM
	jmp console_apply_scroll_


; Hides console
; Preserved: A, X, Y
console_hide:
	jsr console_wait_vbl
	stz INIDISP     ; screen off
	rts


; Prints character to console
; Preserved: A, X, Y
console_print:
	cmp #10
	beq console_newline
	
	; Write to buffer
	phx
	ldx console_pos
	sta console_buf+console_margin,x
	plx
	dec console_pos
	bmi console_newline     ; reached end of line
	
	rts

; Prints new line
; Preserved: A, X, Y
console_newline:
	pha
	jsr console_wait_vbl
	jsr console_flush_
	jsr console_scroll_up_
	jsr console_flush_
	jmp console_apply_scroll_


console_scroll_up_:
	; Scroll up 8 pixels
	lda console_scroll
	clc
	adc #8
	sta console_scroll
	
	phx
	
	; Start new clear line
	lda #' '
	ldx #console_buf_size-1
:       sta console_buf,x
	dex
	bpl :-
	ldx #console_width-1
	stx console_pos
	
	plx
	rts


; Displays current line's contents without scrolling.
; Preserved: A, X, Y
console_flush:
	pha
	jsr console_wait_vbl
	jsr console_flush_
console_apply_scroll_:
	lda console_scroll
	clc
	adc #39
	sta BG1VOFS+bgsel*2
	stz BG1VOFS+bgsel*2
	
	pla
	rts

console_flush_:
	phx
	
	; Address line in nametable
	lda console_scroll
	a16
	and #$00FF
	asl a
	asl a
	ora #nametable
	sta VMADDL
	a8
	
	; Copy line
	ldx #console_buf_size-1
:       lda console_buf,x
	sta VMDATAL
	lda #$10
	sta VMDATAH
	dex
	bpl :-
	
	plx
	rts

load_chr:
	lda #$80        ; display off
	sta INIDISP
	
	; Init PPU regs
	ldx #INIDISP+1
:       stz 0,x ; write twice
	stz 0,x
	inx
	cpx #$21FF
	bne :-
	
	; Copy palette
	stz CGADD
	ldy #$20
:       ldx #0
:       lda @palette,x
	sta CGDATA
	inx
	cpx #8
	bne :-
	dey
	bne :--
	
	; Load tiles
	lda #$80        ; word video writes
	sta VMAIN
	ldx #$0000
	stx VMADDL
	ldx #0
	ldy #128
:       phy
	ldy #8
:       lda @charset,x
	sta VMDATAL
	sta VMDATAH
	inx
	dey
	bne :-
	ply
	dey
	bne :--
	
	rts

@palette:
	.word 0,$FFFF,$FFFF,$FFFF

@charset:
	.incbin "ascii.chr"
