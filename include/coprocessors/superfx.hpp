#pragma once
#include "common.hpp"
#include <iostream>

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

class SuperFX {
public:
	SuperFX() {
		SCMR = 0x10;
		flush_cache(0x0000);
	}

	Byte fetch_opcode(uint16_t pc);
	Byte next_opcode();

	void log_start();
	void log_sfr();
	void log_end();
	
	void try_mov(Byte opcode);
	void try_alu(Byte opcode);
	void try_jmp(Byte opcode);
	void jump(int8_t nn, bool condition);
	void execute_opcode(Byte opcode);
	void execute_next_instruction();
	void tick_component();

	Address compute_tile_address(Byte x, Byte y);
	Address mask_gpram_address(Address address);
	void write_pixel_to_ram(Address base_address, int i, Byte data);
	Byte read_pixel_from_ram(Byte x, Byte y);
	bool is_transparent_colour(Byte colour);
	bool skip_transparent();
	void set_colr(Byte incoming);
	void plot(Byte x, Byte y, Byte colour);
	void flush_primary_to_secondary();
	void flush_secondary_to_ram();
	Byte rpix(Byte x, Byte y);

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

	void set_zero_flag()     { SFR |= 2;     }
	void set_carry_flag()    { SFR |= 4;     }
	void set_sign_flag()     { SFR |= 8;     }
	void set_overflow_flag() { SFR |= 16;    }
	void set_go_flag()       { SFR |= 32; std::cout << "GO" << "\n";  }
	void set_rom_r14_flag()  { SFR |= 64;    }
	void set_alt1_flag()     { SFR |= 256;   }
	void set_alt2_flag()     { SFR |= 512;   }
	void set_il_flag()       { SFR |= 1024;  }
	void set_ih_flag()       { SFR |= 2048;  }
	void set_b_flag()        { SFR |= 4096;  }
	void set_irq_flag()      { SFR |= 32768; }
	
	void clear_zero_flag()     { SFR &= ~2;     }
	void clear_carry_flag()    { SFR &= ~4;     }
	void clear_sign_flag()     { SFR &= ~8;     }
	void clear_overflow_flag() { SFR &= ~16;    }
	void clear_go_flag(bool is_stop = false)       {
		SFR &= ~32;
		if (!is_stop) {
			flush_cache(0x0000);
		}
	}
	void clear_rom_r14_flag()  { SFR &= ~64;    }
	void clear_alt1_flag()     { SFR &= ~256;   }
	void clear_alt2_flag()     { SFR &= ~512;   }
	void clear_il_flag()       { SFR &= ~1024;  }
	void clear_ih_flag()       { SFR &= ~2048;  }
	void clear_b_flag()        { SFR &= ~4096;  }
	void clear_irq_flag()      { SFR &= ~32768; }

	bool zero()     { return SFR & 2;     }
	bool carry()    { return SFR & 4;     }
	bool sign()     { return SFR & 8;     }
	bool overflow() { return SFR & 16;    }
	bool go()       { return SFR & 32;    }
	bool rom_r14()  { return SFR & 64;    }
	bool alt1()     { return SFR & 256;   }
	bool alt2()     { return SFR & 512;   }
	bool il()       { return SFR & 1024;  }
	bool ih()       { return SFR & 2048;  }
	bool b()        { return SFR & 4096;  }
	bool irq()      { return SFR & 32768; }

	bool ht0() { return SCMR & 0x04; }
	bool ron() { return SCMR & 0x10; }
	bool ran() { return SCMR & 0x08; }
	bool ht1() { return SCMR & 0x20; }

	Byte cy() {
		return carry() ? 1 : 0;
	}

	Word add_flags(uint16_t a, uint16_t b, uint16_t carry, uint16_t& result);
	void sub_flags(uint16_t a, uint16_t b, uint16_t borrow, uint16_t& result);
	void set_sz_flags(Word result);

	ALUResult lsr(Word value, unsigned int amount);
	ALUResult asr(Word value, unsigned int amount);
	ALUResult rcl(Word value, bool carry);
	ALUResult rcr(Word value, bool carry);
	Word inc(Word value);
	Word dec(Word value);
	Word sign_extend_8(Byte value);
	Word swap_bytes(Word value);
	Word merge(Word r7, Word r8);
	Word div2(Word value);
	Word lob(Word value);
	Word hib(Word value);
	bool fast_multiply();

	void wait_for_ron() {
		if (!ron()) {
			wait = WaitingState::RON;
		}
	}

	void wait_for_ran() {
		if (!ran()) {
			wait = WaitingState::RAN;
		}
	}

	bool vector_override(SNESAddress address, Byte& value) {
		if (!(go() && ron())) {
			return false;
		}

		if (!allow_mapper(address)) {
			return false;
		}
		if (address.offset < 0xFFE0 || address.offset > 0xFFFF) {
			return false;
		}

		switch (address.offset & 0x0F) {
		case 0x4: value = 0x04; return true;
        case 0x5: value = 0x01; return true;
        case 0x6: value = 0x00; return true;
        case 0x7: value = 0x01; return true;
        case 0x8: value = 0x00; return true;
        case 0x9: value = 0x01; return true;
        case 0xA: value = 0x08; return true;
        case 0xB: value = 0x01; return true;
        case 0xC: value = 0x00; return true;
        case 0xD: value = 0x01; return true;
        case 0xE: value = 0x0C; return true;
        case 0xF: value = 0x01; return true;
        default:  value = 0x00; return true;	
		}
	}

	// GSU side (bsnes used as source for this)
	Byte gsu_read(SNESAddress address);
	void gsu_write(SNESAddress address, Byte value);

	// SNES side (fullsnes used as primary source for this)

	Byte snes_read(SNESAddress address) {
		switch (revision) {
		case SuperFXRevision::MARIO: return snes_mario_read(address);
		case SuperFXRevision::GSU1:  return snes_gsu1_read(address);
		case SuperFXRevision::GSU2:	 return snes_gsu2_read(address);
		default: return 0x00; // This should never happen, so open bus not used here!
		}
	}

	Byte register_read(SNESAddress address) {
		uint8_t offset = address.offset - 0x3000;
		uint8_t register_idx = offset >> 1;
		if (offset & 1) {
			return get_hi(r[register_idx]);
		} else {
			return get_lo(r[register_idx]);
		}
	}

	void register_write(SNESAddress address, Byte value) {
		uint8_t offset = address.offset - 0x3000;
		uint8_t register_idx = offset >> 1;
		if (offset & 1) {
			set_hi(r[register_idx], value);
			set_lo(r[register_idx], REGISTER_LATCH);
			if (register_idx == 15) {
				set_go_flag();
			}
		} else {
			REGISTER_LATCH = value;
		}
	}

	Byte status_reg_read(SNESAddress address) {
		
		switch (address.offset) {
		case 0x3030: return get_lo(SFR);
		case 0x3031: return get_hi(SFR);
		case 0x3034: return PBR;
		case 0x3036: return ROMBR;
		case 0x303C: return RAMBR;
		case 0x303E: return get_lo(CBR);
		case 0x303F: return get_hi(CBR);
		case 0x3033: return get_open_bus();
		case 0x303B: return (revision == SuperFXRevision::GSU2) ? 4 : 1;
		case 0x3037: return get_open_bus();
		case 0x3039: return get_open_bus();
		case 0x3038: return get_open_bus();
		case 0x303A: return get_open_bus();
		}
		return get_open_bus();
	}

	void status_reg_write(SNESAddress address, Byte value) {
		switch (address.offset) {
		case 0x3030: {
			bool go_was_set = go();

			Byte add = value & 0x3E;
			SFR = (SFR & ~0x3E) | add;
			
			bool go_is_set = go();

			if (go_was_set && !go_is_set) {
				clear_go_flag(false);
			}
			break;
		}
		case 0x3031: {
			SFR = (SFR & 0x00FF) | (value << 8);
			break;
		}
		case 0x3034: PBR = value; break;
		case 0x3033: BRAMR = value & 1; break;
		case 0x3037: CFGR = value & 0xA0; break;
		case 0x3039: CLSR = value & 1; break;
		case 0x3038: SCBR = value; break;
		case 0x303A: SCMR = value & 0x3F; break;
		}
	}

	Byte cache_read(SNESAddress address) {
		if (address.offset >= 0x3100 && address.offset <= 0x32FF) {
			uint16_t index = (address.offset - 0x3100 + (CBR & 0x1FF)) & 0x1FF;
			return cache[index];
		}
		return get_open_bus();
	}

	void cache_write(SNESAddress address, Byte value) {
	    if (address.offset >= 0x3100 && address.offset <= 0x32FF) {
	        uint16_t index = (address.offset - 0x3100 + (CBR & 0x1FF)) & 0x1FF;
	        cache[index] = value;
	        Byte line = index >> 4;
	        Byte pos_in_line = (index & 0xF) + 1;
	        if (pos_in_line > cache_line_loaded[line]) {
	            cache_line_loaded[line] = pos_in_line;
	        }
	    }
	}

	Byte black_blob_io_read(SNESAddress address) {
		if (address.offset >= 0x3000 && address.offset <= 0x301F) {
			return register_read(address);
		}
		if (address.offset >= 0x3020 && address.offset <= 0x302F) {
			return get_open_bus();
		}
		if (address.offset >= 0x3030 && address.offset <= 0x3031) {
			return status_reg_read(address);
		}
		if (address.offset == 0x303B) {
			return vcr;
		}
		if (address.offset >= 0x3032 && address.offset <= 0x303F) {
			SNESAddress addr;
			addr.offset = 0x3030 + ((address.offset - 0x3032) & 1);
			return black_blob_io_read(addr);
		}
		if (address.offset >= 0x3040 && address.offset <= 0x305F) {
			SNESAddress addr;
			addr.offset = 0x3000 + (address.offset - 0x3040);
			return black_blob_io_read(addr);
		}
		if (address.offset == 0x307B) {
			return vcr;
		}
		if (address.offset >= 0x3060 && address.offset <= 0x307F) {
			SNESAddress addr;
			addr.offset = 0x3030 + ((address.offset - 0x3060) & 1);
			return black_blob_io_read(addr);
		}
		if (address.offset >= 0x3080 && address.offset <= 0x30FF) {
			return get_open_bus();
		}
		if (address.offset >= 0x3100 && address.offset <= 0x32FF) {
			return cache_read(address);
		}
		if (address.offset >= 0x3300 && address.offset <= 0x332F) {
			return get_open_bus();
		}
		if (address.offset == 0x333B) {
			return vcr;
		}
		if (address.offset >= 0x3330 && address.offset <= 0x333F) {
			SNESAddress addr;
			addr.offset = 0x3030 + ((address.offset - 0x3330) & 1);
			return black_blob_io_read(addr);
		}
		if (address.offset >= 0x3340 && address.offset <= 0x335F) {
			SNESAddress addr;
			addr.offset = 0x3000 + (address.offset - 0x3340);
			return black_blob_io_read(addr);
		}
		if (address.offset == 0x337B) {
			return vcr;
		}
		if (address.offset >= 0x3360 && address.offset <= 0x337F) {
			SNESAddress addr;
			addr.offset = 0x3030 + ((address.offset - 0x3360) & 1);
			return black_blob_io_read(addr);
		}
		if (address.offset >= 0x3380 && address.offset <= 0x342F) {
			return get_open_bus();
		}
		if (address.offset >= 0x3430 && address.offset <= 0x343F) {
			SNESAddress addr;
			addr.offset = 0x3030 + ((address.offset - 0x3430) & 1);
			return black_blob_io_read(addr);
		}
		if (address.offset >= 0x3440 && address.offset <= 0x345F) {
			SNESAddress addr;
			addr.offset = 0x3000 + (address.offset - 0x3440);
			return black_blob_io_read(addr);
		}
		if (address.offset >= 0x3460 && address.offset <= 0x347F) {
			SNESAddress addr;
			addr.offset = 0x3030 + ((address.offset - 0x3460) & 1);
			return black_blob_io_read(addr);
		}
		return get_open_bus();
	}

	Byte gsu_io_read(SNESAddress address) {
		if (address.offset >= 0x3000 && address.offset <= 0x301F) {
			return register_read(address);
		}
		if (address.offset >= 0x3020 && address.offset <= 0x302F) {
			SNESAddress addr;
			addr.offset = address.offset + 0x10;
			return gsu_io_read(addr);
		}
		if (address.offset >= 0x3030 && address.offset <= 0x303F) {
			return status_reg_read(address);
		}
		if (address.offset >= 0x3040 && address.offset <= 0x30FF) {
			SNESAddress addr;
			addr.offset = 0x3000 + ((address.offset - 0x3040) & 0x3F);
			return gsu_io_read(addr);
		}
		if (address.offset >= 0x3100 && address.offset <= 0x32FF) {
			return cache_read(address);
		}
		if (address.offset >= 0x3300 && address.offset <= 0x34FF) {
			SNESAddress addr;
			addr.offset = 0x3000 + ((address.offset - 0x3300) & 0x3F);
			return gsu_io_read(addr);
		}
		return get_open_bus();
	}

	Byte snes_mario_read(SNESAddress address) {
		vcr = 0x01;
		if ((address.bank >= 0x00 && address.bank <= 0x3F) || (address.bank >= 0x80 && address.bank <= 0xBF)) {
			if (address.offset >= 0x3000 && address.offset <= 0x3FFF) {
				return black_blob_io_read(address);
			}
		}
		if ((address.bank >= 0x60 && address.bank <= 0x7D) || (address.bank >= 0xE0 && address.bank <= 0xFF)) {
			if (address.offset >= 0x0000 && address.offset <= 0xFFFF) {
				if (gpram_size == SMALL_GAME_PAK_RAM_SIZE) {
					return gpram[address.offset & 0x7FFF];
				} else {
					return gpram[address.offset];
				}
			}
		}
		return get_open_bus();
	}

	Byte snes_gsu1_read(SNESAddress address) {
		vcr = 0x01;
		if ((address.bank >= 0x00 && address.bank <= 0x3F) || (address.bank >= 0x80 && address.bank <= 0xBF)) {
			if (address.offset >= 0x3000 && address.offset <= 0x3FFF) {
				return gsu_io_read(address);
			}
		}
		if ((address.bank >= 0x00 && address.bank <= 0x3F) || (address.bank >= 0x80 && address.bank <= 0xBF)) {
			if (address.offset >= 0x6000 && address.offset <= 0x7FFF) {
				SNESAddress addr;
				addr.bank = 0x70;
				addr.offset = address.offset - 0x6000;
				return snes_gsu1_read(addr);
			}
		}
		if ((address.bank >= 0x70 && address.bank <= 0x71) || (address.bank >= 0xF0 && address.bank <= 0xF1)) {
			if (address.offset >= 0x0000 && address.offset <= 0xFFFF) {
				if (gpram_size == SMALL_GAME_PAK_RAM_SIZE) {
					return gpram[address.offset & 0x7FFF];
				} else {
					return gpram[address.offset];
				}
			}
		}
		return get_open_bus();
	}

	Byte snes_gsu2_read(SNESAddress address) {
		vcr = 0x04;
		if ((address.bank >= 0x00 && address.bank <= 0x3F) || (address.bank >= 0x80 && address.bank <= 0xBF)) {
			if (address.offset >= 0x3000 && address.offset <= 0x3FFF) {
				return gsu_io_read(address);
			}
		}
		if ((address.bank >= 0x00 && address.bank <= 0x3F) || (address.bank >= 0x80 && address.bank <= 0xBF)) {
			if (address.offset >= 0x6000 && address.offset <= 0x7FFF) {
				SNESAddress addr;
				addr.bank = 0x70;
				addr.offset = address.offset - 0x6000;
				return snes_gsu2_read(addr);
			}
		}
		if (address.bank >= 0x70 && address.bank <= 0x71) {
			if (address.offset >= 0x0000 && address.offset <= 0xFFFF) {
				if (gpram_size == SMALL_GAME_PAK_RAM_SIZE) {
					return gpram[address.offset & 0x7FFF];
				} else {
					return gpram[address.offset];
				}
			}
		}
		if (address.bank >= 0x78 && address.bank <= 0x79) {
			if (BRAMR) {
				return backup_ram[address.offset & 0x7FFF];
			} else {
				return get_open_bus();
			}
		}
		return get_open_bus();
	}

	void snes_write(SNESAddress address, Byte value) {
		switch (revision) {
		case SuperFXRevision::MARIO: snes_mario_write(address, value); break;
		case SuperFXRevision::GSU1:  snes_gsu1_write(address, value); break;
		case SuperFXRevision::GSU2:	 snes_gsu2_write(address, value); break;
		default: break;
		}
	}

	void black_blob_io_write(SNESAddress address, Byte value) {
		if (address.offset >= 0x3000 && address.offset <= 0x301F) {
			register_write(address, value);
		}
		if (address.offset >= 0x3030 && address.offset <= 0x3031) {
			status_reg_write(address, value);
		}
		if (address.offset >= 0x3032 && address.offset <= 0x303F) {
			/*SNESAddress addr;
			addr.offset = 0x3030 + ((address.offset - 0x3032) & 1);
			black_blob_io_write(addr, value);*/
			status_reg_write(address, value);
		}
		if (address.offset >= 0x3040 && address.offset <= 0x305F) {
			SNESAddress addr;
			addr.offset = 0x3000 + (address.offset - 0x3040);
			black_blob_io_write(addr, value);
		}
		if (address.offset >= 0x3060 && address.offset <= 0x307F) {
			SNESAddress addr;
			addr.offset = 0x3030 + ((address.offset - 0x3060) & 1);
			black_blob_io_write(addr, value);
		}
		if (address.offset >= 0x3100 && address.offset <= 0x32FF) {
			return cache_write(address, value);
		}
		if (address.offset >= 0x3330 && address.offset <= 0x333F) {
			SNESAddress addr;
			addr.offset = 0x3030 + ((address.offset - 0x3330) & 1);
			black_blob_io_write(addr, value);
		}
		if (address.offset >= 0x3340 && address.offset <= 0x335F) {
			SNESAddress addr;
			addr.offset = 0x3000 + (address.offset - 0x3340);
			black_blob_io_write(addr, value);
		}
		if (address.offset >= 0x3360 && address.offset <= 0x337F) {
			SNESAddress addr;
			addr.offset = 0x3030 + ((address.offset - 0x3360) & 1);
			black_blob_io_write(addr, value);
		}
		if (address.offset >= 0x3430 && address.offset <= 0x343F) {
			SNESAddress addr;
			addr.offset = 0x3030 + ((address.offset - 0x3430) & 1);
			black_blob_io_write(addr, value);
		}
		if (address.offset >= 0x3440 && address.offset <= 0x345F) {
			SNESAddress addr;
			addr.offset = 0x3000 + (address.offset - 0x3440);
			black_blob_io_write(addr, value);
		}
		if (address.offset >= 0x3460 && address.offset <= 0x347F) {
			SNESAddress addr;
			addr.offset = 0x3030 + ((address.offset - 0x3460) & 1);
			black_blob_io_write(addr, value);
		}
	}

	void gsu_io_write(SNESAddress address, Byte value) {
		if (address.offset >= 0x3000 && address.offset <= 0x301F) {
			register_write(address, value);
		}
		if (address.offset >= 0x3020 && address.offset <= 0x302F) {
			SNESAddress addr;
			addr.offset = address.offset + 0x10;
			gsu_io_write(addr, value);
		}
		if (address.offset >= 0x3030 && address.offset <= 0x303F) {
			status_reg_write(address, value);
		}
		if (address.offset >= 0x3040 && address.offset <= 0x30FF) {
			SNESAddress addr;
			addr.offset = 0x3000 + ((address.offset - 0x3040) & 0x3F);
			gsu_io_write(addr, value);
		}
		if (address.offset >= 0x3100 && address.offset <= 0x32FF) {
			cache_write(address, value);
		}
		if (address.offset >= 0x3300 && address.offset <= 0x34FF) {
			SNESAddress addr;
			addr.offset = 0x3000 + ((address.offset - 0x3300) & 0x3F);
			gsu_io_write(addr, value);
		}
	}

	void snes_mario_write(SNESAddress address, Byte value) {
		vcr = 0x01;
		if ((address.bank >= 0x00 && address.bank <= 0x3F) || (address.bank >= 0x80 && address.bank <= 0xBF)) {
			if (address.offset >= 0x3000 && address.offset <= 0x3FFF) {
				black_blob_io_write(address, value);
			}
		}
		if ((address.bank >= 0x60 && address.bank <= 0x7D) || (address.bank >= 0xE0 && address.bank <= 0xFF)) {
			if (address.offset >= 0x0000 && address.offset <= 0xFFFF) {
				if (gpram_size == SMALL_GAME_PAK_RAM_SIZE) {
					gpram[address.offset & 0x7FFF] = value;
				} else {
					gpram[address.offset] = value;
				}
			}
		}
	}

	void snes_gsu1_write(SNESAddress address, Byte value) {
		vcr = 0x01;
		if ((address.bank >= 0x00 && address.bank <= 0x3F) || (address.bank >= 0x80 && address.bank <= 0xBF)) {
			if (address.offset >= 0x3000 && address.offset <= 0x3FFF) {
				gsu_io_write(address, value);
			}
		}
		if ((address.bank >= 0x00 && address.bank <= 0x3F) || (address.bank >= 0x80 && address.bank <= 0xBF)) {
			if (address.offset >= 0x6000 && address.offset <= 0x7FFF) {
				SNESAddress addr;
				addr.bank = 0x70;
				addr.offset = address.offset - 0x6000;
				snes_gsu1_write(addr, value);
			}
		}
		if ((address.bank >= 0x70 && address.bank <= 0x71) || (address.bank >= 0xF0 && address.bank <= 0xF1)) {
			if (address.offset >= 0x0000 && address.offset <= 0xFFFF) {
				if (gpram_size == SMALL_GAME_PAK_RAM_SIZE) {
					gpram[address.offset & 0x7FFF] = value;
				} else {
					gpram[address.offset] = value;
				}
			}
		}
	}

	void snes_gsu2_write(SNESAddress address, Byte value) {
		vcr = 0x04;
		if ((address.bank >= 0x00 && address.bank <= 0x3F) || (address.bank >= 0x80 && address.bank <= 0xBF)) {
			if (address.offset >= 0x3000 && address.offset <= 0x3FFF) {
				gsu_io_write(address, value);
			}
		}
		if ((address.bank >= 0x00 && address.bank <= 0x3F) || (address.bank >= 0x80 && address.bank <= 0xBF)) {
			if (address.offset >= 0x6000 && address.offset <= 0x7FFF) {
				SNESAddress addr;
				addr.bank = 0x70;
				addr.offset = address.offset - 0x6000;
				snes_gsu2_write(addr, value);
			}
		}
		if (address.bank >= 0x70 && address.bank <= 0x71) {
			if (address.offset >= 0x0000 && address.offset <= 0xFFFF) {
				if (gpram_size == SMALL_GAME_PAK_RAM_SIZE) {
					gpram[address.offset & 0x7FFF] = value;
				} else {
					gpram[address.offset] = value;
				}
			}
		}
		if (address.bank >= 0x78 && address.bank <= 0x79) {
			if (BRAMR) {
				backup_ram[address.offset & 0x7FFF] = value;
			}
		}
		// STILL NEED TO ADD BACKUP RAM
	}

	bool snes_handles(SNESAddress address) {
		switch (revision) {
		case SuperFXRevision::MARIO: return snes_mario_handles(address);
		case SuperFXRevision::GSU1:  return snes_gsu1_handles(address);
		case SuperFXRevision::GSU2:	 return snes_gsu2_handles(address);
		default: return false;
		}
	}

	bool snes_mario_handles(SNESAddress address) {
		bool gsu_io = ((address.bank >= 0x00 && address.bank <= 0x3F) || (address.bank >= 0x80 && address.bank <= 0xBF)) &&
					   (address.offset >= 0x3000 && address.offset <= 0x34FF);
		bool gsu_gpram = ((address.bank >= 0x70 && address.bank <= 0x71) || (address.bank >= 0xF0 && address.bank <= 0xF1)) &&
					      (address.offset >= 0x0000 && address.offset <= 0xFFFF);
		return gsu_io || gsu_gpram;
	}

	bool snes_gsu1_handles(SNESAddress address) {
		bool gsu_io = ((address.bank >= 0x00 && address.bank <= 0x3F) || (address.bank >= 0x80 && address.bank <= 0xBF)) &&
					   (address.offset >= 0x3000 && address.offset <= 0x34FF);
		bool gp_mirror = ((address.bank >= 0x00 && address.bank <= 0x3F) || (address.bank >= 0x80 && address.bank <= 0xBF)) &&
					      (address.offset >= 0x6000 && address.offset <= 0x7FFF);
		bool gp_ram = ((address.bank >= 0x70 && address.bank <= 0x71) || (address.bank >= 0xF0 && address.bank <= 0xF1)) &&
					   (address.offset >= 0x0000 && address.offset <= 0xFFFF);
		bool gp_backup = ((address.bank >= 0x78 && address.bank <= 0x79) || (address.bank >= 0xF8 && address.bank <= 0xF9)) &&
					      (address.offset >= 0x0000 && address.offset <= 0xFFFF);
		return gsu_io || gp_mirror || gp_ram || gp_backup;
	}

	bool snes_gsu2_handles(SNESAddress address) {
		bool gsu_io = (address.bank >= 0x00 && address.bank <= 0x3F) &&
					   (address.offset >= 0x3000 && address.offset <= 0x34FF);
		bool gp_mirror = (address.bank >= 0x00 && address.bank <= 0x3F) &&
					      (address.offset >= 0x6000 && address.offset <= 0x7FFF);
		bool gp_ram = (address.bank >= 0x70 && address.bank <= 0x71 && address.offset <= 0xFFFF);
		bool gp_backup = (address.bank >= 0x78 && address.bank <= 0x79 && address.offset <= 0xFFFF);
		return gsu_io || gp_mirror || gp_ram || gp_backup;
	}

	bool is_lorom() {
		return !hirom_mapper;
	}

	bool is_hirom() {
		return hirom_mapper;
	}

	void set_mapper_type(bool hirom_mapper) {
		this->hirom_mapper = hirom_mapper;
	}

	bool allow_mapper(SNESAddress address) {
		switch (revision) {
		case SuperFXRevision::MARIO: return mario_allow_mapper(address);
		case SuperFXRevision::GSU1:  return gsu1_allow_mapper(address);
		case SuperFXRevision::GSU2:	 return gsu2_allow_mapper(address);
		default: return true;
		}
	}

	bool mario_allow_mapper(SNESAddress address) {
	    bool rom =
	        ((address.bank >= 0x00 && address.bank <= 0x1F) ||
	         (address.bank >= 0x80 && address.bank <= 0x9F)) &&
	        address.offset >= 0x8000;

	    bool ram =
	        ((address.bank >= 0x60 && address.bank <= 0x7D) ||
	         (address.bank >= 0xE0 && address.bank <= 0xFF));

	    return rom || ram;
	}

	bool gsu1_allow_mapper(SNESAddress address) {
		bool bank;
		bool offset;
		if (is_hirom()) {
			bank = (address.bank >= 0x40 && address.bank <= 0x5F) || (address.bank >= 0xC0 && address.bank <= 0xDF);
			offset = address.offset <= 0xFFFF;
		} else {
			bank = (address.bank <= 0x3F) || (address.bank >= 0x80 && address.bank <= 0xBF);
			offset = address.offset >= 0x8000 && address.offset <= 0xFFFF;
		}
		return bank && offset;
	}

	bool gsu2_allow_mapper(SNESAddress address) {
		bool bank;
		bool offset;
		if (is_hirom()) {
			bank = (address.bank >= 0x40 && address.bank <= 0x5F);
			offset = address.offset <= 0xFFFF;
		} else {
			bank = (address.bank <= 0x3F);
			offset = address.offset >= 0x8000 && address.offset <= 0xFFFF;
		}
		return bank && offset;
	}

	void connect_cpu(Ricoh5A22* cpu) {
		this->cpu = cpu;
	}

	void connect_cartridge(Cartridge* cartridge) {
		this->cartridge = cartridge;
	}

	bool waiting() {
		return wait != WaitingState::None;
	}

	Byte get_open_bus();

	void mark_cache_line_loaded(uint8_t line, uint8_t loaded_up_to) {
		if (loaded_up_to > cache_line_loaded[line]) {
			cache_line_loaded[line] = loaded_up_to;
		}
	}

	void flush_cache(uint16_t new_cbr) {
		CBR = new_cbr;
		for (auto& c : cache_line_loaded) {
			c = 0;
		}
	}

	Byte fetch_operand_byte() {
		Byte byte = fetch_opcode(PC);
		if (wait != WaitingState::None) {
			return 0x00;
		}
		PC = PC + 1;
		return byte;
	}

	// ROM-Read-Data Cache
	void reload_rom_buffer() {
		rom_buffer_bank = ROMBR;
		if (ron()) {
			SNESAddress address;
			address.bank = rom_buffer_bank;
			address.offset = r[14];
			rom_buffer = gsu_read(address);
			rom_buffer_valid = true;
		} else {
			rom_buffer_valid = false;
		}
	}

	Byte get_rom_buffer() {
		return rom_buffer;
	}

	void write_register(Byte n, Word value) {
		r[n] = value;
		if (n == 14) {
			reload_rom_buffer();
		}
	}

	// RAM-Address-Cache

	void remember_ram_address(Word address, Byte bank) {
		last_ram_address = address;
		last_ram_bank = bank;
	}

	void sbk_write(Word value) {
		SNESAddress address;
		address.bank = last_ram_bank;
		address.offset = last_ram_address;
		gsu_write(address, get_lo(value));
		address.offset++;
		gsu_write(address, get_hi(value));
	}

	CycleCount get_coprocessor_cycle() {
		return (CycleCount)cycle;
	}

	void schedule_jump(Word target);
	void schedule_ljmp(Byte bank, Word target);
	void commit_pending_jump();


private:
	Byte vcr;

	CycleCount cycle = 0;
	CycleCount cycles_per_clock = 1;

	SuperFXRevision revision = SuperFXRevision::None;

	size_t gpram_size = LARGE_GAME_PAK_RAM_SIZE;
	std::vector<Byte> gpram {}; // gpram means 'Game Pak RAM'
	std::vector<Byte> backup_ram {};

	bool hirom_mapper = false;

	Ricoh5A22* cpu = nullptr;

	Byte REGISTER_LATCH = 0;

	// Registers
	uint16_t r[16] = {0};
	Byte cache[512];
	Byte cache_line_loaded[32]; // gives number of bytes from offset 0 loaded for that page of the cache

	Word SFR = 0;
	Byte PBR = 0;
	Byte ROMBR = 0;
	Byte RAMBR = 0;
	Word CBR = 0;
	Byte BRAMR = 0;
	Byte CFGR = 0;
	Byte CLSR = 0;
	Byte SCBR = 0;
	Byte SCMR = 0;
	Byte COLR = 0;
	Byte POR = 0x1F;
	Byte SREG = 0;
	Byte DREG = 0;

	WaitingState wait = WaitingState::None;

	int clks = 0; // number of clocks the executed instruction took

	Cartridge* cartridge = nullptr;

	bool instruction_in_progress = false;
	Byte pending_opcode = 0;

	PixelCache pixel_primary;
	PixelCache pixel_secondary;

	// ROM-Read-Data Cache
	Byte rom_buffer = 0;
	Byte rom_buffer_bank = 0;
	bool rom_buffer_valid = false;

	bool operand_fetch_done = false;
	Word cached_ram_offset = 0;
	Word cached_operand_word = 0;

	// RAM-Address-Cache

	Word last_ram_address = 0;
	Byte last_ram_bank = 0;

	bool executed = false;

	// For timings
	Byte clocks_taken;

	Word& PC = r[15];

	bool branch_pending = false;
	bool branch_delay = false;

	Word branch_target = 0;
	Byte branch_bank = 0;
	bool branch_has_bank = false;
};