#include "cartridge.hpp"
#include <iomanip>
#include <sstream>

constexpr bool FORCE_LOROM = false;
constexpr bool FORCE_HIROM = false;

constexpr size_t HEADER_BASE = 0xFFC0;
constexpr size_t EXPANDED_HEADER_BASE = 0xFFB0;

constexpr HardwareDatabaseEntry BuildDatabaseEntry(Byte gc0 = 0, Byte gc1 = 0, Byte gc2 = 0, Byte gc3 = 0, Word checksum = 0, Word complement = 0, Coprocessor coprocessor = Coprocessor::None, DSPRevision dsp_revision = DSPRevision::None, SuperFXRevision superfx_revision = SuperFXRevision::None, SA1Revision sa1_revision = SA1Revision::None, SDD1Revision sdd1_revision = SDD1Revision::None) {
	HardwareDatabaseEntry hde;
	hde.game_code[0] = gc0;
	hde.game_code[1] = gc1;
	hde.game_code[2] = gc2;
	hde.game_code[3] = gc3;

	hde.checksum = checksum;
	hde.complement = complement;

	hde.coprocessor = coprocessor;
	hde.dsp_revision = dsp_revision;
	hde.superfx_revision = superfx_revision;
	hde.sa1_revision = sa1_revision;
	hde.sdd1_revision = sdd1_revision;

	return hde;
}

constexpr std::array<HardwareDatabaseEntry, 7> hardware_database_entries = {
	// SuperFX Games
	/* Star Fox */       BuildDatabaseEntry(0xFF, 0xFF, 0xFF, 0xFF, 0xAB12, 0x54ED, Coprocessor::SuperFX, DSPRevision::None, SuperFXRevision::MARIO, SA1Revision::None, SDD1Revision::None),
	/* Dirt Racer */     //BuildDatabaseEntry(), // Only released in Europe, won't run on my emulator
	/* Dirt Trax FX */   BuildDatabaseEntry(0x41, 0x46, 0x39, 0x45, 0x8DC1, 0x723E, Coprocessor::SuperFX, DSPRevision::None, SuperFXRevision::GSU1, SA1Revision::None, SDD1Revision::None),
	/* Stunt Race FX */  BuildDatabaseEntry(0x43, 0x51, 0x20, 0x20, 0xD7C8, 0x2837, Coprocessor::SuperFX, DSPRevision::None, SuperFXRevision::GSU1, SA1Revision::None, SDD1Revision::None),
	/* Vortex */         BuildDatabaseEntry(0x34, 0x56, 0x20, 0x20, 0x4DBF, 0xB240, Coprocessor::SuperFX, DSPRevision::None, SuperFXRevision::GSU1, SA1Revision::None, SDD1Revision::None),
	/* Doom */           BuildDatabaseEntry(0x41, 0x44, 0x38, 0x45, 0xB564, 0x4A9B, Coprocessor::SuperFX, DSPRevision::None, SuperFXRevision::GSU2, SA1Revision::None, SDD1Revision::None),
	/* Yoshi's Island */ BuildDatabaseEntry(0x59, 0x49, 0x20, 0x20, 0x132C, 0xECD3, Coprocessor::SuperFX, DSPRevision::None, SuperFXRevision::GSU2SP1, SA1Revision::None, SDD1Revision::None),
	/* Winter Gold */    // BuildDatabaseEntry(), // Also only a Europe-only release

	// Unreleased SuperFX Games
	/* Star Fox 2 */     BuildDatabaseEntry(0x53, 0x54, 0x32, 0x4A, 0x5F3F, 0xA0C0, Coprocessor::SuperFX, DSPRevision::None, SuperFXRevision::GSU2, SA1Revision::None, SDD1Revision::None)

	// DSP Games
};

const HardwareDatabaseEntry* find_hardware_database_entry(const CartridgeHeader& h) {
	for (auto& e : hardware_database_entries) {
		if (e.game_code == h.game_code && e.checksum == h.checksum && e.complement == h.complement) {
			return &e;
		}
	}
	return nullptr;
}

Coprocessor detect_coprocessor(const CartridgeHeader& h) {
	switch (h.cartridge_type & 0xF0) {
	case 0x00: {
		if (h.cartridge_type >= 0x03 && h.cartridge_type <= 0x0F) {
			return Coprocessor::DSP;
		} else {
			return Coprocessor::None;
		}
		break;
	}
	case 0x10: return Coprocessor::SuperFX;
	case 0x30: return Coprocessor::SA1;
	case 0x40: return Coprocessor::SDD1;
	default:   return Coprocessor::None;
	}
}

void detect_dsp_revision(CartridgeHeader& h, const std::vector<Byte>& rom, CartridgeHardware& hardware) {
	hardware.dsp_revision = DSPRevision::None;	

	if (const auto* entry = find_hardware_database_entry(h)) {
		if (entry->dsp_revision != DSPRevision::None) {
			hardware.dsp_revision = entry->dsp_revision;
			return;
		}
	}

	// Use ROM signature
}

void detect_superfx_revision(CartridgeHeader& h, const std::vector<Byte>& rom, CartridgeHardware& hardware) {
	hardware.superfx_revision = SuperFXRevision::None;	

	if (const auto* entry = find_hardware_database_entry(h)) {
		if (entry->superfx_revision != SuperFXRevision::None) {
			hardware.superfx_revision = entry->superfx_revision;
			return;
		}
	}

	// Use ROM signature
}

void detect_sa1_revision(CartridgeHeader& h, const std::vector<Byte>& rom, CartridgeHardware& hardware) {
	hardware.sa1_revision = SA1Revision::SA1;	
	// No actual 'revisions' of SA1 (just doing this for consistency because I am weird!)
}

void detect_sdd1_revision(CartridgeHeader& h, const std::vector<Byte>& rom, CartridgeHardware& hardware) {
	hardware.sdd1_revision = SDD1Revision::SDD1;	
	// Same thing here!
}

void detect_hardware(CartridgeHeader& h, CartridgeHardware& hardware, const std::vector<Byte>& rom) {
	hardware = {};

	hardware.coprocessor = detect_coprocessor(h);

	detect_memory_features(h, hardware);

	switch (hardware.coprocessor) {
	case Coprocessor::DSP:
		detect_dsp_revision(h, rom, hardware);
		break;
	case Coprocessor::SuperFX:
		detect_superfx_revision(h, rom, hardware);
		break;
	case Coprocessor::SA1:
		detect_sa1_revision(h, rom, hardware);
		break;
	case Coprocessor::SDD1:
		detect_sdd1_revision(h, rom, hardware);
		break;
	default:
		break;
	}
}

void detect_memory_features(const CartridgeHeader& h, CartridgeHardware& hardware) {
	switch (h.cartridge_type & 0x0F) {
	case 0x01:
		hardware.has_ram = true;
		break;
	case 0x02:
		hardware.has_ram = true;
		hardware.has_battery = true;
		break;
	case 0x03:
		hardware.has_battery = true;
		break;
	case 0x04:
		hardware.has_ram = true;
		break;
	case 0x05:
		hardware.has_ram = true;
		hardware.has_battery = true;
		break;
	case 0x06:
		hardware.has_battery = true;
		break;
	default:
		break;
	}
}

CartridgeHeader parse_header(const std::vector<Byte>& rom, size_t offset) {
	CartridgeHeader h {};

	h.title = std::string(reinterpret_cast<const char*>(&rom[offset]), 21);
	h.map_mode = rom[offset + 0x15];
	h.cartridge_type = rom[offset + 0x16];

	h.rom_size = rom[offset + 0x17];
	h.ram_size = rom[offset + 0x18];

	h.region = rom[offset + 0x19];

	h.developer_id = rom[offset + 0x1A];
	h.version = rom[offset + 0x1B];

	h.complement = (rom[offset + 0x1d] << 8) | rom[offset + 0x1c];
	h.checksum = (rom[offset + 0x1f] << 8) | rom[offset + 0x1e];

	h.reset_vector = (rom[offset + 0x3d] << 8) | rom[offset + 0x3c];

	h.maker_code[0] = rom[offset - 0x10];
	h.maker_code[1] = rom[offset - 0x0F];

	h.game_code[0] = rom[offset - 0x0E];
	h.game_code[1] = rom[offset - 0x0D];
	h.game_code[2] = rom[offset - 0x0C];
	h.game_code[3] = rom[offset - 0x0B];

	h.expansion_flash_size = rom[offset - 0x04];
	h.expansion_ram_size = rom[offset - 0x03];
	h.special_version = rom[offset - 0x02];
	h.chipset_subtype = rom[offset - 0x01];

	return h;
}

uint16_t calculate_checksum(const std::vector<Byte>& rom)
{
    uint32_t sum = 0;

    for (auto b : rom)
        sum += b;

    return sum & 0xFFFF;
}

void validate_hardware_mapping(const CartridgeHeader& h, int& score) {
	const Byte map = h.map_mode & 0x0F;
	const Byte chip = h.cartridge_type & 0xF0;

	if (chip == 0x30 && map == 0x03) {
		score += 10;
	}
	if (chip == 0x40 && map == 0x02) {
		score += 10;
	}
}

int score(const MapperCandidate& candidate, size_t rom_size, const std::vector<Byte>& rom) {
	int score = 0;
	CartridgeHeader h = candidate.h;

	/*if (calculate_checksum(rom) == h.checksum) {
    	score += 20;
	}*/

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

	if constexpr (FORCE_LOROM) {
		if (candidate.mapper == MapperType::LoROM) {
			score += 1000;
		}
	}
	if constexpr (FORCE_HIROM) {
		if (candidate.mapper == MapperType::HiROM) {
			score += 1000;
		}
	}

	return score;
}

const char* mapper_to_string(MapperType mapper)
{
	switch (mapper) {
	case MapperType::LoROM:
		return "LoROM";

	case MapperType::HiROM:
		return "HiROM";

	case MapperType::ExHiROM:
		return "ExHiROM";
	}

	return "Unknown";
}

const char* coprocessor_to_string(Coprocessor coprocessor)
{
	switch (coprocessor) {
	case Coprocessor::None:
		return "None";

	case Coprocessor::DSP:
		return "DSP";

	case Coprocessor::SuperFX:
		return "Super FX";

	case Coprocessor::SA1:
		return "SA-1";

	case Coprocessor::SDD1:
		return "S-DD1";
	}

	return "Unknown";
}

const char* dsp_revision_to_string(DSPRevision revision)
{
	switch (revision) {
	case DSPRevision::None:
		return "None";

	case DSPRevision::DSP1:
		return "DSP1";

	case DSPRevision::DSP1B:
		return "DSP1B";

	case DSPRevision::DSP2:
		return "DSP2";

	case DSPRevision::DSP3:
		return "DSP3";

	case DSPRevision::DSP4:
		return "DSP4";
	}

	return "Unknown";
}

const char* superfx_revision_to_string(SuperFXRevision revision)
{
	switch (revision) {
	case SuperFXRevision::None:
		return "None";

	case SuperFXRevision::MARIO:
		return "MARIO";

	case SuperFXRevision::GSU1:
		return "GSU1";

	case SuperFXRevision::GSU2:
		return "GSU2";

	case SuperFXRevision::GSU2SP1:
		return "GSU2-SP1";
	}

	return "Unknown";
}

std::string byte_to_hex(Byte value)
{
	std::ostringstream ss;

	ss << "0x"
	   << std::uppercase
	   << std::hex
	   << std::setfill('0')
	   << std::setw(2)
	   << static_cast<unsigned>(value);

	return ss.str();
}

std::string word_to_hex(Word value)
{
	std::ostringstream ss;

	ss << "0x"
	   << std::uppercase
	   << std::hex
	   << std::setfill('0')
	   << std::setw(4)
	   << static_cast<unsigned>(value);

	return ss.str();
}
