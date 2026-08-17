#pragma once
#include "component.hpp"
#include "ppu_data.hpp"
#include "renderer.hpp"

#define RDNMI_ADDRESS 0x4210
#define TIMEUP_ADDRESS 0x4211

#define XCOORD_BYTE 0
#define YCOORD_BYTE 1
#define TILE_NUMBER_BYTE 2
#define ATTRIBUTE_BYTE 3

#define MAX_OBJECTS 32

// For tile caching...
#define NUM_BPP 3
#define MAX_TILES 4096

class Ricoh5A22;
class Bus;
class DMA;

constexpr Byte PPU1_VERSION = 1;
constexpr Byte PPU2_VERSION = 2;

constexpr int screen_width = 512;
constexpr int screen_height = 448;
constexpr int framebuffer_height = 480;

constexpr int no_overscan_vcount = 224;
constexpr int overscan_vcount = 240;

constexpr std::array<Priority, 8> initialise_priorities() {
	Priority mode25; // modes 2 to 5 use same priority order
	mode25.L2 = 1; mode25.S0 = 2;
	mode25.L1 = 3; mode25.S1 = 4;
	mode25.H2 = 5; mode25.S2 = 6;
	mode25.H1 = 7; mode25.S3 = 8;

	Priority mode6 = mode25;
	mode6.H2 = 0; mode6.L2 = 0;

	Priority mode7 = mode25;
	mode7.H1 = 0;

	Priority mode0;
	mode0.L4 =  1; mode0.L3 =  2;
	mode0.S0 =  3; mode0.H4 =  4;
	mode0.H3 =  5; mode0.S1 =  6;
	mode0.L2 =  7; mode0.L1 =  8;
	mode0.S2 =  9; mode0.H2 = 10;
	mode0.H1 = 11; mode0.S3 = 12;
 
	Priority mode1 = mode0;
	mode1.H4 = 0; mode1.L4 = 0;
	// Also, H3 can be placed at front!

	std::array<Priority, 8> priorities = {mode0, mode1, mode25, mode25, mode25, mode25, mode6, mode7};
	return priorities;
}

class PPU : public Component {
public:
	explicit PPU(Bus* bus) {
		this->bus = bus;
		initialise_bg(bg1, 1);
		initialise_bg(bg2, 2);
		initialise_bg(bg3, 3);
		initialise_bg(bg4, 4);
		initialise_obj(obj);

		object_buffer.reserve(MAX_OBJECTS);

		oam_view_framebuffer.assign(oam_view_width * oam_view_height, 0x000000FF);
	}

	void initialise_bg(BG& bg, int layer) {
		bg.framebuffer.assign(screen_width * framebuffer_height, 0x000000FF);
		bg.layer = layer;
	}

	void initialise_obj(ObjectLayer& obj) {
		obj.framebuffer.assign(screen_width * framebuffer_height, 0x000000FF);
		obj.layer = 0;
	}

	void add_cycles(CycleCount cycles) override {
		this->cycle += cycles;
	};

	// For interrupts
	Word h_time_target = 0;
	Word v_time_target = 0;
	Byte irq_mode = 0;
	bool irq_condition_met = false;
	bool frame_finished = false;

	void call_irq();
	void call_nmi();

	void enter_hblank();
	void leave_hblank();
	void enter_vblank();
	void leave_vblank();
	bool frame_ended();
	void next_frame();
	void update_hblank();
	void update_vblank();
	void end_scanline();
	void tick_component() override;

	void create_window() {
		renderer->create_window(screen_width, screen_height);
		framebuffer.assign(screen_width * framebuffer_height, 0x000000FF);
	}

	bool should_resolve(bool is_window, int value);
	bool is_colour_math_window(int x);
	bool resolve_main_screen_px(Pixel& px, bool is_window);
	void resolve_sub_screen_px(Pixel& px, bool is_window);
	Pixel colour_math(Pixel main, Pixel sub, bool ignore_half);

	void window_mask(std::array<Pixel, 512>& scanline, bool window1_enabled, bool window2_enabled, bool window1_inverted, bool window2_inverted, Byte mask_logic, bool colour_math);
	void composite(std::array<Pixel, 512>& final_scanline);
	Pixel fetch_bg_pixel(BG& bg, uint16_t screen_x);
	Pixel fetch_mode7_pixel(BG& bg, uint16_t screen_x);
	void fetch_objects();
	void push_pixel(BG& bg, const Pixel& px, int& dot);
	void render_bg_scanline(BG& bg);
	void render_obj_scanline(ObjectLayer& obj);
	void render_scanline();
	void render_oam_view();

	void clear_framebuffer(std::vector<uint32_t>& f);
	void add_to_framebuffer(std::vector<uint32_t>& f, std::array<Pixel, 512>& line);
	uint32_t convert_to_rgba(uint16_t colour);

	void push_framebuffer() {
		renderer->display_framebuffer(this->framebuffer);
		if constexpr (DEBUG_WINDOW) {
			renderer->display_separate_framebuffers(this->bg1.framebuffer, this->bg2.framebuffer, this->bg3.framebuffer, this->bg4.framebuffer, this->obj.framebuffer);
			render_oam_view();
			renderer->display_oam_view(this->oam_view_framebuffer);
		}
	}

	static constexpr int oam_cell_size = 64;
	static constexpr int oam_view_width  = 16 * oam_cell_size;
	static constexpr int oam_view_height = 8  * oam_cell_size;

	CycleCount get_cycle() override {
		return cycle;
	}

	TickCount get_tick() override {
		return tick;
	}

	void reset() override {
		hcounter = 0;
		vcounter = 0;
		vblank = false;
		hblank = false;

		forced_blank = true;
		brightness = 0;
		field = false;
		counter_latch = false;
		bg_mode = 0;
		bg3_priority = false;

		bg_ofs_latch = 0;
		vram.vmadd = 0;
		vram.address_increment = 1;
		vram.address_remapping = 0;

		oam.oamadd = 0;
		oam.reload = 0;

		cgram.cgram_address = 0;
		cgram.cgram_byte = 0;
	}

	void initialise() override {
		reset();
	}

	Byte read(Address addr) override {
		return 0xFF;
	}

	void write(Address addr, Byte value) override {
		return;
	}

	bool oam_accessible() {
		return vblank || forced_blank;
	}

	bool cgram_accessible() {
		return vblank || hblank || forced_blank;
	}

	bool vram_accessible() {
		return vblank || forced_blank;
	}

	Word remap_vmadd(Word vmadd) {
		switch (vram.address_remapping) {
		case 0:
			return vmadd;
		case 1:
			return (vmadd & ~0xFF) | ((vmadd & 0x1F) << 3) | ((vmadd >> 5) & 0x07);
		case 2:
			return (vmadd & ~0x1FF) | ((vmadd & 0x3F) << 3) | ((vmadd >> 6) & 0x07);
		case 3:
			return (vmadd & ~0x3FF) | ((vmadd & 0x7F) << 3) | ((vmadd >> 7) & 0x07);
		}
		return vmadd;
	}

	void update_object(int address, Byte value) {
		if (address < 512) {
			Object& o = all_objects[(int)(address / 4)];
			int signed_x;

			switch (address & 3) {
			case XCOORD_BYTE:
				o.x_coordinate = value;
				signed_x = o.x_coordinate;
				if (signed_x > 255) {
					signed_x -= 512;
				}
				o.x_coordinate = signed_x;
				break;

			case YCOORD_BYTE:
				o.y_coordinate = value;
				break;

			case TILE_NUMBER_BYTE:
				o.tile_number = ((get_hi(o.tile_number) & 1) << 8) | (uint8_t)value;
				break;

			case ATTRIBUTE_BYTE:
				o.attributes = value;
				o.tile_number = ((o.attributes & 1) << 8) | get_lo(o.tile_number);

				o.palette = (o.attributes >> 1) & 7;
				o.priority_number = (o.attributes >> 4) & 3;

				o.horizontal_flip = (o.attributes >> 6) & 1;
				o.vertical_flip = (o.attributes >> 7) & 1;
				
				break;

			default:

				break;
			}

		} else {
			int start = 4 * (address - 512);
			for (int i = start; i < start + 4; i++) {
				Object& o = all_objects[i];

				Byte pair = (value >> (2 * (i & 3))) & 3;

				o.x_coordinate = ((pair & 1) << 8) | get_lo(o.x_coordinate);
				int signed_x = o.x_coordinate;
				if (signed_x > 255) {
					signed_x -= 512;
				}
				o.x_coordinate = signed_x;

				bool size = (pair >> 1) & 1;
				o.size = size;
			}
		}
	}

	void update_priority() {
		priority_order = priorities[bg_mode];
		if (bg_mode == 1 && bg3_priority) {
			priority_order.H3 = 20; // just a random value for now, will change later
		}

		if (bg_mode == 0) {
			bg1.bpp = 2;
			bg2.bpp = 2;
			bg3.bpp = 2;
			bg4.bpp = 2;		
		}
		if (bg_mode == 1) {
			bg1.bpp = 4;
			bg2.bpp = 4;
			bg3.bpp = 2;
			bg4.bpp = 2;		
		}
		if (bg_mode == 2) { // offset per tile
			bg1.bpp = 4;
			bg2.bpp = 4;
		}
		if (bg_mode == 3) {
			bg1.bpp = 8;
			bg2.bpp = 4;
		}
		if (bg_mode == 4) { // offset per tile
			bg1.bpp = 8;
			bg2.bpp = 2;
		}
		if (bg_mode == 5) {
			bg1.bpp = 4;
			bg2.bpp = 2;
		}
		if (bg_mode == 6) { // offset per tile
			bg1.bpp = 4;
		}
		if (bg_mode == 7) {
			bg1.bpp = 8;
		}
	}

	// PPU tile caching

	void invalidate_tile(Word tile_address) {
		for (int i = 3; i <= 5; i++) { // shifts for each of the bpp
			int index = tile_address >> i;
			int bpp = i - 3;
			DecodedTile& tile = tile_cache[bpp][index];
			if (tile.valid) {
				for (auto& r : tile.rows) {
					r.valid = false;
				}
				tile.valid = false;
			}
		}
	}

	// bpp is provided as 2, 4, 8, but changed to 0, 1, 2
	DecodedRow* get_tile_row(Word tile_address, int row, int bpp_num) {
		int bpp = (bpp_num >> 2);
		int index = (tile_address >> (bpp + 3));
		DecodedTile& tile = tile_cache[bpp][index];
		if (!tile.rows[row].valid) {
			decode_tile_row(tile_address, row, bpp);
			tile.valid = true;
		}
		return &tile.rows[row];
	}

	// bpp is provided as 0, 1, 2 -> just gets the row and avoids decoding
	DecodedRow* fetch_tile_row(Word tile_address, int row, int bpp) {
		int index = (tile_address >> (bpp + 3));
		DecodedTile& tile = tile_cache[bpp][index];
		return &tile.rows[row];
	}

	// bpp is provided as 0, 1, 2
	void decode_2bpp(Word tile_address, int row) {
		DecodedRow* row_data = fetch_tile_row(tile_address, row, 0);
		Word plane01 = vram.data[(tile_address +      row) & 0x7FFF];
		Byte p0 = get_lo(plane01);
		Byte p1 = get_hi(plane01);
		for (int px = 0; px < 8; px++) {
			int bit = 7 - px;
			Byte colour = ((p0 >> bit) & 1) | (((p1 >> bit) & 1) << 1);
			row_data->data[px] = colour;
		}
		row_data->valid = true;
	}

	void decode_4bpp(Word tile_address, int row) {
		DecodedRow* row_data = fetch_tile_row(tile_address, row, 1);
		Word plane01 = vram.data[(tile_address +      row) & 0x7FFF];
		Word plane23 = vram.data[(tile_address + 8  + row) & 0x7FFF];
		Byte p0 = get_lo(plane01);
		Byte p1 = get_hi(plane01);
		Byte p2 = get_lo(plane23);
		Byte p3 = get_hi(plane23);
		for (int px = 0; px < 8; px++) {
			int bit = 7 - px;
			Byte colour = ((p0 >> bit) & 1)       | (((p1 >> bit) & 1) << 1) |
				         (((p2 >> bit) & 1) << 2) | (((p3 >> bit) & 1) << 3);
			row_data->data[px] = colour;
		}
		row_data->valid = true;
	}

	void decode_8bpp(Word tile_address, int row) {
		DecodedRow* row_data = fetch_tile_row(tile_address, row, 2);
		Word plane01 = vram.data[(tile_address +      row) & 0x7FFF];
		Word plane23 = vram.data[(tile_address + 8  + row) & 0x7FFF];
		Word plane45 = vram.data[(tile_address + 16 + row) & 0x7FFF];
		Word plane67 = vram.data[(tile_address + 24 + row) & 0x7FFF];
		Byte p0 = get_lo(plane01);
		Byte p1 = get_hi(plane01);
		Byte p2 = get_lo(plane23);
		Byte p3 = get_hi(plane23);
		Byte p4 = get_lo(plane45);
		Byte p5 = get_hi(plane45);
		Byte p6 = get_lo(plane67);
		Byte p7 = get_hi(plane67);
		for (int px = 0; px < 8; px++) {
			int bit = 7 - px;
			Byte colour = ((p0 >> bit) & 1)       | (((p1 >> bit) & 1) << 1) |
						 (((p2 >> bit) & 1) << 2) | (((p3 >> bit) & 1) << 3) |
						 (((p4 >> bit) & 1) << 4) | (((p5 >> bit) & 1) << 5) |
						 (((p6 >> bit) & 1) << 6) | (((p7 >> bit) & 1) << 7);	
			row_data->data[px] = colour;
		}
		row_data->valid = true;
	}

	// bpp is provided as 0, 1, 2
	void decode_tile_row(Word tile_address, int row, int bpp) {
		switch (bpp) {
		case 0: decode_2bpp(tile_address, row); break;
		case 1: decode_4bpp(tile_address, row); break;
		case 2: decode_8bpp(tile_address, row); break;
		}
	}

	// PPU registers go here
	Byte communication_read(SNESAddress addr) override;

	void set_bgsc(BG& bg, Byte value) {
		bg.horizontal_tilemap_count = value & 1;
		bg.vertical_tilemap_count = (value >> 1) & 1;
		bg.tilemap_vram_address = ((value >> 2) & 0x3F) << 10;
	}

	void set_chr_word_base(BG& bg, Byte value) {
		bg.word_address = (value << 12);
	}

	void set_bghofs(BG& bg, Byte value) {
		bg.bghofs = (value << 8) | (bg_ofs_latch & ~7) | (bg.hofs_latch & 7);
		bg_ofs_latch = value;
		bg.hofs_latch = value;
	}

	void set_bgvofs(BG& bg, Byte value) {
		bg.bgvofs = (value << 8) | bg_ofs_latch;
		bg_ofs_latch = value;
	}

	void communication_write(SNESAddress addr, Byte value) override {

		if constexpr (LOG_PPU_REGISTER_WRITES) {
			const char* reg_name = nullptr;

			switch (addr.offset) {
				// Display
				case INIDISP_ADDRESS:  reg_name = "INIDISP"; break;
				case OBJSEL_ADDRESS:   reg_name = "OBJSEL"; break;
				case BGMODE_ADDRESS:   reg_name = "BGMODE"; break;
				case MOSAIC_ADDRESS:   reg_name = "MOSAIC"; break;
				case BG1SC_ADDRESS:    reg_name = "BG1SC"; break;
				case BG2SC_ADDRESS:    reg_name = "BG2SC"; break;
				case BG3SC_ADDRESS:    reg_name = "BG3SC"; break;
				case BG4SC_ADDRESS:    reg_name = "BG4SC"; break;
				case BG12NBA_ADDRESS:  reg_name = "BG12NBA"; break;
				case BG34NBA_ADDRESS:  reg_name = "BG34NBA"; break;

				// BG offsets
				case BG1HOFS_ADDRESS:  if (bg_mode == 7) { reg_name = "M7HOFS"; } else { reg_name = "BG1HOFS"; } break;
				case BG1VOFS_ADDRESS:  if (bg_mode == 7) { reg_name = "M7VOFS"; } else { reg_name = "BG1VOFS"; }  break;
				case BG2HOFS_ADDRESS:  reg_name = "BG2HOFS"; break;
				case BG2VOFS_ADDRESS:  reg_name = "BG2VOFS"; break;
				case BG3HOFS_ADDRESS:  reg_name = "BG3HOFS"; break;
				case BG3VOFS_ADDRESS:  reg_name = "BG3VOFS"; break;
				case BG4HOFS_ADDRESS:  reg_name = "BG4HOFS"; break;
				case BG4VOFS_ADDRESS:  reg_name = "BG4VOFS"; break;

				// Screen designation
				case TM_ADDRESS:       reg_name = "TM"; break;
				case TS_ADDRESS:       reg_name = "TS"; break;
				case SETINI_ADDRESS:   reg_name = "SETINI"; break;

				// Mode 7
				case M7SEL_ADDRESS:    reg_name = "M7SEL"; break;
				case M7A_ADDRESS:      reg_name = "M7A"; break;
				case M7B_ADDRESS:      reg_name = "M7B"; break;
				case M7C_ADDRESS:      reg_name = "M7C"; break;
				case M7D_ADDRESS:      reg_name = "M7D"; break;
				case M7X_ADDRESS:      reg_name = "M7X"; break;
				case M7Y_ADDRESS:      reg_name = "M7Y"; break;

				// Windows
				case W12SEL_ADDRESS:   reg_name = "W12SEL"; break;
				case W34SEL_ADDRESS:   reg_name = "W34SEL"; break;
				case WOBJSEL_ADDRESS:  reg_name = "WOBJSEL"; break;
				case WH0_ADDRESS:      reg_name = "WH0"; break;
				case WH1_ADDRESS:      reg_name = "WH1"; break;
				case WH2_ADDRESS:      reg_name = "WH2"; break;
				case WH3_ADDRESS:      reg_name = "WH3"; break;
				case WBGLOG_ADDRESS:   reg_name = "WBGLOG"; break;
				case WOBJLOG_ADDRESS:  reg_name = "WOBJLOG"; break;
				case TMW_ADDRESS:      reg_name = "TMW"; break;
				case TSW_ADDRESS:      reg_name = "TSW"; break;

				// Colour math
				case CGWSEL_ADDRESS:   reg_name = "CGWSEL"; break;
				case CGADSUB_ADDRESS:  reg_name = "CGADSUB"; break;
				case COLDATA_ADDRESS:  reg_name = "COLDATA"; break;

				// OAM
				case OAMADDL_ADDRESS:  reg_name = "OAMADDL"; break;
				case OAMADDH_ADDRESS:  reg_name = "OAMADDH"; break;
				case OAMDATA_ADDRESS:  reg_name = "OAMDATA"; break;

				// CGRAM
				case CGADD_ADDRESS:    reg_name = "CGADD"; break;
				case CGDATA_ADDRESS:   reg_name = "CGDATA"; break;

				// VRAM
				case VMAIN_ADDRESS:    reg_name = "VMAIN"; break;
				case VMADDL_ADDRESS:   reg_name = "VMADDL"; break;
				case VMADDH_ADDRESS:   reg_name = "VMADDH"; break;
				case VMDATAL_ADDRESS:  reg_name = "VMDATAL"; break;
				case VMDATAH_ADDRESS:  reg_name = "VMDATAH"; break;

				default:
					break;
			}

			if (reg_name != nullptr) {
				std::cout
					<< "[PPU WRITE] "
					<< std::left << std::setw(8) << reg_name
					<< " <= 0x"
					<< std::uppercase << std::hex
					<< std::setfill('0') << std::setw(2)
					<< static_cast<unsigned>(value)
					<< std::dec << std::setfill(' ')
					<< '\n';
			}
		}

		// Display configuration
		if (addr.offset == INIDISP_ADDRESS) {
			bool new_forced_blank = ((value >> 7) & 0b1) == 1;

			if (forced_blank && !new_forced_blank && vblank && vcounter == vblank_start) {
				oam.oamadd = oam.reload << 1;
			}

			forced_blank = new_forced_blank;
			brightness = value & 0x0F;
		}

		if (addr.offset == BGMODE_ADDRESS) {
			bg_mode = value & 7;
			bg3_priority = (value >> 3) & 1;

			bg1.character_size = (value >> 4) & 1;
			bg2.character_size = (value >> 5) & 1;
			bg3.character_size = (value >> 6) & 1;
			bg4.character_size = (value >> 7) & 1;

			//std::cout << "CHANGED BG MODE TO "
			         // << std::dec << (int)(bg_mode) << "\n";

			update_hires();
			update_priority();
		}

		if (addr.offset == MOSAIC_ADDRESS) {
			mosaic_size = (value >> 4) & 0xF;

			if (value & 1) {
				bg1.mosaic = true;
			} else {
				bg1.mosaic = false;
			}

			if (value & 2) {
				bg2.mosaic = true;
			} else {
				bg2.mosaic = false;
			}

			if (value & 4) {
				bg3.mosaic = true;
			} else {
				bg3.mosaic = false;
			}

			if (value & 8) {
				bg4.mosaic = true;
			} else {
				bg4.mosaic = false;
			}
		}

		if (addr.offset == BG12NBA_ADDRESS) {
			set_chr_word_base(bg1, (value >> 0) & 0xF);
			set_chr_word_base(bg2, (value >> 4) & 0xF);
		}

		if (addr.offset == BG34NBA_ADDRESS) {
			set_chr_word_base(bg3, (value >> 0) & 0xF);
			set_chr_word_base(bg4, (value >> 4) & 0xF);
		}

		if (addr.offset == BG1HOFS_ADDRESS) {
			set_bghofs(bg1, value);
		}

		if (addr.offset == BG2HOFS_ADDRESS) {
			set_bghofs(bg2, value);
		}

		if (addr.offset == BG3HOFS_ADDRESS) {
			set_bghofs(bg3, value);
		}

		if (addr.offset == BG4HOFS_ADDRESS) {
			set_bghofs(bg4, value);
		}

		if (addr.offset == BG1VOFS_ADDRESS) {
			set_bgvofs(bg1, value);
		}

		if (addr.offset == BG2VOFS_ADDRESS) {
			set_bgvofs(bg2, value);
		}

		if (addr.offset == BG3VOFS_ADDRESS) {
			set_bgvofs(bg3, value);
		}

		if (addr.offset == BG4VOFS_ADDRESS) {
			set_bgvofs(bg4, value);
		}

		if (addr.offset == BG1SC_ADDRESS) {
			set_bgsc(bg1, value);
		}

		if (addr.offset == BG2SC_ADDRESS) {
			set_bgsc(bg2, value);
		}

		if (addr.offset == BG3SC_ADDRESS) {
			set_bgsc(bg3, value);
		}

		if (addr.offset == BG4SC_ADDRESS) {
			set_bgsc(bg4, value);
		}

		if (addr.offset == TM_ADDRESS) {
			bg1.main_screen = (value >> 0) & 1;
			bg2.main_screen = (value >> 1) & 1;
			bg3.main_screen = (value >> 2) & 1;
			bg4.main_screen = (value >> 3) & 1;
			obj.main_screen = (value >> 4) & 1;
		}

		if (addr.offset == TS_ADDRESS) {
			bg1.sub_screen = (value >> 0) & 1;
			bg2.sub_screen = (value >> 1) & 1;
			bg3.sub_screen = (value >> 2) & 1;
			bg4.sub_screen = (value >> 3) & 1;
			obj.sub_screen = (value >> 4) & 1;
		}

		if (addr.offset == SETINI_ADDRESS) {
			screen_interlacing = (value >> 0) & 1;
			obj_interlacing = (value >> 1) & 1;
			overscan_mode = (value >> 2) & 1;
			pseudo_hires_mode = (value >> 3) & 1;

			extbg_mode = (value >> 6) & 1;
			external_sync = (value >> 7) & 1;

			update_hires();
		}

		// Mode 7

		if (addr.offset == M7SEL_ADDRESS) {
			mode7.horizontal_flip = (value & 1);
			mode7.vertical_flip = (value >> 1) & 1;
			mode7.non_tilemap_fill = (value >> 6) & 1;
			mode7.tilemap_repeat = (value >> 7) & 1;
		}

		if (addr.offset == M7HOFS_ADDRESS) {
			uint16_t val = (value << 8) | mode7.latch;
			mode7.m7hofs = signed_13(val);
			mode7.latch = value;
		}

		if (addr.offset == M7VOFS_ADDRESS) {
			uint16_t val = (value << 8) | mode7.latch;
			mode7.m7vofs = signed_13(val);
			mode7.latch = value;
		}

		if (addr.offset == M7A_ADDRESS) {
			mode7.m7a = (value << 8) | mode7.latch;
			mode7.latch = value;
		}

		if (addr.offset == M7B_ADDRESS) {
			mode7.m7b = (value << 8) | mode7.latch;
			mode7.latch = value;
			mode7.last_m7b = value;
		}

		if (addr.offset == M7C_ADDRESS) {
			mode7.m7c = (value << 8) | mode7.latch;
			mode7.latch = value;
		}

		if (addr.offset == M7D_ADDRESS) {
			mode7.m7d = (value << 8) | mode7.latch;
			mode7.latch = value;
		}

		if (addr.offset == M7X_ADDRESS) {
			uint16_t val = (value << 8) | mode7.latch;
			mode7.m7x = signed_13(val);
			mode7.latch = value;
		}

		if (addr.offset == M7Y_ADDRESS) {
			uint16_t val = (value << 8) | mode7.latch;
			mode7.m7y = signed_13(val);
			mode7.latch = value;
		}

		// Windows

		if (addr.offset == W12SEL_ADDRESS) {
			bg1.window1_inverted = (value >> 0) & 1;
			bg1.window1_enabled  = (value >> 1) & 1;
			bg1.window2_inverted = (value >> 2) & 1;
			bg1.window2_enabled  = (value >> 3) & 1;

			bg2.window1_inverted = (value >> 4) & 1;
			bg2.window1_enabled  = (value >> 5) & 1;
			bg2.window2_inverted = (value >> 6) & 1;
			bg2.window2_enabled  = (value >> 7) & 1;
		}

		if (addr.offset == W34SEL_ADDRESS) {
			bg3.window1_inverted = (value >> 0) & 1;
			bg3.window1_enabled  = (value >> 1) & 1;
			bg3.window2_inverted = (value >> 2) & 1;
			bg3.window2_enabled  = (value >> 3) & 1;

			bg4.window1_inverted = (value >> 4) & 1;
			bg4.window1_enabled  = (value >> 5) & 1;
			bg4.window2_inverted = (value >> 6) & 1;
			bg4.window2_enabled  = (value >> 7) & 1;
		}

		if (addr.offset == WOBJSEL_ADDRESS) {
			obj.window1_inverted = (value >> 0) & 1;
			obj.window1_enabled  = (value >> 1) & 1;
			obj.window2_inverted = (value >> 2) & 1;
			obj.window2_enabled  = (value >> 3) & 1;

			col.window1_inverted = (value >> 4) & 1;
			col.window1_enabled  = (value >> 5) & 1;
			col.window2_inverted = (value >> 6) & 1;
			col.window2_enabled  = (value >> 7) & 1;
		}

		if (addr.offset == WH0_ADDRESS) {
			window1.left_position = value;
		}

		if (addr.offset == WH1_ADDRESS) {
			window1.right_position = value;
		}

		if (addr.offset == WH2_ADDRESS) {
			window2.left_position = value;
		}

		if (addr.offset == WH3_ADDRESS) {
			window2.right_position = value;
		}

		if (addr.offset == WBGLOG_ADDRESS) {
			bg1.mask_logic = (value >> 0) & 3;
			bg2.mask_logic = (value >> 2) & 3;
			bg3.mask_logic = (value >> 4) & 3;
			bg4.mask_logic = (value >> 6) & 3;
		}

		if (addr.offset == WOBJLOG_ADDRESS) {
			obj.mask_logic = (value >> 0) & 3;
			col.mask_logic = (value >> 2) & 3;
		}

		if (addr.offset == TMW_ADDRESS) {
			bg1.windows_on_main_screen = (value >> 0) & 1;
			bg2.windows_on_main_screen = (value >> 1) & 1;
			bg3.windows_on_main_screen = (value >> 2) & 1;
			bg4.windows_on_main_screen = (value >> 3) & 1;
			obj.windows_on_main_screen = (value >> 4) & 1;
		}

		if (addr.offset == TSW_ADDRESS) {
			bg1.windows_on_subscreen = (value >> 0) & 1;
			bg2.windows_on_subscreen = (value >> 1) & 1;
			bg3.windows_on_subscreen = (value >> 2) & 1;
			bg4.windows_on_subscreen = (value >> 3) & 1;
			obj.windows_on_subscreen = (value >> 4) & 1;
		}

		// Colour Math

		if (addr.offset == CGWSEL_ADDRESS) {
			col.direct_colour_mode = (value >> 0) & 1;
			col.addend = (value >> 1) & 1;
			col.sub_screen_transparent_region = (value >> 4) & 3;
			col.main_screen_black_region = (value >> 6) & 3;
		}

		if (addr.offset == CGADSUB_ADDRESS) {
			bg1.enable_colour_math = (value >> 0) & 1;
			bg2.enable_colour_math = (value >> 1) & 1;
			bg3.enable_colour_math = (value >> 2) & 1;
			bg4.enable_colour_math = (value >> 3) & 1;
			obj.enable_colour_math = (value >> 4) & 1;

			col.backdrop_colour_math_enabled = (value >> 5) & 1;
			col.half_colour_math = (value >> 6) & 1;
			col.operator_type = (value >> 7) & 1;
		}

		if (addr.offset == COLDATA_ADDRESS) {
			Byte colour = value & 0x1F;

			if (value & 0x20) {
				col.red = colour;
			}

			if (value & 0x40) {
				col.green = colour;
			}

			if (value & 0x80) {
				col.blue = colour;
			}
		}

		// OAM

		if (addr.offset == OAMADDL_ADDRESS) {
			oam.reload = (get_hi(oam.reload) << 8) | value;
		}

		if (addr.offset == OAMADDH_ADDRESS) {
			oam.reload = ((value & 0b1) << 8) | get_lo(oam.reload);
			oam.priority_rotation = ((value >> 7) == 1);
		}

		if (addr.offset == OAMADDL_ADDRESS || addr.offset == OAMADDH_ADDRESS) {
			oam.oamadd = oam.reload << 1;
		}

		/*if (addr.offset == OAMDATA_ADDRESS) {
			if (oam_accessible()) {
				accepted++;
			} else {
				rejected++;
			}

			std::cout << "OAMDATA WRITE "
			          << std::dec << (int)(accepted)
			          << " ACCEPTED "
			          << (int)(rejected)
			          << " REJECTED\n";
		}*/

		if (oam_accessible()) {
			if (addr.offset == OAMDATA_ADDRESS) {
				if ((oam.oamadd & 1) == 0) {
					oam.latch = value;
				}

				if (oam.oamadd < 0x200 && (oam.oamadd & 1) == 1) {
					oam.data[oam.oamadd - 1] = oam.latch;
					oam.data[oam.oamadd] = value;

					update_object(oam.oamadd - 1, oam.latch);
					update_object(oam.oamadd, value);
				}

				if (oam.oamadd >= 0x200) {
					Word idx = 0x200 + ((oam.oamadd - 0x200) & 0x1F);
					oam.data[idx] = value;
					update_object(idx, value);
				}

				oam.oamadd = (oam.oamadd + 1) & 0x3FF;
			}
		}

		if (addr.offset == OBJSEL_ADDRESS) {
			Byte name_base_address = value & 7;
			Byte name_select = (value >> 3) & 3;
			Byte obj_size_index = (value >> 5);

			oam.obj_size = size_table[obj_size_index];
			oam.first_base = (name_base_address << 13);
			oam.second_base = (oam.first_base + ((name_select + 1) * 0x1000));
		}

		// CGRAM

		if (addr.offset == CGADD_ADDRESS) {
			cgram.cgram_address = value;
			cgram.cgram_byte = 0;
		}

		if (cgram_accessible()) {
			if (addr.offset == CGDATA_ADDRESS) {
				if (cgram.cgram_byte == 0) {
					cgram.cgram_latch = value;
				} else {
					cgram.data[cgram.cgram_address] =
						((value & 0x7F) << 8) | cgram.cgram_latch;

					cgram.cgram_address++;
				}

				cgram.cgram_byte = !cgram.cgram_byte;
			}
		}

		// VRAM

		if (addr.offset == VMAIN_ADDRESS) {
			switch (value & 3) {
				// Uses word addresses
				case 0:
					vram.address_increment = 1;
					break;

				case 1:
					vram.address_increment = 32;
					break;

				case 2:
					vram.address_increment = 128;
					break;

				case 3:
					vram.address_increment = 128;
					break;
			}

			vram.address_remapping = (value >> 2) & 3;
			vram.address_increment_mode = (((value >> 7) & 0b1) == 1);
		}

		if (addr.offset == VMADDL_ADDRESS) {
			vram.vmadd = (get_hi(vram.vmadd) << 8) | value;
		}

		if (addr.offset == VMADDH_ADDRESS) {
			vram.vmadd = ((value & 0x7F) << 8) | get_lo(vram.vmadd);
		}

		if (addr.offset == VMADDL_ADDRESS || addr.offset == VMADDH_ADDRESS) {
			vram.vram_latch = vram.data[remap_vmadd(vram.vmadd)];
		}

		if (vram_accessible()) {
			if (addr.offset == VMDATAL_ADDRESS) {
				Word mapped_addr = remap_vmadd(vram.vmadd);

				vram.data[mapped_addr] =
					(get_hi(vram.data[mapped_addr]) << 8) | value;

				if (vram.address_increment_mode == 0) {
					vram.vmadd =
						(vram.vmadd + vram.address_increment) & 0x7FFF;
				}

				invalidate_tile(mapped_addr);
			}

			if (addr.offset == VMDATAH_ADDRESS) {
				Word mapped_addr = remap_vmadd(vram.vmadd);

				vram.data[mapped_addr] =
					(value << 8) | get_lo(vram.data[mapped_addr]);

				if (vram.address_increment_mode == 1) {
					vram.vmadd =
						(vram.vmadd + vram.address_increment) & 0x7FFF;
				}

				invalidate_tile(mapped_addr);
			}
		}

		return;
	}

	bool in_hblank() {
		return hblank;
	}

	void connect_bus(Bus* bus) {
		this->bus = bus;
	}
	void connect_renderer(Renderer* renderer) {
		this->renderer = renderer;
	}
	void connect_cpu(Ricoh5A22* cpu) {
		this->cpu = cpu;
	}

	int get_vcounter() {
		return vcounter;
	}

	int get_hcounter() {
		return hcounter;
	}

	void latch_hv_counters() {
		counter_latch = true;
		ophct = hcounter;
		opvct = vcounter;
	}

	void log_ppu_data() {
		if constexpr (LOG_OAM) {
			std::cout << "\n========== OAM ==========\n";

			for (int i = 0; i < 0x400; i += 16) {
				std::cout
					<< std::uppercase
					<< std::hex
					<< std::setfill('0')
					<< std::setw(3)
					<< i
					<< ": ";

				for (int j = 0; j < 16; j++) {
					std::cout
						<< std::setw(2)
						<< static_cast<unsigned>(oam.data[i + j])
						<< ' ';
				}

				std::cout << '\n';
			}

			std::cout << std::dec;
		}

		if constexpr (LOG_CGRAM) {
			std::cout << "\n========== CGRAM ==========\n";

			for (int i = 0; i < 0x100; i += 8) {
				std::cout
					<< std::uppercase
					<< std::hex
					<< std::setfill('0')
					<< std::setw(3)
					<< i
					<< ": ";

				for (int j = 0; j < 8; j++) {
					std::cout
						<< std::setw(4)
						<< static_cast<unsigned>(cgram.data[i + j])
						<< ' ';
				}

				std::cout << '\n';
			}

			std::cout << std::dec;
		}

		if constexpr (LOG_VRAM) {
			std::cout << "\n========== VRAM ==========\n";

			for (int i = 0; i < 0x8000; i += 8) {
				std::cout
					<< std::uppercase
					<< std::hex
					<< std::setfill('0')
					<< std::setw(4)
					<< i
					<< ": ";

				for (int j = 0; j < 8; j++) {
					std::cout
						<< std::setw(4)
						<< static_cast<unsigned>(vram.data[i + j])
						<< ' ';
				}

				std::cout << '\n';
			}

			std::cout << std::dec;
		}

		std::cout << std::setfill(' ');
	}

	void connect_dma(DMA* dma) {
		this->dma = dma;
	}

	void update_hires() {
		hires_mode = pseudo_hires_mode || (bg_mode == 5) || (bg_mode == 6);
	}

private:

	Ricoh5A22* cpu = nullptr;

	std::vector<Object> object_buffer;
	
	bool time_over = false;
	bool range_over = false;
	bool master_slave_mode = false;
	Byte ppu1_version = 1;
	Byte ppu2_version = 3;
	Byte region = 0;


	int tile_size;
	int tiles_x, tiles_y;
	
	std::vector<uint32_t> framebuffer;
	std::vector<uint32_t> oam_view_framebuffer;

	std::array<bool, 512> window1_dots;
	std::array<bool, 512> window2_dots;

	OAM oam;
	CGRAM cgram;
	VRAM vram;
	Renderer* renderer;

	CycleCount cycle = 0;
	CycleCount instruction_cycle; 
	TickCount tick = 0;

	Byte hvbjoy = 0x00;

	bool forced_blank = false;
	Byte brightness = 0;
	bool vblank = false;
	bool hblank = false;

	Bus* bus = nullptr;
	DMA* dma = nullptr;

	bool field = false;
	int visible_lines = 0;
	int vblank_start = 0;
	int hcounter = 0;
	int vcounter = 0;

	// Display configurations

	int bg_mode = 0;
	bool bg3_priority = false;
	int mosaic_size = 0;

	BG bg1, bg2, bg3, bg4; // stores attributes relevant to bg layers

	Byte bg_ofs_latch = 0x00;
	ObjectLayer obj; // stores attributes relevant to obj layer
	ColorMathLayer col;
	Window window1;
	Window window2;
	Mode7 mode7; // stores attributes relevant to mode 7

	bool screen_interlacing = false;
	bool obj_interlacing = false;
	bool overscan_mode = false;
	bool hires_mode = false;
	bool pseudo_hires_mode = false;
	bool extbg_mode = false;
	bool external_sync = false;

	bool counter_latch = false;

	Word ophct = 0x00;
	Word opvct = 0x00;

	bool ophct_byte = false;
	bool opvct_byte = false;

	std::array<Priority, 8> priorities = initialise_priorities();
	std::array<Object, 128> all_objects;
	Priority priority_order;

	// DEBUGGING OAMDATA
	int accepted = 0;
	int rejected = 0;
	int hdma_transfer_lines = 0;

	// Tile cache
	DecodedTile tile_cache[NUM_BPP][MAX_TILES] {};
};