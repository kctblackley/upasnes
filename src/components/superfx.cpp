#include "superfx.hpp"
#include "cartridge.hpp"
#include "ricoh_5a22.hpp"
#include "snes.hpp"
#include <iostream>
#include <iomanip>

constexpr bool LOG_SUPERFX = true;

// Helper for logs
template<typename T>
std::string hex(T value, int width) {
    std::ostringstream ss;
    ss << std::hex
       << std::uppercase
       << std::setfill('0')
       << std::setw(width)
       << static_cast<uint64_t>(value);
    return ss.str();
}

Byte SuperFX::get_open_bus() {
	return cpu->get_open_bus();
}

void SuperFX::tick_component() {
	if (sfr.g == 0) {
      step(6);
      return;
   }
   instruction(peekpipe());

   if (r[14].modified) {
      r[14].modified = false;
      update_rom_buffer();
   }

   if (r[15].modified) {
      r[15].modified = false;
   } else {
      r[15]++;
   }
}

void SuperFX::unload() {
   reset_rom();
   reset_ram();
}

void SuperFX::power() {
   rom_mask = get_rom_size() - 1;
   ram_mask = get_ram_size() - 1;

   for (int n = 0; n < 512; n++) {
      cache.buffer[n] = 0x00;
   }
   for (int n = 0; n < 32; n++) {
      cache.valid[n] = false;
   }
   for (int n = 0; n < 2; n++) {
      pixel_cache[n].offset = ~0;
      pixel_cache[n].bitpend = 0x00;
   }

   romcl = 0;
   romdr = 0;

   ramcl = 0;
   ramar = 0;
   ramdr = 0;
}

// Instructions (bsnes used as source)

void SuperFX::i_stop() {
   if (cfgr.irq == 0) {
      sfr.irq = 1;
      stop();
   }
   sfr.g = 0;
   pipeline = 0x01;
   reset_registers();
}

void SuperFX::i_nop() {
   reset_registers();
}

void SuperFX::i_cache() {
   if (cbr != (r[15] & 0xFFF0)) {
      cbr = r[15] & 0xFFF0;
      flush_cache();
   }
   reset_registers();
}

void SuperFX::i_lsr() {
   sfr.cy = sr() & 1;
   dr() = sr() >> 1;
   sfr.s = dr() & 0x8000;
   sfr.z = (dr() == 0);
   reset_registers();
}

void SuperFX::i_rol() {
   bool carry = (sr() & 0x8000);
   dr() = (sr() << 1) | sfr.cy;
   sfr.s = dr() & 0x8000;
   sfr.cy = carry;
   sfr.z = (dr() == 0);
   reset_registers();
}

void SuperFX::i_branch(bool condition) {
   int8_t displacement = pipe();
   if (condition) {
      r[15] += displacement;
   }
}

void SuperFX::i_to_move(unsigned int n) {
   if (!sfr.b) {
      dreg = n;
   } else {
      r[n] = sr();
      reset_registers();
   }
}

void SuperFX::i_with(unsigned int n) {
   sreg = n;
   dreg = n;
   sfr.b = 1;
}

void SuperFX::i_store(unsigned int n) {
   ramaddr = r[n];
   write_ram_buffer(ramaddr, sr());
   if (!sfr.alt1) {
      write_ram_buffer(ramaddr ^ 1, sr() >> 8);
   }
   reset_registers();
}

void SuperFX::i_loop() {
   r[12]--;
   sfr.s = r[12] & 0x8000;
   sfr.z = r[12] == 0;
   if (!sfr.z) {
      r[15] = r[13];
   }
   reset_registers();
}

void SuperFX::i_alt1() {
   sfr.b = 0;
   sfr.alt1 = 1;
}

void SuperFX::i_alt2() {
   sfr.b = 0;
   sfr.alt2 = 1;
}

void SuperFX::i_alt3() {
   sfr.b = 0;
   sfr.alt1 = 1;
   sfr.alt2 = 1;
}

void SuperFX::i_load(unsigned int n) {
   ramaddr = r[n];
   dr() = read_ram_buffer(ramaddr);
   if (!sfr.alt1) {
      dr() = dr() | (read_ram_buffer(ramaddr ^ 1) << 8);
   }
   reset_registers();
}

void SuperFX::i_plot_rpix() {
   if (!sfr.alt1) {
      plot(r[1], r[2]);
      r[1]++;
   } else {
      dr() = rpix(r[1], r[2]);
      sfr.s = (dr() & 0x8000);
      sfr.z = (dr() == 0);
   }
   reset_registers();
}

void SuperFX::i_swap() {
   dr() = (sr() >> 8) | (sr() << 8);
   sfr.s = (dr() & 0x8000);
   sfr.z = (dr() == 0);
   reset_registers();
}

void SuperFX::i_colour_cmode() {
   if (!sfr.alt1) {
      colr = colour(sr());
   } else {
      por = sr();
   }
   reset_registers();
}

void SuperFX::i_not() {
   dr() = ~sr();
   sfr.s = (dr() & 0x8000);
   sfr.z = (dr() == 0);
   reset_registers();
}

void SuperFX::i_add_adc(unsigned int n) {
   if (!sfr.alt2) {
      n = r[n];
   }
   int r = sr() + n + (sfr.alt1 ? sfr.cy : 0);
   sfr.ov = ~(sr() ^ n) & (n ^ r) & 0x8000;
   sfr.s  = (r & 0x8000);
   sfr.cy = (r >= 0x10000);
   sfr.z = ((Word)r == 0);
   dr() = r;
   reset_registers();
}

void SuperFX::i_sub_sbc_cmp(unsigned int n) {
   if (!sfr.alt2 || sfr.alt1) {
      n = r[n];
   }
   int r = sr() - n - (!sfr.alt2 && sfr.alt1 ? !sfr.cy : 0);
   sfr.ov = (sr() ^ n) & (sr() ^ r) & 0x8000;
   sfr.s  = (r & 0x8000);
   sfr.cy = (r >= 0);
   sfr.z  = ((Word)r == 0);
   if (!sfr.alt2 || !sfr.alt1) {
      dr() = r;
   }
   reset_registers();
}

void SuperFX::i_merge() {
   dr() = (r[7] & 0xFF00) | (r[8] >> 8);
   sfr.ov = (dr() & 0xC0C0);
   sfr.s  = (dr() & 0x8080);
   sfr.cy = (dr() & 0xE0E0);
   sfr.z  = (dr() & 0xF0F0);
   reset_registers();
}

void SuperFX::i_and_bic(unsigned int n) {
   if (!sfr.alt2) {
      n = r[n];
   }
   dr() = sr() & (sfr.alt1 ? ~n : n);
   sfr.s = (dr() & 0x8000);
   sfr.z = (dr() == 0);
   reset_registers();
}

void SuperFX::i_mult_umult(unsigned int n) {
   if (!sfr.alt2) {
      n = r[n];
   }
   dr() = (!sfr.alt1 ? (Word)((int8_t)sr() * (int8_t)n) : (Word)((uint8_t)sr() * (uint8_t)n));
   sfr.s = (dr() & 0x8000);
   sfr.z = (dr() == 0);
   reset_registers();
   if (!cfgr.ms0) {
      step(clsr ? 1 : 2);
   }
}

void SuperFX::i_sbk() {
   write_ram_buffer(ramaddr ^ 0, sr() >> 0);
   write_ram_buffer(ramaddr ^ 1, sr() >> 8);
   reset_registers();
}

void SuperFX::i_link(unsigned int n) {
   r[11] = r[15] + n;
   reset_registers();
}

void SuperFX::i_sex() {
   dr() = (int8_t)sr();
   sfr.s = (dr() & 0x8000);
   sfr.z = (dr() == 0);
   reset_registers();
}

void SuperFX::i_asr_div2() {
   sfr.cy = (sr() & 1);
   dr() = ((int16_t)sr() >> 1) + (sfr.alt1 ? ((sr() + 1) >> 16) : 0);
   sfr.s = (dr() & 0x8000);
   sfr.z = (dr() == 0);
   reset_registers();
}
void SuperFX::i_ror() {
   bool carry = (sr() & 1);
   dr() = (sfr.cy << 15) | (sr() >> 1);
   sfr.s = (dr() & 0x8000);
   sfr.cy = carry;
   sfr.z = (dr() == 0);
   reset_registers();
}

void SuperFX::i_jmp_ljmp(unsigned int n) {
   if (!sfr.alt1) {
      r[15] = r[n];
   } else {
      pbr = r[n] & 0x7F;
      r[15] = sr();
      cbr = r[15] & 0xFFF0;
      flush_cache();
   }
   reset_registers();
}

void SuperFX::i_lob() {
   dr() = sr() & 0xFF;
   sfr.s = (dr() & 0x80);
   sfr.z = (dr() == 0);
   reset_registers();
}

void SuperFX::i_fmult_lmult() {
   uint32_t result = (int16_t)(sr()) * (int16_t)(r[6]);
   if (sfr.alt1) {
      r[4] = result;
   }
   dr() = result >> 16;
   sfr.s = (dr() & 0x8000);
   sfr.cy = (result & 0x8000);
   sfr.z = (dr() == 0);
   reset_registers();
   step((cfgr.ms0 ? 3 : 7) * (clsr ? 1 : 2));
}

void SuperFX::i_ibt_lms_sms(unsigned int n) {
   if (sfr.alt1) {
      ramaddr = pipe() << 1;
      Byte lo = read_ram_buffer(ramaddr ^ 0) << 0;
      r[n] = read_ram_buffer(ramaddr ^ 1) << 8 | lo;
   } else if (sfr.alt2) {
      ramaddr = pipe() << 1;
      write_ram_buffer(ramaddr ^ 0, r[n] >> 0);
      write_ram_buffer(ramaddr ^ 1, r[n] >> 8);
   } else {
      r[n] = (int8_t)pipe();
   }
   reset_registers();
}

void SuperFX::i_from_moves(unsigned int n) {
   if (!sfr.b) {
      sreg = n;
   } else {
      dr() = r[n];
      sfr.ov = (dr() & 0x80);
      sfr.s = (dr() & 0x8000);
      sfr.z = (dr() == 0);
      reset_registers();
   }
}

void SuperFX::i_hib() {
   dr() = sr() >> 8;
   sfr.s = (dr() & 0x80);
   sfr.z = (dr() == 0);
   reset_registers();
}

void SuperFX::i_or_xor(unsigned int n) {
   if (!sfr.alt2) {
      n = r[n];
   }
   dr() = (!sfr.alt1 ? (sr() | n) : (sr() ^ n));
   sfr.s = (dr() & 0x8000);
   sfr.z = (dr() == 0);
   reset_registers();
}

void SuperFX::i_inc(unsigned int n) {
   r[n]++;
   sfr.s = (r[n] & 0x8000);
   sfr.z = (r[n] == 0);
   reset_registers();
}

void SuperFX::i_getc_ramb_romb() {
   if (!sfr.alt2) {
      colr = colour(read_rom_buffer());
   } else if (!sfr.alt1) {
      sync_ram_buffer();
      rambr = sr() & 0x01;
   } else {
      sync_rom_buffer();
      rombr = sr() & 0x7F;
   }
   reset_registers();
}

void SuperFX::i_dec(unsigned int n) {
   r[n]--;
   sfr.s = (r[n] & 0x8000);
   sfr.z = (r[n] == 0);
   reset_registers();
}

void SuperFX::i_getb() {
   switch (sfr.alt2 << 1 | sfr.alt1 << 0) {
   case 0:
      dr() = read_rom_buffer();
      break;
   case 1:
      dr() = read_rom_buffer() << 8 | (uint8_t)sr();
      break;
   case 2:
      dr() = (sr() & 0xFF00) | read_rom_buffer();
      break;
   case 3:
      dr() = (int8_t)read_rom_buffer();
      break;
   }
   reset_registers();
}

void SuperFX::i_iwt_lm_sm(unsigned int n) {
   if (sfr.alt1) {
      ramaddr = pipe() << 0;
      ramaddr = ramaddr | (pipe() << 8);
      Byte lo = read_ram_buffer(ramaddr ^ 0) << 0;
      r[n] = (read_ram_buffer(ramaddr ^ 1) << 8) | lo;
   } else if (sfr.alt2) {
      ramaddr = pipe() << 0;
      ramaddr = ramaddr | (pipe() << 8);
      write_ram_buffer(ramaddr ^ 0, r[n] >> 0);
      write_ram_buffer(ramaddr ^ 1, r[n] >> 8);
   } else {
      Byte lo = pipe();
      r[n] = (pipe() << 8) | lo;
   }
   reset_registers();
}

// Instruction dispatch

// Using exact same idea as bsnes

void SuperFX::instruction(Byte opcode) {
   #define op(id, name, ...) \
      case id: return i_##name(__VA_ARGS__);
   
   #define op4(id, name) \
      case id + 0: return i_##name(opcode & 0x0F); \
      case id + 1: return i_##name(opcode & 0x0F); \
      case id + 2: return i_##name(opcode & 0x0F); \
      case id + 3: return i_##name(opcode & 0x0F); \

   #define op6(id, name) \
      op4(id, name) \
      case id + 4: return i_##name(opcode & 0x0F); \
      case id + 5: return i_##name(opcode & 0x0F); \
   
   #define op12(id, name) \
      op6(id, name) \
      case id +  6: return i_##name(opcode & 0x0F); \
      case id +  7: return i_##name(opcode & 0x0F); \
      case id +  8: return i_##name(opcode & 0x0F); \
      case id +  9: return i_##name(opcode & 0x0F); \
      case id + 10: return i_##name(opcode & 0x0F); \
      case id + 11: return i_##name(opcode & 0x0F); \

   #define op15(id, name) \
      op12(id, name) \
      case id + 12: return i_##name(opcode & 0x0F); \
      case id + 13: return i_##name(opcode & 0x0F); \
      case id + 14: return i_##name(opcode & 0x0F); \
   
   #define op16(id, name) \
      op15(id, name) \
      case id + 15: return i_##name(opcode & 0x0F); \
   
   switch (opcode) {
      op (0x00, stop)
      op (0x01, nop)
      op (0x02, cache)
      op (0x03, lsr)
      op (0x04, rol)
      op (0x05, branch, 1)
      op (0x06, branch, (sfr.s ^ sfr.ov) == 0)
      op (0x07, branch, (sfr.s ^ sfr.ov) == 1)
      op (0x08, branch, sfr.z == 0)
      op (0x09, branch, sfr.z == 1)
      op (0x0A, branch, sfr.s == 0)
      op (0x0B, branch, sfr.s == 1)
      op (0x0C, branch, sfr.cy == 0)
      op (0x0D, branch, sfr.cy == 1)
      op (0x0E, branch, sfr.ov == 0)
      op (0x0F, branch, sfr.ov == 1)

      op16 (0x10, to_move)
      op16 (0x20, with)
      op12 (0x30, store)

      op (0x3C, loop)
      op (0x3D, alt1)
      op (0x3E, alt2)
      op (0x3F, alt3)

      op12 (0x40, load)

      op (0x4C, plot_rpix)
      op (0x4D, swap)
      op (0x4E, colour_cmode)
      op (0x4F, not)

      op16 (0x50, add_adc)
      op16 (0x60, sub_sbc_cmp)

      op (0x70, merge)

      op15 (0x71, and_bic)

      op16 (0x80, mult_umult)

      op (0x90, sbk)

      op4 (0x91, link)

      op (0x95, sex)
      op (0x96, asr_div2)
      op (0x97, ror)

      op6 (0x98, jmp_ljmp)

      op (0x9e, lob)
      op (0x9f, fmult_lmult)

      op16 (0xA0, ibt_lms_sms)
      op16 (0xB0, from_moves)

      op (0xC0, hib)

      op15 (0xC1, or_xor)
      op15 (0xD0, inc)

      op (0xDF, getc_ramb_romb)

      op15 (0xE0, dec)

      op (0xEF, getb)

      op16 (0xF0, iwt_lm_sm)
   }

   #undef op
   #undef op4
   #undef op6
   #undef op12
   #undef op15
   #undef op16
}

// Memory map

void SuperFX::synchronise_cpu() {
   snes->sync_to_superfx();
}

Byte SuperFX::read(Address address) {
   if ((address & 0xC00000) == 0x000000) {
      while (!scmr.ron) {
         step(6);
         synchronise_cpu();
      }
      return read_rom((((address & 0x3F0000) >> 1) | (address & 0x7FFF)) & rom_mask);
   }

   if ((address & 0xE00000) == 0x400000) {
      while (!scmr.ron) {
         step(6);
         synchronise_cpu();
      }
      return read_rom(address & rom_mask);
   }

   if ((address & 0xE00000) == 0x600000) {
      while (!scmr.ran) {
         step(6);
         synchronise_cpu();
      }
      return read_ram(address & ram_mask);
   }

   return get_open_bus();
}

void SuperFX::write(Address address, Byte data) {
   if ((address & 0xE00000) == 0x600000) {
      while (!scmr.ran) {
         step(6);
         synchronise_cpu();
      }
      write_ram(address & ram_mask, data);
   }
}

Byte SuperFX::read_opcode(Word address) {
   Word offset = address - cbr;
   if (offset < 512) {
      if (!cache.valid[offset >> 4]) {
         unsigned int dp = offset & 0xFFF0;
         unsigned int sp = (pbr << 16) + ((cbr + dp) & 0xFFF0);
         for (int n = 0; n < 16; n++) {
            step(clsr ? 5 : 6);
            cache.buffer[dp++] = read(sp++);
         }
         cache.valid[offset >> 4] = true;
      } else {
         step(clsr ? 1 : 2);
      }

      return cache.buffer[offset];
   }

   if (pbr <= 0x5F) {
      sync_rom_buffer();
      step(clsr ? 5 : 6);
      return read(pbr << 16 | address);
   } else {
      sync_ram_buffer();
      step(clsr ? 5 : 6);
      return read(pbr << 16 | address);
   }
}
Byte SuperFX::peekpipe() {
   Byte result = pipeline;
   pipeline = read_opcode(r[15]);
   r[15].modified = false;
   return result;
}

Byte SuperFX::pipe() {
   Byte result = pipeline;
   pipeline = read_opcode(++r[15]);
   r[15].modified = false;
   return result;
}

void SuperFX::flush_cache() {
   for (int n = 0; n < 32; n++) {
      cache.valid[n] = false;
   }
}

Byte SuperFX::read_cache(Word address) {
   return cache.buffer[(address + cbr) & 511];
}

void SuperFX::write_cache(Word address, Byte data) {
   cache.buffer[(address + cbr) & 511] = data;
   if ((address & 15) == 15) {
      cache.valid[address >> 4] = true;
   }
}

void SuperFX::stop() {
   cpu->signal_irq();
}

Byte SuperFX::colour(Byte source) {
   if (por.high_nibble) {
      return (colr & 0xF0) | (source >> 4);
   }
   if (por.freeze_high) {
      return (colr & 0xF0) | (source & 0x0F);
   }
   return source;
}

void SuperFX::plot(Byte x, Byte y) {
   if (!por.transparent) {
      if (scmr.md == 3) {
         if (por.freeze_high) {
            if ((colr & 0x0F) == 0) {
               return;
            }
         } else {
            if (colr == 0) {
               return;
            }
         }
      } else {
         if ((colr & 0x0F) == 0) {
            return;
         }
      }
   }

   Byte colour = colr;
   if (por.dither && scmr.md != 3) {
      if ((x ^ y) & 1) {
         colour = colour >> 4;
      }
      colour = colour & 0x0F;
   }

   Word offset = (y << 5) + (x >> 3);
   if (offset != pixel_cache[0].offset) {
      flush_pixel_cache(pixel_cache[1]);
      pixel_cache[1] = pixel_cache[0];
      pixel_cache[0].bitpend = 0x00;
      pixel_cache[0].offset = offset;
   }

   x = (x & 7) ^ 7;
   pixel_cache[0].data[x] = colour;
   pixel_cache[0].bitpend = (pixel_cache[0].bitpend) | (1 << x);
   if (pixel_cache[0].bitpend == 0xFF) {
      flush_pixel_cache(pixel_cache[1]);
      pixel_cache[1] = pixel_cache[0];
      pixel_cache[0].bitpend = 0x00;
   }
}

Byte SuperFX::rpix(Byte x, Byte y) {
   flush_pixel_cache(pixel_cache[1]);
   flush_pixel_cache(pixel_cache[0]);

   int cn;
   switch (por.obj ? 3 : scmr.ht) {
   case 0:
      cn = ((x & 0xF8) << 1) + ((y & 0xF8) >> 3);
      break;
   case 1:
      cn = ((x & 0xF8) << 1) + ((x & 0xF8) >> 1) + ((y & 0xF8) >> 3);
      break;
   case 2:
      cn = ((x & 0xF8) << 1) + ((x & 0xF8) << 0) + ((y & 0xF8) >> 3);
      break;
   case 3:
      cn = ((y & 0x80) << 2) + ((x & 0x80) << 1) + ((y & 0x78) << 1) + ((x & 0x78) >> 3);
      break;
   }

   int bpp = 2 << (scmr.md - (scmr.md >> 1));
   int address = 0x700000 + (cn * (bpp << 3)) + (scbr << 10) + ((y & 0x07) * 2);
   Byte data = 0x00;
   x = (x & 7) ^ 7;

   for (int n = 0; n < bpp; n++) {
      int byte = ((n >> 1) << 4) + (n & 1);
      step(clsr ? 5 : 6);
      data = data | ((read(address + byte) >> x) & 1) << n;
   }

   return data;
}

void SuperFX::flush_pixel_cache(PixelCache& cache) {
   if (cache.bitpend == 0x00) {
      return;
   }

   Byte x = cache.offset << 3;
   Byte y = cache.offset >> 5;

   int cn;
   switch (por.obj ? 3 : scmr.ht) {
   case 0:
      cn = ((x & 0xF8) << 1) + ((y & 0xF8) >> 3);
      break;
   case 1:
      cn = ((x & 0xF8) << 1) + ((x & 0xF8) >> 1) + ((y & 0xF8) >> 3);
      break;
   case 2:
      cn = ((x & 0xF8) << 1) + ((x & 0xF8) << 0) + ((y & 0xF8) >> 3);
      break;
   case 3:
      cn = ((y & 0x80) << 2) + ((x & 0x80) << 1) + ((y & 0x78) << 1) + ((x & 0x78) >> 3);
      break;
   }

   int bpp = 2 << (scmr.md - (scmr.md >> 1));
   int address = 0x700000 + (cn * (bpp << 3)) + (scbr << 10) + ((y & 0x07) * 2);
   
   for (int n = 0; n < bpp; n++) {
      int byte = ((n >> 1) << 4) + (n & 1);
      Byte data = 0x00;
      for (int x = 0; x < 8; x++) {
         data = data | (((cache.data[x] >> n) & 1) << x);
      }
      if (cache.bitpend != 0xFF) {
         step(clsr ? 5 : 6);
         data = data & cache.bitpend;
         data = data | (read(address + byte) & ~cache.bitpend);
      }
      step(clsr ? 5 : 6);
      write(address + byte, data);
   }

   cache.bitpend = 0x00;
}

void SuperFX::step(int clocks) {
   if (romcl) {
      romcl -= std::min(clocks, romcl);
      if (romcl == 0) {
         sfr.r = 0;
         romdr = read((rombr << 16) + r[14]);
      }
   }

   if (ramcl) {
      ramcl -= std::min(clocks, ramcl);
      if (ramcl == 0) {
         write(0x700000 + (rambr << 16) + ramar, ramdr);
      }
   }

   cycle += clocks * cycles_per_clock;
   synchronise_cpu();
}

void SuperFX::sync_rom_buffer() {
   if (romcl) {
      step(romcl);
   }
}

Byte SuperFX::read_rom_buffer() {
   sync_rom_buffer();
   return romdr;
}

void SuperFX::update_rom_buffer() {
   sfr.r = 1;
   romcl = clsr ? 5 : 6;
}

void SuperFX::sync_ram_buffer() {
   if (ramcl) {
      step(ramcl);
   }
}

Byte SuperFX::read_ram_buffer(Word address) {
   sync_ram_buffer();
   return read(0x700000 + (rambr << 16) + address);
}

void SuperFX::write_ram_buffer(Word address, Byte data) {
   sync_ram_buffer();
   ramcl = clsr ? 5 : 6;
   ramar = address;
   ramdr = data;
}

size_t SuperFX::get_rom_size() {
   return cartridge->get_rom_size();
}

Byte SuperFX::read_rom(unsigned int address, bool snes_accessing) {
   if (sfr.g && scmr.ron && snes_accessing) {
      const Byte vector[16] = {
         0x00, 0x01, 0x00, 0x01, 0x04, 0x01, 0x00, 0x01,
         0x00, 0x01, 0x08, 0x01, 0x00, 0x01, 0x0c, 0x01
      };
      return vector[address & 15];
   }
   return cartridge->get_from_rom(address & rom_mask);
}

void SuperFX::write_rom(unsigned int address, Byte data) {
   return;
}

Byte SuperFX::read_ram(unsigned int address, bool snes_accessing) {
   if (sfr.g && scmr.ran && snes_accessing) {
      return get_open_bus();
   }
   return gpram[address & ram_mask];
}

void SuperFX::write_ram(unsigned int address, Byte data) {
   gpram[address & ram_mask] = data;
}

Byte SuperFX::read_io(unsigned int address) {
   address = 0x3000 | address & 0x3FF;

   if (address >= 0x3100 && address <= 0x32FF) {
      return read_cache(address - 0x3100);
   }
   if (address >= 0x3000 && address <= 0x301F) {
      return r[(address >> 1) & 15] >> ((address & 1) << 3);
   }

   switch (address) {
      case 0x3030: {
         return sfr >> 0;
      }
      case 0x3031: {
         Byte rr = sfr >> 8;
         sfr.irq = 0;
         cpu->unsignal_irq();
         return rr;
      }

      case 0x3034: {
         return pbr;
      }

      case 0x3036: {
         return rombr;
      }

      case 0x303B: {
         return vcr;
      }

      case 0x303C: {
         return rambr;
      }

      case 0x303E: {
         return cbr >> 0;
      }

      case 0x303F: {
         return cbr >> 8;
      }
   }

   return get_open_bus();
}

void SuperFX::write_io(unsigned int address, Byte data) {
   address = 0x3000 | (address & 0x3FF);
   if (address >= 0x3100 && address <= 0x32FF) {
      return write_cache(address - 0x3100, data);
   }

   if (address >= 0x3000 && address <= 0x301F) {
      unsigned int n = (address >> 1) & 15;
      if ((address & 1) == 0) {
         r[n] = (r[n] & 0xFF00) | data;
      } else {
         r[n] = (data << 8) | (r[n] & 0xFF);
      }
      if (n == 14) {
         update_rom_buffer();
      }

      if (address == 0x301F) {
         sfr.g = 1;
      }
      return;
   }

   switch(address) {
      case 0x3030: {
         bool g = sfr.g;
         sfr = (sfr & 0xFF00) | (data << 0);
         if (g == 1 && sfr.g == 0) {
            cbr = 0x0000;
            flush_cache();
         }
         break;
      }

      case 0x3031: {
         sfr = (data << 8) | (sfr & 0x00FF);
         break;
      }

      case 0x3033: {
         bramr = data & 0x01;
         break;
      }

      case 0x3034: {
         pbr = data & 0x7F;
         flush_cache();
         break;
      }

      case 0x3037 : {
         cfgr = data;
         break;
      }

      case 0x3038: {
         scbr = data;
         break;  
      }

      case 0x3039: {
         clsr = data & 0x01;
         break;
      }

      case 0x303A: {
         scmr = data;
         break;
      }

   }
}

bool SuperFX::handles(SNESAddress address) {
   if (revision == SuperFXRevision::MARIO) {
      bool gsu_io = ((address.bank >= 0x00 && address.bank <= 0x3F) || (address.bank >= 0x80 && address.bank <= 0xBF)) && address.offset >= 0x3000 && address.offset <= 0x347F;
      bool gpram_area = (address.bank >= 0x60 && address.bank <= 0x7D) || (address.bank >= 0xE0 && address.bank <= 0xFF);
      return gsu_io || gpram_area;
   }

   if (revision == SuperFXRevision::GSU1) {
      bool gsu_io = ((address.bank >= 0x00 && address.bank <= 0x3F) || (address.bank >= 0x80 && address.bank <= 0xBF)) && address.offset >= 0x3000 && address.offset <= 0x34FF;
      bool mirror = ((address.bank >= 0x00 && address.bank <= 0x3F) || (address.bank >= 0x80 && address.bank <= 0xBF)) && address.offset >= 0x6000 && address.offset <= 0x7FFF;
      bool gpram_area = (address.bank >= 0x70 && address.bank <= 0x71) || (address.bank >= 0xF0 && address.bank <= 0xF1);  
      return gsu_io || mirror || gpram_area;
   }

   if (revision == SuperFXRevision::GSU2 || revision == SuperFXRevision::GSU2SP1) {
      bool gsu_io = ((address.bank >= 0x00 && address.bank <= 0x3F) || (address.bank >= 0x80 && address.bank <= 0xBF)) && address.offset >= 0x3000 && address.offset <= 0x34FF;
      bool mirror = ((address.bank >= 0x00 && address.bank <= 0x3F) || (address.bank >= 0x80 && address.bank <= 0xBF)) && address.offset >= 0x6000 && address.offset <= 0x7FFF;
      bool gpram_area = (address.bank >= 0x70 && address.bank <= 0x71);
      return gsu_io || mirror || gpram_area;
   }
   
   return false;
}

Byte SuperFX::snes_side_read(SNESAddress address) {
   if (revision == SuperFXRevision::MARIO) {
      bool gsu_io = ((address.bank >= 0x00 && address.bank <= 0x3F) || (address.bank >= 0x80 && address.bank <= 0xBF)) && address.offset >= 0x3000 && address.offset <= 0x347F;
      bool gpram_area = (address.bank >= 0x60 && address.bank <= 0x7D) || (address.bank >= 0xE0 && address.bank <= 0xFF);
      if (gsu_io) {
         return read_io(address.offset);
      }
      if (gpram_area) {
         return read_ram(address.offset, true);
      }
   }
   if (revision == SuperFXRevision::GSU1) {
      bool gsu_io = ((address.bank >= 0x00 && address.bank <= 0x3F) || (address.bank >= 0x80 && address.bank <= 0xBF)) && address.offset >= 0x3000 && address.offset <= 0x34FF;
      bool mirror = ((address.bank >= 0x00 && address.bank <= 0x3F) || (address.bank >= 0x80 && address.bank <= 0xBF)) && address.offset >= 0x6000 && address.offset <= 0x7FFF;
      bool gpram_area = (address.bank >= 0x70 && address.bank <= 0x71) || (address.bank >= 0xF0 && address.bank <= 0xF1);  
      if (gsu_io) {
         return read_io(address.offset);
      }
      if (mirror) {
         return read_ram(address.offset - 0x6000, true);
      }
      if (gpram_area) {
         return read_ram(address.offset, true);
      }
   }
   if (revision == SuperFXRevision::GSU2 || revision == SuperFXRevision::GSU2SP1) {
      bool gsu_io = ((address.bank >= 0x00 && address.bank <= 0x3F) || (address.bank >= 0x80 && address.bank <= 0xBF)) && address.offset >= 0x3000 && address.offset <= 0x34FF;
      bool mirror = ((address.bank >= 0x00 && address.bank <= 0x3F) || (address.bank >= 0x80 && address.bank <= 0xBF)) && address.offset >= 0x6000 && address.offset <= 0x7FFF;
      bool gpram_area = (address.bank >= 0x70 && address.bank <= 0x71);
      if (gsu_io) {
         return read_io(address.offset);
      }
      if (mirror) {
         return read_ram(address.offset - 0x6000, true);
      }
      if (gpram_area) {
         return read_ram(address.offset, true);
      }
   }
   return get_open_bus();
}

void SuperFX::snes_side_write(SNESAddress address, Byte data) {
   if (revision == SuperFXRevision::MARIO) {
      bool gsu_io = ((address.bank >= 0x00 && address.bank <= 0x3F) || (address.bank >= 0x80 && address.bank <= 0xBF)) && address.offset >= 0x3000 && address.offset <= 0x347F;
      bool gpram_area = (address.bank >= 0x60 && address.bank <= 0x7D) || (address.bank >= 0xE0 && address.bank <= 0xFF);
      if (gsu_io) {
         write_io(address.offset, data);
      }
      if (gpram_area) {
         write_ram(address.offset, data);
      }
   }
   if (revision == SuperFXRevision::GSU1) {
      bool gsu_io = ((address.bank >= 0x00 && address.bank <= 0x3F) || (address.bank >= 0x80 && address.bank <= 0xBF)) && address.offset >= 0x3000 && address.offset <= 0x34FF;
      bool mirror = ((address.bank >= 0x00 && address.bank <= 0x3F) || (address.bank >= 0x80 && address.bank <= 0xBF)) && address.offset >= 0x6000 && address.offset <= 0x7FFF;
      bool gpram_area = (address.bank >= 0x70 && address.bank <= 0x71) || (address.bank >= 0xF0 && address.bank <= 0xF1);  
      if (gsu_io) {
         write_io(address.offset, data);
         return;
      }
      if (mirror) {
         write_ram(address.offset - 0x6000, data);
         return;
      }
      if (gpram_area) {
         write_ram(address.offset, data);
         return;
      }
   }
   if (revision == SuperFXRevision::GSU2 || revision == SuperFXRevision::GSU2SP1) {
      bool gsu_io = ((address.bank >= 0x00 && address.bank <= 0x3F) || (address.bank >= 0x80 && address.bank <= 0xBF)) && address.offset >= 0x3000 && address.offset <= 0x34FF;
      bool mirror = ((address.bank >= 0x00 && address.bank <= 0x3F) || (address.bank >= 0x80 && address.bank <= 0xBF)) && address.offset >= 0x6000 && address.offset <= 0x7FFF;
      bool gpram_area = (address.bank >= 0x70 && address.bank <= 0x71);
      if (gsu_io) {
         write_io(address.offset, data);
         return;
      }
      if (mirror) {
         write_ram(address.offset - 0x6000, data);
         return;
      }
      if (gpram_area) {
         write_ram(address.offset, data);
         return;
      }
   }
}
