; Delays X milliseconds
; Preserved: A, Y
delay_x_msec:
	phx             ; 3
	ldx #210        ; 2
:       phx             ; *3 delay
	plx             ; *4 delay
	dex             ; *2
	bne :-          ; *3
			; -1
	plx             ; 4
	dex             ; 2
	bne delay_x_msec; 3
	rts

; Delays n clocks
; Preserved: A, X, Y, P
.macro delay n
	.if n < 2
		.error "Delay must be 2 or more"
	.elseif n = 2
		delay_2
	.elseif n = 3
		delay_3
	.elseif n = 4
		delay_2
		delay_2
	.elseif n = 5
		delay_2
		delay_3
	.elseif n = 6
		delay_3
		delay_3
	.elseif n = 7
		delay_7
	.elseif n = 8
		delay_2
		delay_2
		delay_2
		delay_2
	.elseif n = 9
		delay_9
	.elseif n = 10
		delay_7
		delay_3
	.elseif n = 11
		delay_9
		delay_2
	.elseif n = 12
		delay_9
		delay_3
	.elseif n = 13
		delay_9
		delay_2
		delay_2
	.elseif n = 14
		delay_7
		delay_7
	.elseif n = 15
		delay_9
		delay_2
		delay_2
		delay_2
	.elseif n = 16
		delay_9
		delay_7
	.elseif n = 17
		delay_7
		delay_7
		delay_3
	.elseif n = 18
		delay_9
		delay_9
	.elseif n = 19
		delay_9
		delay_7
		delay_3
	.elseif n = 20
		delay_9
		delay_9
		delay_2
	.elseif n = 21
		delay_7
		delay_7
		delay_7
	.elseif n = 22
		delay_9
		delay_9
		delay_2
		delay_2
	.elseif n = 23
		delay_9
		delay_7
		delay_7
	.elseif n = 24
		delay_7
		delay_7
		delay_7
		delay_3
	.elseif n = 25
		delay_9
		delay_9
		delay_7
	.elseif n = 26
		delay_9
		delay_7
		delay_7
		delay_3
	.elseif n = 27
		delay_9
		delay_9
		delay_9
	.else
		.error "Delay must be 27 or less"
	.endif
.endmacro

; Delays are composed of these
.define delay_2 nop

.define delay_3 sep #0

.macro delay_7
	php
	plp
.endmacro

.macro delay_9
	phd
	pld
.endmacro
