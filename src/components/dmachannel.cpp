#include "dma.hpp"
#include "bus.hpp"

bool DMAChannel::load_descriptor() {
	Byte src_bank = 0x00;
	Word src_address = 0x00;

	new_indirect_address = false;
	reload_penalty = 24;
	set_ntrl(read_a_bus());
	if (ntrl == 0) {
		return true;
	}
	if (addressing_mode) { // Indirect
		increment_table_address();
		set_dasl(read_a_bus());
		increment_table_address();
		set_dash(read_a_bus());
		reload_penalty += 16;

		if (indirect_address != prev_indirect_address) {
			new_indirect_address = true;
		}

		prev_indirect_address = indirect_address;

		src_bank = indirect_bank;
		src_address = indirect_address;

		increment_table_address(); 

	} else { // Direct

		increment_table_address();

		src_bank = a1_bank;
		src_address = table_address;

	}

	if (repeat) { 

		int unit_size = transfer_units[transfer_unit_select].size;
		
		for (int i = 0; i < lines_to_transfer; i++) {
			Address src = (src_bank << 16) | src_address;

			Byte ntrl_after = 0x80 | ((lines_to_transfer - (i + 1)) & 0x7F);
			push_unit(src, bbad, true, transfer_unit_select, 0, ntrl_after);

			src_address += unit_size;
		}

	} else {

		Address src = (src_bank << 16) | src_address;

		push_unit(src, bbad, true, transfer_unit_select, 0, (lines_to_transfer - 1) & 0x7F);

		for (int i = 1; i < lines_to_transfer; i++) {
			Byte ntrl_after = (lines_to_transfer - (i + 1)) & 0x7F;
			push_unit(src, bbad, false, transfer_unit_select, 0, ntrl_after);
		}

		src_address += transfer_units[transfer_unit_select].size;

	}

	if (!addressing_mode) {
		set_table_address(src_address);
	}

	return false;
}

Unit DMAChannel::do_transfer() {
	bool terminated = false;
	if (hdma_units.empty()) {
		terminated = load_descriptor();
		if (terminated) {
			terminate_hdma();
			Unit unit = default_unit;
			unit.cycle_penalty += reload_penalty;
			reload_penalty = 0;
			return unit;
		}
	}
	if (!terminated) {
		auto unit = pop_unit();
		unit.cycle_penalty += reload_penalty;
		reload_penalty = 0;
		if (new_indirect_address) {
			unit.cycle_penalty += 16;
			new_indirect_address = false;
		}
		ntrl = unit.ntrl_after;
		return unit;
	}
	return default_unit;
}

Byte DMAChannel::read_a_bus() {
	SNESAddress snes_address = SNESAddress{a1_bank, table_address};
	if (is_forbidden_a_bus_address(snes_address)) {
		return bus->get_open_bus();
	}
	return bus->read((a1_bank << 16) | table_address, true);
}