!JOYSER0 = $4016

!BUTTON_B         = #$0001
!BUTTON_Y         = #$0002
!BUTTON_SELECT    = #$0004
!BUTTON_START     = #$0008
!BUTTON_UP        = #$0010
!BUTTON_DOWN      = #$0020
!BUTTON_LEFT      = #$0040
!BUTTON_RIGHT     = #$0080
!BUTTON_A         = #$0100
!BUTTON_X         = #$0200
!BUTTON_L         = #$0400
!BUTTON_R         = #$0800
!JOYPAD_CONNECTED = #$8000

;farcall numtohex()
;  a.w     = input
;  dbr:x.w = output <4 bytes>
numtohex() {
  php : rep #$30
  pha : phx : phy

  pha : and #$f000 : xba : lsr #4
  cmp #$000a : bcs +
  clc : adc #$0030 : sep #$20 : sta $0000,x : rep #$20 : bra ++
+ clc : adc #$0057 : sep #$20 : sta $0000,x : rep #$20
++
  inx : pla

  pha : and #$0f00 : xba
  cmp #$000a : bcs +
  clc : adc #$0030 : sep #$20 : sta $0000,x : rep #$20 : bra ++
+ clc : adc #$0057 : sep #$20 : sta $0000,x : rep #$20
++
  inx : pla

  pha : and #$00f0 : lsr #4
  cmp #$000a : bcs +
  clc : adc #$0030 : sep #$20 : sta $0000,x : rep #$20 : bra ++
+ clc : adc #$0057 : sep #$20 : sta $0000,x : rep #$20
++
  inx : pla

  and #$000f
  cmp #$000a : bcs +
  clc : adc #$0030 : sep #$20 : sta $0000,x : rep #$20 : bra ++
+ clc : adc #$0057 : sep #$20 : sta $0000,x : rep #$20
++

  ply : plx : pla
  plp : rtl
}

;farcall numtodec()
;  a.w     = input
;  dbr:x.w = output <5 bytes>
numtodec() {
  php : rep #$30
  pha : phx : phy

;set dbr:x.w[0-3] to '00000'
;dbr:x.w[4] is set manually
  pha : lda #$3030
  sta $0000,x
  sta $0002,x
  pla

- cmp.w #10000 : bcc +
  inc $0000,x
  sec : sbc.w #10000 : bra -
+
- cmp.w #1000 : bcc +
  inc $0001,x
  sec : sbc.w #1000 : bra -
+
- cmp.w #100 : bcc +
  inc $0002,x
  sec : sbc.w #100 : bra -
+
- cmp.w #10 : bcc +
  inc $0003,x
  sec : sbc.w #10 : bra -
+
- clc : adc #$0030 : sep #$20 : sta $0004,x : rep #$20

  ply : plx : pla
  plp : rtl
}

;farcall poll_joypad()
;  a.w = output
poll_joypad() {
  php : rep #$10 : phx
  sep #$20

  lda #$01 : sta !JOYSER0
  stz !JOYSER0
  ldx #$0000

  lda !JOYSER0 : and #$01 : beq +
  rep #$20 : txa : ora !BUTTON_B : tax : sep #$20
+ lda !JOYSER0 : and #$01 : beq +
  rep #$20 : txa : ora !BUTTON_Y : tax : sep #$20
+ lda !JOYSER0 : and #$01 : beq +
  rep #$20 : txa : ora !BUTTON_SELECT : tax : sep #$20
+ lda !JOYSER0 : and #$01 : beq +
  rep #$20 : txa : ora !BUTTON_START : tax : sep #$20
+ lda !JOYSER0 : and #$01 : beq +
  rep #$20 : txa : ora !BUTTON_UP : tax : sep #$20
+ lda !JOYSER0 : and #$01 : beq +
  rep #$20 : txa : ora !BUTTON_DOWN : tax : sep #$20
+ lda !JOYSER0 : and #$01 : beq +
  rep #$20 : txa : ora !BUTTON_LEFT : tax : sep #$20
+ lda !JOYSER0 : and #$01 : beq +
  rep #$20 : txa : ora !BUTTON_RIGHT : tax : sep #$20
+ lda !JOYSER0 : and #$01 : beq +
  rep #$20 : txa : ora !BUTTON_A : tax : sep #$20
+ lda !JOYSER0 : and #$01 : beq +
  rep #$20 : txa : ora !BUTTON_X : tax : sep #$20
+ lda !JOYSER0 : and #$01 : beq +
  rep #$20 : txa : ora !BUTTON_L : tax : sep #$20
+ lda !JOYSER0 : and #$01 : beq +
  rep #$20 : txa : ora !BUTTON_R : tax : sep #$20
+ lda !JOYSER0
  lda !JOYSER0
  lda !JOYSER0
  lda !JOYSER0
  lda !JOYSER0 : and #$01 : beq +
  rep #$20 : txa : ora !JOYPAD_CONNECTED
+

  plx : plp : rtl
}

;farcall wait_for_vblank()
wait_for_vblank() {
  php : sep #$20 : pha

- lda $4212 : bmi -
- lda $4212 : bpl -

  pla : plp : rtl
}
