;test_hdmadisable
;version 1.00 ~byuu (2008-07-28)

lorom
org $008000 : fill $020000

incsrc header_lorom.asm

org $018000 : incsrc libx816.asm
org $028000 : incsrc libclock.asm
org $038000 : incsrc libmenu.asm

org $00f800
  pha : lda $4211 : pla
  sta $2180
  rti

org $008000
  incsrc libinit.asm

  sep #$20 : rep #$10

  lda #$01 : sta $0080
  lda #$01 : sta $0081
  lda #$ae : sta $0082

  lda #$00 : sta $4300 : sta $4310 : sta $4320 : sta $4330 : sta $4340 : sta $4350 : sta $4360 : sta $4370
  lda #$c0 : sta $4301 : sta $4311 : sta $4321 : sta $4331 : sta $4341 : sta $4351 : sta $4361 : sta $4371
  lda #$00 : sta $4302 : sta $4312 : sta $4322 : sta $4332 : sta $4342 : sta $4352 : sta $4362 : sta $4372
  lda #$00 : sta $4303 : sta $4313 : sta $4323 : sta $4333 : sta $4343 : sta $4353 : sta $4363 : sta $4373
  lda #$00 : sta $4304 : sta $4314 : sta $4324 : sta $4334 : sta $4344 : sta $4354 : sta $4364 : sta $4374
  lda #$00 : sta $4305 : sta $4315 : sta $4325 : sta $4335 : sta $4345 : sta $4355 : sta $4365 : sta $4375
  lda #$00 : sta $4306 : sta $4316 : sta $4326 : sta $4336 : sta $4346 : sta $4356 : sta $4366 : sta $4376
  lda #$00 : sta $4307 : sta $4317 : sta $4327 : sta $4337 : sta $4347 : sta $4357 : sta $4367 : sta $4377
  lda #$80 : sta $4308 : sta $4318 : sta $4328 : sta $4338 : sta $4348 : sta $4358 : sta $4368 : sta $4378
  lda #$00 : sta $4309 : sta $4319 : sta $4329 : sta $4339 : sta $4349 : sta $4359 : sta $4369 : sta $4379
  lda #$01 : sta $430a : sta $431a : sta $432a : sta $433a : sta $434a : sta $435a : sta $436a : sta $437a
  lda #$00 : sta $430b : sta $431b : sta $432b : sta $433b : sta $434b : sta $435b : sta $436b : sta $437b

  jsl seek_frame
  lda #$ff : sta $420c

  ldx #$0040
  - : dex : bne -

  stz $420c

  lda $2137
  lda $213c : sta $700000
  lda $213c : and #$01 : sta $700001
  lda $213d : sta $700002
  lda $213d : and #$01 : sta $700003

  lda #$00
  sta $700004 : sta $700005 : sta $700006 : sta $700007

  lda #$01 : sta $2180
  lda $430a : sta $700008
  lda $431a : sta $700009
  lda $432a : sta $70000a
  lda $433a : sta $70000b
  lda $434a : sta $70000c
  lda $435a : sta $70000d
  lda $436a : sta $70000e
  lda $437a : sta $70000f

  jmp pass

pass() {
  sei
  sep #$20
  stz $4200
  stz $2121
  lda #$00 : sta $2122
  lda #$7c : sta $2122
  lda #$0f : sta $2100
  stp
}

fail() {
  sei
  sep #$20
  stz $4200
  stz $2121
  lda #$1f : sta $2122
  lda #$00 : sta $2122
  lda #$0f : sta $2100
  stp
}
