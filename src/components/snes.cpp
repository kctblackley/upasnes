#include "snes.hpp"
#include <chrono>
#include <thread>
#include <iostream>
#include <algorithm>

SNES::SNES() : master_cycle(0) {
	// Create component and link to Bus

	bus = std::make_unique<Bus>();
	
	devices.push_back(std::make_unique<Ricoh5A22>(bus.get()));
	devices.push_back(std::make_unique<PPU>(bus.get()));
	
	// Do not push SPC700 as the SNES does not tick that itself (separate component)
	
	ricoh_5a22 = static_cast<Ricoh5A22*>(devices[0].get());
	ppu = static_cast<PPU*>(devices[1].get());

	spc_700 = std::make_unique<SPC700>();

	renderer = std::make_unique<Renderer>();

	ricoh_5a22->connect_renderer(renderer.get());
	ricoh_5a22->connect_ppu(ppu);
	ppu->connect_cpu(ricoh_5a22);

	bus->connect_cpu(ricoh_5a22);
	bus->connect_apu(spc_700.get());
	bus->connect_ppu(ppu);

	ppu->connect_bus(bus.get());
	ppu->connect_renderer(renderer.get());
	ppu->create_window();

	bus->set_wait_callback([this](CycleCount cycles) {
		ricoh_5a22->add_cycles(cycles);
	});
}

void SNES::load_cartridge(const std::string& directory) {
	std::cout << "LOADING CARTRIDGE!";
	bus->load_cartridge(directory, ricoh_5a22);
	initialise();
}

void SNES::tick_snes() {
	if (ricoh_5a22->get_cycle() <= ppu->get_cycle()) {
		ricoh_5a22->tick_component();
	} else {
		ppu->tick_component();
	}
	master_cycle = std::min(ricoh_5a22->get_cycle(), ppu->get_cycle());
}

void SNES::poll() {
	if (ricoh_5a22->get_cycle() == master_cycle) {
		ricoh_5a22->tick_component();
	}
}

void SNES::run() {

	bool running = true;

	auto start = std::chrono::high_resolution_clock::now();
	int seconds_to_run = 200;
	int seconds = 0;
	int total_ticks = 0;

	auto fps_timer = std::chrono::steady_clock::now();
	int fps_frames = 0;	
	int frame_count = 0;

	constexpr double TARGET_FRAME_SECONDS = 1.0 / 60.0988; // NTSC SNES refresh rate
	auto frame_start = std::chrono::steady_clock::now();

	CycleCount prev_cpu_cycle = ricoh_5a22->get_cycle();

	while (running) {
		
		tick_snes();

		CycleCount new_cpu_cycle = ricoh_5a22->get_cycle();
		CycleCount delta = new_cpu_cycle - prev_cpu_cycle;
		prev_cpu_cycle = new_cpu_cycle;
		spc_700->accumulate(delta);
		spc_700->accumulate_dsp(delta);

		if (ppu->frame_finished) {
			ppu->push_framebuffer();
			renderer->loop();
			ppu->frame_finished = false;

			if constexpr (SHOW_SDSP_LOGS) {
				static int diag_frame = 0;
				if (++diag_frame % 60 == 0) {
					std::cout << "[sdsp] ticks last frame: " << std::dec << (int)spc_700->sdsp_ticks_this_frame
					          << " (expect ~533)\n";
				}
			}
			spc_700->sdsp_ticks_this_frame = 0;
			fps_frames++;
			frame_count++;

			auto target = frame_start + std::chrono::duration<double>(TARGET_FRAME_SECONDS);
			std::this_thread::sleep_until(target);
			frame_start = std::chrono::steady_clock::now();
		}

		total_ticks += 1;

		running = renderer->running;
	}

	renderer->close_window();
	spc_700->close_audio();
	
	auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed = end - start;

    std::cout << "Time taken: " << elapsed.count() << " seconds\n";
	std::cout << std::dec << (int)(ricoh_5a22->get_tick()) << std::endl;
	std::cout << "Frames: " << (int)(frame_count) << std::endl;
	std::cout << "FPS: " << (float)(frame_count) / (float)(elapsed.count()) << std::endl;
}

void SNES::reset() {
	for (const auto& d : devices) {
		d->reset();
	}
	spc_700->reset();
}

void SNES::initialise() {
	for (const auto& d : devices) {
		d->initialise();
	}
	spc_700->initialise();
}
