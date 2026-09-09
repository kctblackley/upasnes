#pragma once
#include "common.hpp"
#include "stereo_sample.hpp"
#include "gauss_table.hpp"
#include "audio_buffer.hpp"
#include <cmath>
#include <numbers>

constexpr double pi = std::numbers::pi;
using Sample = i32;

constexpr int ENVELOPE_COUNTER_RANGE = 30720;

constexpr u32 envelope_period_table[32] = {
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

constexpr u32 envelope_offset_table[32] = {
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

enum class EnvelopeState {
	ATTACK,
	DECAY,
	SUSTAIN,
	RELEASE,
	DIRECT,
	NONE
};

struct Gauss {
	Sample newest = 0;
	Sample old    = 0;
	Sample older  = 0;
	Sample oldest = 0;
};

class APUBus;

class Voice {
public:
	Voice() {}

	void reset_gauss() {
		gauss.newest = 0;
		gauss.old    = 0;
		gauss.older  = 0;
		gauss.oldest = 0;
	}

	u8 read(u8 reg) {
		switch (reg) {
		case 0x0: return voll;   break;
		case 0x1: return volr;   break;
		case 0x2: return pitchl; break;
		case 0x3: return pitchr; break;
		case 0x4: return srcn;   break;
		case 0x5: return adsr1;  break;
		case 0x6: return adsr2;  break;
		case 0x7: return gain;   break;
		case 0x8: return envx;   break;
		case 0x9: return outx;   break;
		}
		return 0x00;
	}

	void write(u8 reg, u8 value) {
		switch (reg) {
		case 0x0: voll   = (i8)value; break;
		case 0x1: volr   = (i8)value; break;
		case 0x2: pitchl =         value; break;
		case 0x3: pitchr =  value & 0x3F; break;
		case 0x4: srcn   =         value; break;
		case 0x5:
			adsr1 = value;
			break;
		case 0x6:
			adsr2 = value;
			break;
		case 0x7:
			gain = value;
			gain_mode  = (gain >> 5) & 0x03;
			gain_value =  gain & 0x1F;
			break;
		}
	}

	void decode_brr_block();
	void advance_brr_address();
	void calculate_envelope();
	void apply_envelope();
	bool fire(int rate);

	void reset_adsr_gain() {
		is_adsr = adsr1 & 0x80;

		if (is_adsr) {
			// ADSR
			envelope_state = EnvelopeState::ATTACK;
			envelope = 0;

			int n;
			n = adsr1 & 0xF;
			attack_rate = (n * 2) + 1;
			attack_step = (attack_rate == 31) ? 1024 : 32;

			n = (adsr1 >> 4) & 0x7;
			decay_rate = (n * 2) + 16;

			sustain_rate = adsr2 & 0x1F;
			n = (adsr2 >> 5) & 0x7;
			sustain_boundary = n * 0x100;

		} else {
			gain_mode = (gain >> 5) & 0x03;
			gain_value = gain & 0x1F;

			envelope = 0x0;
		}
	}

	void key_on(u8 dir) {
		directory = dir * 0x100;
		entry = (directory + srcn * 4) & 0xFFFF;
		current_brr_address = (mem_read(entry + 1) << 8) | mem_read(entry + 0);
		loop_brr_address    = (mem_read(entry + 3) << 8) | mem_read(entry + 2);

		prev1 = 0;
		prev2 = 0;

		reset_gauss();
		reset_adsr_gain();

		pitch_counter = 0;

		active = true;

		decode_brr_block();
		sample_index = 0;

		endx_flag = false;

		return;
	}

	void release() {
		envelope_state = EnvelopeState::RELEASE;
	}

	bool endx() { // <- IMPORTANT
		return endx_flag;
	}

	void clear_endx() {
		endx_flag = false;
	}

	void pmon(bool enabled) { pitch_mod_enabled = enabled; }
	void  non(bool enabled) { noise_enabled = enabled; }
	void  eon(bool enabled) { echo_enabled = enabled; }

	void set_id(int id) {
		this->id = id;
	}

	void reset() {
		pitch_mod_enabled = false;
		noise_enabled = false;
		echo_enabled = false;

		envx = 0;
		outx = 0;

		endx_flag = false;
	}

	void connect_voice_to_sdsp(APUBus* bus) {
		this->bus = bus;
	}

	Sample gauss_interpolate();
	
	Sample modulation_output() const {
		return outx;
	}

	void tick(Sample modulation = 0, Sample noise = 0);
	u8 mem_read(u16 address);
	void mem_write(u16 address, u8 value);

	bool is_echo_enabled() {
		return echo_enabled;
	}
	// Sample outputting
	StereoSample output();

private:
	APUBus* bus = nullptr;

	bool pitch_mod_enabled = false;
	bool noise_enabled = false;
	bool echo_enabled = false;

	int id = 0;

	i8 voll = 0x00;
	i8 volr = 0x00;

	u8 pitchl =  0x00;
	u8 pitchr = 0x00;

	u8 srcn = 0x00;

	u8 adsr1 = 0x00;
	u8 adsr2 = 0x00;

	bool is_adsr = false;
	u8 decay_rate = 0x00;
	u8 attack_rate = 0x00;
	u16 attack_step = 0x00;
	u16 sustain_boundary = 0x00;
	u8 sustain_rate = 0x00;
	u8 release_rate = 31;

	u8 gain = 0x00; // how this is written depends on whether bit 7 is set/clear
	u8 gain_value = 0x00;
	u8 gain_mode  = 0x00;

	int envelope = 0;
	u32 envelope_tick = 0;
	EnvelopeState envelope_state = EnvelopeState::NONE;

	u32 fire_countdown[32] = {};

	u8 envx = 0x00;
	i8 outx =  0x00;

	bool endx_flag = false;

	// BRR decoding
	u16 directory;
	u16 entry;
	u16 current_brr_address;
	u16 next_brr_address;
	u16 loop_brr_address;

	u8 brr_header = 0;
	u8 brr_position = 0;
	bool looping = false;
	bool active = false;

	// Current sample
	Sample current_sample = 0;
	Sample prev1  = 0;
	Sample prev2  = 0;

	Sample final_sample = 0;

	u16 sample_index  = 16;
	u16 nibble_index  = 16;
	u32 pitch_counter = 0;

	std::array<Sample, 16> decoded_samples {};

	u8 header = 0x00;
	u8 shift = 0x00;
	u8 filter = 0x00;

	bool loop_flag = false;
	bool end_flag = false;

	Gauss gauss;
};

class SDSP {
public:
	SDSP() {
		int id = 0;
		for (auto& v : voices) {
			v.set_id(id);
			id++;
		}

		echo_index = 0;
		echo_buffer_size = echo_buffer_entries();

		for (auto& sample : echo_history) {
			sample = {0, 0};
		}
	}

	void close_audio() {
		audio_buffer.close_audio();
	}

	// For DSP registers
	void write(u8 address, u8 value) {
		if (address & 0x80) {
			return;
		}
		// Voies registers
		if (address < 0x80 && (address & 0x0F) <= 0x09) {
			voices[(address & 0xF0) >> 4].write(address & 0xF, value);
			return;
		}

		// S-DSP registers
		if ( (address & 0xF) == 0xF && address < 0x80) {
			fir[(address & 0xF0) >> 4] = value;
		}

		switch (address) {
		case 0x0C: mvoll = (i8)value; break;
		case 0x1C: mvolr = (i8)value; break;
		case 0x2C: evoll = (i8)value; break;
		case 0x3C: evolr = (i8)value; break;
		case 0x4C:
			kon = value;
			kon_pending = kon_pending | value;
			break;
		case 0x5C:
			koff = value;
			koff_pending = koff_pending | value;
			break;
		case 0x6C:
			flg = value;
			noise_frequency = flg & 0x1F;
			echo_disable = (flg & 0x20);
			mute_all = (flg & 0x40);
			soft_reset = (flg & 0x80);
			break;
		case 0x7C:
			for (auto& v : voices) {
				v.clear_endx();
			}
			break;
		case 0x0D: efb = (i8)value; break;
		case 0x2D: {
			pmon = value;
			u8 tmp = pmon;
			for (int v = 0; v < 8; v++) {
				voices[v].pmon(tmp & 1);
				tmp = tmp >> 1;
			}
			break;
		}
		case 0x3D: {
			non = value;
			u8 tmp = non;
			for (int v = 0; v < 8; v++) {
				voices[v].non(tmp & 1);
				tmp = tmp >> 1;
			}
			break;
		}
		case 0x4D: {
			eon = value;
			u8 tmp = eon;
			for (int v = 0; v < 8; v++) {
				voices[v].eon(tmp & 1);
				tmp = tmp >> 1;
			}
			break;
		}
		case 0x5D: dir = value; break;
		case 0x6D: esa = value; break;
		case 0x7D: edl = value & 0xF; break;
		default: break;
		}

		registers[address & 0x7F] = value;
	}

	u8 read(u8 address) {
		// Voice registers
		if (address < 0x80 && (address & 0x0F) <= 0x09) {
			return voices[(address & 0xF0) >> 4].read(address & 0xF);
		}

		// S-DSP registers
		switch(address) {
		case 0x0C: return mvoll; break;
		case 0x1C: return mvolr; break;
		case 0x2C: return evoll; break;
		case 0x3C: return evolr; break;
		case 0x4C: return kon; break;
		case 0x5C: return koff; break;
		case 0x6C: return flg; break;
		case 0x7C:
			{
				endx = 0x00;
				int shift = 0;
				for (auto& v : voices) {
					if (v.endx()) {
						endx = endx | (1 << shift);
					}
					shift++;
				}
				return endx;
			}
			break;
		case 0x0D: return efb; break;
		case 0x2D: return pmon; break;
		case 0x3D: return non; break;
		case 0x4D: return eon; break;
		case 0x5D: return dir; break;
		case 0x6D: return esa; break;
		case 0x7D: return edl; break;
		default:
			break;
		}

		if ( (address & 0xF) == 0xF && address < 0x80) {
			return fir[(address & 0xF0) >> 4];
		}

		return registers[address & 0x7F];
	}

	void connect_bus_to_sdsp(APUBus* bus) {
		this->bus = bus;
		for (auto& v : voices) {
			v.connect_voice_to_sdsp(bus);
		}
	}

	void process_kon() {
		if (kon_pending == 0 && kon_delay_active == 0) {
			return;
		}

		for (int v = 0; v < 8; v++) {
			if (kon_pending & (1 << v)) {
				kon_delay[v] = 5;
				kon_delay_active |= (1 << v);
				kon_pending = kon_pending & ~(1 << v);
				continue;
			}

			if (kon_delay[v] > 0) {
				kon_delay[v]--;

				//if (kon_delay[v] == 0) {
					voices[v].key_on(dir);
					kon_delay_active &= ~(1 << v);
					kon_delay[v] = 0;
				//}
			}
		}
	}

	void process_koff() {
		if (koff_pending == 0) {
			return;
		}

		for (int v = 0; v < 8; v++) {
			if (koff_pending & (1 << v)) {
				voices[v].release();
				koff_pending = koff_pending & ~(1 << v);
			}
		}
	}

	void tick();

	// For reading/writing ARAM
	void mem_write(u16 address, u8 value);
	u8 mem_read(u16 address);

	bool above_half_capacity() {
		return audio_buffer.above_half_capacity();
	}

	size_t audio_buffer_size() {
		return audio_buffer.samples_available();
	}

	void update_noise();

	StereoSample output();

	u16 echo_address() const {
		return (static_cast<u16>(esa) << 8) + (echo_index * 4);
	}

	StereoSample read_echo_sample() {
		u16 address = echo_address();

		u16 left = (mem_read(address + 1) << 8) | mem_read(address + 0);
		u16 right = (mem_read(address + 3) << 8) | mem_read(address + 2);

		return { static_cast<i16>(left), static_cast<i16>(right) };
	}

	void write_echo_sample(StereoSample sample) {
		u16 address = echo_address();

		mem_write(address + 0, sample.left & 0xFE);
		mem_write(address + 1, sample.left >> 8);

		mem_write(address + 2, sample.right & 0xFE);
		mem_write(address + 3, sample.right >> 8);
	}

	u16 echo_buffer_entries() const {
		if (edl == 0) {
			return 1;
		}

		return edl << 9;
	}

	void advance_echo_index() {
		echo_index++;

		if (echo_index >= echo_buffer_size) {
			echo_index = 0;
			echo_buffer_size = echo_buffer_entries();
		}
	}

	void push_echo_history(StereoSample sample) {
		for (int i = 7; i > 0; i--) {
			echo_history[i] = echo_history[i - 1];
		}

		echo_history[0] = sample;
	}

	StereoSample process_fir() {
		i16 left = 0;
		i16 right = 0;

		for (int i = 0; i < 7; i++) {
			i32 coefficient = fir[i];

			left += ((i32)(echo_history[7 - i].left) * coefficient) >> 7;
			right += ((i32)(echo_history[7 - i].right) * coefficient) >> 7;
		}

		i32 coefficient7 = fir[7];
		i32 left_final = (i32)left + (((i32)echo_history[0].left * coefficient7) >> 7);
		i32 right_final = (i32)right + (((i32)echo_history[0].right * coefficient7) >> 7);

		left_final = std::clamp(left_final, -32768, 32767);
		right_final = std::clamp(right_final, -32768, 32767);
		
		return { (i16)(left_final), (i16)(right_final) };
	}

private:
	APUBus* bus = nullptr;

	std::array<Voice, 8> voices {};
	std::array<u8, 128> registers {}; // fallback (temporary)
	std::array<i8, 8> fir {};
	
	// main volume (left and right)
	i8 mvoll = 0x00;
	i8 mvolr = 0x00;
	
	// echo volume (left and right)
	i8 evoll = 0x00;
	i8 evolr = 0x00;

	// key on, key off
	u8 kon = 0x00;
	u8 koff = 0x00;
	u8 kon_pending = 0x00;
	u8 koff_pending = 0x00;

	std::array<u8, 8> kon_delay {};
	u8 kon_delay_active = 0x00;

	u8 flg = 0xE0; // RMEN NNNN
	bool soft_reset = true;
	bool mute_all = true;
	bool echo_disable = true;
	u8 noise_frequency = 0x00;

	u8 endx = 0x00; // end of sample flag for each channel

	i8 efb = 0x00; // echo feedback

	u8 pmon = 0x00; // enable pitch modulation
	u8 non  = 0x00; // replace sample waveform with noise generator output
	u8 eon  = 0x00; // send to echo unit

	u8 dir = 0x00; // pointer to sample source directory page at $DD00
	u8 esa = 0x00; // pointer to start of echo memory region at $EE00
	u8 edl = 0x00; // echo delay time

	AudioBuffer audio_buffer;

	u16 noise_lfsr = 0x7FFF;
	u32 noise_counter = 0;
	Sample noise_sample = 0;

	u16 echo_index = 0;
	u16 echo_buffer_size = 1;
	std::array<StereoSample, 8> echo_history {};

};