#include "common.hpp"
#include <array>

// Separate file for storing structs etc. used by ppu.hpp

// OAM
#define OBJSEL_ADDRESS 0x2101
#define OAMADDL_ADDRESS 0x2102
#define OAMADDH_ADDRESS 0x2103
#define OAMDATA_ADDRESS 0x2104
#define OAMDATAREAD_ADDRESS 0x2138
#define OAM_SIZE 544

// VRAM
#define VMAIN_ADDRESS 0x2115
#define VMADDL_ADDRESS 0x2116
#define VMADDH_ADDRESS 0x2117
#define VMDATAL_ADDRESS 0x2118
#define VMDATAH_ADDRESS 0x2119
#define VMDATALREAD_ADDRESS 0x2139
#define VMDATAHREAD_ADDRESS 0x213A
#define VRAM_SIZE 65536
#define VRAM_WORD_SIZE 32768

// CGRAM
#define CGADD_ADDRESS 0x2121
#define CGDATA_ADDRESS 0x2122
#define CGDATAREAD_ADDRESS 0x213b
#define CGRAM_SIZE 256

// Display Configuration Registers
#define INIDISP_ADDRESS 0x2100
#define BGMODE_ADDRESS 0x2105
#define MOSAIC_ADDRESS 0x2106

#define BG1SC_ADDRESS 0x2107
#define BG2SC_ADDRESS 0x2108
#define BG3SC_ADDRESS 0x2109
#define BG4SC_ADDRESS 0x210A

#define BG12NBA_ADDRESS 0x210B
#define BG34NBA_ADDRESS 0x210C

#define BG1HOFS_ADDRESS 0x210D
#define BG1VOFS_ADDRESS 0x210E
#define BG2HOFS_ADDRESS 0x210F
#define BG2VOFS_ADDRESS 0x2110
#define BG3HOFS_ADDRESS 0x2111
#define BG3VOFS_ADDRESS 0x2112
#define BG4HOFS_ADDRESS 0x2113
#define BG4VOFS_ADDRESS 0x2114

#define TM_ADDRESS 0x212C
#define TS_ADDRESS 0x212D
#define SETINI_ADDRESS 0x2133

// Mode 7 Registers
#define M7SEL_ADDRESS 0x211A
#define M7HOFS_ADDRESS 0x210D
#define M7VOFS_ADDRESS 0x210E
#define M7A_ADDRESS 0x211B
#define M7B_ADDRESS 0x211C
#define M7C_ADDRESS 0x211D
#define M7D_ADDRESS 0x211E
#define M7X_ADDRESS 0x211F
#define M7Y_ADDRESS 0x2120

// Windows Registers
#define W12SEL_ADDRESS 0x2123
#define W34SEL_ADDRESS 0x2124
#define WOBJSEL_ADDRESS 0x2125
#define WH0_ADDRESS 0x2126
#define WH1_ADDRESS 0x2127
#define WH2_ADDRESS 0x2128
#define WH3_ADDRESS 0x2129
#define WBGLOG_ADDRESS 0x212A
#define WOBJLOG_ADDRESS 0x212B
#define TMW_ADDRESS 0x212E
#define TSW_ADDRESS 0x212F

// Color Math Registers
#define CGWSEL_ADDRESS 0x2130
#define CGADSUB_ADDRESS 0x2131
#define COLDATA_ADDRESS 0x2132

// Multiplication Registers
#define MPYL_ADDRESS 0x2134
#define MPYM_ADDRESS 0x2135
#define MPYH_ADDRESS 0x2136

// H/V counters
#define SLHV_ADDRESS 0x2137
#define OPHCT_ADDRESS 0x213C
#define OPVCT_ADDRESS 0x213D

// Status
#define STAT77_ADDRESS 0x213E
#define STAT78_ADDRESS 0x213F

static int16_t signed_13(uint16_t value) {
	value = value & 0x1FFF;

	if (value & 0x1000) {
		value = value | 0xE000;
	}

	return static_cast<int16_t>(value);
}

struct Priority {
	uint8_t S0, S1, S2, S3; // sprite priority
	uint8_t L1, L2, L3, L4; // low BG priority
	uint8_t H1, H2, H3, H4; // high BG priority
};

struct Pixel {
	bool transparent = false;
	uint8_t priority = 0x00;
	uint16_t colour = 0x00;
	uint8_t layer = 0;
	bool colour_math = false;
};

struct Object {
	int16_t x_coordinate = 0;
	Word y_coordinate = 0;
	Word tile_number = 0;
	Word attributes = 0;

	Byte priority_number = 0;
	Byte palette = 0;
	Byte priority = 0;

	bool horizontal_flip = false;
	bool vertical_flip = false;

	int width = 0;
	int height = 0;

	int render_width = 0;
	int render_height = 0;
	int line_in_sprite = 0;

	bool size = false;
};

struct SizePair {
	int small_width, small_height;
	int large_width, large_height;
};

struct OAM {
	// Internal registers
	uint16_t oamadd;
	uint16_t reload;
	Byte latch;

	uint32_t first_base;
	uint32_t second_base;
	SizePair obj_size;

	bool priority_rotation;

	// OAM data
	std::array<Byte, OAM_SIZE> data {};
};

struct CGRAM {
	std::array<Word, CGRAM_SIZE> data {};
	Byte cgram_address;
	Byte cgram_latch;
	bool cgram_byte = false;
};

struct VRAM {
	std::array<Word, VRAM_WORD_SIZE> data {};

	Byte address_increment; // Docs provide words, but will need to increment by bytes
	Byte address_remapping;
	bool address_increment_mode;
	
	Word vram_latch;
	Word vmadd;
};

static constexpr SizePair size_table[8] = {
    { 8,  8, 16, 16},
    { 8,  8, 32, 32},
    { 8,  8, 64, 64},
    {16, 16, 32, 32},
    {16, 16, 64, 64},
    {32, 32, 64, 64},
    {16, 32, 32, 64},
    {16, 32, 32, 32},
};

struct BG {
	bool character_size = false;
	bool mosaic = false;

	bool horizontal_tilemap_count = false;
	bool vertical_tilemap_count = false;

	Address tilemap_vram_address = 0x00;
	Address word_address = 0x00;

	Word bghofs = 0x00;
	Word bgvofs = 0x00;

	// Per-background low-byte latch for HOFS bits 0-2 (see PPU::set_bghofs)
	Byte hofs_latch = 0x00;

	// Below enable BG on the screens (must be enabled on at least one to be visible)
	bool sub_screen = false;
	bool main_screen = false;

	bool windows_on_subscreen = false;
	bool windows_on_main_screen = false;

	bool window1_inverted = false;
	bool window1_enabled = false;
	bool window2_inverted = false;;
	bool window2_enabled = false;

	Byte bpp = 2;

	Byte mask_logic = 0x00;

	bool enable_colour_math = false;

	std::array<Pixel, 512> scanline;
	std::array<Pixel, 512> main_scanline;
	std::array<Pixel, 512> sub_scanline;
	std::vector<uint32_t> framebuffer;

	int layer;
};

struct ObjectLayer {
	bool sub_screen = false;
	bool main_screen = false;

	bool windows_on_subscreen = false;
	bool windows_on_main_screen = false;

	bool window1_inverted = false;
	bool window1_enabled = false;
	bool window2_inverted = false;;
	bool window2_enabled = false;

	Byte mask_logic = 0x00;

	bool enable_colour_math = false;

	std::array<Pixel, 512> scanline;
	std::array<Pixel, 512> main_scanline;
	std::array<Pixel, 512> sub_scanline;
	std::vector<uint32_t> framebuffer;

	int layer = 0;
};

struct ColorMathLayer {
	bool window1_inverted = false;
	bool window1_enabled = false;
	bool window2_inverted = false;;
	bool window2_enabled = false;

	Byte mask_logic = 0x00;

	bool direct_colour_mode = false; // Relevant for 8bpp! NEED TO ADD THIS STILL!
	bool addend = false; // 0 = fixed colour, 1 = subscreen

	bool backdrop_colour_math_enabled = false;
	bool half_colour_math = false;

	Byte sub_screen_transparent_region = 0x00;
	Byte main_screen_black_region = 0x00;

	// Fixed colour
	Byte red = 0x00;
	Byte green = 0x00;
	Byte blue = 0x00;

	bool operator_type = false; // 0 = add, 1 = subtract
};

struct Window {
	Byte left_position = 0x00;
	Byte right_position = 0x00;
};

struct Mode7 {
	bool horizontal_flip = false;
	bool vertical_flip = false;
	bool non_tilemap_fill = false;
	bool tilemap_repeat = false;

	int16_t m7hofs = 0x00;
	int16_t m7vofs = 0x00;
	Byte latch = 0x00;

	uint32_t mpy = 0x00;
	int8_t last_m7b = 0x00;

	int16_t m7a = 0x00;
	int16_t m7b = 0x00;
	int16_t m7c = 0x00;
	int16_t m7d = 0x00;
	int16_t m7x = 0x00;
	int16_t m7y = 0x00;
};

// BG fetching optimisation structs

struct DecodedRow {
	bool valid = false;
	std::array<Byte, 8> data {}; // Important note to self: this contains colour indices, not actual colours!
};

struct DecodedTile {
	bool valid = false;
	std::array<DecodedRow, 8> rows {};
};