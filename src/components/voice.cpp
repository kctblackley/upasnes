#include "sdsp.hpp"
#include "apubus.hpp"

constexpr int ENVELOPE_COUNTER_RANGE = 30720;

constexpr uint32_t envelope_period_table[32] = {
    ENVELOPE_COUNTER_RANGE + 1, 2048, 1536,
    1280, 1024, 768,
    640,  512,  384,
    320,  256,  192,
    160,  128,  96,
    80,   64,   48,
    40,   32,   24,
    20,   16,   12,
    10,   8,    6,
    5,    4,    3,
    2,    1
};

constexpr uint32_t envelope_offset_table[32] = {
    0,   0,    1040,
    536, 0,    1040,
    536, 0,    1040,
    536, 0,    1040,
    536, 0,    1040,
    536, 0,    1040,
    536, 0,    1040,
    536, 0,    1040,
    536, 0,    1040,
    536, 0,    1040,
    536, 0
};

void Voice::mem_write(Word address, Byte value) {
	if (bus) {
		bus->write(address, value);
	}
}

Byte Voice::mem_read(Word address) {
	if (bus) {
		return bus->read(address);
	} else {
		return 0x00;
	}
}

void Voice::decode_brr_block() {
	header = mem_read(current_brr_address);
	
	shift     = header >> 4;
	filter    = (header >> 2) & 3;
	loop_flag = (header & 0x02) != 0;
	end_flag  = (header & 0x01) != 0;

	for (int i = 0; i < 16; i++) {
		Byte data = mem_read(current_brr_address + 1 + i / 2);
		Byte nibble;
		if ((i & 1) == 0) {
			nibble = data >> 4;
		} else {
			nibble = data & 0x0F;
		}

		Sample sample = nibble;

		if (sample >= 8) {
			sample -= 16;
		}

		if (shift <= 12) {
			sample = sample << shift;
			sample = sample >> 1;
		} else {
			sample = (sample < 0) ? -2048 : 2048;
		}

		switch(filter) {
		case 1:
			sample += prev1;
			sample -= prev1 >> 4;
			break;
		case 2: 
			sample += (prev1 << 1);
			sample -= (prev1 * 3) >> 5;

			sample -= prev2;
			sample += prev2 >> 4;
			break;
		case 3:
			sample += (prev1 << 1);
			sample -= (prev1 * 13) >> 6;

			sample -= prev2;
			sample += (prev2 * 3) >> 4;
			break;
		}

		sample = std::clamp(sample, -32768, 32767);
		sample = sample & ~1;
		
		decoded_samples[i] = sample;
		prev2 = prev1;
		prev1 = sample;
	}

	next_brr_address = current_brr_address + 9;
}

StereoSample Voice::output() {
	if (!active) {
		return {0, 0};
	}

	return {
		static_cast<int16_t>(final_sample * voll / 128),
		static_cast<int16_t>(final_sample * volr / 128)
	};
}

void Voice::advance_brr_address() {
	if (end_flag) {
		endx_flag = true;
		if (loop_flag) {
			current_brr_address = loop_brr_address;
		} else {
			envelope = 0;
			active = false;
		}
	} else {
		current_brr_address = next_brr_address;
	}
}

Sample Voice::gauss_interpolate() {
	int i = (pitch_counter >> 4) & 0xFF;
	Sample out = (gauss_table[0xFF - i] * gauss.oldest) >> 10;
	out += (gauss_table[0x1FF - i] * gauss.older) >> 10;
	out += (gauss_table[0x100 + i] * gauss.old) >> 10;
	out += (gauss_table[0x000 + i] * gauss.newest) >> 10;
	out = out >> 1;
	return out;
};

bool Voice::fire(int rate) {
	if (rate == 0) {
		return false;
	}
	return ((envelope_tick + envelope_offset_table[rate]) % envelope_period_table[rate]) == 0;
}

void Voice::calculate_envelope() {
	if (envelope_tick == 0) {
		envelope_tick = ENVELOPE_COUNTER_RANGE - 1;
	} else {
		envelope_tick--;
	}
	if (is_adsr) {
		switch (envelope_state) {
		case EnvelopeState::ATTACK:
			if (fire(attack_rate)) {
				envelope += attack_step;
			}
			if (envelope >= 0x7E0) {
				envelope_state = EnvelopeState::DECAY;
			}
			if (envelope >= 0x800) {
				envelope = 0x7FF;
			}
			break;
		case EnvelopeState::DECAY:
			if (fire(decay_rate)) {
				envelope -= (((envelope - 1) >> 8) + 1);
			}
			if (envelope <= sustain_boundary) {
				envelope_state = EnvelopeState::SUSTAIN;
				envelope = sustain_boundary;
			}
			break;
		case EnvelopeState::SUSTAIN:
			if (fire(sustain_rate)) {
				envelope -= (((envelope - 1) >> 8) + 1);
			}
			break;
		case EnvelopeState::RELEASE:
			envelope -= 8;
			if (envelope <= 0) {
				active = false;
			}
			break;
		}
	} else {
		// GAIN (to implement)
		envelope = 0x7FF;
	}
}

void Voice::apply_envelope() {
	final_sample = (current_sample * envelope) >> 11; 
}

void Voice::tick() {
	if (!active) {
		current_sample = 0;
		return;
	}
	
	uint16_t pitch = ((pitchr & 0x3F) << 8) | pitchl;
	pitch_counter += pitch;

	while (pitch_counter >= 0x1000) {
		pitch_counter -= 0x1000;
		sample_index += 1;

		if (sample_index == 16) {
			advance_brr_address();

			if (!active) {
				current_sample = 0;
				return;
			}

			decode_brr_block();
			sample_index = 0;
		}

		gauss.oldest   = gauss.older;
		gauss.older    = gauss.old;
		gauss.old      = gauss.newest;
		gauss.newest   = decoded_samples[sample_index];
	}
	
	current_sample = gauss_interpolate();
	
	calculate_envelope();
	apply_envelope();

	outx = final_sample >> 8;
	envx = envelope >> 4;

	return;
}