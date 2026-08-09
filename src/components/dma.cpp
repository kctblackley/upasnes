#include "dma.hpp"
#include "bus.hpp"

Byte DMA::get_open_bus() {
	return bus->get_open_bus();
}

void DMA::set_open_bus(Byte value) {
	bus->set_open_bus(value);
}