

#include "dma.hpp"
#include "bus.hpp"
#include "ppu.hpp"

Byte DMA::get_open_bus() {
	return bus->get_open_bus();
}

void DMA::set_open_bus(Byte value) {
	bus->set_open_bus(value);
}

// For these implementations, the state machine goes master cycle by master cycle
// So that HDMA can suspend GPDMA when GPDMA is in the middle of a particular operation

void DMA::hdma_init() {
	int penalty = 0;
	bool any_enabled = false;
	hdma_initialised = true;
	for (auto& ch : channels) {
		ch.reset_hdma_queue();
		ch.reset_hdma_termination();

		if (ch.register_enabled_for_hdma()) {
			any_enabled = true;

			if (ch.is_indirect()) {
				penalty += 24;
			} else {
				penalty += 8;
			}

			ch.new_indirect_address = false;
			ch.prev_indirect_address = 0x00;
			ch.reload_table_address();

			if (ch.load_descriptor()) {
				ch.terminate_hdma();
			}

			ch.new_indirect_address = false;
			ch.reload_penalty = 0;
		} else {
			ch.new_indirect_address = false;
			ch.prev_indirect_address = 0x00;
		}
	}

	if (any_enabled) {
		penalty += 18;
	}

	cpu->add_cycles(penalty);
}

void DMA::hdma_transfer() {
	int penalty = 0;
	bool any_channel_active = false;
	for (auto& ch : channels) {
		if (ch.hdma_enabled()) {
			any_channel_active = true;
			auto unit = ch.do_transfer();
			bool b_to_a = ch.get_transfer_direction();
			if (unit.transfer_this) {
				//std::cout << "TRANSFER OCCURRING\n";
				int unit_size = transfer_units[unit.unit_type].size;
				Address a_bus = unit.a_bus;
				for (int i = 0; i < unit_size; i++) {
					uint8_t b_bus = get_b_bus(unit.b_bus, unit.unit_type, i);
					if (b_to_a) {
						transfer_b_to_a(b_bus, a_bus);
					} else {
						transfer_a_to_b(a_bus, b_bus);
					}
					unit.cycle_penalty += 8; // 8 cycles as penalty per byte transferred
					ch.increment_a_bus_address(a_bus);
				}
			}
			penalty += unit.cycle_penalty; // calculated during above transfer loop (if a transfer did occur)
			penalty += 8; // baseline 8-cycle penalty (exists whether or not transfer actually occurred)
		}
	}
	if (any_channel_active) {
		penalty += 18;
	}
	cpu->add_cycles(penalty);
}

uint8_t DMA::get_b_bus(Byte bbad, Byte transfer_unit_select, uint32_t byte_tick) {
	TransferUnit unit = transfer_units[transfer_unit_select];
	int idx = byte_tick & (unit.size - 1);
	int offset = unit.pattern[idx];
	return bbad + offset;
}

void DMA::tick_gpdma() {
	if (gpdma.cycle > 0) {
		gpdma.cycle--;
	}
	if (gpdma.cycle <= 0) {
		switch (gpdma.state) {
			case GPDMAState::GPDMAInit: {
				gpdma.cycle = 8;
				gpdma.state = GPDMAState::ChannelInit;
				gpdma.ch = get_earliest_gpdma_channel();
				break;
			}
			case GPDMAState::ChannelInit: {
				//std::cout << "GPDMA Channel Init " <<  std::dec << *cpu_cycle << "\n";
				
				DMAChannel* ch = gpdma.ch;
				gpdma.transfer_direction = ch->get_transfer_direction();
				gpdma.a_bus_address_step = ch->get_a_bus_address_step();
				gpdma.transfer_unit_select = ch->get_transfer_unit_select();
				gpdma.bbad = ch->get_bbad();
				gpdma.byte_counter = ch->get_byte_counter();
				gpdma.byte_tick = 0;
				gpdma.channel_number = ch->channel_number;

				if (gpdma.byte_counter == 0) {
					gpdma.byte_counter = 0x10000;
				}

				/*std::cout << "Transfer direction: "   << std::dec << (int)gpdma.transfer_direction   << "\n";
				std::cout << "A-Bus Address Step: "   << std::hex << (int)gpdma.a_bus_address_step   << "\n";
				std::cout << "Transfer Unit Select: " << std::dec << (int)gpdma.transfer_unit_select << "\n";
				std::cout << "B-Bus Address: "        << std::hex << (int)(0x2100 | gpdma.bbad)      << "\n";
				std::cout << "Bytes to transfer "     << std::hex << (int)gpdma.byte_counter         << "\n";
				std::cout << "Channel number: "       << std::dec << (int)gpdma.channel_number       << "\n";*/

				gpdma.cycle = 8;
				gpdma.state = GPDMAState::TransferByte;
				break;
			}
			case GPDMAState::TransferByte: {
				//std::cout << "GPDMA Transfer Bytes " <<  std::dec << *cpu_cycle << "\n";
				//gpdma.cycle = 8; // what it should be if not the final transfer byte
				// for the final transfer byte

				Address a_bus = gpdma.ch->get_a_bus();
				uint8_t b_bus = get_b_bus(gpdma.bbad, gpdma.transfer_unit_select, gpdma.byte_tick);

				if (gpdma.a_bus_address_step == 0) {
					gpdma.ch->increment_a_bus();
				}
				if (gpdma.a_bus_address_step == 2) {
					gpdma.ch->decrement_a_bus();
				}

				if (gpdma.transfer_direction) {
					transfer_b_to_a(b_bus, a_bus);
				} else {
					transfer_a_to_b(a_bus, b_bus);
				}
				
				gpdma.byte_counter--;
				gpdma.byte_tick++;

				gpdma.ch->update_das(gpdma.byte_counter);

				if (gpdma.byte_counter == 0) {
					gpdma.ch->disable_gpdma();
					gpdma.ch = get_earliest_gpdma_channel();

					if (gpdma.ch) {
						gpdma.cycle = 8;
						gpdma.state = GPDMAState::ChannelInit;
					} else {
						gpdma.cycle = (7 - (*cpu_cycle & 7)) & 7;
						gpdma.state = GPDMAState::End;
					}
				} else {
					gpdma.cycle = 8;
					gpdma.state = GPDMAState::TransferByte;
				}
				break;
			}
			case GPDMAState::End: {
				//std::cout << "GPDMA End " <<  std::dec << *cpu_cycle << "\n";
				///std::cout << "Execution will continue from cycle " << std::dec << ((*cpu_cycle) + 1) << "\n";
				gpdma_active = false;
				gpdma.state = GPDMAState::None;
				ppu->log_ppu_data();
				break;
			}
		}
	}
}

Byte DMA::dma_read(SNESAddress addr) {
	Address address = (addr.bank << 16) | addr.offset;
	return bus->read(address, true);
}

void DMA::dma_write(SNESAddress addr, Byte value) {
	Address address = (addr.bank << 16) | addr.offset;
	bus->write(address, value, true);
}