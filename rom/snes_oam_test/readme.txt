Tests pass on original and mini SNES units.

Asm uses the following:

mov addr, #immediate
	
	lda #immediate
	sta addr

movw addr,#immediate

	lda #<immediate
	sta addr
	lda #>immediate
	sta addr+1

set_test code, text

	Saves code and text for printing if test fails.

Jcc addr

	Like Bcc, but can jump anywhere if condition holds.

test_failed

	Reports failure and prints most recent set_test code and text.

check_oam addr, data

	Verifies that OAM matches data, starting at addr.
