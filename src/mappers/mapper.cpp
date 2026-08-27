#include "mapper.hpp"
#include "cartridge.hpp"

constexpr int SUPERFX_SRAM_SIZE = 8192;

int superfx_sram_size(CartridgeHardware* hardware, CartridgeHeader* header) {
	Word checksum = header->checksum;
	if (checksum == 0xD7C8 || checksum == 0x5F3F || checksum == 0x132C) { // These values are the checksum values for Stunt Race FX, Star Fox 2, and Yoshi's Island (Winter Gold not supported as it is not an NTSC game)
		return SUPERFX_SRAM_SIZE;
	} else {
		return 0;
	}
}

template <class MapperT>
void Mapper<MapperT>::load_sram(Byte ram_size, CartridgeHardware* hardware, CartridgeHeader* header) {
	size_t sram_size = 0;
	if (ram_size != 0) {
		sram_size = (1024u << ram_size);
	}

	if (hardware->superfx_revision != SuperFXRevision::None) {
		sram_size = superfx_sram_size(hardware, header);
	}
	sram.assign(sram_size, 0);

	if constexpr (false) { // Just a temporary thing for testing Earthbound, which is experiencing some issues at the moment
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

	if constexpr (false) { // Just a temporary thing for testing Final Fantasy VI
		std::ifstream file("saves/final_fantasy_vi/final_fantasy_vi.srm", std::ios::binary);

		file.seekg(0, std::ios::end);
		std::size_t size = file.tellg();
		file.seekg(0, std::ios::beg);

		std::size_t read_size = std::min(size, sram.size());
		if (size != sram.size()) {
		    std::cout << "WARNING: final_fantasy_vi.srm is " << size
		              << " bytes, but cartridge SRAM is " << sram.size()
		              << " bytes. Reading " << read_size << " bytes.\n";
		}
		file.read(reinterpret_cast<char*>(sram.data()), read_size);
	}
	if constexpr (false) { // Just a temporary thing for testing DKC
		std::ifstream file("saves/donkey_kong_country/donkey_kong_country.srm", std::ios::binary);

		file.seekg(0, std::ios::end);
		std::size_t size = file.tellg();
		file.seekg(0, std::ios::beg);

		std::size_t read_size = std::min(size, sram.size());
		if (size != sram.size()) {
		    std::cout << "WARNING: donkey_kong_country.srm is " << size
		              << " bytes, but cartridge SRAM is " << sram.size()
		              << " bytes. Reading " << read_size << " bytes.\n";
		}
		file.read(reinterpret_cast<char*>(sram.data()), read_size);
	}
}

template <class MapperT>
Byte Mapper<MapperT>::get_open_bus() {
	return cpu->get_open_bus();
}


template class Mapper<LoROM>;
template class Mapper<HiROM>;
template class Mapper<ExHiROM>;