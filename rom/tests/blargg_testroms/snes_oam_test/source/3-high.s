; Tests behavior for upper 16 words of OAM, and combinations

.include "common.inc"

main:
	mov INIDISP,#$80
	
	set_test 2,"Organized as words, not bytes"
	jsr clear_oam
	movw OAMADDL,#$101
	mov OAMDATA,#$12
	mov OAMDATA,#$34
	check_oam $100, {$00,$00,$12,$34,$00,$00}
	
	set_test 3,"Read of even byte toggles flag"
	jsr clear_oam
	movw OAMADDL,#$101
	bit OAMDATAREAD
	mov OAMDATA,#$34
	check_oam $100, {$00,$00,$00,$34,$00,$00}
	
	set_test 4,"First write affects OAM"
	jsr clear_oam
	movw OAMADDL,#$101
	mov OAMDATA,#$12
	check_oam $100, {$00,$00,$12,$00,$00,$00}
	
	set_test 5,"First write sets buffer"
	jsr clear_oam
	movw OAMADDL,#$101
	mov OAMDATA,#$12
	movw OAMADDL,#0
	bit OAMDATAREAD
	mov OAMDATA,#$34
	check_oam 0, {$12,$34}
	
	set_test 6,"Setting address clears flag"
	jsr clear_oam
	movw OAMADDL,#$101
	mov OAMDATA,#$12
	movw OAMADDL,#$101
	mov OAMDATA,#$34
	mov OAMDATA,#$56
	check_oam $100, {$00,$00,$34,$56,$00,$00}
	
	set_test 7,"Write of odd byte just writes byte, not word"
	jsr clear_oam
	movw OAMADDL,#$100
	mov OAMDATA,#$12
	movw OAMADDL,#$101
	bit OAMDATAREAD
	mov OAMDATA,#$34
	check_oam $100, {$12,$00,$00,$34,$00,$00}
	
	set_test 8,"Buffer keeps contents after odd write"
	jsr clear_oam
	movw OAMADDL,#$101
	mov OAMDATA,#$12
	mov OAMDATA,#$34
	movw OAMADDL,#0
	bit OAMDATAREAD
	mov OAMDATA,#$56
	check_oam 0, {$12,$56}

	set_test 9,"Read of odd byte toggles flag and increments addr"
	jsr clear_oam
	movw OAMADDL,#$100
	mov OAMDATA,#$12
	bit OAMDATAREAD
	mov OAMDATA,#$34
	mov OAMDATA,#$56
	check_oam $100, {$12,$00,$34,$56,$00,$00}
	
	set_test 10,"Reading doesn't affect buffer"
	jsr clear_oam
	movw OAMADDL,#$100
	mov OAMDATA,#$12
	bit OAMDATAREAD
	bit OAMDATAREAD
	movw OAMADDL,#0
	bit OAMDATAREAD
	mov OAMDATA,#$34
	check_oam 0, {$12,$34}
	
	set_test 11,"Low byte read is unbuffered"
	jsr fill_oam
	movw OAMADDL,#$101
	lda OAMDATAREAD
	cmp #$02
	jne test_failed
	
	set_test 12,"High byte read is unbuffered"
	jsr fill_oam
	movw OAMADDL,#$101
	bit OAMDATAREAD
	lda OAMDATAREAD
	cmp #$03
	jne test_failed
	
	set_test 13,"Writing low byte makes next read get high byte"
	jsr fill_oam
	movw OAMADDL,#$101
	mov OAMDATA,#$12
	lda OAMDATAREAD
	cmp #$03
	jne test_failed

	set_test 14,"First write to low bank doesn't affect addr"
	jsr clear_oam
	movw OAMADDL,#1
	mov OAMDATA,#$12
	mov OAMADDH,#1
	mov OAMDATA,#$34
	check_oam $100, {$00,$00,$34,$00,$00,$00}
	
	set_test 15,"High byte of addr incremented when low wraps"
	jsr clear_oam
	movw OAMADDL,#$FF
	mov OAMDATA,#$12
	mov OAMDATA,#$34
	mov OAMDATA,#$56
	check_oam $FF, {$12,$34,$56,$00}
	
	set_test 16,"Upper bits of OAMADDH ignored"
	jsr clear_oam
	movw OAMADDL,#$FE00
	mov OAMDATA,#$12
	mov OAMDATA,#$34
	movw OAMADDL,#$FF00
	mov OAMDATA,#$56
	check_oam 0, {$12,$34}
	check_oam $100, {$56,$00}
	
	jmp tests_passed
