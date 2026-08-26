incsrc header_lorom.asm

org $018000 : incsrc libx816.asm
org $028000 : incsrc libclock.asm
org $038000 : incsrc libmenu.asm

!ctr0 = $1f00
!ctr1 = $1f02
!ctr2 = $1f04
!ctr3 = $1f06
!ctr4 = $1f08
!ctr5 = $1f0a
!ctr6 = $1f0c
!ctr7 = $1f0e

!ctr0_min = $1f10
!ctr1_min = $1f12
!ctr2_min = $1f14
!ctr3_min = $1f16
!ctr4_min = $1f18
!ctr5_min = $1f1a
!ctr6_min = $1f1c
!ctr7_min = $1f1e

!ctr0_max = $1f20
!ctr1_max = $1f22
!ctr2_max = $1f24
!ctr3_max = $1f26
!ctr4_max = $1f28
!ctr5_max = $1f2a
!ctr6_max = $1f2c
!ctr7_max = $1f2e

!res0 = $1f30
!res1 = $1f32
!res2 = $1f34
!res3 = $1f36
!res4 = $1f38
!res5 = $1f3a
!res6 = $1f3c
!res7 = $1f3e

!active_ctr = $1f40
!result_ctr = $1f42

org $008000
  incsrc libinit.asm

  jsl setup_display
  jsl load_font
  jsl load_palette

  rep #$20
  lda #$0000 : sta $1ffe
  lda.w #image_title  : ldx #$0002 : ldy #$0002 : jsl write_string
  lda.w #image_desc   : ldx #$0002 : ldy #$0003 : jsl write_string
  lda.w #image_ctr0   : ldx #$0002 : ldy #$0005 : jsl write_string
  lda.w #image_ctr1   : ldx #$0002 : ldy #$0006 : jsl write_string
  lda.w #image_ctr2   : ldx #$0002 : ldy #$0007 : jsl write_string
  lda.w #image_ctr3   : ldx #$0002 : ldy #$0008 : jsl write_string
  lda.w #image_ctr4   : ldx #$0002 : ldy #$0009 : jsl write_string
  lda.w #image_ctr5   : ldx #$0002 : ldy #$000a : jsl write_string
  lda.w #image_ctr6   : ldx #$0002 : ldy #$000b : jsl write_string
  lda.w #image_ctr7   : ldx #$0002 : ldy #$000c : jsl write_string
  lda.w #image_resctr : ldx #$0002 : ldy #$0014 : jsl write_string
  lda.w #image_ophvct : ldx #$0002 : ldy #$0015 : jsl write_string
  lda.w #image_res01  : ldx #$0002 : ldy #$0016 : jsl write_string
  lda.w #image_res23  : ldx #$0002 : ldy #$0017 : jsl write_string
  lda.w #image_res45  : ldx #$0002 : ldy #$0018 : jsl write_string
  lda.w #image_res67  : ldx #$0002 : ldy #$0019 : jsl write_string

  lda.w #!ctr0min : sta !ctr0_min
  lda.w #!ctr1min : sta !ctr1_min
  lda.w #!ctr2min : sta !ctr2_min
  lda.w #!ctr3min : sta !ctr3_min
  lda.w #!ctr4min : sta !ctr4_min
  lda.w #!ctr5min : sta !ctr5_min
  lda.w #!ctr6min : sta !ctr6_min
  lda.w #!ctr7min : sta !ctr7_min

  lda.w #!ctr0max : sta !ctr0_max
  lda.w #!ctr1max : sta !ctr1_max
  lda.w #!ctr2max : sta !ctr2_max
  lda.w #!ctr3max : sta !ctr3_max
  lda.w #!ctr4max : sta !ctr4_max
  lda.w #!ctr5max : sta !ctr5_max
  lda.w #!ctr6max : sta !ctr6_max
  lda.w #!ctr7max : sta !ctr7_max

  lda !ctr0_min : sta !ctr0
  lda !ctr1_min : sta !ctr1
  lda !ctr2_min : sta !ctr2
  lda !ctr3_min : sta !ctr3
  lda !ctr4_min : sta !ctr4
  lda !ctr5_min : sta !ctr5
  lda !ctr6_min : sta !ctr6
  lda !ctr7_min : sta !ctr7

  stz !active_ctr
  stz !result_ctr
  stz !res0 : stz !res1 : stz !res2 : stz !res3
  stz !res4 : stz !res5 : stz !res6 : stz !res7

  jsr update_counters
  jsl refresh_screen

  sep #$20
  lda #$0f : sta $2100
  rep #$20

main() {
  jsl wait_for_vblank
  jsl poll_joypad
  sta $1c00

  lda !BUTTON_UP : bit $1c00 : beq +
;up_event
  lda !active_ctr : dec : and #$0007 : sta !active_ctr
  jsr update_counters
+ lda !BUTTON_DOWN : bit $1c00 : beq +
;down_event
  lda !active_ctr : inc : and #$0007 : sta !active_ctr
  jsr update_counters
+ lda !BUTTON_LEFT : bit $1c00 : beq +
;left_event
  ldy #$0001 : jsr ctr_dec
  jsr update_counters
+ lda !BUTTON_RIGHT : bit $1c00 : beq +
;right_event
  ldy #$0001 : jsr ctr_inc
  jsr update_counters
+ lda !BUTTON_B : bit $1c00 : beq +
;b_event
  ldy.w #10 : jsr ctr_dec
  jsr update_counters
+ lda !BUTTON_A : bit $1c00 : beq +
;a_event
  ldy.w #10 : jsr ctr_inc
  jsr update_counters
+ lda !BUTTON_Y : bit $1c00 : beq +
;y_event
  ldy.w #100 : jsr ctr_dec
  jsr update_counters
+ lda !BUTTON_X : bit $1c00 : beq +
;x_event
  ldy.w #100 : jsr ctr_inc
  jsr update_counters
+ lda !BUTTON_L : bit $1c00 : beq +
;l_event
  ldy.w #1000 : jsr ctr_dec
  jsr update_counters
+ lda !BUTTON_R : bit $1c00 : beq +
;r_event
  ldy.w #1000 : jsr ctr_inc
  jsr update_counters
+ lda !BUTTON_SELECT : bit $1c00 : beq +
;select_event
  jsr ctr_setmin
  jsr update_counters
+ lda !BUTTON_START : bit $1c00 : beq +
;start_event
  jsr run_test
  jsr update_results
  jsr update_counters
+

  jmp main
}

ctr_dec() {
  sty $1c02
  lda !active_ctr : asl : tax
  lda !ctr0,x
  sec : sbc $1c02

  cmp !ctr0_min,x : bcs +
  lda !ctr0_max,x
+ cmp !ctr0_max,x : bcc + : beq +
  lda !ctr0_max,x
+ sta !ctr0,x

  rts
}

ctr_inc() {
  sty $1c02
  lda !active_ctr : asl : tax
  lda !ctr0,x
  clc : adc $1c02

  cmp !ctr0_min,x : bcs +
  lda !ctr0_min,x
+ cmp !ctr0_max,x : bcc + : beq +
  lda !ctr0_min,x
+ sta !ctr0,x

  rts
}

ctr_setmin() {
  lda !active_ctr : asl : tax
  lda !ctr0_min,x
  sta !ctr0,x

  rts
}

update_counters() {
  php : rep #$30 : pha : phx : phy

  ldx #$0000
  ctr_loop() {
    phx
    txa : asl : tax
    lda !ctr0,x
    ldx #$1e02 : jsl numtodec
    stz $1e07
    plx

    cpx !active_ctr : bne +
    lda #$f300 : sta $1e00 : bra ++
  + lda #$f000 : sta $1e00
  ++
    txa : clc : adc #$0005 : tay
    phx : lda.w #$1e01 : ldx #$0019 : jsl write_string : plx

    inx : cpx #$0008 : bcc ctr_loop
  }

  jsl wait_for_vblank
  jsl wait_for_vblank
  jsl wait_for_vblank
  jsl wait_for_vblank
  jsl wait_for_vblank
  jsl wait_for_vblank

  jsl refresh_screen

  rep #$30 : ply : plx : pla : plp : rts
}

update_results() {
  php : rep #$30 : pha : phx : phy

  sep #$20
  lda $2137
  lda $213f : sta $1b00

  rep #$20
  lda !result_ctr : inc : sta !result_ctr
  ldx #$1e00 : jsl numtodec
  stz $1e05
  lda #$1e00 : ldx #$0019 : ldy #$0014 : jsl write_string

  sep #$20
  lda $213d : xba
  lda $213d : and #$01 : xba
  rep #$20
  ldx #$1e00 : jsl numtodec
  stz $1e05
  lda #$1e02 : ldx #$0008 : ldy #$0015 : jsl write_string
  sep #$20
  lda $213c : xba
  lda $213c : and #$01 : xba
  rep #$20
  ldx #$1e00 : jsl numtodec
  stz $1e05
  lda #$1e02 : ldx #$0012 : ldy #$0015 : jsl write_string
  lda $1b00 : and #$00ff
  ldx #$1e00 : jsl numtohex
  stz $1e04
  lda #$1e02 : ldx #$001c : ldy #$0015 : jsl write_string

  ldx #$0000
  res_loop() {
    phx
    txa : asl : tax
    lda !res0,x
    ldx #$1e00 : jsl numtodec
    stz $1e05
    plx

    txa : lsr : clc : adc #$0016 : tay
    phx
    txa : bit #$0001 : bne +
    ldx #$0005 : bra ++
+   ldx #$0014
++  lda.w #$1e00 : jsl write_string
    plx

    phx
    txa : asl : tax
    lda !res0,x
    ldx #$1e00 : jsl numtohex
    stz $1e04
    plx

    txa : lsr : clc : adc #$0016 : tay
    phx
    txa : bit #$0001 : bne +
    ldx #$000b : bra ++
+   ldx #$001a
++  lda.w #$1e00 : jsl write_string
    plx

    inx : cpx #$0008 : bcc res_loop
  }

  jsl refresh_screen

  rep #$30 : ply : plx : pla : plp : rts
}
