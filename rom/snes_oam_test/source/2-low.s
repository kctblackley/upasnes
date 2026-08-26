; Tests behavior for low 256 words of OAM

.include "common.inc"

main:
	mov INIDISP,#$80
	
	set_test 2,"Organized as words, not bytes"
	jsr clear_oam
	movw OAMADDL,#1
	mov OAMDATA,#$12
	mov OAMDATA,#$34
	check_oam 0, {$00,$00,$12,$34,$00,$00}
	
	set_test 3,"Read of even byte toggles flag"
	jsr clear_oam
	movw OAMADDL,#1
	bit OAMDATAREAD
	mov OAMDATA,#$34
	check_oam 0, {$00,$00,$00,$34,$00,$00}
	
	set_test 4,"First write doesn't affect OAM"
	jsr clear_oam
	movw OAMADDL,#1
	mov OAMDATA,#$12
	check_oam 0, {$00,$00,$00,$00,$00,$00}
	
	set_test 5,"Setting address clears flag"
	jsr clear_oam
	movw OAMADDL,#1
	mov OAMDATA,#$12
	movw OAMADDL,#1
	mov OAMDATA,#$34
	mov OAMDATA,#$56
	check_oam 0, {$00,$00,$34,$56,$00,$00}
	
	set_test 6,"First write sets buffer, second writes word"
	jsr clear_oam
	movw OAMADDL,#0
	mov OAMDATA,#$12
	movw OAMADDL,#1
	bit OAMDATAREAD
	mov OAMDATA,#$34
	check_oam 0, {$00,$00,$12,$34,$00,$00}
	
	set_test 7,"Buffer keeps contents after write"
	jsr clear_oam
	movw OAMADDL,#0
	mov OAMDATA,#$12
	mov OAMDATA,#$34
	bit OAMDATAREAD
	mov OAMDATA,#$56
	bit OAMDATAREAD
	mov OAMDATA,#$78
	check_oam 0, {$12,$34,$12,$56,$12,$78}
	
	set_test 8,"Read of odd byte toggles flag and increments addr"
	jsr clear_oam
	movw OAMADDL,#0
	mov OAMDATA,#$12
	bit OAMDATAREAD
	mov OAMDATA,#$34
	mov OAMDATA,#$56
	check_oam 0, {$00,$00,$34,$56,$00,$00}
	
	set_test 9,"Reading doesn't affect buffer"
	jsr clear_oam
	movw OAMADDL,#0
	mov OAMDATA,#$12
	bit OAMDATAREAD
	bit OAMDATAREAD
	mov OAMDATA,#$34
	check_oam 0, {$00,$00,$12,$34,$00,$00}
	
	set_test 10,"Low byte read is unbuffered"
	jsr fill_oam
	movw OAMADDL,#1
	lda OAMDATAREAD
	cmp #$02
	jne test_failed
	
	set_test 11,"High byte read is unbuffered"
	jsr fill_oam
	movw OAMADDL,#1
	bit OAMDATAREAD
	lda OAMDATAREAD
	cmp #$03
	jne test_failed
	
	set_test 12,"Writing low byte makes next read get high byte"
	jsr fill_oam
	movw OAMADDL,#1
	mov OAMDATA,#$12
	lda OAMDATAREAD
	cmp #$03
	jne test_failed
	
	set_test 13,"Writing low byte of addr resets flag"
	jsr clear_oam
	movw OAMADDL,#1
	mov OAMDATA,#$12
	mov OAMADDL,#1
	mov OAMDATA,#$34
	check_oam 0, {$00,$00,$00,$00,$00,$00}
	
	set_test 14,"Writing high byte of addr resets flag"
	jsr clear_oam
	movw OAMADDL,#1
	mov OAMDATA,#$12
	mov OAMADDH,#0
	mov OAMDATA,#$34
	check_oam 0, {$00,$00,$00,$00,$00,$00}
	
	jmp tests_passed
