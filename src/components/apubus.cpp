#include "apubus.hpp"
#include <array>
#include <cstdint>
#include <fstream>
#include <stdexcept>

APUBus::APUBus() {
	sdsp.connect_bus_to_sdsp(this);

	std::ifstream file("ipl/ipl.rom", std::ios::binary);
	if (!file) {
		throw std::runtime_error("Dump of IPL ROM is required for emulation to begin. Please provide the IPL ROM in the 'ipl' folder under the name 'ipl.rom'.");
	}

	file.read(reinterpret_cast<char*>(ipl.data()), ipl.size());

	if (file.gcount() != static_cast<std::streamsize>(ipl.size())) {
		throw std::runtime_error("Invalid IPL ROM provided. Must be 64 bytes in size.");
	}
}

u8 APUBus::read(u16 address) {
	u8 fetched = data[address];
	if (ipl_enabled && address >= 0xFFC0) {
		fetched = ipl[address - 0xFFC0];
	}

	if (address == 0xF2) {
		fetched = dsp_address;
	}
	if (address == 0xF3) {
		fetched = sdsp.read(dsp_address);
	}

	return fetched;
}

void APUBus::write(u16 address, u8 value) {
	if (address == 0xF2) {
		dsp_address = value;
		return;
	}
	if (address == 0xF3) {
		sdsp.write(dsp_address, value);
		return;
	}

	data[address] = value;
}

void APUBus::enable_ipl()  { ipl_enabled = true; }
void APUBus::disable_ipl() { ipl_enabled = false; }

void APUBus::enable_test_mode()  { disable_ipl(); }
void APUBus::disable_test_mode() { return; }

void APUBus::reset_test_memory() {
	data.fill(0);
	ipl_enabled = false;
}