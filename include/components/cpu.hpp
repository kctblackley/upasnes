#pragma once
#include <functional>
#include <type_traits>

#include "component.hpp"


class CPU : public Component {
public:
	
	struct Registers {

		// Both
		u16 A = 0;
		u16 X = 0;
		u16 Y = 0;
		u16 PC = 0;
		u16 S = 0;
		u8 P = 0;

		// Ricoh 5A22

		u16 D = 0;
		u8 DB = 0;
		u8 PB = 0;
		bool emulation_mode = false;
		
	};

	Registers regs;

	u16 BufferOpCode = 0;
	u16 BufferPointer = 0;
	u16 BufferAddress = 0;
	u16 BufferOperand = 0;
	u16 BufferOperand16 = 0; // Additional operand buffer just for 16-bit ops in SPC-700
	u16 BufferOperand0 = 0;
	u16 BufferOperand1 = 0;
	u16 BufferBank = 0;
	u16 BufferOrig = 0;
	u16 BufferMVDest = 0;
	u16 BufferStackAddress = 0;
	u16 BufferTmp  = 0;
	u16 BufferTmpR = 0;
	u16 BufferUnderflow = 0;
	u16 BufferOverflow = 0;
	u16 BufferLowZero = 0;
	u16 Vector = 0;

	u16 YABuffer = 0; // just a buffer, does not give actual value of YA for SPC-700
	u16 BufferJump = 0;

	u32 DivYa = 0;
	u32 ShiftedX = 0;

	u16 TransferCount = 0;

	u16 DiscardBuffer = 0;

	bool condition = 0;
	bool waiting = false;
	bool Branching = 0;
	bool BoundaryCrossed = 0;

	u8 open_bus = 0x00;

	u8 get_open_bus() {
		return open_bus;
	}

	void set_open_bus(u8 value) {
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
	
	void set_flag_N(u8 value) = delete;
	void set_flag_V(u8 value) = delete;
	void set_flag_M(u8 value) = delete;
	void set_flag_X(u8 value) = delete;
	void set_flag_D(u8 value) = delete;
	void set_flag_I(u8 value) = delete;
	void set_flag_Z(u16 value) = delete;
	void set_flag_C(u8 value) = delete;
	void set_flag_P(u8 value) = delete;
	void set_flag_H(u8 value) = delete;
	void set_flag_B(u8 value) = delete;

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
	virtual u8 test_peek(u32 addr) = 0;
	virtual void test_poke(u32 addr, u8 value) = 0;
};