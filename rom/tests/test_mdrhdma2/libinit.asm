init_snes() {
  sei : stz $4200
  clc : xce
  rep #$10 : ldx #$01ff : txs
  sep #$30

  lda #$00 : pha : plb
  pea $0000 : pld

  lda #$8f : sta $2100
  ldx #$01
- stz $2100,x : inx : cpx #$0d : bcc -
- stz $2100,x : stz $2100,x : inx : cpx #$15 : bcc -
- stz $2100,x : inx : cpx #$1b : bcc -
- stz $2100,x : stz $2100,x : inx : cpx #$21 : bcc -
- stz $2100,x : inx : cpx #$34 : bcc -
  stz $4200
  ldx #$02
- stz $4200,x : inx : cpx #$0e : bcc -

  lda #$80 : sta $2115 ;increment on $2119 write
  lda #$01 : sta $420d ;enable fastrom
  rep #$10

;clear VRAM
  lda #$00   : sta $7fffff
  lda #$09   : sta $4300
  lda #$18   : sta $4301
  lda #$ff   : sta $4302
  lda #$ff   : sta $4303
  lda #$7f   : sta $4304
  ldx #$0000 : stx $4305
  lda #$01   : sta $420b

;clear OAM
  stz $2102 : stz $2103
  ldx #$0080
- stz $2104
  lda #$e0 : sta $2104
  stz $2104
  stz $2104
  dex : bne -
  ldx #$0020
- stz $2104
  dex : bne -

;clear CGRAM
  stz $2121
  ldx #$0200
- stz $2122 : dex : bne -
  stz $2121

;fallthrough
}
