;generic functions for menu-driven interface

!screen_data = $7ef800

font8:
  incbin font8.bin
end_font8:

hexmap:
  incbin hexmap.bin
end_hexmap:

setup_display() {
  php : rep #$20 : pha

  sep #$20
  lda #$00 : sta $2105 ;mode0
  lda #$00 : sta $2107 ;bg1 tilemap  [$0000]
  lda #$01 : sta $210b ;bg1 tiledata [$1000]
  lda #$01 : sta $212c ;bg1 enable
  lda #$00 : sta $212d

  rep #$20
  lda #$0000
  ldx.w #2046
- sta !screen_data,x
  dex #2 : bpl -

;add screen border curve
  ldx #$0000 : lda #$1c01 : sta !screen_data,x
  ldx #$003e : lda #$5c01 : sta !screen_data,x
  ldx #$06c0 : lda #$9c01 : sta !screen_data,x
  ldx #$06fe : lda #$dc01 : sta !screen_data,x

  pla : plp : rtl
}

load_font() {
  php : rep #$30
  pha : phx : phy
  sep #$20

  ldx #$1000 : stx $2116
  lda #$01 : sta $4300
  lda #$18 : sta $4301
  lda.b #font8     : sta $4302
  lda.b #font8>>8  : sta $4303
  lda.b #font8>>16 : sta $4304
  ldx.w #end_font8-font8 : stx $4305
  lda #$01 : sta $420b

  ldx #$6000 : stx $2116
  lda #$01 : sta $4300
  lda #$18 : sta $4301
  lda.b #hexmap     : sta $4302
  lda.b #hexmap>>8  : sta $4303
  lda.b #hexmap>>16 : sta $4304
  ldx.w #end_hexmap-hexmap : stx $4305
  lda #$01 : sta $420b

  rep #$30 : ply : plx : pla
  plp : rtl
}

font8_palette:
  dw %0010100100000100,%0001110011100111,$0000,%0111111111111111 ;white
  dw %0000000000000000,%0000000000000111,$0000,%0000000000011111 ;red
  dw %0000000000000000,%0000000011100000,$0000,%0000001111100000 ;green
  dw %0000000000000000,%0000000011100111,$0000,%0000001111111111 ;yellow
  dw 0,0,0,0
  dw 0,0,0,0
  dw 0,0,0,0
  dw %0000000000000000,%0000000000000000,$0000,%0010100100000100 ;screen border
end_font8_palette:

load_palette() {
  php : rep #$30
  pha : phx : phy
  sep #$20 : rep #$10

  jsl wait_for_vblank
  stz $2121
  ldx #$0000
- lda font8_palette,x : sta $2122
  inx : cpx.w #end_font8_palette-font8_palette : bcc -

  rep #$30 : ply : plx : pla
  plp : rtl
}

;farcall write_string()
;  dbr:a.w = pointer to string
;  x.w     = x position
;  y.w     = y position
;  string formatting codes:
;    $00 = end string
;    $f0 = white (default)
;    $f1 = red
;    $f2 = green
;    $f3 = yellow
write_string() {
  php : rep #$30 : pha : phx : phy

  pha
  lda #$0000 : sta $1ffe
  stx $1ffc
  tya : asl #6
  clc : adc $1ffc
  clc : adc $1ffc
  tax
  pla

  tay
- lda $0000,y : iny
  and #$00ff : beq .end
;color change code
  cmp #$00f0 : bcc +
  and #$0003 : xba : asl #2 : sta $1ffe
  bra -
+ ora $1ffe
  sta !screen_data,x
  inx #2 : bra -

.end
  rep #$30 : ply : plx : pla : plp : rtl
}

refresh_screen() {
  php : rep #$30 : pha : phx : phy

  sep #$20
- lda $4212 : bmi -
- lda $4212 : bpl -
  ldx #$0000 : stx $2116
  lda #$01 : sta $4300
  lda #$18 : sta $4301
  lda.b #!screen_data     : sta $4302
  lda.b #!screen_data>>8  : sta $4303
  lda.b #!screen_data>>16 : sta $4304
  ldx.w #2048 : stx $4305
  lda #$01 : sta $420b

  rep #$30 : ply : plx : pla : plp : rtl
}
