#pragma once
#include "common.hpp"

#define LARGE_GAME_PAK_RAM_SIZE 65536
#define SMALL_GAME_PAK_RAM_SIZE 32768

enum class SuperFXRevision {
	None,
	MARIO,
	GSU1,
	GSU2,
	GSU2SP1 // functionally-identical to GSU2, I'm just including detection of this revision for the sake of it
};

class SuperFX {
public:
	SuperFX() { }

	void build_game_pak_ram(int checksum) {
		if (checksum == 0xAB12 || checksum == 0x4DBF || checksum == 0x132C) {
			gpram_size = SMALL_GAME_PAK_RAM_SIZE;
		}
		gpram.assign(gpram_size, 0);
		std::cout << "Game Pak RAM Size: " << std::dec << (int)(gpram_size) << "\n";
	}

private:
	SuperFXRevision revision = SuperFXRevision::None;

	size_t gpram_size = LARGE_GAME_PAK_RAM_SIZE;
	std::vector<Byte> gpram {}; // gpram means 'Game Pak RAM'
};