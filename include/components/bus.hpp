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
class SNES;

class Bus {
public:
	using WaitCallback = std::function<void(i64 cycles)>;

	Bus();
	~Bus();

	void connect_cpu(Ricoh5A22* cpu);
	void connect_apu(SPC700* apu);
	void connect_ppu(PPU* ppu);

	void tick_coprocessor() {
		cartridge->tick_coprocessor();
	}

	bool has_coprocessor() {
		return cartridge->has_coprocessor();
	}

	i64 get_coprocessor_cycle() {
		return cartridge->get_coprocessor_cycle();
	}
	
	void set_wait_callback(WaitCallback callback);

	Store* system_area(SNESAddress address); 
	Store* route(SNESAddress address);

	Component* system_area_component(SNESAddress address);
	Component* route_to_component(SNESAddress address);

	i64 component_penalty(SNESAddress address);

	void write(u32 addr, u8 value, bool is_dma = false);
	u8 read(u32 addr, bool is_dma = false);

	void load_cartridge(const std::string& directory, Ricoh5A22* ricoh, const std::string& game_name);

	void enable_test_mode();
	void disable_test_mode();
	void reset_test_memory();
	u8 test_peek(u32 addr);
	void test_poke(u32 addr, u8 value);

	void connect_cpu_to_cartridge(Ricoh5A22* cpu);

	u8 get_open_bus();
	void set_open_bus(u8 value);

	bool is_cartridge_mapped(u32 addr);

	void set_fastrom(bool fastrom_enabled) {
		cartridge->set_fastrom(fastrom_enabled);
	}

	void connect_snes(SNES* snes) {
		cartridge->connect_snes(snes);
	}

	void wram_refresh_pause();

	Region get_cartridge_region() const {
		return cartridge->get_region();
	}

private:
	WaitCallback callback;
	u8 data_bus;

	bool test_mode = false;
	std::unordered_map<u32, u8> test_memory;

	std::unique_ptr<OpenBus> open_bus;
	std::unique_ptr<WRAM> wram;
	std::unique_ptr<Cartridge> cartridge;
	std::unique_ptr<DMA> dma;

	SPC700* apu;
	Ricoh5A22* cpu;
	PPU* ppu;
};