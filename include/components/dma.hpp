#pragma once
#include "component.hpp"
#include "common.hpp"
#include <deque>

#define MDMAEN_ADDRESS 0x420B
#define HDMAEN_ADDRESS 0x420C

class Bus;
class PPU;
class Ricoh5A22;

struct TransferUnit {
	std::array<int, 4> pattern {};
	int size = 0;
};

constexpr std::array<TransferUnit, 8> transfer_units = {
	TransferUnit{{0, 0, 0, 0}, 1}, // Mode 0
	TransferUnit{{0, 1, 0, 0}, 2}, // Mode 1
	TransferUnit{{0, 0, 0, 0}, 2}, // Mode 2
	TransferUnit{{0, 0, 1, 1}, 4}, // Mode 3
	TransferUnit{{0, 1, 2, 3}, 4}, // Mode 4
	TransferUnit{{0, 1, 0, 1}, 4}, // Mode 5
	TransferUnit{{0, 0, 0, 0}, 2}, // Mode 6
	TransferUnit{{0, 0, 1, 1}, 4}  // Mode 7
};

struct Unit {
	Address a_bus = 0; // This is the A-Bus address of the starting unit value, DMA reads the entirety of the unit
	uint8_t b_bus = 0; // Whatever bbad is for the HDMA channel
	bool transfer_this = false; // Should that unit be transferred?
	int unit_type = 0; // Used to know the actual unit type (on the DMA controller side)
	int cycle_penalty = 0; // DMA controller calculates this cycle penalty value (either zero or non-zero based on descriptor loading)
	Byte ntrl_after = 0; // CPU-visible NLTR (0x43xA) value once this scanline's line has been consumed
};

constexpr Unit default_unit = Unit{0, 0, false, 0, 0, 0};

class DMAChannel {
public:
	bool is_forbidden_a_bus_address(SNESAddress addr) {
		if ((addr.bank >= 0x00 && addr.bank <= 0x3F) || (addr.bank >= 0x80 && addr.bank <= 0xBF)) {
			return (addr.offset >= 0x2100 && addr.offset <= 0x21FF) ||
			       (addr.offset >= 0x4000 && addr.offset <= 0x421F) ||
			       (addr.offset >= 0x4300 && addr.offset <= 0x437F);
		}
		return false;
	}

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

		dmap = value;
	}

	void set_bbad(Byte value) {
		bbad = value;
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
		if (lines_to_transfer == 0) {
			lines_to_transfer = 128;
		}
		ntrl = value;
	}

	bool is_indirect() {
		return addressing_mode;
	}

	void enable_gpdma()  { doing_gpdma = true;  }
	void disable_gpdma() { doing_gpdma = false; }

	void enable_hdma()   { hdma_reg_enabled = true;   }
	void disable_hdma()  { hdma_reg_enabled = false;  }

	void terminate_hdma()          { hdma_terminated = true;  }
	void reset_hdma_termination()  { hdma_terminated = false; }

	bool gpdma_enabled() const { return doing_gpdma; }

	bool hdma_enabled() const { return hdma_reg_enabled && !hdma_terminated; }

	bool register_enabled_for_hdma() const { return hdma_reg_enabled; }

	// a1b stays put
	void increment_a_bus() {
		a1_address++;
	}

	void decrement_a_bus() {
		a1_address--;
	}

	void set_table_address(Word value) {
		table_address = value;
		a2al = get_lo(value);
		a2ah = get_hi(value);
	}

	void reload_table_address() {
		set_table_address(a1_address);
	}

	void increment_table_address() {
		set_table_address(table_address + 1);
	}

	void increment_a_bus_address(Address& addr) {
		uint8_t bank = (addr >> 16) & 0xFF;
		uint16_t address = addr & 0xFFFF;
		address++;
		addr = (bank << 16) | address;
	}

	void decrement_a_bus_address(Address& addr) {
		uint8_t bank = (addr >> 16) & 0xFF;
		uint16_t address = addr & 0xFFFF;
		address--;
		addr = (bank << 16) | address;
	}


	Address get_a_bus() {
		return (a1_bank << 16) | a1_address;
	}

	int channel_number = 0;

	bool get_transfer_direction() const {
	    return transfer_direction;
	}

	Byte get_a_bus_address_step() const {
	    return a_bus_address_step;
	}

	Byte get_transfer_unit_select() const {
	    return transfer_unit_select;
	}

	Byte get_bbad() const {
	    return bbad;
	}

	uint32_t get_byte_counter() const {
	    return (dash << 8) | dasl;
	}

	void update_das(uint32_t byte_counter) {
		dasl = byte_counter & 0xFF;
		dash = (byte_counter >> 8) & 0xFF;
	}

	bool load_descriptor();
	Unit do_transfer();

	void push_unit(Address a_bus, uint8_t b_bus, bool transfer_this, int unit_type, int cycle_penalty, Byte ntrl_after) {
		hdma_units.push_back(Unit{a_bus, b_bus, transfer_this, unit_type, cycle_penalty, ntrl_after});
	}

	Unit pop_unit() {
		if (hdma_units.empty()) {
			return default_unit;
		}

		Unit unit = hdma_units.front();
		hdma_units.pop_front();
		return unit;
	}

	void reset_hdma_queue() {
		hdma_units.clear();
	}

	void connect_bus(Bus* bus) {
		this->bus = bus;
	}

	Byte read_a_bus();

	bool new_indirect_address = false;
	Word prev_indirect_address = 0x00;

	CycleCount reload_penalty = 0;

private:

	Bus* bus = nullptr;

	std::deque<Unit> hdma_units {};

	ZByte dmap = 0xFF; // transfer direction, addressing mode, a-bus address step, transfer unit select (GPDMA and HDMA)
	Byte bbad = 0xFF; // b-bus address, mapped to 0x2100 and 0x21FF -> 0x2100h + BBAD
	Byte a1tl = 0xFF; // HDMA table start address (low) or DMA current address (low)
	Byte a1th = 0xFF; // same, but high
	Byte a1b  = 0xFF; // gives bank

	bool transfer_direction = true; // 0 = A:CPU to B:I/O, 1=B:I/O to A:CPU (derived from dmap=0xFF)
	bool addressing_mode = true; // 0 = direct table, 1 = indirect table (HDMA only) (derived from dmap=0xFF)
	Byte a_bus_address_step = 3; // 0 = increment, 2 = decrement, 1/3 = fixed (derived from dmap=0xFF)
	Byte transfer_unit_select = 7; // from the pattern table (derived from dmap=0xFF)

	Byte  a1_bank = 0xFF;
	Word  a1_address = 0xFFFF;
	Byte& hdma_table_bank   = a1_bank;
	Word& hdma_table_reload = a1_address;

	// a1tl, a1th, a1b in GPDMA and HDMA
	// In GPDMA: 23-16 gives constant CPU-BUs Data Address Bank and 15-0 gives the data address which is either increment/decremented or kept fixed
	// In HDMA: same as above, but its the table bank and table address, both are kept constant
	// In HDMA: table bank is the bank number for a2al and a2ah, effectively acting as a2b as well as a1b
	// In HDMA: table address is a constant and acts as the reload value for a2al and a2ah

	Byte dasl = 0xFF; // indirect HDMA address or DMA byte counter (low)
	Byte dash = 0xFF; // same as above, but high
	Byte dasb = 0xFF; // indirect HDMA Address bank

	uint32_t byte_counter = 0; // not set in registers, set when GPDMA begins
	Byte indirect_bank = 0xFF;
	Word indirect_address = 0xFFFF;

	// In GPDMA: do not use 23-16, 15-0 acts as a byte counter (NOT unit-counter, just a BYTE counter) -> 0 means 10000h
	// In direct HDMA: 23-0 is not used (data is read directly from table)
	// In indirect HDMA: 23-16 is set by software (current CPU-Bus data address bank), and 16-0 is the current CPU-Bus data address automatically loaded from the table

	Byte a2al = 0xFF; // HDMA table current address (low)
	Byte a2ah = 0xFF; // HDMA table current address (high)

	Word table_address = 0xFFFF;

	// In GPDMA: not used/
	// In HDMA: the current table address bank is taken from a1b, and 15-0 is the current table address that is reloaded from a1tl and a1th

	Byte ntrl = 0xFF; // HDMA line-counter from the current table entry
	
	bool repeat = true; // derived from ntrl=0xFF
	Byte lines_to_transfer = 128; // derived from ntrl=0xFF (0x7F count wraps to 128)

	// In GPDMA: not used
	// In HDMA: 7 is the repeat flag (loaded from table), 6-0 number of lines to be transferred (decremented per scanline)

	Byte unused = 0xFF; // an unused byte corresponding to 43xB, but can be used as a fast RAM location

	// Below are handled in DMA class itself
	// 43xC to 43xE are open bus
	// 43xF is a mirror to 43xB

	bool doing_gpdma = false;

	bool hdma_reg_enabled = false; // mirrors HDMAEN bit; persists across frames
	bool hdma_terminated  = false; // this frame's table hit 0x00; reset every hdma_init()
};

enum class GPDMAState {
	GPDMAInit,
	ChannelInit,
	TransferByte,
	End,
	None
};

struct GPDMA {
	bool transfer_direction = false;
	Byte a_bus_address_step = 0;
	Byte transfer_unit_select = 0;
	Byte bbad = 0;
	uint32_t byte_counter = 0;
	uint32_t byte_tick = 0;
	DMAChannel* ch = nullptr;
	GPDMAState state = GPDMAState::None;
	int channel_number = 0;
	CycleCount cycle = 0;
};

struct HDMA {

};

class DMA : public Component {
public:
	bool is_forbidden_a_bus_address(SNESAddress addr) {
		if ((addr.bank >= 0x00 && addr.bank <= 0x3F) || (addr.bank >= 0x80 && addr.bank <= 0xBF)) {
			return (addr.offset >= 0x2100 && addr.offset <= 0x21FF) ||
			       (addr.offset >= 0x4000 && addr.offset <= 0x421F) ||
			       (addr.offset >= 0x4300 && addr.offset <= 0x437F);
		}
		return false;
	}

	DMA() {

		int channel_number = 0;
		for (auto& ch : channels) {
			ch.channel_number = channel_number;
			channel_number++;
		}

	}

	// HDMA occurs atomically
	void hdma_init();
	void hdma_transfer();

	// GPDMA is cycle-stepped
	void tick_gpdma();

	Byte get_open_bus();
	void set_open_bus(Byte value);

	DMAChannel* get_earliest_gpdma_channel() {
		for (auto& ch : channels) {
			if (ch.gpdma_enabled()) {
				return &ch;
			}
		}
		return nullptr;
	}

	DMAChannel* get_earliest_hdma_channel() {
		for (auto& ch : channels) {
			if (ch.hdma_enabled()) {
				return &ch;
			}
		}
		return nullptr;
	}

	void set_mdmaen(Byte value) {
		int ch = 0;

		mdmaen_value = value;

		if (value) {
			//std::cout << "GPDMA TRIGGERED AT CPU CYCLE " << std::dec << (*cpu_cycle)  << "\n";
			gpdma_pending = true;
		} else {
			//std::cout << "GPDMA TRIGGERED AT CPU CYCLE " << std::dec << (*cpu_cycle)  << "\n";
			gpdma_pending = false;
			gpdma_active  = false;
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

		hdmaen_value = value;

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

		if (address.offset == MDMAEN_ADDRESS) {
			fetched = mdmaen_value;
		} else if (address.offset == HDMAEN_ADDRESS) {
			fetched = hdmaen_value;
		} else if (address.offset >= 0x4300 && address.offset <= 0x437F) {
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

	void log_gpdma() {
		return;
	}

	void gpdma_init() {
		//std::cout << "GPDMA INIT AT CPU CYCLE " << std::dec << (*cpu_cycle)  << "\n";
		gpdma_pending = false;
		gpdma_active = true;
		gpdma.state = GPDMAState::GPDMAInit;
		log_gpdma();
	}

	// More steps for HDMA to actually occur
	bool hdma_enabled = false; // HDMAEN updates this
	bool hdma_initialised = false; // when hdma_init() is called, only do so if hdma is enabled

	bool hdma_active   = false; // set to active only when there is HBlank and HDMA is enabled
	bool gpdma_active  = false; // GPDMA begins as soon as MDMAEN is written to (that is, a few cycles after)
	bool gpdma_pending = false;

	void connect_bus(Bus* bus) {
		this->bus = bus;
		for (auto& ch : channels) {
			ch.connect_bus(bus);
		}
	}

	// Unimplemented to allow this to be a Component*

	void add_cycles(CycleCount cycles) override { return; }
	void tick_component() override { return; }
	CycleCount get_cycle() override { return 0; }
	TickCount get_tick() override { return 0; }

	Byte read(Address addr) override { return 0; }
	void write(Address addr, Byte value) override { return; }

	void connect_cpu_cycle_counter(CycleCount* cpu_cycle) {
		this->cpu_cycle = cpu_cycle;
	}

	uint8_t get_b_bus(Byte bbad, Byte transfer_unit_select, uint32_t byte_tick);

	Byte dma_read(SNESAddress addr);
	void dma_write(SNESAddress addr, Byte value);

	Byte a_bus_read(SNESAddress addr) {
		if (is_forbidden_a_bus_address(addr)) {
			return get_open_bus();
		}
		return dma_read(addr);
	}

	void a_bus_write(SNESAddress addr, Byte value) {
		if (is_forbidden_a_bus_address(addr)) {
			return;
		}
		return dma_write(addr, value);
	}

	Byte read_from_a_bus(Address addr) {
		SNESAddress address = to_snes_address(addr);
		if (is_forbidden_a_bus_address(address)) {
			return get_open_bus();
		}
		return dma_read(address);
	}

	Byte b_bus_read(uint8_t b_bus) {
		SNESAddress addr;
		addr.offset = 0x2100 | b_bus;
		addr.bank = 0x00;
		return dma_read(addr);
	}

	void b_bus_write(uint8_t b_bus, Byte value) {
		SNESAddress addr;
		addr.offset = 0x2100 | b_bus;
		addr.bank = 0x00;
		dma_write(addr, value);
	}

	void transfer_a_to_b(Address a_bus, uint8_t b_bus) {
		SNESAddress address = to_snes_address(a_bus);
		Byte value = a_bus_read(address);
		b_bus_write(b_bus, value);
	}

	void transfer_b_to_a(uint8_t b_bus, Address a_bus) {
		SNESAddress address = to_snes_address(a_bus);
		Byte value = b_bus_read(b_bus);
		a_bus_write(address, value);
	}

	void connect_ppu(PPU* ppu) {
		this->ppu = ppu;
	}

	void give_dma_access_to_cpu(Ricoh5A22* cpu) {
		this->cpu = cpu;
	}

private:
	CycleCount* cpu_cycle = nullptr;
	DMAChannel channels[8] {};
	Bus* bus = nullptr;
	PPU* ppu = nullptr;
	Ricoh5A22* cpu = nullptr;

	Byte mdmaen_value = 0; // raw last-written value, for readback of $420B
	Byte hdmaen_value = 0; // raw last-written value, for readback of $420C

	GPDMA gpdma;
	HDMA hdma;
};