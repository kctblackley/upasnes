#pragma once
#include <functional>
#include <unordered_map>
#include "common.hpp"
#include "open_bus.hpp"
#include "wram.hpp"
#include "cartridge.hpp"
#include "dma.hpp"

class SPC700;
class Ricoh5A22;
class PPU;
class Component;
class DMA;

class Bus {
public:
	using WaitCallback = std::function<void(CycleCount cycles)>;

	Bus();
	~Bus();

	void connect_cpu(Ricoh5A22* cpu);
	void connect_apu(SPC700* apu);
	void connect_ppu(PPU* ppu);
	
	void set_wait_callback(WaitCallback callback);

	Store* system_area(SNESAddress address); 
	Store* route(SNESAddress address);

	Component* system_area_component(SNESAddress address);
	Component* route_to_component(SNESAddress address);

	CycleCount component_penalty(SNESAddress address);

	void write(Address addr, Byte value, bool is_dma = false);
	Byte read(Address addr, bool is_dma = false);

	void load_cartridge(const std::string& directory, Ricoh5A22* ricoh);

	void enable_test_mode();
	void disable_test_mode();
	void reset_test_memory();
	Byte test_peek(Address addr);
	void test_poke(Address addr, Byte value);

	void connect_cpu_to_cartridge(Ricoh5A22* cpu);

	Byte get_open_bus();
	void set_open_bus(Byte value);

	bool is_cartridge_mapped(Address addr);

	void set_fastrom(bool fastrom_enabled) {
		cartridge->set_fastrom(fastrom_enabled);
	}

	void wram_refresh_pause();

private:
	WaitCallback callback;
	Byte data_bus;

	bool test_mode = false;
	std::unordered_map<Address, Byte> test_memory;

	std::unique_ptr<OpenBus> open_bus;
	std::unique_ptr<WRAM> wram;
	std::unique_ptr<Cartridge> cartridge;
	std::unique_ptr<DMA> dma;

	SPC700* apu;
	Ricoh5A22* cpu;
	PPU* ppu;
};