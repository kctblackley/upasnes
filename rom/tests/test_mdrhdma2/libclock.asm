;libclock
;version 1.01
;written by: byuu

;this library is used to align OPHCT / OPVCT to specific
;half dot values. this is necessary to perform critical
;timing tests.

;public functions:
;  seek_frame()

;private functions:
;  seek_cycles()

;farcall seek_frame()
;  when function returns, first bus cycle of first opcode
;  will be at { 0, 0, 0 }
;* FastROM must be enabled, NMI + IRQ + DMA + HDMA +
;  joypad polling must be disabled, and you must be in
;  native mode for this routine to work!
seek_frame() {
;make sure A23 is clear
  jml seek_frame_slowrom
seek_frame_slowrom:

  php : phb : phd
  rep #$30 : pha : phx : phy
  sep #$20

;make sure DBR and D are set to zero
  pea $0000 : pld
  pea $0000 : plb : plb

;first, seek to the start of an odd frame
- lda $213f : bmi -
- lda $213f : bpl -

;next, seek to start of vblank
- lda $4212 : bpl -

;read OPHCT twice, to determine whether the first read
;was on a half dot or not (0.5 or 0.0?)
  lda $2137 ;* counters latched at 225,  48; +2 = 225, 12
  lda $213c : xba
  lda $213c : and #$01 : xba
  rep #$20
  sta $1ffc
  sep #$20

;between the two counter latches, (278 - 48 -> 230) cycles
;have passed... 222 / 4 = 57.5 dots
;if $1ffc = $1ffe+57, first latch occurred on an even half-dot
;if $1ffc = $1ffe+58, first latch occurred on an odd half-dot
;second latch occurred on the opposite side of the half-dot
;both latches should always occur before DRAM refresh
  lda $2137 ;* counters latched at 225, 278; +2 = 225, 70
  lda $213c : xba
  lda $213c : and #$01 : xba
  rep #$20
  sta $1ffe

  sec : sbc.w #57
  cmp $1ffc
;can't use any branches if we're to keep a consistent cycle
;count for the below code...
;if equal, zero is clear and second latch occurred on odd
;half-dot. otherwise, second latch occurred on even half-dot.
;flip the result to give a value to subtract from main cycle
;count to reach the next frame...
  php : php : pla
  and #$0002 : eor #$0002
  sta $1ffc
  lda $1ffe
  asl #2 : sec : sbc $1ffc
  sta $1ffe

;remove the overhead of this function
;see libclock_cycles.txt for analysis of function cycles
;(262-225(=37))*1324-4=48984
  lda.w #48984-184-408-140-244
  sec : sbc.w $1ffe
  jsl seek_cycles

  ply : plx : pla
  pld : plb : plp : rtl
}

;farcall seek_cycles()
;  a.w = input
;  will execute a.w clock cycles
;* never call this function with a.w < 640!
;  it's not possible to get perfect precision (e.g. with a.w = 2),
;  as some time is needed to perform the calculations to determine
;  how long to run for...
;  also, never call the function with (a.w & 1) != 0, it's impossible
;  to single-step clock cycles, and doing such will break the indexed
;  jump table call.
;* DRAM refresh may occur during this function. if so, results will
;  be offset as such. try waiting for the end of hblank or such if
;  you need to call this function and account for that overhead.
;* this function will destroy the value in a.w and x.w, but will save p.
;* this function does not deduct the 62 clock cycle jsl calling delay to
;  reach this function. the reason is because the delay would be less if
;  this function was called while A23 was set, and I prefer not to make
;  it a requirement that A23 is clear /before/ calling this function.
;* note: this function was not designed to be used by itself, but I have
;  taken some precaution to allow it to be. most issues I could account
;  for, but by doing such, I would increase the minimum number of cycles
;  this function could seek by significantly.
seek_cycles() {
;remove the overhead of this function
;see libclock_cycles.txt for analysis of function cycles
  php : rep #$30
  sec : sbc.w #82+46+120+72

;this loop consumes exactly 100 clock cycles per iteration, so use
;it to remove as much from the cycle count as possible, so a smaller
;cycle skipping table can be used...
- cmp.w #256 : bcc +
  sec : sbc.w #100 : bra -

;156 <= a <= 254 here...
;this gives us a range of 100, or 50 entries needed by the lookup
;table to reach any given cycle position...
+ sec : sbc.w #156 : tax
  sep #$20
  jmp (cycle_skip_table,x)

;each entry in the jump table ends with the following:
; plp : rtl
}

incsrc cyclegen.asm
