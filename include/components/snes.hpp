#pragma once

#include <string>

#include "bus.hpp"
#include "ricoh_5a22.hpp"
#include "ppu.hpp"
#include "spc_700.hpp"
#include "renderer.hpp"
#include "common.hpp"

class SNES {
public:

	SNES();

	void load_cartridge(const std::string& directory);
	void poll(); 
	void tick_snes();
	void run();
	void sync_to_superfx();

	void reset();
	void initialise();

private:

	std::unique_ptr<Bus> bus;
	std::unique_ptr<Renderer> renderer;

	std::vector<std::unique_ptr<Component>> devices;

	Ricoh5A22* ricoh_5a22 = nullptr;
	PPU* ppu = nullptr;
	SPC700* spc_700 = nullptr;

	CycleCount master_cycle;
};