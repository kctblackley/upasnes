template<typename MapperT>
Byte Mapper<MapperT>::read(SNESAddress address) {
	if (auto idx = derived().rom_idx(address)) {
		Byte value = rom[(*idx) % rom.size()];
		cpu->set_open_bus(value);
		return value;
	}

	if (auto idx = derived().sram_idx(address)) {
		if (sram.size() == 0) {
			return cpu->get_open_bus();
		}
		Byte value = sram[(*idx) % sram.size()];
		cpu->set_open_bus(value);
		return value;
	}

	return cpu->get_open_bus();
}

template<typename MapperT>
void Mapper<MapperT>::write(SNESAddress address, Byte value) {
	if (auto idx = derived().sram_idx(address)) {
		if (sram.size() == 0) {
			return;
		}
		sram[(*idx) % sram.size()] = value;
	}
}