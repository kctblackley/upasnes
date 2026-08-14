#pragma once
#include <string>
#include <iostream>
#include <array>
#include "cpu.hpp"
#include "ricoh_5a22_native_optable.hpp"
#include "ricoh_5a22_emulation_optable.hpp"
#include "renderer.hpp"
#include "ppu.hpp"
#include "ricoh_5a22_opcode_info.hpp"

// Multiplier addresses
#define WRMPYA_ADDRESS 0x4202
#define WRMPYB_ADDRESS 0x4203
#define RDMPYL_ADDRESS 0x4216
#define RDMPYH_ADDRESS 0x4217

#define MULTIPLIER_HALF_CYCLES 16

// Division addresses
#define WRDIVL_ADDRESS 0x4204
#define WRDIVH_ADDRESS 0x4205
#define WRDIVB_ADDRESS 0x4206
#define RDDIVL_ADDRESS 0x4214
#define RDDIVH_ADDRESS 0x4215

#define DIVISOR_HALF_CYCLES 32

// CPU register addresses
#define HVBJOY_ADDRESS 0x4212
#define JOY1L_ADDRESS 0x4218
#define JOY1H_ADDRESS 0x4219
#define WRIO_ADDRESS 0x4201
#define RDIO_ADDRESS 0x4213

// Interrupts
#define OPCODE_NMI 0x100
#define OPCODE_IRQ 0x101
#define NMITIMEN_ADDRESS 0x4200
#define HTIMEL_ADDRESS 0x4207
#define HTIMEH_ADDRESS 0x4208
#define VTIMEL_ADDRESS 0x4209
#define VTIMEH_ADDRESS 0x420A
#define MEMSEL_ADDRESS 0x420D
#define RDNMI_ADDRESS 0x4210
#define TIMEUP_ADDRESS 0x4211

class Bus;
class DMA;

// The Multiplier's RDMPYH and RDMPYL also store the remainder of division by the divisor
// Multiplication takes 16 half-cycles of the CPU (not master clock)
struct Multiplier {
	Byte WRMPYA, WRMPYB; // first and second number to multiply, writing to WRDMPYB starts multiplication
	Byte RDMPYH, RDMPYL; // read result of multiplication
	CycleCount half_cycles_since_init = 0; // Only when this is 0 can 
	bool completed = true;
};

// Division takes 32 half-cycles of the CPU (not master clock)
struct Divisor {
	Byte WRDIVH, WRDIVL; // dividend
	Byte WRDIVB; // divisor (starts division)
	Byte RDDIVH, RDDIVL; // read result of division
	CycleCount half_cycles_since_init = 0;
	bool completed = true;
};

// Adding interrupt registers
struct CPURegisters {
	Byte HVBJOY;
	Byte NMITIMEN;
	Byte HTIMEL, HTIMEH;
	Byte VTIMEL, VTIMEH;
	Byte RDNMI, TIMEUP;
	Byte WRIO = 0xFF; // Programmable I/O port; bit 7 doubles as the PPU counter latch line
};

class Ricoh5A22 final : public CPU {
public:
	explicit Ricoh5A22(Bus* bus);

	void add_cycles(CycleCount cycles) override;

	void run_half_cycle();
	void tick_cpu();
	void tick_component() override;
	CycleCount get_cycle() override;
	TickCount get_tick() override;

	void reset() override;
	void initialise() override;

	Byte read(Address addr) override;

	void write(Address addr, Byte value) override;

	// Handles CPU ports!
	Byte communication_read(SNESAddress addr) override {
		// Multiplier
		if (addr.offset == RDMPYL_ADDRESS) { return multiplier.RDMPYL; }
		if (addr.offset == RDMPYH_ADDRESS) { return multiplier.RDMPYH; }
		
		// Divisor
		if (addr.offset == RDDIVL_ADDRESS) { return divisor.RDDIVL; }
		if (addr.offset == RDDIVH_ADDRESS) { return divisor.RDDIVH; }

		// Interrupt handling
		if (addr.offset == RDNMI_ADDRESS) {
			Byte value = mregs.RDNMI;
			mregs.RDNMI = mregs.RDNMI & 0x7F;
			nmi_line = false;
			return value;
		}
		if (addr.offset == TIMEUP_ADDRESS) {
			Byte value = mregs.TIMEUP;
			mregs.TIMEUP = mregs.TIMEUP & 0x7F;
			irq_line = false;
			return value;
		}


		if (addr.offset == HVBJOY_ADDRESS) {
			//std::cout << "READING HVBJOY AS " << std::hex << mregs.HVBJOY << "\n" ;
			return mregs.HVBJOY;
		}
		
		if (addr.offset == JOY1L_ADDRESS || addr.offset == JOY1H_ADDRESS) {
			return renderer->get_joypad(addr.offset);
		}

		if (addr.offset == RDIO_ADDRESS) {
			return mregs.WRIO;
		}

		// Miscellaneous
		if (addr.offset == 0x4016) {
			return 0x01;
		}



		return 0x00;
	}

	void set_fastrom_from_bus(bool fastrom_enabled);

	void communication_write(SNESAddress addr, Byte value) override {
		if (addr.offset == MEMSEL_ADDRESS) {
			fastrom_enabled = value & 1;
			set_fastrom_from_bus(fastrom_enabled);
		}

		if (addr.offset == WRIO_ADDRESS) {
			bool old_bit7 = mregs.WRIO & 0x80;
			bool new_bit7 = value & 0x80;
			if (old_bit7 && !new_bit7 && ppu) {
				ppu->latch_hv_counters();
			}
			mregs.WRIO = value;
		}

		// Multiplier
		if (addr.offset == WRMPYA_ADDRESS) { multiplier.WRMPYA = value; }
		if (addr.offset == WRMPYB_ADDRESS) { multiplier.WRMPYB = value; }
			
		if (addr.offset == WRMPYB_ADDRESS) {
			uint16_t result = multiplier.WRMPYA * multiplier.WRMPYB;
			multiplier.RDMPYL = get_lo(result);
			multiplier.RDMPYH = get_hi(result);

			multiplier.half_cycles_since_init = MULTIPLIER_HALF_CYCLES;
			multiplier.completed = true;
		}

		// Divisor
		if (addr.offset == WRDIVH_ADDRESS) { divisor.WRDIVH = value; }
		if (addr.offset == WRDIVL_ADDRESS) { divisor.WRDIVL = value; }
		if (addr.offset == WRDIVB_ADDRESS) { divisor.WRDIVB = value; }

		if (addr.offset == WRDIVB_ADDRESS) {
			uint16_t dividend_value = (divisor.WRDIVH << 8) | divisor.WRDIVL;
			uint8_t divisor_value = divisor.WRDIVB;
			uint16_t result = 0xFFFF;
			uint16_t remainder = dividend_value;
			if (divisor_value != 0) {
				result = (unsigned int)(dividend_value) / (unsigned int)(divisor_value);
				remainder = dividend_value % divisor_value;
			}

			divisor.RDDIVL = get_lo(result);
			divisor.RDDIVH = get_hi(result);
			multiplier.RDMPYL = get_lo(remainder);
			multiplier.RDMPYH = get_hi(remainder);

			divisor.half_cycles_since_init = DIVISOR_HALF_CYCLES;
			divisor.completed = true;
		}

		// Interrupts
		if (addr.offset == NMITIMEN_ADDRESS) {
			Byte irq_bits_before = mregs.NMITIMEN & 0x30;
			nmi_enabled = (((value >> 7) & 0b1) == 1);
			auto_read_enabled = ((value & 0b1) == 1);

			this->irq_mode = (value >> 4) & 3;
			ppu->irq_mode = this->irq_mode;
			mregs.NMITIMEN = value;
			
			if (irq_bits_before != 0 && (value & 0x30) == 0) {
				mregs.TIMEUP = mregs.TIMEUP & ~0x80;
				irq_line = false;
			}
		}
		if (addr.offset == HTIMEL_ADDRESS) {
			mregs.HTIMEL = value;
			ppu->h_time_target = (mregs.HTIMEH << 8) | mregs.HTIMEL;
		}
		if (addr.offset == HTIMEH_ADDRESS) {
			mregs.HTIMEH = value;
			ppu->h_time_target = (mregs.HTIMEH << 8) | mregs.HTIMEL;
		}
		if (addr.offset == VTIMEL_ADDRESS) {
			mregs.VTIMEL = value;
			ppu->v_time_target = (mregs.VTIMEH << 8) | mregs.VTIMEL;
		}
		if (addr.offset == VTIMEH_ADDRESS) {
			mregs.VTIMEH = value;
			ppu->v_time_target = (mregs.VTIMEH << 8) | mregs.VTIMEL;
		}
	}

	void signal_nmi_start() {
		mregs.RDNMI = mregs.RDNMI | 0x80;
		if (nmi_enabled) {
			nmi_line = true;
		}
	}

	void signal_nmi_end() {
		mregs.RDNMI = 0x00;
	}

	void signal_irq() {
		mregs.TIMEUP = mregs.TIMEUP | 0x80;
		irq_line = true;
	}

	void set_hvbjoy_flag(Byte bit_mask, bool set) {
		if (set) {
			mregs.HVBJOY = mregs.HVBJOY | bit_mask;
		} else {
			mregs.HVBJOY = mregs.HVBJOY & ~bit_mask;
		}
	}

	void tick_multiply_divisor();

	void apply_invariants() override;

	void poll_interrupts() override;

	bool interrupt_pending() {
	    return nmi_line || (irq_line && !get_flag_I());
	}

	bool get_flag_N() { return (regs.P >> 7) & 0b1; }
	bool get_flag_V() { return (regs.P >> 6) & 0b1; }
	bool get_flag_M() { return regs.emulation_mode ? 0b1 : (regs.P >> 5) & 0b1; }
 	bool get_flag_X() { return regs.emulation_mode ? 0b1 : (regs.P >> 4) & 0b1; }
	bool get_flag_D() { return (regs.P >> 3) & 0b1; }
	bool get_flag_I() { return (regs.P >> 2) & 0b1; } 
	bool get_flag_Z() { return (regs.P >> 1) & 0b1;  }
	bool get_flag_C() { return  regs.P       & 0b1;  }

	void set_flag_N(Byte value) {
		condition = ( ( (value >> 7) & 0b1 ) == 1);
		regs.P = condition ? set_bit(regs.P, 7) : clear_bit(regs.P, 7); 
	}

	void set_flag_V(Byte value) { return; }
	void set_flag_M(Byte value) { return; }
	void set_flag_X(Byte value) { return; }
	void set_flag_D(Byte value) { return; }
	void set_flag_I(Byte value) { return; }

	void set_flag_Z(Word value) {
		condition = (value == 0);
		regs.P = condition ? set_bit(regs.P, 1) : clear_bit(regs.P, 1); 
	}

	void set_flag_C(Byte value) { return; }

	void set_flag_N() { regs.P = set_bit(regs.P, 7); }
	void set_flag_V() { regs.P = set_bit(regs.P, 6); }
	void set_flag_M() { regs.P = set_bit(regs.P, 5); }
	void set_flag_X() { regs.P = set_bit(regs.P, 4); }
	void set_flag_D() { regs.P = set_bit(regs.P, 3); }
	void set_flag_I() { regs.P = set_bit(regs.P, 2); }
	void set_flag_Z() { regs.P = set_bit(regs.P, 1); }
	void set_flag_C() { regs.P = set_bit(regs.P, 0); }

	void clear_flag_N() { regs.P = clear_bit(regs.P, 7); }
 	void clear_flag_V() { regs.P = clear_bit(regs.P, 6); }
	void clear_flag_M() { regs.P = clear_bit(regs.P, 5); }
	void clear_flag_X() { regs.P = clear_bit(regs.P, 4); }
	void clear_flag_D() { regs.P = clear_bit(regs.P, 3); }
	void clear_flag_I() { regs.P = clear_bit(regs.P, 2); }
	void clear_flag_Z() { regs.P = clear_bit(regs.P, 1); }
	void clear_flag_C() { regs.P = clear_bit(regs.P, 0); }

	// Unused flags (SPC700 only)
	bool get_flag_P() { return false; }
	bool get_flag_H() { return false; }
	bool get_flag_B() { return false; }

	void set_flag_P(Byte value) { return; }
	void set_flag_H(Byte value) { return; }
	void set_flag_B(Byte value) { return; }

	void set_flag_P() { return; }
	void set_flag_H() { return; }
	void set_flag_B() { return; }

	void clear_flag_P() { return; }
	void clear_flag_H() { return; }
	void clear_flag_B() { return; }

	void enable_test_mode();
	void disable_test_mode();
	void reset_test_memory();
	Byte test_peek(Address addr);
	void test_poke(Address addr, Byte value);

	void connect_renderer(Renderer* renderer) {
		this->renderer = renderer;
	}
	void connect_ppu(PPU* ppu) {
		this->ppu = ppu;
	}

	void connect_dma(DMA* dma);

	void log();

	CycleCount cycle;

private:
	
	Bus* bus = nullptr;

	CycleCount instruction_cycle; 
	TickCount tick;

	Multiplier multiplier;
	Divisor divisor;

	CPURegisters mregs; // memory-mapped registers

	Renderer* renderer = nullptr;
	PPU* ppu = nullptr;
	DMA* dma = nullptr;

	Byte irq_mode;
	bool nmi_line = false;
	bool irq_line = false;
	bool nmi_enabled = false;
	bool auto_read_enabled = false;
	bool fastrom_enabled = false;

	// For disassembler
	bool was_interrupt = false;
	bool interrupt_type = false; // false = NMI, true = IRQ
	Word prev_PC = 0;
	Byte prev_PB = 0;
	Byte prev_opcode = 0;

	bool new_operand = false;

	int executed = 0;
};