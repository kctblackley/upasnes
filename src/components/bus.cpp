#include "bus.hpp"
#include "spc_700.hpp"
#include "ricoh_5a22.hpp"
#include "ppu.hpp"
#include "cartridge.hpp"

#define WRAM_REFRESH_PAUSE_CYCLES 40

Bus::Bus() {
	open_bus = std::make_unique<OpenBus>();
	wram = std::make_unique<WRAM>();
	cartridge = std::make_unique<Cartridge>();
	dma = std::make_unique<DMA>();
}

void Bus::connect_cpu_to_cartridge(Ricoh5A22* cpu) {
	cartridge->connect_cpu(cpu);
}

void Bus::wram_refresh_pause() {
	cpu->add_cycles(WRAM_REFRESH_PAUSE_CYCLES);
}

bool Bus::is_cartridge_mapped(Address addr) {
	return route(split_address(addr)) == cartridge.get();
}

Byte Bus::get_open_bus() {
	return cpu->get_open_bus();
}

void Bus::set_open_bus(Byte value) {
	cpu->set_open_bus(value);
}

Bus::~Bus() = default;

void Bus::connect_cpu(Ricoh5A22* cpu) {
	this->cpu = cpu;
	cpu->connect_dma(dma.get());
	dma->give_dma_access_to_cpu(cpu);
}

void Bus::connect_apu(SPC700* apu) {
	this->apu = apu;
}

void Bus::connect_ppu(PPU* ppu) {
	this->ppu = ppu;
	dma->connect_ppu(ppu);
	ppu->connect_dma(dma.get());
}

void Bus::set_wait_callback(WaitCallback callback) {
	this->callback = callback;
}

Store* Bus::system_area(SNESAddress address) {
	if (address.offset >= WRAM_SECTION && address.offset < OPEN_BUS_SECTION) {
		return wram.get();
	}
	if (address.offset >= OPEN_BUS_SECTION && address.offset < PPU_PORTS_SECTION) {
		return open_bus.get();
	}
	if (address.offset >= WMDATA_ADDRESS && address.offset <= WMADDH_ADDRESS) {
		return wram.get();
	}
	if (address.offset > WMADDH_ADDRESS && address.offset < 0x3000) {
		return open_bus.get();
	}
	if (address.offset >= 0x3000 && address.offset < 0x6000) {
        return cartridge.get();
    }
	if (address.offset >= EXPANSION_DATA_SECTION && address.offset < CARTRIDGE_SECTION) {
		//std::cout << "ACCESSING EXPANSION DATA\n";
		return cartridge.get();
	}
	if (address.offset >= CARTRIDGE_SECTION && address.offset <= MAX_OFFSET_SECTION) {
		return cartridge.get();
	}
	return open_bus.get();
}

Component* Bus::system_area_component(SNESAddress address) {
	if (address.offset >= PPU_PORTS_SECTION && address.offset < APU_PORTS_SECTION) {
		return ppu;
	}
	if (address.offset >= APU_PORTS_SECTION && address.offset < WRAM_ACCESS_SECTION) {
		return apu; // Just stubbed for now, to be finished later!
	}
	if (address.offset >= CPU_PORTS_SECTION && address.offset < CPU_DMA_PORTS_SECTION) {
		if (address.offset == 0x420B || address.offset == 0x420C) {
			return dma.get();
		}
		return cpu; // Not all ports have been created just yet 
	}
	if (address.offset >= CPU_PORTS_NON_PENALTY_SECTION && address.offset < CPU_DMA_PORTS_ENDING) {
		return dma.get();
	}
	return nullptr;
}

inline Component* Bus::route_to_component(SNESAddress address) {
	Quadrant quadrant = get_quadrant(address.bank);
	switch (quadrant) {
	case 1:
	case 3:
		return system_area_component(address);
		break;
	default:
		break;
	}

	return nullptr;
}

inline Store* Bus::route(SNESAddress address) {
	Quadrant quadrant = get_quadrant(address.bank);
	switch(quadrant) {
	case 1:
		return system_area(address);
		break;
	case 2:
		if (address.bank == 0x7E || address.bank == 0x7F) {
			return wram.get();
		} else {
			return cartridge.get();
		}
		break;
	case 3:
		return system_area(address);
		break;
	case 4:
		return cartridge.get();
	default:
		return cartridge.get();
	}

	return open_bus.get();
}

CycleCount Bus::component_penalty(SNESAddress address) {
	if (address.offset >= CPU_PORTS_SECTION && address.offset < CPU_PORTS_NON_PENALTY_SECTION) {
		return CPU_PORTS_PENALTY;
	}
	if (address.offset >= CPU_PORTS_NON_PENALTY_SECTION && address.offset < CPU_DMA_PORTS_ENDING) {
		return 0;
	}
	return WRAM_PENALTY;
}

void Bus::write(Address addr, Byte value, bool is_dma) {
	if (test_mode) {
		test_memory[addr & 0xFFFFFF] = value;
		return;
	}

	cpu->set_open_bus(value);

	SNESAddress address = split_address(addr);

	Component* component = route_to_component(address);
	if (component) {
		data_bus = value;
		component->communication_write(address, value);
		if (!is_dma) {
			callback(component_penalty(address));
		}
		return;
	}

	Store* store = route(address);
	
	if (store->is_not_open_bus()) {
		data_bus = value;
		store->write(address, value);
		if (!is_dma) {
			callback(store->penalty());
		}
	}
}

Byte Bus::read(Address addr, bool is_dma) {
	if (test_mode) {
		auto it = test_memory.find(addr & 0xFFFFFF);
		return it != test_memory.end() ? it->second : 0x00;
	}

	SNESAddress address = split_address(addr);

	Component* component = route_to_component(address);
	if (component) {
		data_bus = component->communication_read(address);
		cpu->set_open_bus(data_bus);
		if (!is_dma) {
			callback(component_penalty(address));
		}
		return data_bus;
	}

	Store* store = route(address);

	if (store->is_not_open_bus()) {
		data_bus = store->read(address);
		cpu->set_open_bus(data_bus);
		if (!is_dma) {
			callback(store->penalty());
		}
	} else {
		data_bus = cpu->get_open_bus();
	}

	return data_bus;
}

void Bus::load_cartridge(const std::string& directory, Ricoh5A22* ricoh) {
	cartridge->load_cartridge(directory, ricoh);
}

void Bus::enable_test_mode() {
	test_mode = true;
}

void Bus::disable_test_mode() {
	test_mode = false;
	test_memory.clear();
}

void Bus::reset_test_memory() {
	test_memory.clear();
}

Byte Bus::test_peek(Address addr) {
	auto it = test_memory.find(addr & 0xFFFFFF);
	return it != test_memory.end() ? it->second : 0x00;
}

void Bus::test_poke(Address addr, Byte value) {
	test_memory[addr & 0xFFFFFF] = value;
}