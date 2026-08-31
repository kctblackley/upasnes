#include "mapper.hpp"
#include "cartridge.hpp"
#include <filesystem>

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
void Mapper<MapperT>::load_sram(Byte ram_size, const std::string& game_name, CartridgeHardware* hardware, CartridgeHeader* header) {
	this->game_name = game_name;

	size_t sram_size = 0;
	if (ram_size != 0) {
		sram_size = (1024u << ram_size);
	}

	if (hardware->superfx_revision != SuperFXRevision::None) {
		sram_size = superfx_sram_size(hardware, header);
	}

	sram.assign(sram_size, 0);

	if (sram_size > 0) {
		std::string save_directory = std::filesystem::path("saves") / this->game_name / "save.sav";
		std::filesystem::path save_path = save_directory;

		save_exists = std::filesystem::exists(save_path);

		if (save_exists) {
			std::ifstream file(save_directory, std::ios::binary);
			
			file.read(
				reinterpret_cast<char*>(sram.data()),
				sram.size()
			);
		}
	}
}

template <class MapperT>
Byte Mapper<MapperT>::get_open_bus() {
	return cpu->get_open_bus();
}

template <class MapperT>
Mapper<MapperT>::~Mapper() {

	// Save game method

	if (sram.empty()) {
		return;
	}

	std::filesystem::path save_path = std::filesystem::path("saves") / this->game_name / "save.sav";

	std::filesystem::create_directories(save_path.parent_path());

	std::ofstream file(
		save_path,
		std::ios::binary | std::ios::trunc
	);

	if (!file) {
		return;
	}

	file.write(
		reinterpret_cast<const char*>(sram.data()),
		sram.size()
	);
}

template class Mapper<LoROM>;
template class Mapper<HiROM>;
template class Mapper<ExHiROM>;