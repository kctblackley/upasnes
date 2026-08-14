#include "spc_700.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <sstream>
#include <iomanip>
#include <cstdint>

#include "logging_options.hpp"

namespace {

	std::string byte_hex(Byte b) {
		std::ostringstream s;
		s << std::uppercase
		  << std::hex
		  << std::setfill('0')
		  << std::setw(2)
		  << static_cast<unsigned>(b);
		return s.str();
	}

	std::string word_hex(Word w) {
		std::ostringstream s;
		s << std::uppercase
		  << std::hex
		  << std::setfill('0')
		  << std::setw(4)
		  << static_cast<unsigned>(w);
		return s.str();
	}

}

Byte SPC700::trace_read(Address addr) const
{
	addr &= 0xFFFF;

	if (addr >= 0xFD && addr <= 0xFF) {
		// Timer output registers are destructive on a real read.
		// Return their current value without clearing them.
		return timers[addr - 0xFD].output;
	}

	if (addr >= 0xF4 && addr <= 0xF7) {
		return cpu_to_spc_ports[addr - 0xF4];
	}

	if (ipl_rom_enabled && addr >= 0xFFC0) {
		return ipl_rom[addr - 0xFFC0];
	}

	return bus->read(addr);
}

std::string SPC700::trace_operands(Byte opcode, Address pc) const
{
	const SPC700OpCodeInfo& info = spc700_opcode_info[opcode];

	std::ostringstream out;

	out << std::uppercase
	    << std::hex
	    << std::setfill('0');

	// The opcode itself is byte 0, so only print the bytes following it.
	for (unsigned i = 1; i < info.size; ++i) {
		Address operand_pc =
			static_cast<Address>((pc + i) & 0xFFFF);

		Byte operand = trace_read(operand_pc);

		out << " "
		    << std::setw(2)
		    << static_cast<unsigned>(operand);
	}

	return out.str();
}

void SPC700::log_instruction()
{
	if (BufferOpCode >= 256) {
		return;
	}

	const Byte opcode =
		static_cast<Byte>(BufferOpCode);

	const SPC700OpCodeInfo& info =
		spc700_opcode_info[opcode];

	std::cout
		<< "[SPC700] "

		// PC
		<< std::uppercase
		<< std::hex
		<< std::setfill('0')
		<< std::setw(4)
		<< static_cast<unsigned>(regs.PC)

		<< " "

		// Opcode + mnemonic
		<< std::setw(2)
		<< static_cast<unsigned>(opcode)
		<< " "
		<< info.mnemonic;

	// Operand bytes
	std::cout << trace_operands(opcode, regs.PC);

	// Registers
	std::cout
		<< "  "
		<< "A:"
		<< std::setw(2)
		<< static_cast<unsigned>(
			static_cast<Byte>(regs.A))

		<< " "
		<< "X:"
		<< std::setw(2)
		<< static_cast<unsigned>(
			static_cast<Byte>(regs.X))

		<< " "
		<< "Y:"
		<< std::setw(2)
		<< static_cast<unsigned>(
			static_cast<Byte>(regs.Y))

		<< " "
		<< "S:"
		<< std::setw(2)
		<< static_cast<unsigned>(
			static_cast<Byte>(regs.S))

		<< " "
		<< "P:"
		<< std::setw(2)
		<< static_cast<unsigned>(regs.P);

	// SPC <-> SNES communication ports
	std::cout
		<< " "
		<< "C2S(F4-F7):"
		<< std::setw(2)
		<< static_cast<unsigned>(cpu_to_spc_ports[0])
		<< ","
		<< std::setw(2)
		<< static_cast<unsigned>(cpu_to_spc_ports[1])
		<< ","
		<< std::setw(2)
		<< static_cast<unsigned>(cpu_to_spc_ports[2])
		<< ","
		<< std::setw(2)
		<< static_cast<unsigned>(cpu_to_spc_ports[3])

		<< " "
		<< "S2C(F4-F7):"
		<< std::setw(2)
		<< static_cast<unsigned>(spc_to_cpu_ports[0])
		<< ","
		<< std::setw(2)
		<< static_cast<unsigned>(spc_to_cpu_ports[1])
		<< ","
		<< std::setw(2)
		<< static_cast<unsigned>(spc_to_cpu_ports[2])
		<< ","
		<< std::setw(2)
		<< static_cast<unsigned>(spc_to_cpu_ports[3]);

	std::cout << '\n';
}

SPC700::SPC700() : cycle(0), instruction_cycle(0) {
	std::cout << "Loading SPC700 IPL ROM\n";

	std::ifstream file("ipl/ipl.rom", std::ios::binary);

	if (!file) {
		std::cerr << "Missing IPL ROM.\n";
		std::cerr << "Please provide the IPL ROM, named 'ipl.rom', in the folder 'ipl'.\n";
		std::cerr << "Refer to README.md for instructions.\n";
		std::exit(EXIT_FAILURE);
	}

	file.read(
		reinterpret_cast<char*>(ipl_rom.data()),
		ipl_rom.size()
	);

	if (file.gcount() != 64) {
		std::cerr << "The IPL ROM provided is of the incorrect size.\n";
		std::cerr << "It must be 64 bytes in size to be valid.\n";
		std::cerr << "Refer to README.md for instructions.\n";
		std::exit(EXIT_FAILURE);
	}

	std::cout << "Loaded SPC700 IPL ROM\n";

	bus = std::make_unique<APUBus>();
	initialise();
}

void SPC700::add_cycles(CycleCount cycles) {
	this->cycle += cycles;
}

TickCount SPC700::get_tick() {
	return this->tick;
}

void SPC700::poll_interrupts() {
	return;
}

void SPC700::apply_invariants() {
	return;
}

void SPC700::run_half_cycle() {
	Opcode op = get_opcode(optable, BufferOpCode, instruction_cycle, *this);
	op.function(*this, op.skipped);
}

void SPC700::accumulate_dsp(CycleCount delta) {
	dsp_accumulated_cycles += delta;
	while (dsp_accumulated_cycles > SDSP_CYCLE_CONSTANT) {
		sdsp_ticks_this_frame++;
		bus->tick_sdsp();
		dsp_accumulated_cycles -= SDSP_CYCLE_CONSTANT;
	}
}

void SPC700::log_spc() {
	if constexpr (SHOW_LOGS) {
		if (instruction_cycle == 0 && BufferOpCode < 256) {
			log_instruction();
		}
	}
}

void SPC700::tick_component() { // when the component is ticked, it does a half tick in actuality
	if (first_tick) {
		log_spc();
		first_tick = false;
	}
	if constexpr (HALF_CYCLES) {
		if (tick & 1) {
			tick_timer(0, 128);
			tick_timer(1, 128);
			tick_timer(2, 16);
		}
		tick++;
		run_half_cycle();
		master_cycle += SPC_700_CYCLE_CONSTANT / 2.00f;
	} else {
		tick_timer(0, 128);
		tick_timer(1, 128);
		tick_timer(2, 16);
		tick++;
		run_half_cycle();
		run_half_cycle();
		master_cycle += SPC_700_CYCLE_CONSTANT;
	}
}

CycleCount SPC700::get_cycle() {
	return static_cast<CycleCount>(master_cycle);
}

void SPC700::reset() { // RUN IPL ROM HERE! MEMORY MAP THE IPL ROM!
	regs.A = 0x00;
	regs.X = 0x00;
	regs.Y = 0x00;
	regs.S = 0xEF;
	regs.P = 0x00;
	regs.PC = 0xFFC0;
	master_cycle = 0;

	for (SPCTimer& timer : timers) {
		timer = SPCTimer{};
	}

	ipl_rom_enabled = true;

	BufferOpCode = read(regs.PC);
	write(0xF0, 0x0A);
	spc_to_cpu_ports[0] = 0xAA;
	spc_to_cpu_ports[1] = 0xBB;

	while (regs.PC != 0xFFC5 || instruction_cycle != 0) {
		tick_component();
	}

	master_cycle = 0;
}

void SPC700::initialise() {
	reset();
}