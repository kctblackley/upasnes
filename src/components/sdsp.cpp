#include "sdsp.hpp"
#include "apubus.hpp"

// SDSP

void SDSP::mem_write(Word address, Byte value) {
	if (bus) {
		bus->write(address, value);
	}
}

Byte SDSP::mem_read(Word address) {
	if (bus) {
		return bus->read(address);
	} else {
		return 0x00;
	}
}

StereoSample SDSP::output() {
	int32_t left = 0;
	int32_t right = 0;

	for (auto& v : voices) {
		auto sample = v.output();

		left += sample.left;
		right += sample.right;
	}

	left  = (left  * mvoll) / 128;
	right = (right * mvolr) / 128;

	left = std::clamp(left, -32768, 32767);
	right = std::clamp(right, -32768, 32767);

	return {
		(int16_t)left,
		(int16_t)right
	};
}

void SDSP::tick() {
	process_kon();
	process_koff();
	for (auto& v : voices) {
		v.tick();
	}

	StereoSample out = output();
	audio_buffer.push(out);
}