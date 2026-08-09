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

	void load_cartridge(const std::string& directory); // This ROM is routed through the bus and into the cartridge store
	void poll(); // Poll finds components that are due to be ticked for the current master cycle
	void tick_snes(); // Polls components, ticks components if there are any, then increments master cycle by 1
	void run();

	void reset();
	void initialise();

private:

	std::unique_ptr<Bus> bus;
	std::unique_ptr<SPC700> spc_700;
	std::unique_ptr<Renderer> renderer;

	std::vector<std::unique_ptr<Component>> devices;

	Ricoh5A22* ricoh_5a22 = nullptr;
	PPU* ppu = nullptr;

	CycleCount master_cycle;
};