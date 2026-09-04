#pragma once
#include "cpu.hpp"
#include "spc_700_optable.hpp"
#include "spc_700_opcode_info.hpp"
#include "common.hpp"
#include "apubus.hpp"

#define SPC_700_CYCLE_CONSTANT 20.9738991477
#define SDSP_CYCLE_CONSTANT (SPC_700_CYCLE_CONSTANT * /*128.0*/ 32.0)

enum class APUStubState {
	WaitForCC,
	Transfer
};

struct SPCTimer {
	bool enabled = false;

	Byte target = 0;
	Byte output = 0;

	uint16_t divider_counter = 0;
	uint16_t internal_counter = 0;
};

class SPC700 final : public CPU {
public:
	SPC700();

	void add_cycles(CycleCount cycles) override;

	void run_half_cycle();
	void accumulate_dsp(CycleCount delta);
	void tick_component() override;
	CycleCount get_cycle() override;
	TickCount get_tick() override;

	void reset() override;
	void initialise() override;
	
	Byte communication_read(SNESAddress addr) override {
		return spc_to_cpu_ports[addr.offset & 3];
	}

	void communication_write(SNESAddress addr, Byte value) override {
		cpu_to_spc_ports[addr.offset & 3] = value;
	}

	void close_audio() {
		bus->close_audio();
	}

	Byte read(Address addr) override {
		cycle++;
		if (addr >= 0xFD && addr <= 0xFF) {
			Byte value = timers[addr - 0xFD].output;
			timers[addr - 0xFD].output = 0;
			return value;
		}
		if (addr >= 0xF4 && addr <= 0xF7) {
			return cpu_to_spc_ports[addr - 0xF4];
		}
		if (ipl_rom_enabled && addr >= 0xFFC0) {
			return ipl_rom[addr - 0xFFC0];
		}
		return bus->read(addr);
	}

	void write(Address addr, Byte value) override {
		cycle++;
		if (addr >= 0xFA && addr <= 0xFC) {
			timers[addr - 0xFA].target = value;
			bus->write(addr, value);
			return;
		}
		if (addr == 0xF1) {
			int count = 0;
			for (auto& t : timers) {
				bool old = t.enabled;
				t.enabled = value & (0b1 << count);
				if (!old && t.enabled) {
					t.output = 0;
					t.internal_counter = 0;
				}
				count++;
			}

		    ipl_rom_enabled = value & 0x80;
		    
		    if (value & 0x10) {
		    	cpu_to_spc_ports[0] = 0;
		    	cpu_to_spc_ports[1] = 0;
		    }
		    if (value & 0x20) {
		    	cpu_to_spc_ports[2] = 0;
		    	cpu_to_spc_ports[3] = 0;
		    }

		    bus->write(addr, value);
		    return;
		}
		if (addr >= 0xF4 && addr <= 0xF7) {
			spc_to_cpu_ports[addr - 0xF4] = value;
			bus->write(addr, value);
		   	return;
		}
		bus->write(addr, value);
	}

	void tick_timer(int index, int divider) {
	    SPCTimer& timer = timers[index];

	    timer.divider_counter++;
	    if (timer.divider_counter < divider) {
	        return;
	    }
	    timer.divider_counter -= divider;   // see note below on -= vs = 0

	    if (!timer.enabled) {
	        return;   // only the counter/output stage freezes when disabled
	    }

	    timer.internal_counter++;

	    uint16_t target = (timer.target == 0) ? 256 : timer.target;

	    if (timer.internal_counter >= target) {
	        timer.internal_counter = 0;
	        timer.output = (timer.output + 1) & 0x0F;
	    }
	}

	void apply_invariants() override;

	void poll_interrupts() override;

	// Not all correct, check later when I am implementing the APU!
	bool get_flag_N() { return (regs.P >> 7) & 0b1; }
	bool get_flag_V() { return (regs.P >> 6) & 0b1; }
	bool get_flag_P() { return (regs.P >> 5) & 0b1; }
 	bool get_flag_X() { return (regs.P >> 4) & 0b1; }
	bool get_flag_H() { return (regs.P >> 3) & 0b1; }
	bool get_flag_I() { return (regs.P >> 2) & 0b1; } 
	bool get_flag_Z() { return (regs.P >> 1) & 0b1;  }
	bool get_flag_C() { return  regs.P       & 0b1;  }

	void set_flag_N(Byte value) {
		condition = ( ( (value >> 7) & 0b1 ) == 1);
		regs.P = condition ? set_bit(regs.P, 7) : clear_bit(regs.P, 7); 
	}

	void set_flag_V(Byte value) { return; }
	void set_flag_P(Byte value) { return; }
	void set_flag_X(Byte value) { return; }
	void set_flag_H(Byte value) { return; }
	void set_flag_I(Byte value) { return; }

	void set_flag_Z(Word value) {
		condition = (value == 0);
		regs.P = condition ? set_bit(regs.P, 1) : clear_bit(regs.P, 1); 
	}

	void set_flag_C(Byte value) { return; }

	void set_flag_N() { regs.P = set_bit(regs.P, 7); }
	void set_flag_V() { regs.P = set_bit(regs.P, 6); }
	void set_flag_P() { regs.P = set_bit(regs.P, 5); }
	void set_flag_X() { regs.P = set_bit(regs.P, 4); }
	void set_flag_H() { regs.P = set_bit(regs.P, 3); }
	void set_flag_I() { regs.P = set_bit(regs.P, 2); }
	void set_flag_Z() { regs.P = set_bit(regs.P, 1); }
	void set_flag_C() { regs.P = set_bit(regs.P, 0); }

	void clear_flag_N() { regs.P = clear_bit(regs.P, 7); }
 	void clear_flag_V() { regs.P = clear_bit(regs.P, 6); }
	void clear_flag_P() { regs.P = clear_bit(regs.P, 5); }
	void clear_flag_X() { regs.P = clear_bit(regs.P, 4); }
	void clear_flag_H() { regs.P = clear_bit(regs.P, 3); }
	void clear_flag_I() { regs.P = clear_bit(regs.P, 2); }
	void clear_flag_Z() { regs.P = clear_bit(regs.P, 1); }
	void clear_flag_C() { regs.P = clear_bit(regs.P, 0); }

	// Unused flags (SPC700 only)
	bool get_flag_M() { return false; }
	bool get_flag_D() { return false; }
	bool get_flag_B() { return false; }

	void set_flag_M(Byte value) { return; }
	void set_flag_D(Byte value) { return; }
	void set_flag_B(Byte value) { return; }

	void set_flag_M() { return; }
	void set_flag_D() { return; }
	void set_flag_B() { return; }

	void clear_flag_M() { return; }
	void clear_flag_D() { return; }
	void clear_flag_B() { return; }



	void enable_test_mode() override {
		bus->enable_test_mode();
	}

	void disable_test_mode() override {
		bus->disable_test_mode();
	}

	void reset_test_memory() override {
		bus->reset_test_memory();
	}

	Byte test_peek(Address addr) override {
		return bus->read(addr);
	}

	void test_poke(Address addr, Byte value) override {
		bus->write(addr, value);
	}

	bool audio_buffer_above_half() {
		return bus->sdsp_above_half();
	}

	size_t audio_buffer_size() {
		return bus->audio_buffer_size();
	}

	int sdsp_ticks_this_frame = 0;

	void log_instruction();

	void log_spc();

private:
	Byte trace_read(Address addr) const;
	std::string trace_operands(Byte opcode, Address pc) const;

	std::unique_ptr<APUBus> bus;

	Byte cpu_to_spc_ports[4] {};
	Byte spc_to_cpu_ports[4] {};

	CycleCount cycle; 
	CycleCount instruction_cycle; 
	TickCount tick;

	double master_cycle = 0;

	bool ipl_rom_enabled = false;

	std::array<Byte, 64> ipl_rom {};
	SPCTimer timers[3];

	int delay_cycles = 0;

	bool first_tick = true;

	double dsp_accumulated_cycles = 0;
};