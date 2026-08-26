#pragma once
#include "common.hpp"
#include <iostream>

// (Second attempt at) implementation based on bsnes

#define LARGE_GAME_PAK_RAM_SIZE 65536
#define SMALL_GAME_PAK_RAM_SIZE 32768
#define BACKUP_RAM_SIZE 32768

enum class SuperFXRevision {
	None,
	MARIO,
	GSU1,
	GSU2,
	GSU2SP1 // functionally-identical to GSU2, I'm just including detection of this revision for the sake of it
};

enum class WaitingState {
	None,
	RON,
	RAN
};

struct PixelCache {
	Byte data[8] = {0};
	bool flags[8] = {false};
	bool has_data = false;
	Byte x = 0;
	Byte y = 0;
};

struct ALUResult {
	Word value;
	bool carry;
};

class Ricoh5A22;
class Cartridge;

// Inspired by bsnes, easy handling for registers for flags

struct Bit {
	Word* data;
	Word mask;

	operator bool() const {
		return (*data & mask) != 0;
	}

	Bit& operator=(bool value) {
		if (value) {
			*data = *data | mask;
		} else {
			*data = *data & ~mask;
		}
		return *this;
	}
};

class SuperFX {
public:
	SuperFX() {
		initialise();
	}

	void initialise() {
		for (auto& reg : r) {
			reg.data = 0x0000;
			reg.modified = false;
		}

		sfr = 0x0000;
		pbr = 0x00;
		rombr = 0x00;
		rambr = 0;
		cbr = 0x0000;
		scbr = 0x00;
		scmr = 0x00;
		colr = 0x00;
		por = 0x00;
		bramr = 0;
		vcr = 0x04;
		cfgr = 0x00;
		clsr = 0;
		pipeline = 0x01;
		ramaddr = 0x0000;
		reset_registers();
	}

	void tick_component();

	void build_game_pak_ram(int checksum) {
		if (checksum == 0xAB12 || checksum == 0x4DBF || checksum == 0x132C) {
			gpram_size = SMALL_GAME_PAK_RAM_SIZE;
		}
		gpram.assign(gpram_size, 0);
		backup_ram.assign(BACKUP_RAM_SIZE, 0);
	}

	void set_revision(SuperFXRevision revision) {
		this->revision = revision;

		if (this->revision == SuperFXRevision::GSU2SP1) {
			this->revision = SuperFXRevision::GSU2; // GSU2SP1 is same as GSU2
		}

		if (this->revision == SuperFXRevision::MARIO) {
			cycles_per_clock = 2;
		}
	}

	void set_mapper_type(bool hirom_mapper) {
		this->hirom_mapper = hirom_mapper;
	}

	Byte get_open_bus();

	void connect_cpu(Ricoh5A22* cpu) {
		this->cpu = cpu;
	}

	void connect_cartridge(Cartridge* cartridge) {
		this->cartridge = cartridge;
	}

	CycleCount get_coprocessor_cycle() {
		return cycle;
	}

private:
	CycleCount cycle = 0;
	CycleCount cycles_per_clock = 1;

	SuperFXRevision revision = SuperFXRevision::None;

	size_t gpram_size = LARGE_GAME_PAK_RAM_SIZE;
	std::vector<Byte> gpram {}; // gpram means 'Game Pak RAM'
	std::vector<Byte> backup_ram {};

	bool hirom_mapper = false;

	Ricoh5A22* cpu = nullptr;
	Cartridge* cartridge = nullptr;

	// Registers (implementation inspired by bsnes)

	Byte pipeline;
	Word ramaddr;

	// Major credits to bsnes for this! This idea made my life easier. From gsu/registers.hpp
	struct Register {
		Word data = 0;
		bool modified = false;

		operator Word() const {
			return data;
		}

		Word assign(Word value) {
			modified = true;
			data = value;
			return data;
		}

		auto operator++() { return assign(data + 1); }
		auto operator--() { return assign(data - 1); }
		auto operator=(Word i) { return assign(i); }
		auto operator=(const Register& value) { return assign(value); }

		Register() = default;
		Register(const Register&) = delete;
	} r[16];

	Byte pbr;
	Byte rombr;
	bool rambr;
	Word cbr;
	Byte scbr;
	Byte colr;
	bool bramr;
	Byte vcr;
	bool clsr;

	Byte romdr;
	Word ramar;
	Byte ramdr;

	Word sreg;
	Word dreg;

	Register& sr() { return r[sreg]; }
	Register& dr() { return r[dreg]; }

	void reset_registers() {
		sfr.b = 0;
		sfr.alt1 = 0;
		sfr.alt2 = 0;

		sreg = 0;
		dreg = 0;
	}

	// SFR
	struct SFR {
		Word data = 0;

		Bit z    {&data, Word(1) << 1};
		Bit cy   {&data, Word(1) << 2};
		Bit s    {&data, Word(1) << 3};
		Bit ov   {&data, Word(1) << 4};
		Bit g    {&data, Word(1) << 5};
		Bit r    {&data, Word(1) << 6};

		Bit alt1 {&data, Word(1) << 8};
		Bit alt2 {&data, Word(1) << 9};
		Bit il   {&data, Word(1) << 10};
		Bit ih   {&data, Word(1) << 11};
		Bit b    {&data, Word(1) << 12};
		Bit irq  {&data, Word(1) << 15};

		constexpr operator Word() const {
			return data & 0x9F7E;
		}

		SFR& operator=(Word value) {
			data = value;
			return *this;
		}

		void set_bit(int bit) {
			data = data | (1 << bit);
		}

		void clear_bit(int bit) {
			data = data & ~(1 << bit);
		}

		bool get_bit(int bit) const {
			return (data & (1 << bit)) != 0;
		}
	} sfr;

	// SCMR
	struct SCMR {
		int ht;
		bool ron;
		bool ran;
		int md;

		operator Byte() const {
			return ((ht >> 1) << 5) | (ron << 4) | (ran << 3) | ((ht & 1) << 2) | (md);
		}

		SCMR& operator=(Byte data) {
			ht  = (bool)(data & 0x20) << 1;
			ht  = ht | (bool)(data & 0x04) << 0;
			ron = data & 0x10;
			ran = data & 0x08;
			md  = data & 0x03;
			
			return *this;
		}
	} scmr;

	// POR
	struct POR {
		bool obj;
		bool freeze_high;
		bool high_nibble;
		bool dither;
		bool transparent;

		operator Byte() const {
			return (obj << 4) | (freeze_high << 3) | (high_nibble << 2) | (dither << 1) | (transparent);
		}

		POR& operator=(Word data) {
			obj = data & 0x10;
			freeze_high = data & 0x08;
			high_nibble = data & 0x04;
			dither = data & 0x02;
			transparent = data & 0x01;

			return *this;
		}
	} por;

	// CFGR

	struct CFGR {
		bool irq;
		bool ms0;

		operator Byte() const {
			return (irq << 7) | (ms0 << 5);
		}

		CFGR& operator=(Byte data) {
			irq = data & 0x80;
			ms0 = data & 0x20;
			return *this;
		}
	} cfgr;

	// Cache

	struct Cache {
		Byte buffer[512];
		bool valid[32];
	} cache;

	// Pixel Cache

	struct PixelCache {
		Word offset;
		Byte bitpend;
		Byte data[8];
	} pixel_cache[2];
};