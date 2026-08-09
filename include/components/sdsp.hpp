#pragma once
#include "common.hpp"
#include "stereo_sample.hpp"
#include "gauss_table.hpp"
#include "audio_buffer.hpp"
#include <cmath>
#include <numbers>

constexpr double pi = std::numbers::pi;
using Sample = int32_t;

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

	Byte read(Byte reg) {
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

	void write(Byte reg, Byte value) {
		switch (reg) {
		case 0x0: voll   = (int8_t)value; break;
		case 0x1: volr   = (int8_t)value; break;
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
			// GAIN (TO DO)
		}
	}

	void key_on(Byte dir) {
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

		return;
	}

	void release() {
		if (is_adsr) {
			// ADSR
			envelope_state = EnvelopeState::RELEASE;
		} else {
			// GAIN (to implement)
			active = false;
		}
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

	void tick();
	Byte mem_read(Word address);
	void mem_write(Word address, Byte value);

	// Sample outputting
	StereoSample output();

private:
	APUBus* bus = nullptr;

	bool pitch_mod_enabled = false;
	bool noise_enabled = false;
	bool echo_enabled = false;

	int id = 0;

	int8_t voll = 0x00;
	int8_t volr = 0x00;

	Byte pitchl =  0x00;
	Byte pitchr = 0x00;

	Byte srcn = 0x00;

	Byte adsr1 = 0x00;
	Byte adsr2 = 0x00;

	bool is_adsr = false;
	Byte decay_rate = 0x00;
	Byte attack_rate = 0x00;
	Word attack_step = 0x00;
	Word sustain_boundary = 0x00;
	Byte sustain_rate = 0x00;
	Byte release_rate = 31;

	Byte gain = 0x00; // how this is written depends on whether bit 7 is set/clear
	Byte gain_value = 0x00;
	Byte gain_mode  = 0x00;

	int envelope = 0;
	uint32_t envelope_tick = 0;
	EnvelopeState envelope_state = EnvelopeState::NONE;

	Byte envx = 0x00;
	int8_t outx =  0x00;

	bool endx_flag = false;

	// BRR decoding
	Word directory;
	Word entry;
	Word current_brr_address;
	Word next_brr_address;
	Word loop_brr_address;

	Byte brr_header = 0;
	Byte brr_position = 0;
	bool looping = false;
	bool active = false;

	// Current sample
	Sample current_sample = 0;
	Sample prev1  = 0;
	Sample prev2  = 0;

	Sample final_sample = 0;

	uint16_t sample_index  = 16;
	uint16_t nibble_index  = 16;
	uint32_t pitch_counter = 0;

	std::array<Sample, 16> decoded_samples {};

	Byte header = 0x00;
	Byte shift = 0x00;
	Byte filter = 0x00;

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
	}

	void close_audio() {
		audio_buffer.close_audio();
	}

	// For DSP registers
	void write(Byte address, Byte value) {
		// Voies registers
		if (address < 0x80 && (address & 0x0F) <= 0x09) {
			voices[(address & 0xF0) >> 4].write(address & 0xF, value);
			return;
		}

		// S-DSP registers
		if (address == 0x0C) { mvoll = (int8_t)value; }
		if (address == 0x1C) { mvolr = (int8_t)value; }
		if (address == 0x2C) { evoll = (int8_t)value; }
		if (address == 0x3C) { evolr = (int8_t)value; }
		
		if (address == 0x4C) {
			kon = value;
			kon_pending = kon_pending | value;
		}
		if (address == 0x5C) {
			koff = value;
			koff_pending = koff_pending | value;
		}
		if (address == 0x6C) {
			flg = value;
			noise_frequency = flg & 0x1F;
			echo_disable = (flg & 0x20);
			mute_all = (flg & 0x40);
			soft_reset = (flg & 0x80);
		}
		if (address == 0x7C) {
			for (auto& v : voices) {
				v.clear_endx();
			}
		}
		if (address == 0x0D) { efb = (int8_t)value; }
		
		if (address == 0x2D) {
			pmon = value;
			Byte tmp = pmon;
			for (int v = 0; v < 8; v++) {
				voices[v].pmon(tmp & 1);
				tmp = tmp >> 1;
			}
		}
		if (address == 0x3D) {
			non = value;
			Byte tmp = non;
			for (int v = 0; v < 8; v++) {
				voices[v].non(tmp & 1);
				tmp = tmp >> 1;
			}
		}
		if (address == 0x4D) {
			eon = value;
			Byte tmp = eon;
			for (int v = 0; v < 8; v++) {
				voices[v].eon(tmp & 1);
				tmp = tmp >> 1;
			}
		}
		if (address == 0x5D) { dir = value; }
		if (address == 0x6D) { esa = value; }
		if (address == 0x7D) { edl = value & 0xF; }
		if ( (address & 0xF) == 0xF && address < 0x80) {
			fir[(address & 0xF0) >> 4] = value;
		}	

		registers[address & 0x7F] = value;
	}

	Byte read(Byte address) {
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
		for (int v = 0; v < 8; v++) {
			if (kon_pending & (1 << v)) {
				kon_delay[v] = 5;
				kon_pending = kon_pending & ~(1 << v);
				continue;
			}

			if (kon_delay[v] > 0) {
				kon_delay[v]--;

				if (kon_delay[v] == 0) {
					voices[v].key_on(dir);
				}
			}
		}
	}

	void process_koff() {
		for (int v = 0; v < 8; v++) {
			if (koff_pending & (1 << v)) {
				voices[v].release();
				koff_pending = koff_pending & ~(1 << v);
			}
		}
	}

	void tick();

	// For reading/writing ARAM
	void mem_write(Word address, Byte value);
	Byte mem_read(Word address);

	bool above_half_capacity() {
		return audio_buffer.above_half_capacity();
	}

	StereoSample output();

private:
	APUBus* bus = nullptr;

	std::array<Voice, 8> voices {};
	std::array<Byte, 128> registers {}; // fallback (temporary)
	std::array<Byte, 8> fir {};
	
	// main volume (left and right)
	int8_t mvoll = 0x00;
	int8_t mvolr = 0x00;
	
	// echo volume (left and right)
	int8_t evoll = 0x00;
	int8_t evolr = 0x00;

	// key on, key off
	Byte kon = 0x00;
	Byte koff = 0x00;
	Byte kon_pending = 0x00;
	Byte koff_pending = 0x00;

	std::array<Byte, 8> kon_delay {};

	Byte flg = 0x00; // RMEN NNNN
	bool soft_reset = false;
	bool mute_all = false;
	bool echo_disable = false;
	Byte noise_frequency = 0x00;

	Byte endx = 0x00; // end of sample flag for each channel

	int8_t efb = 0x00; // echo feedback

	Byte pmon = 0x00; // enable pitch modulation
	Byte non  = 0x00; // replace sample waveform with noise generator output
	Byte eon  = 0x00; // send to echo unit

	Byte dir = 0x00; // pointer to sample source directory page at $DD00
	Byte esa = 0x00; // pointer to start of echo memory region at $EE00
	Byte edl = 0x00; // echo delay time

	AudioBuffer audio_buffer;

	// Buffer testing features

	double sine_phase = 0.0;

	uint32_t sine_phase_accum = 0;
	static constexpr uint32_t SINE_PHASE_STEP =
	    static_cast<uint32_t>((440.0 / 32000.0) * 4294967296.0); // 440Hz @ 32kHz, Q32 fixed-point

	  long dropped_pushes = 0;
};