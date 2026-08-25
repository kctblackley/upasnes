#include "sdsp.hpp"
#include "apubus.hpp"

// SDSP

void SDSP::mem_write(Word address, Byte value) {
	if (bus) {
		bus->aram_write(address, value);
	}
}

Byte SDSP::mem_read(Word address) {
	if (bus) {
		return bus->aram_read(address);
	} else {
		return 0x00;
	}
}

StereoSample SDSP::output() {
	int32_t left = 0;
	int32_t right = 0;

	int32_t echo_left = 0;
	int32_t echo_right = 0;

	for (auto& v : voices) {
		auto sample = v.output();

		left += sample.left;
		right += sample.right;

		if (v.is_echo_enabled()) {
			echo_left += sample.left;
			echo_right += sample.right;
		}
	}

	left  = std::clamp(left, -32768, 32767);
	right = std::clamp(right, -32768, 32767);
	
	int32_t mixed_left  = (left  * mvoll) >> 7;
	int32_t mixed_right = (right * mvolr) >> 7;

	echo_left  = std::clamp(echo_left,  -32768, 32767);
	echo_right = std::clamp(echo_right, -32768, 32767);

	StereoSample old_echo = read_echo_sample();
	push_echo_history(old_echo);

	StereoSample filtered_echo = process_fir();

	mixed_left += ((int32_t)(filtered_echo.left) * evoll) >> 7;
	mixed_right += ((int32_t)(filtered_echo.right) * evolr) >> 7;

	int32_t feedback_left = echo_left + (((int32_t)(filtered_echo.left) * efb) >> 7);
	int32_t feedback_right = echo_right + (((int32_t)(filtered_echo.right) * efb) >> 7);

	feedback_left = std::clamp(feedback_left, -32768, 32767);
	feedback_right = std::clamp(feedback_right, -32768, 32767);

	StereoSample echo_input { (int16_t)(feedback_left & ~1), (int16_t)(feedback_right & ~1) };

	if (!(flg & 0x20)) {
		write_echo_sample(echo_input);
	}

	advance_echo_index();

	left = std::clamp(mixed_left, -32768, 32767);
	right = std::clamp(mixed_right, -32768, 32767);

	return {
		(int16_t)left,
		(int16_t)right
	};
}

void SDSP::update_noise() {
	if (noise_counter == 0) {
		noise_counter = envelope_period_table[noise_frequency];
		Byte bit0 = noise_lfsr & 1;
		Byte bit1 = (noise_lfsr >> 1) & 1;

		noise_lfsr = (noise_lfsr >> 1) | ((bit0 ^ bit1) << 14);
	} else {
		noise_counter--;
	}

	noise_sample = (noise_lfsr & 0x4000)
		? static_cast<Sample>(noise_lfsr) - 0x8000
		: static_cast<Sample>(noise_lfsr);
}

void SDSP::tick() {

	Sample previous_sample = 0;
	update_noise();
	process_kon();
	process_koff();
	for (auto& v : voices) {
		v.tick(previous_sample, noise_sample);
		previous_sample = v.modulation_output();
	}

	StereoSample out = output();
	audio_buffer.push(out);
}