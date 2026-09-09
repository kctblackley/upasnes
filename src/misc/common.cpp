#include "common.hpp"

u8 set_bit(u8 byte, u8 bit) {
	return byte | (0b1 << bit);
}

u8 clear_bit(u8 byte, u8 bit) {
	return byte & ~(0b1 << bit);
}

void set_lo(u16& val, u8 lo) {
   val = (get_hi(val) << 8) | lo;
}

void set_hi(u16& val, u8 hi) {
   val = (hi << 8) | (get_lo(val));
}

Region region_from_header_byte(u8 code) {
   switch (code) {
      case 0x00:
      case 0x01:
      case 0x0D:
      case 0x0F:
      case 0x10:
         return Region::NTSC;
      default:
         return Region::PAL;
   }
}

