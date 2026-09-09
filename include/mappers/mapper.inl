template<typename MapperT>
u8 Mapper<MapperT>::read(SNESAddress address) {
	if (auto idx = derived().rom_idx(address)) {
		u8 value = rom[(*idx) % rom.size()];
		cpu->set_open_bus(value);
		return value;
	}

	if (auto idx = derived().sram_idx(address)) {
		if (sram.size() == 0) {
			return cpu->get_open_bus();
		}
		u8 value = sram[(*idx) % sram.size()];
		cpu->set_open_bus(value);
		return value;
	}

	return cpu->get_open_bus();
}

template<typename MapperT>
void Mapper<MapperT>::write(SNESAddress address, u8 value) {
	if (auto idx = derived().sram_idx(address)) {
		if (sram.size() == 0) {
			return;
		}
		sram[(*idx) % sram.size()] = value;
	}
}