#pragma once
#include <array>
#include <memory>
#include "sdsp.hpp"

#define APU_DATA_SIZE 65536
#define IPL_SIZE 64

class SDSP;

class APUBus {
public:
	APUBus();

	Byte read(Word address);

	void write(Word address, Byte value);

	Byte aram_read(Word address) {
		return data[address];
	}

	void aram_write(Word address, Byte value) {
		data[address] = value;
	}

	void enable_ipl();
	void disable_ipl();

	void enable_test_mode();
	void disable_test_mode();

	void reset_test_memory();

	void tick_sdsp() {
		sdsp.tick();
	}

	bool sdsp_above_half() {
		return sdsp.above_half_capacity();
	}

	size_t audio_buffer_size() {
		return sdsp.audio_buffer_size();
	}

	void close_audio() {
		sdsp.close_audio();
	}

private:

	SDSP sdsp;

	bool ipl_enabled = false;
	std::array<Byte, APU_DATA_SIZE> data {};

	std::array<Byte, IPL_SIZE> ipl {};

	Byte dsp_address = 0x00;

};