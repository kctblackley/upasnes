#pragma once
#include "component.hpp"
#include "common.hpp"

#define MDMAEN_ADDRESS 0x420B
#define HDMAEN_ADDRESS 0x420C

class Bus;

class DMAChannel {
public:

	Byte read(int register_number) {
		switch (register_number) {
			case 0x0: return dmap; break;
			case 0x1: return bbad; break;
			case 0x2: return a1tl; break;
			case 0x3: return a1th; break;
			case 0x4: return a1b;  break;
			case 0x5: return dasl; break;
			case 0x6: return dash; break;
			case 0x7: return dasb; break;
			case 0x8: return a2al; break;
			case 0x9: return a2ah; break;
			case 0xA: return ntrl; break;
		}
		return unused;
	}

	void write(int register_number, Byte value) {
		switch (register_number) {
			case 0x0: set_dmap(value); break;
			case 0x1: set_bbad(value); break;
			case 0x2: set_a1tl(value); break;
			case 0x3: set_a1th(value); break;
			case 0x4:  set_a1b(value); break;
			case 0x5: set_dasl(value); break;
			case 0x6: set_dash(value); break;
			case 0x7: set_dasb(value); break;
			case 0x8: set_a2al(value); break;
			case 0x9: set_a2ah(value); break;
			case 0xA: set_ntrl(value); break;
			default:  unused = value;
		}
	}

	void set_dmap(Byte value) {
		transfer_direction = (value & 0x80) != 0;
		addressing_mode = (value & 0x40) != 0;
		a_bus_address_step = (value >> 3) & 3;
		transfer_unit_select = value & 7;

		dmap = value & ~0x20;
	}

	void set_bbad(Byte value) {
		bbad = value;
		b_bus_address = 0x2100 + bbad;
	}

	void set_a1tl(Byte value) {
		a1_address = (get_hi(a1_address) << 8) | value;
		a1tl = value;
	}

	void set_a1th(Byte value) {
		a1_address = (value << 8) | (get_lo(a1_address));
		a1th = value;
	}

	void set_a1b(Byte value) {
		a1_bank = value;
		a1b = value;
	}

	void set_dasl(Byte value) {
		dasl = value;
		indirect_address = (get_hi(indirect_address) << 8) | value;
	}

	void set_dash(Byte value) {
		dash = value;
		indirect_address = (value << 8) | get_lo(indirect_address);
	}

	void set_dasb(Byte value) {
		dasb = value;
		indirect_bank = value;
	}

	void set_a2al(Byte value) {
		table_address = (get_hi(table_address) << 8) | value;
		a2al = value;
	}

	void set_a2ah(Byte value) {
		table_address = (value << 8) | get_lo(table_address);
		a2ah = value;
	}

	void set_ntrl(Byte value) {
		repeat = (value & 0x80) != 0;
		lines_to_transfer = value & 0x7F;
		ntrl = value;
	}

	void enable_gpdma()  { doing_gpdma = true;  }
	void disable_gpdma() { doing_gpdma = false; }
	void enable_hdma()   { doing_hdma = true;   }
	void disable_hdma()  { doing_hdma = false;  }

private:

	// all are R/W
	Byte dmap = 0; // transfer direction, addressing mode, a-bus address step, transfer unit select (GPDMA and HDMA)
	Byte bbad = 0; // b-bus address, mapped to 0x2100 and 0x21FF -> 0x2100h + BBAD
	Byte a1tl = 0; // HDMA table start address (low) or DMA current address (low)
	Byte a1th = 0; // same, but high
	Byte a1b  = 0; // gives bank

	bool transfer_direction = false; // 0 = A:CPU to B:I/O, 1=B:I/O to A:CPU
	bool addressing_mode = false; // 0 = direct table, 1 = indirect table (HDMA only)
	Byte a_bus_address_step = 0; // 0 = increment, 2 = decrement, 1/3 = fixed
	Byte transfer_unit_select = 0; // from the pattern table

	Word b_bus_address = 0; // equal to 0x2100 + bbad

	Byte  a1_bank = 0;
	Word  a1_address = 0;
	Byte& hdma_table_bank   = a1_bank;
	Word& hdma_table_reload = a1_address;

	// a1tl, a1th, a1b in GPDMA and HDMA
	// In GPDMA: 23-16 gives constant CPU-BUs Data Address Bank and 15-0 gives the data address which is either increment/decremented or kept fixed
	// In HDMA: same as above, but its the table bank and table address, both are kept constant
	// In HDMA: table bank is the bank number for a2al and a2ah, effectively acting as a2b as well as a1b
	// In HDMA: table address is a constant and acts as the reload value for a2al and a2ah

	Byte dasl = 0; // indirect HDMA address or DMA byte counter (low)
	Byte dash = 0; // same as above, but high
	Byte dasb = 0; // indirect HDMA Address bank

	uint32_t byte_counter = 0; // not set in registers, set when GPDMA begins
	Byte indirect_bank = 0;
	Word indirect_address = 0;

	// In GPDMA: do not use 23-16, 15-0 acts as a byte counter (NOT unit-counter, just a BYTE counter) -> 0 means 10000h
	// In direct HDMA: 23-0 is not used (data is read directly from table)
	// In indirect HDMA: 23-16 is set by software (current CPU-Bus data address bank), and 16-0 is the current CPU-Bus data address automatically loaded from the table

	Byte a2al = 0; // HDMA table current address (low)
	Byte a2ah = 0; // HDMA table current address (high)

	Word table_address = 0;

	// In GPDMA: not used/
	// In HDMA: the current table address bank is taken from a1b, and 15-0 is the current table address that is reloaded from a1tl and a1th

	Byte ntrl = 0; // HDMA line-counter from the current table entry
	
	bool repeat = 0;
	Byte lines_to_transfer = 0;

	// In GPDMA: not used
	// In HDMA: 7 is the repeat flag (loaded from table), 6-0 number of lines to be transferred (decremented per scanline)

	Byte unused = 0; // an unused byte corresponding to 43xB, but can be used as a fast RAM location

	// Below are handled in DMA class itself
	// 43xC to 43xE are open bus
	// 43xF is a mirror to 43xB

	bool doing_gpdma = false;
	bool doing_hdma  = false;
};

class DMA : public Component {
public:
	DMA() {}

	Byte get_open_bus();
	void set_open_bus(Byte value);

	void set_mdmaen(Byte value) {
		int ch = 0;

		if (value) {
			gpdma_active = true;
		} else {
			gpdma_active = false;
		}

		while (ch < 8) {
			bool bit = value & 1;
			value = value >> 1;
			if (bit) {
				channels[ch].enable_gpdma();
			} else {
				channels[ch].disable_gpdma();
			}
			ch++;
		}
	}

	void set_hdmaen(Byte value) {
		int ch = 0;

		if (value) {
			hdma_enabled = true;
		} else {
			hdma_enabled = false;
		}

		while (ch < 8) {
			bool bit = value & 1;
			value = value >> 1;
			if (bit) {
				channels[ch].enable_hdma();
			} else {
				channels[ch].disable_hdma();
			}
			ch++;
		}
	}

	int get_register_number(SNESAddress address) {
		int register_number = address.offset & 0xF;
		if (register_number >= 0xC && register_number <= 0xE) {
			return -1; // -1 means open bus
		}
		if (register_number == 0xF) {
			return 0xB;
		}
		return register_number;
	}

	int get_channel_number(SNESAddress address) {
		int channel_number = (address.offset >> 4) & 0xF;
		if (channel_number > 7) {
			return -1;
		}
		return channel_number;
	}

	Byte communication_read(SNESAddress address) {
		Byte fetched = get_open_bus();

		if (address.offset >= 0x4300 && address.offset <= 0x437F) {
			int channel_number  = get_channel_number(address);
			int register_number = get_register_number(address);
			if (register_number >= 0 && channel_number >= 0) {
				DMAChannel& ch = channels[get_channel_number(address)];
				fetched = ch.read(register_number);
			}
		}

		if (fetched != get_open_bus()) {
			set_open_bus(fetched);
		}
		// Set open bus here
		return fetched;
	}

	void communication_write(SNESAddress address, Byte value) {
		set_open_bus(value);
		if (address.offset >= 0x4300 && address.offset <= 0x437F) {
			int channel_number  = get_channel_number(address);
			int register_number = get_register_number(address);
			if (register_number >= 0 && channel_number >= 0) {
				DMAChannel& ch = channels[get_channel_number(address)];
				ch.write(register_number, value);
			}
			return;
		}

		if (address.offset == MDMAEN_ADDRESS) {
			set_mdmaen(value);
		}
		if (address.offset == HDMAEN_ADDRESS) {
			set_hdmaen(value);
		}
	}

	bool dma_active() {
		return hdma_active || gpdma_active;
	}

	// More steps for HDMA to actually occur
	bool hdma_enabled = false; // HDMAEN updates this
	bool hdma_initialised = false; // when hdma_init() is called, only do so if hdma is enabled

	bool hdma_active  = false; // set to active only when there is HBlank and HDMA is enabled
	bool gpdma_active = false; // GPDMA begins as soon as MDMAEN is written to (that is, a few cycles after)

	void connect_bus(Bus* bus) {
		this->bus = bus;
	}

	// Unimplemented to allow this to be a Component*

	void add_cycles(CycleCount cycles) override { return; }
	void tick_component() override { return; }
	CycleCount get_cycle() override { return 0; }
	TickCount get_tick() override { return 0; }

	Byte read(Address addr) override { return 0; }
	void write(Address addr, Byte value) override { return; }


private:
	DMAChannel channels[8] {};
	Bus* bus = nullptr;
};