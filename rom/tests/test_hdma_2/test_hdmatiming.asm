;test_hdmatiming
;version 1.00 ~byuu (2008-08-03)

;this test verifies timing inside HDMA run function, specifically:
;- no transfer + no line load = 8 clocks/channel overhead
;- no transfer + line load = 8 clocks/channel overhead
;- transfer + no line load = 16 clocks/channel overhead
;- transfer + line load = 16 clocks/channel overhead
;- no transfer + line load + indirect address load = 24 clocks/channel overhead
;- indirect transfer + no line load = 16 clocks/channel overhead
;- HDMA performs channel 0-7 transfer; followed by channel 0-7 line load:
;--- that is to say, transfer and line load operations are not interleaved per channel
;it also shows that HDMA triggers at H=1104

lorom
org $008000 : fill $020000

incsrc header_lorom.asm

org $018000 : incsrc libx816.asm
org $028000 : incsrc libclock.asm
org $038000 : incsrc libmenu.asm

org $008000
  incsrc libinit.asm

  lda #$aa : sta $0080
  lda #$80 : sta $0081
  lda #$00 : sta $0082

;HDMA 1:
;- do not transfer byte
;- do not load line counter
test1:
  ;HDMA table setup
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
  lda #$02 : sta $430a : sta $431a : sta $432a : sta $433a : sta $434a : sta $435a : sta $436a : sta $437a
  lda #$00 : sta $430b : sta $431b : sta $432b : sta $433b : sta $434b : sta $435b : sta $436b : sta $437b

  jsl seek_frame
  lda #$ff : sta $420c

  ;CPU sync cycle = 6
  nop #80

  stz $420c

  lda $2137
  lda $213c : sta $700000
  lda $213c : and #$01 : sta $700001
  lda $2137
  lda $213c : sta $700002
  lda $213c : and #$01 : sta $700003
  lda $213d : sta $700004
  lda $213d : and #$01 : sta $700005
  lda $430a : sta $700006
  lda $437a : sta $700007

;HDMA 1:
;- do not transfer byte
;- load line counter
test2:
  ;HDMA table setup
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

  ;CPU sync cycle = 8
  db $42,$00,$42,$00,$42,$00
  nop #80

  stz $420c

  lda $2137
  lda $213c : sta $700008
  lda $213c : and #$01 : sta $700009
  lda $2137
  lda $213c : sta $70000a
  lda $213c : and #$01 : sta $70000b
  lda $213d : sta $70000c
  lda $213d : and #$01 : sta $70000d
  lda $430a : sta $70000e
  lda $437a : sta $70000f

;HDMA 1:
;- do not transfer byte
;- load line counter
;HDMA 2:
;- transfer byte
;- do not load line counter
test3:
  ;HDMA table setup
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

  ;CPU sync cycle = 6, 6
  nop #200

  stz $420c

  lda $2137
  lda $213c : sta $700010
  lda $213c : and #$01 : sta $700011
  lda $2137
  lda $213c : sta $700012
  lda $213c : and #$01 : sta $700013
  lda $213d : sta $700014
  lda $213d : and #$01 : sta $700015
  lda $430a : sta $700016
  lda $437a : sta $700017

;HDMA 1:
;- do not transfer byte
;- load line counter
;- load indirect address
test4:
  ;HDMA table setup
  lda #$40 : sta $4300 : sta $4310 : sta $4320 : sta $4330 : sta $4340 : sta $4350 : sta $4360 : sta $4370
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

  ;CPU sync cycle = 6
  nop #80

  stz $420c

  lda $2137
  lda $213c : sta $700018
  lda $213c : and #$01 : sta $700019
  lda $2137
  lda $213c : sta $70001a
  lda $213c : and #$01 : sta $70001b
  lda $213d : sta $70001c
  lda $213d : and #$01 : sta $70001d
  lda $4305 : sta $70001e
  lda $4306 : sta $70001f

;HDMA 1:
;- do not transfer byte
;- load line counter
;- load indirect address
;HDMA 2:
;- transfer byte
;- do not load line counter
test5:
  ;HDMA table setup
  lda #$40 : sta $4300 : sta $4310 : sta $4320 : sta $4330 : sta $4340 : sta $4350 : sta $4360 : sta $4370
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

  ;CPU sync cycle = 6, 8
  nop #200

  stz $420c

  lda $2137
  lda $213c : sta $700020
  lda $213c : and #$01 : sta $700021
  lda $2137
  lda $213c : sta $700022
  lda $213c : and #$01 : sta $700023
  lda $213d : sta $700024
  lda $213d : and #$01 : sta $700025
  lda $4305 : sta $700026
  lda $4306 : sta $700027

;HDMA 1:
;- do not transfer byte
;- load line counter
;HDMA 2:
;- transfer byte
;- load line counter
test6:
  ;HDMA table setup
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
  lda #$81 : sta $430a : sta $431a : sta $432a : sta $433a : sta $434a : sta $435a : sta $436a : sta $437a
  lda #$00 : sta $430b : sta $431b : sta $432b : sta $433b : sta $434b : sta $435b : sta $436b : sta $437b

  lda #$81 : sta $0080

  jsl seek_frame
  lda #$ff : sta $420c

  ;CPU sync cycle = 6, 8
  nop #200

  stz $420c

  lda $2137
  lda $213c : sta $700028
  lda $213c : and #$01 : sta $700029
  lda $2137
  lda $213c : sta $70002a
  lda $213c : and #$01 : sta $70002b
  lda $213d : sta $70002c
  lda $213d : and #$01 : sta $70002d
  lda $430a : sta $70002e
  lda $437a : sta $70002f

;latch $2137 via channel 0
test7:
  ;HDMA table setup
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
  lda #$81 : sta $430a : sta $431a : sta $432a : sta $433a : sta $434a : sta $435a : sta $436a : sta $437a
  lda #$00 : sta $430b : sta $431b : sta $432b : sta $433b : sta $434b : sta $435b : sta $436b : sta $437b

  lda #$80 : sta $4300
  lda #$37 : sta $4301
  lda #$81 : sta $0080

  jsl seek_frame
  lda #$ff : sta $420c

  ;CPU sync cycle = 6
  nop #200

  stz $420c

  lda $213c : sta $700030
  lda $213c : and #$01 : sta $700031
  lda $2137
  lda $213c : sta $700032
  lda $213c : and #$01 : sta $700033
  lda $2137
  lda $213c : sta $700034
  lda $213c : and #$01 : sta $700035
  lda $213d : sta $700036
  lda $213d : and #$01 : sta $700037

;latch $2137 via channel 7
test8:
  ;HDMA table setup
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
  lda #$81 : sta $430a : sta $431a : sta $432a : sta $433a : sta $434a : sta $435a : sta $436a : sta $437a
  lda #$00 : sta $430b : sta $431b : sta $432b : sta $433b : sta $434b : sta $435b : sta $436b : sta $437b

  lda #$80 : sta $4370
  lda #$37 : sta $4371
  lda #$81 : sta $0080

  jsl seek_frame
  lda #$ff : sta $420c

  ;CPU sync cycle = 6
  nop #200

  stz $420c

  lda $213c : sta $700038
  lda $213c : and #$01 : sta $700039
  lda $2137
  lda $213c : sta $70003a
  lda $213c : and #$01 : sta $70003b
  lda $2137
  lda $213c : sta $70003c
  lda $213c : and #$01 : sta $70003d
  lda $213d : sta $70003e
  lda $213d : and #$01 : sta $70003f

;HDMA during DMA, overhead = 12
test9:
  ;DMA table setup
  lda #$00 : sta $4300
  lda #$c0 : sta $4301
  lda #$00 : sta $4302
  lda #$00 : sta $4303
  lda #$7e : sta $4304
  lda #$00 : sta $4305
  lda #$02 : sta $4306

  ;HDMA table setup
  lda #$00 : sta $4310
  lda #$c0 : sta $4311
  lda #$00 : sta $4312
  lda #$00 : sta $4313
  lda #$00 : sta $4314
  lda #$00 : sta $4315
  lda #$00 : sta $4316
  lda #$00 : sta $4317
  lda #$80 : sta $4318
  lda #$00 : sta $4319
  lda #$01 : sta $431a

  lda #$00 : sta $0080

  jsl seek_frame
  lda #$02 : sta $420c
  lda #$01 : sta $420b
  nop #4

  stz $420c

  lda $2137
  lda $213c : sta $700040
  lda $213c : and #$01 : sta $700041
  lda $2137
  lda $213c : sta $700042
  lda $213c : and #$01 : sta $700043
  lda $213d : sta $700044
  lda $213d : and #$01 : sta $700045
  lda #$00 : sta $700046
  lda #$00 : sta $700047

;HDMA during DMA, overhead = 6
test10:
  ;DMA table setup
  lda #$00 : sta $4300
  lda #$c0 : sta $4301
  lda #$00 : sta $4302
  lda #$00 : sta $4303
  lda #$7e : sta $4304
  lda #$00 : sta $4305
  lda #$02 : sta $4306

  ;HDMA table setup
  lda #$00 : sta $4310
  lda #$c0 : sta $4311
  lda #$00 : sta $4312
  lda #$00 : sta $4313
  lda #$00 : sta $4314
  lda #$00 : sta $4315
  lda #$00 : sta $4316
  lda #$00 : sta $4317
  lda #$80 : sta $4318
  lda #$00 : sta $4319
  lda #$01 : sta $431a

  lda #$00 : sta $0080

  jsl seek_frame
  nop
  lda #$02 : sta $420c
  lda #$01 : sta $420b
  nop #4

  stz $420c

  lda $2137
  lda $213c : sta $700048
  lda $213c : and #$01 : sta $700049
  lda $2137
  lda $213c : sta $70004a
  lda $213c : and #$01 : sta $70004b
  lda $213d : sta $70004c
  lda $213d : and #$01 : sta $70004d
  lda #$00 : sta $70004e
  lda #$00 : sta $70004f

;HDMA during DMA, overhead = 6
test11:
  ;DMA table setup
  lda #$00 : sta $4300
  lda #$c0 : sta $4301
  lda #$00 : sta $4302
  lda #$00 : sta $4303
  lda #$7e : sta $4304
  lda #$00 : sta $4305
  lda #$02 : sta $4306

  ;HDMA table setup
  lda #$00 : sta $4310
  lda #$c0 : sta $4311
  lda #$00 : sta $4312
  lda #$00 : sta $4313
  lda #$00 : sta $4314
  lda #$00 : sta $4315
  lda #$00 : sta $4316
  lda #$00 : sta $4317
  lda #$80 : sta $4318
  lda #$00 : sta $4319
  lda #$01 : sta $431a

  lda #$00 : sta $0080

  jsl seek_frame
  nop #2
  lda #$02 : sta $420c
  lda #$01 : sta $420b
  nop #4

  stz $420c

  lda $2137
  lda $213c : sta $700050
  lda $213c : and #$01 : sta $700051
  lda $2137
  lda $213c : sta $700052
  lda $213c : and #$01 : sta $700053
  lda $213d : sta $700054
  lda $213d : and #$01 : sta $700055
  lda #$00 : sta $700056
  lda #$00 : sta $700057

;HDMA during DMA, overhead = 6
test12:
  ;DMA table setup
  lda #$00 : sta $4300
  lda #$c0 : sta $4301
  lda #$00 : sta $4302
  lda #$00 : sta $4303
  lda #$7e : sta $4304
  lda #$00 : sta $4305
  lda #$02 : sta $4306

  ;HDMA table setup
  lda #$00 : sta $4310
  lda #$c0 : sta $4311
  lda #$00 : sta $4312
  lda #$00 : sta $4313
  lda #$00 : sta $4314
  lda #$00 : sta $4315
  lda #$00 : sta $4316
  lda #$00 : sta $4317
  lda #$80 : sta $4318
  lda #$00 : sta $4319
  lda #$01 : sta $431a

  lda #$00 : sta $0080

  jsl seek_frame
  nop #3
  lda #$02 : sta $420c
  lda #$01 : sta $420b
  nop #4

  stz $420c

  lda $2137
  lda $213c : sta $700058
  lda $213c : and #$01 : sta $700059
  lda $2137
  lda $213c : sta $70005a
  lda $213c : and #$01 : sta $70005b
  lda $213d : sta $70005c
  lda $213d : and #$01 : sta $70005d
  lda #$00 : sta $70005e
  lda #$00 : sta $70005f

test_end:
  rep #$20
  ldx #$0000
  - lda compdata,x : cmp $700000,x : bne fail
  inx #2 : cpx.w #64 : bcc -
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

compdata:
  ;HDMA overhead
  dw $014e,$0028,$0001,$0101
  dw $0006,$0035,$0001,$aaaa
  dw $0078,$00b1,$0002,$a9a9
  dw $001b,$0049,$0001,$0080
  dw $00a2,$00d0,$0002,$0081
  dw $0078,$00b1,$0002,$0000

  ;HDMA position
  dw $011c,$00a9,$00d8,$0002
  dw $0129,$00a9,$00d8,$0002

  ;HDMA during DMA
  dw $005e,$0097,$0003,$0000
  dw $0060,$0099,$0003,$0000
  dw $0065,$009e,$0003,$0000
  dw $0069,$00a1,$0003,$0000
