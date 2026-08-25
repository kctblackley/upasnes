#include "common.hpp"

Byte set_bit(Byte byte, Byte bit) {
	return byte | (0b1 << bit);
}

Byte clear_bit(Byte byte, Byte bit) {
	return byte & ~(0b1 << bit);
}

void set_lo(uint16_t& val, uint8_t lo) {
   val = (get_hi(val) << 8) | lo;
}

void set_hi(uint16_t& val, uint8_t hi) {
   val = (hi << 8) | (get_lo(val));
}

