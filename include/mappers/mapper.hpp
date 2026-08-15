#pragma once
#include "common.hpp"
#include <vector>
#include <fstream>
#include <optional>
#include "ricoh_5a22.hpp"

template <class MapperT>
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

	void load_sram(Byte ram_size) {
		size_t sram_size = 0;
		if (ram_size != 0) {
			sram_size = (1024u << ram_size);
		}
		sram.assign(sram_size, 0);

		if constexpr (true) { // Just a temporary thing for testing Earthbound, which is experiencing some issues at the moment
			std::ifstream file("saves/earthbound/earthbound.srm", std::ios::binary);

			file.seekg(0, std::ios::end);
			std::size_t size = file.tellg();
			file.seekg(0, std::ios::beg);

			std::size_t read_size = std::min(size, sram.size());
			if (size != sram.size()) {
			    std::cout << "WARNING: earthbound.srm is " << size
			              << " bytes, but cartridge SRAM is " << sram.size()
			              << " bytes. Reading " << read_size << " bytes.\n";
			}
			file.read(reinterpret_cast<char*>(sram.data()), read_size);
		}
	}

	void connect_cpu(Ricoh5A22* cpu) {
		std::cout << "CARTRIDGE LOADED...\n";
		this->cpu = cpu;
		if (cpu) {
			std::cout << "SUCCESSFULLY!\n";
		}
	}

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