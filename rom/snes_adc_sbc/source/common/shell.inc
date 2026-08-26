; Included at beginning of program

; Devcart
.ifdef BUILD_DEVCART
	.include "build_devcart.s"
.endif

; SMC ROM (default)
.ifndef SHELL_INCLUDED
	.include "build_smc.s"
.endif
