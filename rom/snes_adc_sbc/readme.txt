SNES ADC/SBC tests
------------------
These tests verify 8-bit and 16-bit ADC and SBC operation for binary and
decimal modes. Each takes about 15 seconds.

Each tests with the indicated flags over all 65536 combinations of A and
the operand (for 16-bit versions, only interesting combinations), and
verifies the resulting A and flags (all 8 flags). The final test has
various other flags set which shouldn't affect operation, just in case
an emulator incorrectly uses them.

Included in source/ are C++ implementations of the operations that pass
the above tests.


Building with ca65
------------------
To assemble a test ROM with ca65, use the following commands:

	ca65 --cpu 65816 -I common -o test.o source_filename_here.s
	ld65 -C smc.cfg test.o -o test.smc

-- 
Shay Green <gblargg@gmail.com>
