#pragma once
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <memory>
#include <vector>
#include <chrono>
#include <SDL3/SDL.h>
#include <sstream>

#include "options.hpp"
#include "utility.hpp"

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

// SNESAddress and Quadrant allow for easier access to areas of the SNES address space
typedef struct {
	u8 bank;
	u16 offset;
} SNESAddress;

// Macros
#define RICOH_5A22_START ;
#define RICOH_5A22_END ;

static std::string hex8(u8 value)
{
    std::ostringstream ss;
    ss << std::uppercase
       << std::hex
       << std::setfill('0')
       << std::setw(2)
       << static_cast<unsigned>(value);
    return ss.str();
}

static std::string hex16(u16 value)
{
    std::ostringstream ss;
    ss << std::uppercase
       << std::hex
       << std::setfill('0')
       << std::setw(4)
       << static_cast<unsigned>(value);
    return ss.str();
}

inline SNESAddress split_address(u32 address) {
	u8 bank = (address & 0xFF0000) >> 16;
	u16 offset = address & 0xFFFF;
	return {bank, offset};
}

inline u8 get_quadrant(u8 bank) {
	if (bank  <  0x40) { return 1; }
	if (bank  <  0x80) { return 2; }
	if (bank  <  0xC0) { return 3; }
	return 4;
}

inline SNESAddress to_snes_address(u32 address) {
	SNESAddress result;
    result.offset = address & 0xFFFF;
    result.bank   = (address >> 16) & 0xFF;
    return result;
}

inline u8 get_lo(u16 word) {
	return (word & 0xFF);
}

inline u8 get_hi(u16 word) {
	return (word & 0xFF00) >> 8;
}


u8 set_bit(u8 byte, u8 bit);
u8 clear_bit(u8 byte, u8 bit);

void set_lo(u16& val, u8 lo);
void set_hi(u16& val, u8 hi);
// Timing constants
constexpr i64 MASTER_CLOCK_NTSC = 21477272;
constexpr i64 MASTER_CLOCK_PAL  = 21281370;

constexpr i64 RICOH_5A22_CYCLE = 6;

// WRAM constants and WRAM Access Constants
constexpr size_t WRAM_SIZE = 128 * 1024;
constexpr size_t WRAM_BANK_SIZE = 64 * 1024;

// Wait states
constexpr i64 WRAM_PENALTY = 2; // The CPU is ticked every 6 master cycles, to bring it to WRAM speed, which is every 8 master cycles, just wait 2 further master cycles
constexpr i64 EXPANSION_DATA_PENALTY = 2;
constexpr i64 CPU_PORTS_PENALTY = 6;

// System Area Constants (where each section of system area quadrants begin)
constexpr u16 WRAM_SECTION = 0x0000;
constexpr u16 OPEN_BUS_SECTION = 0x2000;
constexpr u16 PPU_PORTS_SECTION = 0x2100;
constexpr u16 APU_PORTS_SECTION = 0x2140;
constexpr u16 WRAM_ACCESS_SECTION = 0x2180;
constexpr u16 CPU_PORTS_SECTION = 0x4000;
constexpr u16 CPU_PORTS_NON_PENALTY_SECTION = 0x4200;
constexpr u16 CPU_DMA_PORTS_SECTION = 0x4300;
constexpr u16 CPU_DMA_PORTS_ENDING = 0x4380;
constexpr u16 EXPANSION_DATA_SECTION = 0x6000;
constexpr u16 CARTRIDGE_SECTION = 0x8000;
constexpr u16 MAX_OFFSET_SECTION = 0xFFFF;

constexpr size_t PPU_PORTS_SIZE = APU_PORTS_SECTION - PPU_PORTS_SECTION;
constexpr size_t APU_PORTS_SIZE = WRAM_ACCESS_SECTION - APU_PORTS_SECTION;
constexpr size_t WRAM_ACCESS_SIZE = CPU_PORTS_SECTION - WRAM_ACCESS_SECTION;
constexpr size_t CPU_PORTS_SIZE = CPU_DMA_PORTS_SECTION - CPU_PORTS_SECTION;
constexpr size_t CPU_DMA_PORTS_SIZE = EXPANSION_DATA_SECTION - CPU_DMA_PORTS_SECTION;
constexpr size_t EXPANSION_DATA_SIZE = CARTRIDGE_SECTION - EXPANSION_DATA_SECTION;

// SNES I/O Map

// PPU Write-Only Ports
constexpr u16 INIDISP = 0x2100;
constexpr u16 OBSEL = 0x2101;
constexpr u16 OAMADDL = 0x2102;
constexpr u16 OAMADDH = 0x2103;
constexpr u16 OAMDATA = 0x2104;
constexpr u16 BGMODE = 0x2105;
constexpr u16 MOSAIC = 0x2106;
constexpr u16 BG1SC = 0x2107;
constexpr u16 BG2SC = 0x2108;
constexpr u16 BG3SC = 0x2109;
constexpr u16 BG4SC = 0x210A;
constexpr u16 BG12NBA = 0x210B;
constexpr u16 BG34NBA = 0x210C;
constexpr u16 BG1HOFS = 0x210D;
constexpr u16 BG1VOFS = 0x210E;
constexpr u16 BG2HOFS = 0x210F;
constexpr u16 BG2VOFS = 0x2110;
constexpr u16 BG3HOFS = 0x2111;
constexpr u16 BG3VOFS = 0x2112;
constexpr u16 BG4HOFS = 0x2113;
constexpr u16 BG4VOFS = 0x2114;
constexpr u16 VMAIN = 0x2115;
constexpr u16 VMADDL = 0x2116;
constexpr u16 VMADDH = 0x2117;
constexpr u16 VMDATAL = 0x2118;
constexpr u16 VMDATAH = 0x2119;
constexpr u16 M7SEL = 0x211A;
constexpr u16 M7A = 0x211B;
constexpr u16 M7B = 0x211C;
constexpr u16 M7C = 0x211D;
constexpr u16 M7D = 0x211E;
constexpr u16 M7X = 0x211F;
constexpr u16 M7Y = 0x2120;
constexpr u16 CGADD = 0x2121;
constexpr u16 CGDATA = 0x2122;
constexpr u16 W12SEL = 0x2123;
constexpr u16 W34SEL = 0x2124;
constexpr u16 WOBJSEL = 0x2125;
constexpr u16 WH0 = 0x2126;
constexpr u16 WH1 = 0x2127;
constexpr u16 WH2 = 0x2128;
constexpr u16 WH3 = 0x2129;
constexpr u16 WBGLOG = 0x212A;
constexpr u16 WOBJLOG = 0x212B;
constexpr u16 TM = 0x212C;
constexpr u16 TS = 0x212D;
constexpr u16 TMW = 0x212E;
constexpr u16 TSW = 0x212F;
constexpr u16 CGWSEL = 0x2130;
constexpr u16 CGADSUB = 0x2131;
constexpr u16 COLDATA = 0x2132;
constexpr u16 SETINI = 0x2133;

// PPU Read-Only Ports
constexpr u16 MPYL = 0x2134;
constexpr u16 MPYM = 0x2135;
constexpr u16 MPYH = 0x2136;
constexpr u16 SLHV = 0x2137;
constexpr u16 RDOAM = 0x2138;
constexpr u16 RDVRAML = 0x2139;
constexpr u16 RDVRAMH = 0x213A;
constexpr u16 RDCGRAM = 0x213B;
constexpr u16 OPHCT = 0x213C;
constexpr u16 OPVCT = 0x213D;
constexpr u16 STAT77 = 0x213E;
constexpr u16 STAT78 = 0x213F;

// APU Ports
constexpr u16 APUI00 = 0x2140;
constexpr u16 APUI01 = 0x2141;
constexpr u16 APUI02 = 0x2142;
constexpr u16 APUI03 = 0x2143;


// WRAM Access

// CPU On-Chip Ports

// CPU Write Only Ports (Read is open bus)

// CPU Read Only Ports

// CPU DMA Ports

constexpr u16 EXPANSION_B_BUS_START = 0x2184;
constexpr u16 EXPANSION_A_BUS_START = 0x2200;

// Region handling

enum class Region { NTSC, PAL };

Region region_from_header_byte(u8 code);

