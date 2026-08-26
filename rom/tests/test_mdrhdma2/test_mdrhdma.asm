lorom
org $008000 : fill $020000

incsrc header_lorom.asm

org $018000 : incsrc libx816.asm
org $028000 : incsrc libclock.asm
org $038000 : incsrc libmenu.asm

org $008000
  incsrc libinit.asm

  stz $420c

  lda #$80 : sta $4300
  lda #$37 : sta $4301
  lda #$00 : sta $4302
  lda #$00 : sta $4303
  lda #$7f : sta $4304
  lda #$00 : sta $4308
  lda #$00 : sta $4309
  lda #$ff : sta $430a

  lda #$01 : sta $7f0000
  lda #$00 : sta $7f0001
  lda #$00 : sta $7f0002

  jsl seek_frame
- bit $4212 : bpl -
  lda #$01 : sta $420c
- bit $4212 : bmi -

  rep #$20
  nop #69

  lda $2138
  stz $420c

  sep #$20 : lda $213c : xba : lda $213c : and #$01 : xba
  rep #$20 : sta $700000 : sep #$20

;

  lda #$80 : sta $4300
  lda #$38 : sta $4301
  lda #$00 : sta $4302
  lda #$00 : sta $4303
  lda #$7f : sta $4304
  lda #$00 : sta $4308
  lda #$00 : sta $4309
  lda #$ff : sta $430a

  lda #$01 : sta $7f0000
  lda #$00 : sta $7f0001
  lda #$00 : sta $7f0002

  jsl seek_frame
- bit $4212 : bpl -
  lda #$01 : sta $420c
- bit $4212 : bmi -

  rep #$20
  nop #69

  lda $2137
  stz $420c

  sep #$20 : lda $213c : xba : lda $213c : and #$01 : xba
  rep #$20 : sta $700002 : sep #$20

;

  lda #$00 : sta $4300
  lda #$ff : sta $4301
  lda #$00 : sta $4302
  lda #$00 : sta $4303
  lda #$7f : sta $4304
  lda #$00 : sta $4308
  lda #$00 : sta $4309
  lda #$ff : sta $430a

  lda #$ff : sta $7f0000
  lda #$5a : sta $7f0001
  lda #$a5 : sta $7f0002

  jsl seek_frame
- bit $4212 : bpl -
  lda #$01 : sta $420c
- bit $4212 : bmi -

  ldx #$0004
  rep #$20
- lda $21fc : sta $700000,x : inx : inx
  cmp #$2121 : beq -
  stz $420c
  lda #$aaaa : sta $700000,x

;

  sep #$20
  stz $2121
  lda #$00 : sta $2122
  lda #$7c : sta $2122
  lda #$0f : sta $2100
- bra -
