#include "superfx.hpp"
#include "cartridge.hpp"
#include "ricoh_5a22.hpp"
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
	cycle += 6 * cycles_per_clock;
}