;test_hdmasync
;version 1.00 ~byuu (2008-08-01)

;this test executes 2 * 256 HDMA transfers
;the first 256 test DMA counter at positions 0,4
;the second 256 test DMA counter at positions 2,6
;the position alternates each run, as frame length % 8 != 0, but frame length % 4 == 0
;latch twice to verify half-dot position
;when finished, compare all latch values to hardware-cached copy of results
;results are verified to be the same on both CPUr1 and CPur2

;test verifies that:
;hdma begins at H=1100+DMA_counter, eg H=1100+(cycle_counter&7)

;note:
;if using this test on a copier / flash cart with BIOS, this test may fail on
;the first hardware iteration due to misalignment of the DMA counter (50% chance).
;reset the SNES once to get the unit into a consistent state.
;the test should then pass after every reset.

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

  lda #$00 : sta $2181
  lda #$00 : sta $2182
  lda #$7f : sta $2183

  lda #$01 : sta $0080
  lda #$01 : sta $0081
  lda #$ae : sta $0082

  lda #$00 : sta $00ff

loop04:
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
  lda $ff : sta $420c

  ;without NOP -- DMA counter = 0,4 (rotates)
  ;with NOP    -- DMA counter = 2,6 (rotates)
  ;nop

  ldx #$0030
  - : dex : bne -

  stz $420c

  lda $2137
  lda $213c : sta $2180
  lda $213c : and #$01 : sta $2180
  lda $2137
  lda $213c : sta $2180
  lda $213c : and #$01 : sta $2180

  lda $ff : cmp #$ff : beq finish04
  inc : sta $ff : jmp loop04

finish04:
  lda #$00 : sta $00ff

loop26:
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
  lda $ff : sta $420c

  ;without NOP -- DMA counter = 0,4 (rotates)
  ;with NOP    -- DMA counter = 2,6 (rotates)
  nop

  ldx #$0030
  - : dex : bne -

  stz $420c

  lda $2137
  lda $213c : sta $2180
  lda $213c : and #$01 : sta $2180
  lda $2137
  lda $213c : sta $2180
  lda $213c : and #$01 : sta $2180

  lda $ff : cmp #$ff : beq finish26
  inc : sta $ff : jmp loop26

finish26:
  ;copy to RAM to verify values
  rep #$20 : ldx #$0000
  - lda $7f0000,x : sta $700000,x
  inx #2 : cpx.w #2048 : bcc -

  ;verify hardware match
  ldx #$0002
  - lda hdmasync_data,x : cmp $7f0000,x : bne fail
  inx #2 : cpx.w #2048 : bcc -

  lda $4210 : and #$000f
  cmp #$0001 : beq test_cpur1
  cmp #$0002 : beq test_cpur2
  bra pass

  ;DRAM refresh will occur before first HDMA (CPUr1 DRAM refresh is static @ H=530)
test_cpur1:
  lda $7f0000 : cmp #$008f : bne fail : bra pass

  ;DRAM refresh will not occur before first HDMA (CPUr2 DRAM refresh is cyclic @ H=534,538)
test_cpur2:
  lda $7f0000 : cmp #$0085 : bne fail : bra pass

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

hdmasync_data:
  incbin test_hdmasync_data.bin
