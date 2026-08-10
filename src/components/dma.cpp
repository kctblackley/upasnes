#include "dma.hpp"
#include "bus.hpp"

Byte DMA::get_open_bus() {
	return bus->get_open_bus();
}

void DMA::set_open_bus(Byte value) {
	bus->set_open_bus(value);
}

// For these implementations, the state machine goes master cycle by master cycle
// So that HDMA can suspend GPDMA when GPDMA is in the middle of a particular operation

void DMA::tick_hdma() {
	hdma_active = false; // Just cancel immediately for now
	return;
}

// SIMPLE VERSION FOR NOW, WILL NEED TO ALTER FOR HDMA WHICH MUST KNOW WHEN A UNIT HAS ENDED
uint8_t DMA::get_b_bus(Byte bbad, Byte transfer_unit_select, uint32_t byte_tick) {
	switch (transfer_unit_select) {
		case 0: {
			return bbad;
			break;
		}
		case 1: {
			Byte bit = byte_tick & 1;
			return bbad + bit;
			break;
		}
		case 2:
		case 6: {
			return bbad;
			break;
		}
		case 3:
		case 7: {
			Byte bit = (byte_tick >> 1) & 1;
			return bbad + bit;
			break;
		}
		case 4: {
			Byte bit = byte_tick & 3;
			return bbad + bit;
			break;
		}
		case 5: {
			Byte bit = byte_tick & 1;
			return bbad + bit;
		}
	}
	return bbad;
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
				std::cout << "GPDMA Channel Init " <<  std::dec << *cpu_cycle << "\n";
				
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

				std::cout << "Transfer direction: "   << std::dec << (int)gpdma.transfer_direction   << "\n";
				std::cout << "A-Bus Address Step: "   << std::hex << (int)gpdma.a_bus_address_step   << "\n";
				std::cout << "Transfer Unit Select: " << std::dec << (int)gpdma.transfer_unit_select << "\n";
				std::cout << "B-Bus Address: "        << std::hex << (int)(0x2100 | gpdma.bbad)      << "\n";
				std::cout << "Bytes to transfer "     << std::hex << (int)gpdma.byte_counter         << "\n";
				std::cout << "Channel number: "       << std::dec << (int)gpdma.channel_number       << "\n";

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
				std::cout << "Execution will continue from cycle " << std::dec << ((*cpu_cycle) + 1) << "\n";
				gpdma_active = false;
				gpdma.state = GPDMAState::None;
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