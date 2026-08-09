#pragma once
#include <functional>
#include <type_traits>

#include "component.hpp"


class CPU : public Component {
public:
	
	struct Registers {

		// Both
		Word A = 0;
		Word X = 0;
		Word Y = 0;
		Word PC = 0;
		Word S = 0;
		Byte P = 0;

		// Ricoh 5A22

		Word D = 0;
		Byte DB = 0;
		Byte PB = 0;
		bool emulation_mode = false;
		
	};

	Registers regs;

	Word BufferOpCode = 0;
	Word BufferPointer = 0;
	Word BufferAddress = 0;
	Word BufferOperand = 0;
	Word BufferOperand16 = 0; // Additional operand buffer just for 16-bit ops in SPC-700
	Word BufferOperand0 = 0;
	Word BufferOperand1 = 0;
	Word BufferBank = 0;
	Word BufferOrig = 0;
	Word BufferMVDest = 0;
	Word BufferStackAddress = 0;
	Word BufferTmp  = 0;
	Word BufferTmpR = 0;
	Word BufferUnderflow = 0;
	Word BufferOverflow = 0;
	Word BufferLowZero = 0;
	Word Vector = 0;

	Word YABuffer = 0; // just a buffer, does not give actual value of YA for SPC-700
	Word BufferJump = 0;

	uint32_t DivYa = 0;
	uint32_t ShiftedX = 0;

	Word TransferCount = 0;

	Word DiscardBuffer = 0;

	bool condition = 0;
	bool waiting = false;
	bool Branching = 0;
	bool BoundaryCrossed = 0;

	Byte open_bus = 0x00;

	Byte get_open_bus() {
		return open_bus;
	}

	void set_open_bus(Byte value) {
		open_bus = value;
	}

	virtual void apply_invariants() = 0;
	virtual void poll_interrupts() = 0;

	bool get_flag_N() = delete;
	bool get_flag_V() = delete;
	bool get_flag_M() = delete;
	bool get_flag_X() = delete;
	bool get_flag_D() = delete;
	bool get_flag_I() = delete;
	bool get_flag_Z() = delete;
	bool get_flag_C() = delete;
	bool get_flag_P() = delete;
	bool get_flag_H() = delete;
	bool get_flag_B() = delete;
	
	void set_flag_N(Byte value) = delete;
	void set_flag_V(Byte value) = delete;
	void set_flag_M(Byte value) = delete;
	void set_flag_X(Byte value) = delete;
	void set_flag_D(Byte value) = delete;
	void set_flag_I(Byte value) = delete;
	void set_flag_Z(Word value) = delete;
	void set_flag_C(Byte value) = delete;
	void set_flag_P(Byte value) = delete;
	void set_flag_H(Byte value) = delete;
	void set_flag_B(Byte value) = delete;

	void set_flag_N() = delete;
	void set_flag_V() = delete;
	void set_flag_M() = delete;
	void set_flag_X() = delete;
	void set_flag_D() = delete;
	void set_flag_I() = delete;
	void set_flag_Z() = delete;
	void set_flag_C() = delete;
	void set_flag_P() = delete;
	void set_flag_H() = delete;
	void set_flag_B() = delete;

	void clear_flag_N() = delete;
	void clear_flag_V() = delete;
	void clear_flag_M() = delete;
	void clear_flag_X() = delete;
	void clear_flag_D() = delete;
	void clear_flag_I() = delete;
	void clear_flag_Z() = delete;
	void clear_flag_C() = delete;
	void clear_flag_P() = delete;
	void clear_flag_H() = delete;
	void clear_flag_B() = delete;
	
	virtual void enable_test_mode() = 0;
	virtual void disable_test_mode() = 0;
	virtual void reset_test_memory() = 0;
	virtual Byte test_peek(Address addr) = 0;
	virtual void test_poke(Address addr, Byte value) = 0;
};