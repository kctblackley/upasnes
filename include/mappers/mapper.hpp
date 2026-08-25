#pragma once
#include "common.hpp"
#include <vector>
#include <fstream>
#include <optional>
#include "superfx.hpp"
#include "ricoh_5a22.hpp"

struct CartridgeHardware;
struct CartridgeHeader;

template<typename MapperT>
class Mapper {
public:
	Byte read(SNESAddress address);
	void write(SNESAddress address, Byte value);

	void load_rom(std::vector<Byte> rom) {
		this->rom = rom;

		// Non-power-of-two ROM support...
		while (!std::has_single_bit(this->rom.size())) {
	        size_t chunk = this->rom.size() & -this->rom.size();
	        std::cout << "REPADDED ROM\n";
	        this->rom.insert(this->rom.end(), this->rom.end() - chunk, this->rom.end());
	    }
	}

	void load_sram(Byte ram_size, CartridgeHardware* hardware = nullptr, CartridgeHeader* header = nullptr);

	void connect_cpu(Ricoh5A22* cpu) {
		std::cout << "CARTRIDGE LOADED...\n";
		this->cpu = cpu;
		if (cpu) {
			std::cout << "SUCCESSFULLY!\n";
		}
	}

	Byte get_open_bus();

protected:

	void log_info() {
		std::cout << "ROM size: " << rom.size() << "\n";
		std::cout << "SRAM size: " << sram.size() << "\n";
	}

	std::vector<Byte> rom;
	std::vector<Byte> sram;

	Ricoh5A22* cpu = nullptr;

private:

	MapperT& derived() {
		return static_cast<MapperT&>(*this);
	}

	const MapperT& derived() const {
		return static_cast<const MapperT&>(*this);
	}
};

#include "mapper.inl"