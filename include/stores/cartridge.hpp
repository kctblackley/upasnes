#pragma once
#include <algorithm>
#include <variant>
#include "store.hpp"
#include "mapper.hpp"
#include "lorom_mapper.hpp"
#include "hirom_mapper.hpp"
#include "exhirom_mapper.hpp"
#include "superfx.hpp"
#include "dsp.hpp"
#include "sa1.hpp"
#include "sdd1.hpp"

class Ricoh5A22;

enum class MapperType {
	LoROM,
	HiROM,
	ExHiROM
};

enum class Coprocessor {
	None,
	DSP,
	SuperFX,
	SA1,
	SDD1
};

struct HardwareDatabaseEntry {
	std::array<Byte, 4> game_code {};
	Coprocessor coprocessor;

	Word checksum;
	Word complement;

	DSPRevision dsp_revision = DSPRevision::None;
	SuperFXRevision superfx_revision = SuperFXRevision::None;
	SA1Revision sa1_revision = SA1Revision::None;
	SDD1Revision sdd1_revision = SDD1Revision::None;
};

struct CartridgeHardware {
	Coprocessor coprocessor = Coprocessor::None;

	DSPRevision dsp_revision = DSPRevision::None;
	SuperFXRevision superfx_revision = SuperFXRevision::None;
	SA1Revision sa1_revision = SA1Revision::None;
	SDD1Revision sdd1_revision = SDD1Revision::None;

	bool has_ram = false;
	bool has_battery = false;
};

struct CartridgeHeader {
	std::string title;

	Byte map_mode;
	Byte cartridge_type;

	Byte rom_size;
	Byte ram_size;

	Byte region;
	Byte version;

	Byte developer_id;

	Word checksum;
 	Word complement;

	Word reset_vector;

	std::array<Byte, 2> maker_code {};
	std::array<Byte, 4> game_code {};
	Byte expansion_flash_size;
	Byte expansion_ram_size;
	Byte special_version;
	Byte chipset_subtype;
};

struct MapperCandidate {
	MapperType mapper;
	CartridgeHeader h;
	int score;
};

CartridgeHeader parse_header(const std::vector<Byte>& rom, size_t offset);
int score(const MapperCandidate& candidate, size_t rom_size, const std::vector<Byte>& rom);
void validate_hardware_mapping(const CartridgeHeader& h, int& score);
const HardwareDatabaseEntry* find_hardware_database_entry(const CartridgeHeader& h);
Coprocessor detect_coprocessor(const CartridgeHeader& h);
void detect_dsp_revision(CartridgeHeader& h, const std::vector<Byte>& rom, CartridgeHardware& hardware);
void detect_superfx_revision(CartridgeHeader& h, const std::vector<Byte>& rom, CartridgeHardware& hardware);
void detect_sa1_revision(CartridgeHeader& h, const std::vector<Byte>& rom, CartridgeHardware& hardware);
void detect_sdd1_revision(CartridgeHeader& h, const std::vector<Byte>& rom, CartridgeHardware& hardware);
void detect_hardware(CartridgeHeader& h, CartridgeHardware& hardware, const std::vector<Byte>& rom);
void detect_memory_features(const CartridgeHeader& h, CartridgeHardware& hardware);
const char* mapper_to_string(MapperType mapper);
const char* coprocessor_to_string(Coprocessor coprocessor);
const char* dsp_revision_to_string(DSPRevision revision);
const char* superfx_revision_to_string(SuperFXRevision revision);
std::string byte_to_hex(Byte value);
std::string word_to_hex(Word value);

class Cartridge : public Store {
public:

	Byte get_open_bus() {
		return std::visit([&](auto& m) { return m.get_open_bus(); }, mapper);
	}

	Byte read(SNESAddress address) override {
		address_bus = address;
		return std::visit(
		    [&](auto& m)
		    {
		        return m.read(address);
		    },
		    mapper
		);
	}

	void write(SNESAddress address, Byte value) override {
		address_bus = address;
		std::visit(
	        [&](auto& m)
	        {
	            m.write(address, value);
	        },
	        mapper
	    );
	}

	Byte gsu_read(SNESAddress address) {
		address_bus = address;
		return std::visit(
		    [&](auto& m)
		    {
		        return m.read(address);
		    },
		    mapper
		);
	}

	SNESAddress get_address_bus() override {
		return address_bus;
	}

	CycleCount penalty() override {
		bool memory2 = (address_bus.bank >= 0x80 && address_bus.bank <= 0xBF &&
						address_bus.offset >= 0x8000) || (address_bus.bank >= 0xC0);
		if (memory2 && fastrom_enabled && is_fastrom_cartridge) {
			return 0;
		}
		return 2;
	}

	void set_fastrom(bool fastrom_enabled) {
		this->fastrom_enabled = fastrom_enabled;
	}

	MapperCandidate make_candidate(const std::vector<Byte>& rom, MapperType mapper, size_t offset) {
	    MapperCandidate c{
	        mapper,
	        parse_header(rom, offset),
	        0
	    };

	    c.score = score(c, rom.size(), rom);

	    return c;
	}

	bool is_exhirom_half_swapped(const std::vector<Byte>& rom) {
		if (rom.size() != 0x600000) {
			return false;
		}

		if (rom.size() < 0x410000) {
			return false;
		}

		auto normal = make_candidate(rom, MapperType::ExHiROM, 0x40ffc0);
		std::vector<Byte> test = rom;

		std::rotate(test.begin(), test.begin() + 0x400000, test.end());
		auto swapped = make_candidate(test, MapperType::ExHiROM, 0x40ffc0);

		return swapped.score > normal.score;
	}

	void fix_exhirom_half_swap(std::vector<Byte>& rom) {
		std::rotate(rom.begin(), rom.begin() + 0x400000, rom.end());
	}

	void load_cartridge(const std::string& directory, Ricoh5A22* cpu) {
		std::vector<Byte> rom = load_rom(directory);

		std::vector<MapperCandidate> candidates;
		
		auto try_add = [&](MapperType type, size_t offset) {
	        if (offset >= 0x10 && offset + 0x40 <= rom.size()) {   // header needs up to offset+0x3e
	            candidates.push_back({type, parse_header(rom, offset), 0});
	        }
	    };

	    try_add(MapperType::LoROM,   0x7fc0);
	    try_add(MapperType::HiROM,   0xffc0);
	    try_add(MapperType::ExHiROM, 0x40ffc0);

	    if (candidates.empty()) {
	        throw std::runtime_error("ROM too small to contain a valid header");
	    }

		for (auto& c : candidates) {
			c.score = score(c, rom.size(), rom);
		}

		auto best = std::max_element(
			candidates.begin(),
			candidates.end(),
			[](const auto& a, const auto& b) {
				return a.score < b.score;
			}
		);

		if (best->mapper == MapperType::ExHiROM && is_exhirom_half_swapped(rom)) {
			std::cout << "Detected ExHiROM half-swapped dump; correcting\n";
			fix_exhirom_half_swap(rom);
		
			candidates.clear();

		    try_add(MapperType::LoROM,   0x7fc0);
		    try_add(MapperType::HiROM,   0xffc0);
		    try_add(MapperType::ExHiROM, 0x40ffc0);

		    for (auto& c : candidates) {
		        c.score = score(c, rom.size(), rom);
		    }

		    best = std::max_element(
		        candidates.begin(),
		        candidates.end(),
		        [](const auto& a, const auto& b) {
		            return a.score < b.score;
		        }
		    );
		}

		switch (best->mapper) {
		case MapperType::LoROM:
			mapper = LoROM{};
			break;
		case MapperType::HiROM:
			mapper = HiROM{};
			break;
		case MapperType::ExHiROM:
			mapper = ExHiROM{};
			break;
		}

		header = best->h;

		detect_hardware(header, hardware, rom);

		std::visit(
		    [&](auto& m)
		    {
		        m.load_rom(rom);
		        m.load_sram(header.ram_size, &hardware, &header);
		        m.connect_cpu(cpu);
		        m.to_string();
		    },
		    mapper
		);

		if (hardware.coprocessor == Coprocessor::SuperFX) {
			superfx.build_game_pak_ram(header.checksum);
			superfx.set_revision(hardware.superfx_revision);
			superfx.set_mapper_type(best->mapper == MapperType::HiROM);
			superfx.connect_cpu(cpu);
		}

		is_fastrom_cartridge = (header.map_mode & 0x10) != 0;
		std::cout << header.title << "\n";

		superfx.connect_cartridge(this);
		print_info();
	}

	void connect_cpu(Ricoh5A22* cpu) {
	    std::visit([&](auto& m) { m.connect_cpu(cpu); }, mapper);
	    superfx.connect_cpu(cpu);
	}

	void print_info() const {
		std::cout << "Cartridge Information\n";
		
		std::cout << "Title:           " << header.title << '\n';
		std::cout << "Mapper:          " << mapper_to_string(
			std::visit(
				[](const auto& m) -> MapperType {
					using T = std::decay_t<decltype(m)>;

					if constexpr (std::is_same_v<T, LoROM>)
						return MapperType::LoROM;
					else if constexpr (std::is_same_v<T, HiROM>)
						return MapperType::HiROM;
					else
						return MapperType::ExHiROM;
				},
				mapper
			)
		) << '\n';

		std::cout << "FastROM:         "
		          << ((header.map_mode & 0x10) ? "Yes" : "No")
		          << '\n';

		std::cout << "Map mode:        " << byte_to_hex(header.map_mode) << '\n';
		std::cout << "Cartridge type:  " << byte_to_hex(header.cartridge_type) << '\n';
		std::cout << "ROM size field:  " << byte_to_hex(header.rom_size) << '\n';
		std::cout << "RAM size field:  " << byte_to_hex(header.ram_size) << '\n';
		std::cout << "Region:          " << static_cast<unsigned>(header.region) << '\n';
		std::cout << "Version:         " << static_cast<unsigned>(header.version) << '\n';

		std::cout << "Coprocessor:     "
		          << coprocessor_to_string(hardware.coprocessor)
		          << '\n';

		switch (hardware.coprocessor) {
		case Coprocessor::DSP:
			std::cout << "DSP revision:    "
			          << dsp_revision_to_string(hardware.dsp_revision)
			          << '\n';
			break;

		case Coprocessor::SuperFX:
			std::cout << "Super FX rev:    "
			          << superfx_revision_to_string(hardware.superfx_revision)
			          << '\n';
			break;

		default:
			break;
		}

		std::cout << "Checksum:        " << word_to_hex(header.checksum) << '\n';
		std::cout << "Complement:      " << word_to_hex(header.complement) << '\n';
		std::cout << "Reset vector:    " << word_to_hex(header.reset_vector) << '\n';

		
		std::cout << "Game code:       \n";
		for (auto& c : header.game_code) {
			std::cout << std::hex << (int)c << " ";
		}
		std::cout << "\n";

		std::cout << "Maker code:      \n";
		for (auto& c : header.maker_code) {
			std::cout << std::hex << (int)c << " ";
		}
		std::cout << "\n";

		std::cout << "Chipset subtype: "
		          << byte_to_hex(header.chipset_subtype)
		          << '\n';
	}

	void tick_coprocessor() {
		if (hardware.coprocessor == Coprocessor::SuperFX) {
			superfx.tick_component();
		}
	}

	bool has_coprocessor() {
		return hardware.coprocessor != Coprocessor::None;
	}

	CycleCount get_coprocessor_cycle() {
		if (hardware.coprocessor == Coprocessor::SuperFX) {
			return superfx.get_coprocessor_cycle();
		}
		return 0;
	}

private:
	Byte mapping; // Stores cartridge's mapping
	bool is_fastrom_cartridge = false;
	bool fastrom_enabled = false;
	CycleCount penalty_value = 0;
	CartridgeHeader header;
	CartridgeHardware hardware;
	
	std::variant<LoROM, HiROM, ExHiROM> mapper;

	SNESAddress address_bus;
	Ricoh5A22* ricoh_5a22 = nullptr;
	SuperFX superfx;
};