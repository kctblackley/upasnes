#include "cartridge.hpp"

// Just defines some cartridge helper functions
// Used to detect the mapping mode for the cartridge (inspired by bsnes)

CartridgeHeader parse_header(const std::vector<Byte>& rom, size_t offset) {
	CartridgeHeader h;

	h.title = std::string(reinterpret_cast<const char*>(&rom[offset]), 21);
	h.map_mode = rom[offset + 0x15];
	h.cartridge_type = rom[offset + 0x16];

	h.rom_size = rom[offset + 0x17];
	h.ram_size = rom[offset + 0x18];

	h.region = rom[offset + 0x19];
	h.version = rom[offset + 0x1B];

	h.complement = (rom[offset + 0x1d] << 8) | rom[offset + 0x1c];
	h.checksum = (rom[offset + 0x1f] << 8) | rom[offset + 0x1e];

	h.reset_vector = (rom[offset + 0x3d] << 8) | rom[offset + 0x3c];

	return h;
}

uint16_t calculate_checksum(const std::vector<Byte>& rom)
{
    uint32_t sum = 0;

    for (auto b : rom)
        sum += b;

    return sum & 0xFFFF;
}

int score(const MapperCandidate& candidate, size_t rom_size, const std::vector<Byte>& rom) {
	int score = 0;
	CartridgeHeader h = candidate.h;

	if (calculate_checksum(rom) == h.checksum) {
    	score += 20;
	}

	if ((h.map_mode >= 0x20 && h.map_mode <= 0x25) || (h.map_mode >= 0x30 && h.map_mode <= 0x35)) {
		score += 4;
	}

	bool printable = true;

	for (char c : h.title) {
		if (c == '\0') {
			break;
		}

		if (c < 32 || c > 126) {
			printable = false;
			break;
		}
	}

	if (printable) {
		score += 2;
	}

	if (h.rom_size >= 8 && h.rom_size <= 15) {
		score += 2;
	}

	if (h.ram_size <= 8) {
		score += 1;
	}

	if (h.region <= 13) {
		score += 1;
	}

	if (h.reset_vector >= 0x8000) {
		score += 4;
	}

	if (h.version <= 3) {
		score += 1;
	}

	if (h.map_mode == 0x20 || h.map_mode == 0x30) {
		if (candidate.mapper == MapperType::LoROM) {
			score += 6;
		}
	}
	if (h.map_mode == 0x21 || h.map_mode == 0x31) {
		if (candidate.mapper == MapperType::HiROM) {
			score += 6;
		}
	}
	if (h.map_mode == 0x25 || h.map_mode == 0x35) {
		if (candidate.mapper == MapperType::ExHiROM) {
			score += 6;
		}
	}

	switch (candidate.mapper) {
	case MapperType::ExHiROM:
		if (rom_size > 0x400000) {
			score += 6;
		}
		break;
	case MapperType::HiROM:
		if (rom_size <= 0x400000) {
			score += 2;
		}
		break;
	default:
		break;
	}

	if (candidate.mapper == MapperType::LoROM)
	{
	    if (h.map_mode == 0x21 || h.map_mode == 0x31)
	        score -= 8;
	}

	if (candidate.mapper == MapperType::HiROM)
	{
	    if (h.map_mode == 0x20 || h.map_mode == 0x30)
	        score -= 8;
	}

	return score;
}