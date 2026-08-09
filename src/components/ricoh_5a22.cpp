#include <sstream>
#include <iomanip>
#include <cstdint>
#include <array>
#include <cstdlib>
#include "ricoh_5a22.hpp"
#include "ricoh_5a22_opcode_info.hpp"
#include "bus.hpp"
#include "dma.hpp"

namespace {
	std::string byte_hex(Byte b) {
		std::ostringstream s;
		s << std::uppercase << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(b);
		return s.str();
	}
	std::string word_hex(Word w) {
		std::ostringstream s;
		s << std::uppercase << std::hex << std::setfill('0') << std::setw(4) << static_cast<int>(w);
		return s.str();
	}
	std::string long_hex(Address a) {
		std::ostringstream s;
		s << std::uppercase << std::hex << std::setfill('0') << std::setw(6) << (a & 0xFFFFFF);
		return s.str();
	}
}

Ricoh5A22::Ricoh5A22(Bus* bus) : bus(bus), cycle(0), instruction_cycle(0) {}

void Ricoh5A22::add_cycles(CycleCount cycles) {
	this->cycle += cycles;
}

CycleCount Ricoh5A22::get_cycle() {
	return this->cycle;
}

TickCount Ricoh5A22::get_tick() {
	return this->tick;
}

void Ricoh5A22::poll_interrupts() {
	if (nmi_line) {
		was_interrupt = false;
		waiting = false;
		nmi_line = false;
		BufferOpCode = OPCODE_NMI;
		instruction_cycle = 0;
		was_interrupt = true;
		interrupt_type = false;
	} else if (irq_line && !get_flag_I()) {
		BufferOpCode = OPCODE_IRQ;
		instruction_cycle = 0;
		was_interrupt = true;
		waiting = false;
		interrupt_type = true;
	}
}

void Ricoh5A22::apply_invariants() {
	if (regs.emulation_mode) {
		regs.X = regs.X & 0x00FF;
		regs.Y = regs.Y & 0x00FF;
		regs.S = (regs.S & 0x00FF) | 0x0100;
	}
}

void Ricoh5A22::tick_multiply_divisor() {
	if (multiplier.half_cycles_since_init || divisor.half_cycles_since_init) {
		if (multiplier.half_cycles_since_init > 0) {
			multiplier.half_cycles_since_init--;
		}
		if (divisor.half_cycles_since_init > 0) {
			divisor.half_cycles_since_init--;
		}
	}
}

void Ricoh5A22::run_half_cycle() {
	if (regs.emulation_mode) { apply_invariants(); }
	//log();
	tick_multiply_divisor();
	if constexpr (SHOW_LOGS) {
		if (instruction_cycle == 0) {
			executed++;
		}
		if (instruction_cycle == 0 && BufferOpCode < 256) {
			if (executed >= 12) {
				executed = 0;
			}
			const OpCodeInfo& info = ricoh_5a22_opcode_info[BufferOpCode];
			std::cout <<
			 std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(regs.PB) << ":" <<
			 std::hex << std::uppercase << std::setw(4) << std::setfill('0') << static_cast<int>(regs.PC) << " ";
			std::cout << info.mnemonic;
			SizeType size_type = info.size_type;

			if (regs.PB == 0x00 && regs.PC == 0x000B) {
				std::abort();
			}

			bool flag = false;
			switch(size_type) {
			case SizeType::Fixed:      flag = false;        break;
			case SizeType::MDependent: flag = get_flag_M(); break;
			case SizeType::XDependent: flag = get_flag_X(); break;
			}

			int size = flag ? info.size_when_set : info.size_when_clear;

			Word trace_pc = regs.PC;
			Byte trace_pb = regs.PB;

			for (int i = 0; i < size - 1; i++) {
				trace_pc++;
				Byte operand = read((trace_pb << 16) | trace_pc);
				std::cout << " " << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(operand);
			}
			
			std::cout
			    << "  "
			    << "A:"  << std::setw(4) << std::setfill('0') << std::hex << std::uppercase << regs.A
			    << " "
			    << "X:"  << std::setw(4) << std::setfill('0') << regs.X
			    << " "
			    << "Y:"  << std::setw(4) << std::setfill('0') << regs.Y
			    << " "
			    << "S:"  << std::setw(4) << std::setfill('0') << regs.S
			    << " "
			    << "D:"  << std::setw(4) << std::setfill('0') << regs.D
			    << " "
			    << "DB:" << std::setw(2) << std::setfill('0') << static_cast<int>(regs.DB)
			    << " "
			    << "P:"  << std::setw(2) << std::setfill('0') << static_cast<int>(regs.P)
			    << " "
			    << "V:"  << std::dec << static_cast<int>(ppu->get_vcounter())
			    << " "
			    << "H:"  << std::dec << static_cast<int>(ppu->get_hcounter());

			std::cout << '\n';
		}
	}
	if (instruction_cycle == 0 && BufferOpCode != 0x100 && BufferOpCode != 0x101) {
		poll_interrupts();
	}
	prev_PC = regs.PC;
	prev_PB = regs.PB;
	Opcode op = get_opcode(regs.emulation_mode ? emulation_optable : native_optable, BufferOpCode, instruction_cycle, *this);
	op.function(*this, op.skipped);
	if (regs.PC != prev_PC && regs.PB != prev_PB) {
		new_operand = true;
	}
}

void Ricoh5A22::tick_component() { // when the component is ticked, it does a half tick in actuality
	tick++;
	if constexpr (!HALF_CYCLES) {
		run_half_cycle();
	}
	run_half_cycle();
	this->add_cycles(RICOH_5A22_CYCLE);
}

void Ricoh5A22::reset() {
	return;
}

void Ricoh5A22::connect_dma(DMA* dma) {
	this->dma = dma;
	this->dma->connect_bus(bus);
}

void Ricoh5A22::initialise() {
	uint8_t lo = read(0x00FFFC);
	uint8_t hi = read(0x00FFFD);

	std::cout << std::hex << "RESET LO: " << (int)lo << std::endl;
	std::cout << std::hex << "RESET HI: " << (int)hi << std::endl;

	regs.PB = 0;
	regs.PC = (hi << 8) | lo;

	regs.P = 0x34;
	regs.S = 0x01FF;
	regs.emulation_mode = true;
	BufferOpCode = read(regs.PC);
	
	mregs.RDNMI = mregs.RDNMI & 0x7F;
	mregs.TIMEUP = mregs.TIMEUP & 0x7F;
	mregs.VTIMEL = 0xFF;
	mregs.VTIMEH = 0x01;
	mregs.HTIMEL = 0xFF;
	mregs.HTIMEH = 0x01;
	mregs.NMITIMEN = 0x00;

	cycle = 0;
	return;
}

void Ricoh5A22::log() {
	std::cout << "PC:PB: " << std::hex << std::setw(2) << std::setfill('0') << (int)regs.PB << ":"
	                       << std::hex << std::setw(4) << std::setfill('0') << (int)regs.PC << " "
	          << "OP: "    << std::hex << std::setw(2) << std::setfill('0') << (int)BufferOpCode << " "
	          << "CYC: "   << std::dec << (int)instruction_cycle
	          << "\n";
}

Byte Ricoh5A22::read(Address addr) {
	return bus->read(addr);
}

void Ricoh5A22::write(Address addr, Byte value) {
	bus->write(addr, value);
}

void Ricoh5A22::enable_test_mode() {
	bus->enable_test_mode();
}

void Ricoh5A22::disable_test_mode() {
	bus->disable_test_mode();
}

void Ricoh5A22::reset_test_memory() {
	bus->reset_test_memory();
}

Byte Ricoh5A22::test_peek(Address addr) {
	return bus->test_peek(addr);
}

void Ricoh5A22::test_poke(Address addr, Byte value) {
	bus->test_poke(addr, value);
}

void Ricoh5A22::set_fastrom_from_bus(bool fastrom_enabled) {
	bus->set_fastrom(fastrom_enabled);
}
