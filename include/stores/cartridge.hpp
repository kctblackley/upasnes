#pragma once
#include <algorithm>
#include <variant>
#include "store.hpp"
#include "mapper.hpp"
#include "lorom_mapper.hpp"
#include "hirom_mapper.hpp"
#include "exhirom_mapper.hpp"

class Ricoh5A22;

enum class MapperType {
	LoROM,
	HiROM,
	ExHiROM
};

struct CartridgeHeader {
	std::string title;

	Byte map_mode;
	Byte cartridge_type;

	Byte rom_size;
	Byte ram_size;

	Byte region;
	Byte version;

	Word checksum;
 	Word complement;

	Word reset_vector;
};

struct MapperCandidate {
	MapperType mapper;
	CartridgeHeader h;
	int score;
};

CartridgeHeader parse_header(const std::vector<Byte>& rom, size_t offset);
int score(const MapperCandidate& candidate, size_t rom_size, const std::vector<Byte>& rom);

class Cartridge : public Store {
public:

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
	        if (offset + 0x40 <= rom.size()) {   // header needs up to offset+0x3e
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
		std::visit(
		    [&](auto& m)
		    {
		        m.load_rom(rom);
		        m.load_sram(header.ram_size);
		        m.connect_cpu(cpu);
		        m.to_string();
		    },
		    mapper
		);

		is_fastrom_cartridge = (header.map_mode & 0x10) != 0;
		std::cout << header.title << "\n";
	}

	void connect_cpu(Ricoh5A22* cpu) {
	    std::visit([&](auto& m) { m.connect_cpu(cpu); }, mapper);
	}

private:
	Byte mapping; // Stores cartridge's mapping
	bool is_fastrom_cartridge = false;
	bool fastrom_enabled = false;
	CycleCount penalty_value = 0;
	CartridgeHeader header;
	
	std::variant<LoROM, HiROM, ExHiROM> mapper;

	SNESAddress address_bus;
};