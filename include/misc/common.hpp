#pragma once
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <memory>
#include <vector>
#include <chrono>
#include <SDL3/SDL.h>

#include "options.hpp"
#include "utility.hpp"

using Byte = uint8_t;
using Bank = uint8_t;
using Quadrant = uint8_t;
using Word = uint16_t;
using Offset = uint16_t;
using Address = uint32_t;
using CycleCount = uint64_t;
using TickCount = uint64_t;


// SNESAddress and Quadrant allow for easier access to areas of the SNES address space
typedef struct {
	Bank bank;
	Offset offset;
} SNESAddress;

// Macros
#define RICOH_5A22_START ;
#define RICOH_5A22_END ;

inline SNESAddress split_address(Address address) {
	Bank bank = (address & 0xFF0000) >> 16;
	Offset offset = address & 0xFFFF;
	return {bank, offset};
}

inline Quadrant get_quadrant(Bank bank) {
	if (bank  <  0x40) { return 1; }
	if (bank  <  0x80) { return 2; }
	if (bank  <  0xC0) { return 3; }
	return 4;
}

inline SNESAddress to_snes_address(Address address) {
	SNESAddress result;
    result.offset = address & 0xFFFF;
    result.bank   = (address >> 16) & 0xFF;
    return result;
}

inline Byte get_lo(Word word) {
	return (word & 0xFF);
}

inline Byte get_hi(Word word) {
	return (word & 0xFF00) >> 8;
}
Byte set_bit(Byte byte, Byte bit);
Byte clear_bit(Byte byte, Byte bit);

// Timing constants
constexpr CycleCount MASTER_CLOCK = 21477272;
constexpr CycleCount RICOH_5A22_CYCLE = 6;

// WRAM constants and WRAM Access Constants
constexpr size_t WRAM_SIZE = 128 * 1024;
constexpr size_t WRAM_BANK_SIZE = 64 * 1024;

// Wait states
constexpr CycleCount WRAM_PENALTY = 2; // The CPU is ticked every 6 master cycles, to bring it to WRAM speed, which is every 8 master cycles, just wait 2 further master cycles
constexpr CycleCount EXPANSION_DATA_PENALTY = 2;
constexpr CycleCount CPU_PORTS_PENALTY = 6;

// System Area Constants (where each section of system area quadrants begin)
constexpr Offset WRAM_SECTION = 0x0000;
constexpr Offset OPEN_BUS_SECTION = 0x2000;
constexpr Offset PPU_PORTS_SECTION = 0x2100;
constexpr Offset APU_PORTS_SECTION = 0x2140;
constexpr Offset WRAM_ACCESS_SECTION = 0x2180;
constexpr Offset CPU_PORTS_SECTION = 0x4000;
constexpr Offset CPU_PORTS_NON_PENALTY_SECTION = 0x4200;
constexpr Offset CPU_DMA_PORTS_SECTION = 0x4300;
constexpr Offset CPU_DMA_PORTS_ENDING = 0x4380;
constexpr Offset EXPANSION_DATA_SECTION = 0x6000;
constexpr Offset CARTRIDGE_SECTION = 0x8000;
constexpr Offset MAX_OFFSET_SECTION = 0xFFFF;

constexpr size_t PPU_PORTS_SIZE = APU_PORTS_SECTION - PPU_PORTS_SECTION;
constexpr size_t APU_PORTS_SIZE = WRAM_ACCESS_SECTION - APU_PORTS_SECTION;
constexpr size_t WRAM_ACCESS_SIZE = CPU_PORTS_SECTION - WRAM_ACCESS_SECTION;
constexpr size_t CPU_PORTS_SIZE = CPU_DMA_PORTS_SECTION - CPU_PORTS_SECTION;
constexpr size_t CPU_DMA_PORTS_SIZE = EXPANSION_DATA_SECTION - CPU_DMA_PORTS_SECTION;
constexpr size_t EXPANSION_DATA_SIZE = CARTRIDGE_SECTION - EXPANSION_DATA_SECTION;

// SNES I/O Map

// PPU Write-Only Ports
constexpr Offset INIDISP = 0x2100;
constexpr Offset OBSEL = 0x2101;
constexpr Offset OAMADDL = 0x2102;
constexpr Offset OAMADDH = 0x2103;
constexpr Offset OAMDATA = 0x2104;
constexpr Offset BGMODE = 0x2105;
constexpr Offset MOSAIC = 0x2106;
constexpr Offset BG1SC = 0x2107;
constexpr Offset BG2SC = 0x2108;
constexpr Offset BG3SC = 0x2109;
constexpr Offset BG4SC = 0x210A;
constexpr Offset BG12NBA = 0x210B;
constexpr Offset BG34NBA = 0x210C;
constexpr Offset BG1HOFS = 0x210D;
constexpr Offset BG1VOFS = 0x210E;
constexpr Offset BG2HOFS = 0x210F;
constexpr Offset BG2VOFS = 0x2110;
constexpr Offset BG3HOFS = 0x2111;
constexpr Offset BG3VOFS = 0x2112;
constexpr Offset BG4HOFS = 0x2113;
constexpr Offset BG4VOFS = 0x2114;
constexpr Offset VMAIN = 0x2115;
constexpr Offset VMADDL = 0x2116;
constexpr Offset VMADDH = 0x2117;
constexpr Offset VMDATAL = 0x2118;
constexpr Offset VMDATAH = 0x2119;
constexpr Offset M7SEL = 0x211A;
constexpr Offset M7A = 0x211B;
constexpr Offset M7B = 0x211C;
constexpr Offset M7C = 0x211D;
constexpr Offset M7D = 0x211E;
constexpr Offset M7X = 0x211F;
constexpr Offset M7Y = 0x2120;
constexpr Offset CGADD = 0x2121;
constexpr Offset CGDATA = 0x2122;
constexpr Offset W12SEL = 0x2123;
constexpr Offset W34SEL = 0x2124;
constexpr Offset WOBJSEL = 0x2125;
constexpr Offset WH0 = 0x2126;
constexpr Offset WH1 = 0x2127;
constexpr Offset WH2 = 0x2128;
constexpr Offset WH3 = 0x2129;
constexpr Offset WBGLOG = 0x212A;
constexpr Offset WOBJLOG = 0x212B;
constexpr Offset TM = 0x212C;
constexpr Offset TS = 0x212D;
constexpr Offset TMW = 0x212E;
constexpr Offset TSW = 0x212F;
constexpr Offset CGWSEL = 0x2130;
constexpr Offset CGADSUB = 0x2131;
constexpr Offset COLDATA = 0x2132;
constexpr Offset SETINI = 0x2133;

// PPU Read-Only Ports
constexpr Offset MPYL = 0x2134;
constexpr Offset MPYM = 0x2135;
constexpr Offset MPYH = 0x2136;
constexpr Offset SLHV = 0x2137;
constexpr Offset RDOAM = 0x2138;
constexpr Offset RDVRAML = 0x2139;
constexpr Offset RDVRAMH = 0x213A;
constexpr Offset RDCGRAM = 0x213B;
constexpr Offset OPHCT = 0x213C;
constexpr Offset OPVCT = 0x213D;
constexpr Offset STAT77 = 0x213E;
constexpr Offset STAT78 = 0x213F;

// APU Ports
constexpr Offset APUI00 = 0x2140;
constexpr Offset APUI01 = 0x2141;
constexpr Offset APUI02 = 0x2142;
constexpr Offset APUI03 = 0x2143;


// WRAM Access

// CPU On-Chip Ports

// CPU Write Only Ports (Read is open bus)

// CPU Read Only Ports

// CPU DMA Ports

constexpr Offset EXPANSION_B_BUS_START = 0x2184;
constexpr Offset EXPANSION_A_BUS_START = 0x2200;