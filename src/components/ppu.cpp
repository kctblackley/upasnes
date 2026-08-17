#include "ppu.hpp"
#include "ricoh_5a22.hpp"
#include "bus.hpp"
#include "dma.hpp"
#include <iostream>
#include <algorithm>

constexpr int DOTS_PER_LINE = 341;
constexpr int HBLANK_DOTS = 278;
constexpr int CPU_PAUSE = 134;
constexpr Address HVBJOY = 0x4212;
constexpr CycleCount PPU_CYCLE = 1;

constexpr int HDMA_INIT_DOT = 6;
constexpr int HDMA_TRANSFER_DOT = 278;

void PPU::window_mask(std::array<Pixel, 512>& scanline, bool window1_enabled, bool window2_enabled, bool window1_inverted, bool window2_inverted, Byte mask_logic, bool colour_math) {
	int step = hires_mode ? 1 : 2;
	for (int dot = 0; dot < 512; dot += step) {
		bool window1_mask = window1_dots[dot];
		bool window2_mask = window2_dots[dot];
		if (window1_inverted) { window1_mask = !window1_mask; }
		if (window2_inverted) { window2_mask = !window2_mask; }
				
		bool mask = false;
		if (window1_enabled && !window2_enabled) { mask = window1_mask; }
		if (!window1_enabled && window2_enabled) { mask = window2_mask; }
		if (window1_enabled && window2_enabled) {
			switch (mask_logic) {
			case 0: mask =   window1_mask || window2_mask;  break;
			case 1: mask =   window1_mask && window2_mask;  break;
			case 2: mask =   window1_mask ^  window2_mask;  break;
			case 3: mask = !(window1_mask ^  window2_mask); break;
			}
		}

		if (mask) {
			scanline[dot].transparent = true;
			if (!hires_mode) {
				scanline[dot + 1].transparent = true;
			}
		}
	}
}

// Note: tile caching cannot apply here
Pixel PPU::fetch_mode7_pixel(BG& bg, uint16_t xcounter) {
	int screen_x = xcounter;
	int screen_y = vcounter;

	int rel_x = screen_x + mode7.m7hofs - mode7.m7x;
	int rel_y = screen_y + mode7.m7vofs - mode7.m7y;

	int source_x = ((mode7.m7a * rel_x) + (mode7.m7b * rel_y)) >> 8; 
	int source_y = ((mode7.m7c * rel_x) + (mode7.m7d * rel_y)) >> 8; 

	source_x += mode7.m7x;
	source_y += mode7.m7y;

	source_x = source_x & 0x3FF;
	source_y = source_y & 0x3FF;

	int tile_x = source_x >> 3;
	int tile_y = source_y >> 3;

	int pixel_x = source_x & 0x7;
	int pixel_y = source_y & 0x7;

	uint32_t tile_index = (tile_x + (tile_y * 128)) & 0x3FFF;
	Byte tile_number = get_lo(vram.data[tile_index]);

	uint32_t pixel_offset = (pixel_y * 8) + pixel_x;
	uint32_t char_word_addr = ((tile_number * 64) + pixel_offset) & 0x3FFF;
	Byte colour = get_hi(vram.data[char_word_addr]);

	bool extbg_priority = false;
	if (bg.layer == 2 && extbg_mode) {
		extbg_priority = (colour & 0x80);
		colour = colour & 0x7F;
	}

	Word snes_colour;
	if (col.direct_colour_mode && bg.layer == 1) {
		Byte r3 =  colour       & 0x7;
		Byte g3 = (colour >> 3) & 0x7;
		Byte b2 = (colour >> 6) & 0x3;

		Byte r5 = r3 << 2;
		Byte g5 = g3 << 2;
		Byte b5 = b2 << 3;

		snes_colour = (b5 << 10) | (g5 << 5) | r5;
	} else {
		snes_colour = cgram.data[colour];
	}
	Pixel px;
	px.transparent = (colour == 0);
	px.colour = snes_colour;
	
	if (bg.layer == 1) {
		px.layer = 1;
		px.priority = priority_order.L1;
	} else {
		px.layer = 2;
		if (extbg_priority) {
			px.priority = priority_order.H2;
		} else {
			px.priority = priority_order.L2;
		}
		if (!extbg_mode) {
			px.transparent = true;
		}
	}

	return px;
}

void PPU::render_oam_view() {
	std::fill(oam_view_framebuffer.begin(), oam_view_framebuffer.end(), 0x000000FF);

	for (int i = 0; i < 128; i++) {
		Word tile_number = oam.data[(4 * i) + 2];
		Word attributes  = oam.data[(4 * i) + 3];

		tile_number = ((attributes & 1) << 8) | tile_number;
		Byte palette = (attributes >> 1) & 0x7;

		bool horizontal_flip = (attributes >> 6) & 1;
		bool vertical_flip   = (attributes >> 7) & 1;

		Byte high_byte = oam.data[512 + (i / 4)];
		Byte high_byte_pair = (high_byte >> (2 * (i % 4))) & 0x3;
		bool size = (high_byte_pair >> 1) & 1;

		int width  = size ? oam.obj_size.large_width  : oam.obj_size.small_width;
		int height = size ? oam.obj_size.large_height : oam.obj_size.small_height;

		int cell_col = i % 16;
		int cell_row = i / 16;
		int cell_x0 = cell_col * oam_cell_size;
		int cell_y0 = cell_row * oam_cell_size;

		for (int sprite_y = 0; sprite_y < height && sprite_y < oam_cell_size; sprite_y++) {
			int flipped_y = sprite_y;
			if (vertical_flip) {
				if (width == height) {
					flipped_y = height - 1 - sprite_y;
				} else if (sprite_y < width) {
					flipped_y = width - 1 - sprite_y;
				} else {
					flipped_y = width + (width - 1) - (sprite_y - width);
				}
			}

			int tile_row = flipped_y / 8;
			int pixel_y = flipped_y & 7;

			for (int sprite_x = 0; sprite_x < width && sprite_x < oam_cell_size; sprite_x++) {
				int flipped_x = horizontal_flip ? (width - 1 - sprite_x) : sprite_x;

				int tile_col = flipped_x / 8;
				int pixel_x = flipped_x & 7;

				int base_col = tile_number & 0xF;
				int base_row = (tile_number >> 4) & 0xF;

				int name_col = (base_col + tile_col) & 0xF;
				int name_row = (base_row + tile_row) & 0xF;

				int tile_index = (name_row << 4) | name_col;

				bool second_base = (tile_number & 0x100) != 0;
				Word tile_base = second_base ? oam.second_base : oam.first_base;
				Word tile_address = (tile_base + (tile_index * 16)) & 0x7FFF;

				Word p01 = vram.data[(tile_address + 0 + pixel_y) & 0x7FFF];
				Word p23 = vram.data[(tile_address + 8 + pixel_y) & 0x7FFF];

				Byte p0 = get_lo(p01);
				Byte p1 = get_hi(p01);
				Byte p2 = get_lo(p23);
				Byte p3 = get_hi(p23);

				int bit = 7 - pixel_x;
				Byte colour = (((p0 >> bit) & 1) << 0) |
							  (((p1 >> bit) & 1) << 1) |
							  (((p2 >> bit) & 1) << 2) |
							  (((p3 >> bit) & 1) << 3);

				if (colour == 0) {
					continue;
				}

				Byte cgram_index = 128 + (palette * 16) + colour;
				Word snes_colour = cgram.data[cgram_index];
				uint32_t rgba = convert_to_rgba(snes_colour);

				int px = cell_x0 + sprite_x;
				int py = cell_y0 + sprite_y;
				oam_view_framebuffer[py * oam_view_width + px] = rgba;
			}
		}
	}
}

void PPU::push_pixel(BG& bg, const Pixel& px, int& dot) {
	bg.main_scanline[dot] = px;
	bg.sub_scanline[dot]  = px;

	if (!hires_mode) {
		bg.main_scanline[dot + 1] = px;
		bg.sub_scanline [dot + 1]  = px;
		dot += 2;
	} else {
		dot++;
	}
}

void PPU::render_bg_scanline(BG& bg) {
	Pixel fetched_pixel;

	bool native_hires = (bg_mode == 5 || bg_mode == 6);
	int sub_px = native_hires ? (bg.bghofs & 15) : (bg.bghofs & 7);
	int dot = 0;
	
	while (dot < 512) {

		uint16_t xcounter = hires_mode ? dot : (dot >> 1);
		if (bg_mode == 7) {
			fetched_pixel = fetch_mode7_pixel(bg, xcounter);
			push_pixel(bg, fetched_pixel, dot);
		} else {
			int bg_x, bg_y;
			int mosaic_x, mosaic_y;
			
			if (bg.mosaic) {
				mosaic_x = xcounter - (xcounter % (mosaic_size + 1));
				mosaic_y = vcounter - (vcounter % (mosaic_size + 1));

				bg_x = (mosaic_x + bg.bghofs) & 0x3FF;
				bg_y = (mosaic_y + bg.bgvofs) & 0x3FF;
			} else {
				bg_x = (xcounter + bg.bghofs) & 0x3FF;
				bg_y = (vcounter + bg.bgvofs) & 0x3FF;
			}

			int tile_x = bg_x >> (native_hires ? 4 : 3);
			int tile_y = bg_y >> 3;

			int pixel_x = bg_x & (native_hires ? 0xF : 7);
			int pixel_y = bg_y & 7;

			int map_width_tiles  = bg.horizontal_tilemap_count ? 64 : 32;
			int map_height_tiles = bg.vertical_tilemap_count   ? 64 : 32;

			tile_x = tile_x & (map_width_tiles - 1);
			tile_y = tile_y & (map_height_tiles - 1);

			int screen_x = tile_x >> 5;
			int screen_y = tile_y >> 5;
			int screen = 0;

			if (bg.horizontal_tilemap_count) {
				screen += screen_x;
			}

			if (bg.vertical_tilemap_count) {
				screen += screen_y * (bg.horizontal_tilemap_count ? 2 : 1);
			}

			int local_x = tile_x & 31;
			int local_y = tile_y & 31;

			Word tilemap_address = (bg.tilemap_vram_address + (screen * 0x400) + (local_y * 32) + local_x) & 0x7FFF;
			Word entry = vram.data[tilemap_address];

			int tile_number = entry & 0x3FF;
			bool hflip = entry & 0x4000;
			bool vflip = entry & 0x8000;

			int palette  = (entry >> 10) & 0x7;
			int priority = (entry >> 13) & 0x1;

			if (native_hires) {
				if (bg.character_size) {
					int sub_y = (bg_y >> 3) & 1;

					if (vflip) { sub_y ^= 1; }

					tile_number += sub_y * 16;
				}
			} else if (bg.character_size) {
				int sub_x = (bg_x >> 3) & 1;
				int sub_y = (bg_y >> 3) & 1;

				if (hflip) { sub_x ^= 1; }
				if (vflip) { sub_y ^= 1; }

				tile_number += sub_x;
				tile_number += sub_y * 16;
			}

			if (vflip) {
				pixel_y = pixel_y ^ 7;
			}

			int priority_value = 0;

			if (priority) {
				if (bg.layer == 1) { priority_value = priority_order.H1; }
				if (bg.layer == 2) { priority_value = priority_order.H2; }
				if (bg.layer == 3) { priority_value = priority_order.H3; }
				if (bg.layer == 4) { priority_value = priority_order.H4; }
			} else {
				if (bg.layer == 1) { priority_value = priority_order.L1; }
				if (bg.layer == 2) { priority_value = priority_order.L2; }
				if (bg.layer == 3) { priority_value = priority_order.L3; }
				if (bg.layer == 4) { priority_value = priority_order.L4; }
			}

			int stride = 0;
			if (bg.bpp == 2) { stride = 4; }
			if (bg.bpp == 4) { stride = 16; }

			int palette_base = palette * stride;

			// ONCE WORKING, ADD MOSAIC

			Pixel px;
			px.priority = priority_value;
			px.layer = bg.layer;
			px.colour_math = bg.enable_colour_math;

			int character_width = native_hires ? 16 : 8;

			while (sub_px < character_width && dot < 512) {

				int current_tile_number = tile_number;

				if (native_hires) {
					int character_pixel = hflip ? (15 - sub_px) : sub_px;
					current_tile_number += character_pixel >> 3;
				}

				Word tile_address = (bg.word_address + current_tile_number * (4 * bg.bpp)) & 0x7FFF;

				DecodedRow* row = get_tile_row(tile_address, pixel_y, bg.bpp);
				const auto& row_data = row->data;

				int source_pixel_x;

				if (native_hires) {
					int character_pixel = hflip ? (15 - sub_px) : sub_px;
					source_pixel_x = character_pixel & 7;
				} else {
					source_pixel_x = hflip ? (7 - sub_px) : sub_px;
				}

				Byte colour;

				if (bg.mosaic) {
					int actual_xcounter = xcounter + sub_px;
					int mosaic_size_pixels = mosaic_size + 1;
					int mosaic_x = actual_xcounter - (actual_xcounter % mosaic_size_pixels);

					int source_x = (mosaic_x + bg.bghofs) & 0x3FF;
					int source_pixel_x = source_x & 7;

					colour = hflip ? row_data[7 - source_pixel_x] : row_data[source_pixel_x];
				} else {
					colour = row_data[source_pixel_x];
				}

				int cgram_index = palette_base + colour;
				Word snes_colour;
				if (col.direct_colour_mode && bg.bpp == 8) {
					Byte r3 =  colour       & 0x7;
					Byte g3 = (colour >> 3) & 0x7;
					Byte b2 = (colour >> 6) & 0x3;

					Byte r5 = (r3 << 2) | ((palette & 0x1) << 1);
					Byte g5 = (g3 << 2) | ((palette & 0x2) << 0);
					Byte b5 = (b2 << 3) | ((palette & 0x4) << 0);

					snes_colour = (b5 << 10) | (g5 << 5) | r5;
				} else {
					int cgram_index = palette_base + colour;
					snes_colour = cgram.data[cgram_index];
				}
				
				px.transparent = (colour == 0);
				px.colour = snes_colour;
			
				push_pixel(bg, px, dot);

				sub_px++;
			}

			sub_px = 0;
		}

	}

	if (bg.windows_on_subscreen) {
		window_mask(bg.sub_scanline, bg.window1_enabled, bg.window2_enabled, bg.window1_inverted, bg.window2_inverted, bg.mask_logic, bg.enable_colour_math);
	}
	if (bg.windows_on_main_screen) {
		window_mask(bg.main_scanline, bg.window1_enabled, bg.window2_enabled, bg.window1_inverted, bg.window2_inverted, bg.mask_logic, bg.enable_colour_math);
	}
}

void PPU::fetch_objects() {

	object_buffer.clear();
	int first_object = 0;

	if (oam.priority_rotation) {
		first_object = (oam.reload >> 1) & 0x7F;
	}

	for (int n = 0; n < 128; n++) {
		int i = (first_object + n) & 0x7F;
		Word x_coordinate = oam.data[(4 * i) + 0];
		Word y_coordinate = oam.data[(4 * i) + 1];
		Word tile_number  = oam.data[(4 * i) + 2];
		Word attributes   = oam.data[(4 * i) + 3];

		tile_number       = ((attributes & 1) << 8) | tile_number;
		Byte palette      = (attributes >> 1) & 0x7;
		Byte priority     = 0;

		switch ((attributes >> 4) & 0x3) {
		case 0:
			priority = priority_order.S0;
			break;
		case 1:
			priority = priority_order.S1;
			break;
		case 2:
			priority = priority_order.S2;
			break;
		case 3:
			priority = priority_order.S3;
			break;
		}

		bool horizontal_flip = (attributes >> 6) & 1;
		bool vertical_flip   = (attributes >> 7) & 1;

		Byte high_byte = oam.data[512 + (int)(i / 4)];
		Byte high_byte_pair = (high_byte >> (2 * (i % 4))) & 0x3;
		
		x_coordinate = ((high_byte_pair & 1) << 8) | x_coordinate;
		int signed_x = x_coordinate;
		if (signed_x > 255) {
			signed_x -= 512;
		}

		bool size = (high_byte_pair >> 1) & 1;

		int width  = size ? oam.obj_size.large_width  : oam.obj_size.small_width;
		int height = size ? oam.obj_size.large_height : oam.obj_size.small_height;

		int render_width  = hires_mode ? width : (2 * width);
		int render_height = hires_mode ? height : (2 * height);

		int line_in_sprite = (vcounter - y_coordinate) & 0xFF;

		bool x_in_range = (signed_x > -width) && (signed_x < 256);

		if (line_in_sprite < height && x_in_range) {

			if (object_buffer.size() >= MAX_OBJECTS) {
				if (!range_over) {
					//std::cout << "[DIAG] range_over SET at vcounter=" << std::dec << vcounter
					          //<< " (sprite index " << i << ")\n";
				}
				range_over = true;
				continue;
			}

			Object obj;

			obj.x_coordinate = signed_x;
			obj.y_coordinate = y_coordinate;
			obj.tile_number = tile_number;
			obj.attributes = attributes;
			obj.palette = palette;
			obj.priority = priority;
			obj.horizontal_flip = horizontal_flip;
			obj.vertical_flip = vertical_flip;
			obj.width = width;
			obj.height = height;
			obj.render_width = render_width;
			obj.render_height = render_height;
			obj.line_in_sprite = line_in_sprite;
			object_buffer.push_back(obj);
		} 
	}

	int tiles_fetched = 0;
	for (auto it = object_buffer.rbegin(); it != object_buffer.rend(); ++it) {
		Object& o = *it;
		if (o.x_coordinate <= -8 || o.x_coordinate >= 256) {
			continue;
		}
		int tiles_this_sprite = o.width / 8;
		tiles_fetched += tiles_this_sprite;
		if (tiles_fetched > 34) {
			if (!time_over) {
				/*std::cout << "[DIAG] time_over SET at vcounter=" << std::dec << vcounter << "\n";*/
			}
			time_over = true;
			break;
		}
	}

}

void PPU::render_obj_scanline(ObjectLayer& obj) {

	Pixel transparent_pixel;
	transparent_pixel.transparent = true;
	transparent_pixel.colour = 0x00;
	transparent_pixel.priority = 0;

	std::array<Pixel, 512>& scanline = obj.scanline;
	scanline.fill(transparent_pixel);

	for (auto& o : object_buffer) {
		int x = o.x_coordinate * 2;

		int sprite_y = o.line_in_sprite;

		if (o.vertical_flip) {
			if (o.width == o.height) {
				sprite_y = o.height - 1 - sprite_y;
			} else if (sprite_y < o.width) {
				sprite_y = o.width - 1 - sprite_y;
			} else {
				sprite_y = o.width + (o.width - 1) - (sprite_y - o.width);
			}
		}

		int tile_row = sprite_y / 8;
		int pixel_y = sprite_y & 7;

		int base_col = o.tile_number & 0xF;
		int base_row = (o.tile_number >> 4) & 0xF;

		bool second_base = (o.tile_number & 0x100) != 0;
		Word tile_base = second_base ? oam.second_base : oam.first_base;

		int last_tile_col = -1;
		DecodedRow* row_data = nullptr;

		for (int i = 0; i < o.width; i++) {

			int sprite_x = i;

			if (o.horizontal_flip) {
				sprite_x = o.width - 1 - sprite_x;
			}

			int tile_col = sprite_x / 8;
			int pixel_x = sprite_x & 7;

			if (tile_col != last_tile_col) {
				int col = (base_col + tile_col) & 0xF;
				int row = (base_row + tile_row) & 0xF;

				int tile_index = (row << 4) | col;

				Word tile_address = (tile_base + (tile_index * 16)) & 0x7FFF;
				row_data = get_tile_row(tile_address, pixel_y, 4);
				last_tile_col = tile_col;
			}

			Byte colour = row_data->data[pixel_x];

			Pixel px;

			px.transparent = (colour == 0);

			Byte cgram_index = 128 + (o.palette * 16) + colour;
			Word snes_colour = cgram.data[cgram_index];

			px.priority = o.priority;
			px.colour = snes_colour;

			px.colour_math =  obj.enable_colour_math && (o.palette >= 4);

			// Sprites are not affected by hires mode
			if (x >= 0 && x < 512) {
				if (scanline[x].transparent && !px.transparent) {
					scanline[x] = px;
				}
			}
			if (((x + 1) >= 0 && (x + 1) < 512)) {
				if (scanline[x + 1].transparent && !px.transparent) {
					scanline[x + 1] = px;
				}
			}

			x += 2;
		}
	}

	obj.main_scanline = obj.scanline;
	obj.sub_scanline = obj.scanline;

	if (obj.windows_on_subscreen) {
		window_mask(obj.sub_scanline, obj.window1_enabled, obj.window2_enabled, obj.window1_inverted, obj.window2_inverted, obj.mask_logic, obj.enable_colour_math);
	}
	if (obj.windows_on_main_screen) {
		window_mask(obj.main_scanline, obj.window1_enabled, obj.window2_enabled, obj.window1_inverted, obj.window2_inverted, obj.mask_logic, obj.enable_colour_math);
	}
}

bool PPU::should_resolve(bool is_window, int value) {
	// Used by sub screen
	bool resolve = false;
	switch (value) {
	case 0: resolve = false; break;
	case 1: resolve = !is_window; break;
	case 2: resolve = is_window; break;
	case 3: resolve = true; break;
	}
	return resolve;
}

bool PPU::is_colour_math_window(int x) {
	bool window1_mask = (x >= window1.left_position) && (x <= window1.right_position);
	bool window2_mask = (x >= window2.left_position) && (x <= window2.right_position);
	if (col.window1_inverted) { window1_mask = !window1_mask; }
	if (col.window2_inverted) { window2_mask = !window2_mask; }
			
	bool mask = false;
	if (col.window1_enabled && !col.window2_enabled) { mask = window1_mask; }
	if (!col.window1_enabled && col.window2_enabled) { mask = window2_mask; }
	if (col.window1_enabled && col.window2_enabled) {
		switch (col.mask_logic) {
		case 0: mask =   window1_mask || window2_mask;  break;
		case 1: mask =   window1_mask && window2_mask;  break;
		case 2: mask =   window1_mask ^  window2_mask;  break;
		case 3: mask = !(window1_mask ^  window2_mask); break;
		}
	}

	return mask;
}

bool PPU::resolve_main_screen_px(Pixel& px, bool is_window) {
	bool resolve = should_resolve(is_window, col.main_screen_black_region);
	if (resolve) {
		px.colour = 0;
		px.transparent = false;
	}
	return resolve;
}

void PPU::resolve_sub_screen_px(Pixel& px, bool is_window) {
	bool resolve = should_resolve(is_window, col.sub_screen_transparent_region);
	if (resolve) {
		px.transparent = true;
	}
}

inline int clamp(int value, int min, int max) {
	if (value < min) { return min; }
	if (value > max) { return max; }
	return value;
}

Pixel PPU::colour_math(Pixel main, Pixel sub, bool ignore_half) {
	/*
	// POSSIBLY INCORRECT
	if (!main.colour_math || (col.addend == 1 && sub.transparent)) {
		return main;
	}*/

	if (!main.colour_math || sub.transparent) {
		return main;
	}

	Byte main_red   = (main.colour >> 0)  & 0x1F;
	Byte main_green = (main.colour >> 5)  & 0x1F;
	Byte main_blue  = (main.colour >> 10) & 0x1F;

	Byte sub_red, sub_green, sub_blue;
	if (!col.addend) {
		sub_red   = col.red;
		sub_green = col.green;
		sub_blue  = col.blue;
	} else {
		sub_red   = (sub.colour >> 0)   & 0x1F;
		sub_green = (sub.colour >> 5)   & 0x1F;
		sub_blue  = (sub.colour >> 10)  & 0x1F;
	}
	
	Byte red, green, blue;

	Byte divide = (col.half_colour_math && !ignore_half) ? 2 : 1;
	if (col.operator_type) {
		red   = clamp( (main_red   - sub_red) / divide,   0, 31);
		green = clamp( (main_green - sub_green) / divide, 0, 31);
		blue  = clamp( (main_blue  - sub_blue) / divide,  0, 31);
	} else {
		red   = clamp( (main_red   + sub_red) / divide,   0, 31);
		green = clamp( (main_green + sub_green) / divide, 0, 31);
		blue  = clamp( (main_blue  + sub_blue) / divide,  0, 31);
	}

	Pixel result = main;
	result.colour = (blue << 10) | (green << 5) | red;

	return result;
}

void PPU::composite(std::array<Pixel, 512>& final_scanline) {
	// JUST RESOLVES PRIORITIES, NO COLOUR MATH JUST YET!
	Pixel main_default_pixel;
	main_default_pixel.transparent = false;
	main_default_pixel.colour = cgram.data[0];
	main_default_pixel.colour_math = col.backdrop_colour_math_enabled;
	main_default_pixel.priority = 0;

	Pixel sub_default_pixel;
	sub_default_pixel.transparent = false;
	sub_default_pixel.colour = (col.blue << 10) | (col.green << 5) | col.red;
	sub_default_pixel.colour_math = col.backdrop_colour_math_enabled;
	sub_default_pixel.priority = 0;

	bool has_bg4 = (bg_mode == 0);

	std::array<bool, 512> cm_window;
	if (col.window1_enabled || col.window2_enabled) {
		for (int dot = 0; dot < 512; dot++) {
			uint16_t screen_x = hires_mode ? dot : (dot >> 1);
			cm_window[dot] = is_colour_math_window(screen_x);
		}
	}

	for (int dot = 0; dot < 512; dot++) {
		Pixel* winner = nullptr;

		auto consider = [&winner](bool enabled, Pixel& px) {
			if (!enabled || px.transparent) { return; }
			if (!winner || px.priority > winner->priority ||
				(px.priority == winner->priority && px.layer > winner->layer)) {
				winner = &px;
			}
		};

		consider(bg1.main_screen, bg1.main_scanline[dot]);
		consider(bg2.main_screen, bg2.main_scanline[dot]);
		consider(bg3.main_screen, bg3.main_scanline[dot]);
		if (has_bg4) { consider(bg4.main_screen, bg4.main_scanline[dot]); }
		consider(obj.main_screen, obj.main_scanline[dot]);

		Pixel main_screen_px = winner ? *winner : main_default_pixel;

		winner = nullptr;

		consider(bg1.sub_screen, bg1.sub_scanline[dot]);
		consider(bg2.sub_screen, bg2.sub_scanline[dot]);
		consider(bg3.sub_screen, bg3.sub_scanline[dot]);
		if (has_bg4) { consider(bg4.sub_screen, bg4.sub_scanline[dot]); }
		consider(obj.sub_screen, obj.sub_scanline[dot]);

		Pixel sub_screen_px = winner ? *winner : sub_default_pixel;
		bool sub_is_backdrop = (winner == nullptr);

		bool is_window = false;
		if (col.window1_enabled || col.window2_enabled) {
			is_window = cm_window[dot];
		}
		bool main_forced_black = resolve_main_screen_px(main_screen_px, is_window);
		resolve_sub_screen_px(sub_screen_px, is_window);

		bool ignore_half = main_forced_black || (col.addend && sub_is_backdrop);
		final_scanline[dot] = colour_math(main_screen_px, sub_screen_px, ignore_half);
	}
}

static int dump_timer = 0;

void PPU::clear_framebuffer(std::vector<uint32_t>& f) {
	f.assign(screen_width * framebuffer_height, 0x000000FF);
}

void PPU::add_to_framebuffer(std::vector<uint32_t>& f, std::array<Pixel, 512>& line) {
	int idx1 = screen_width * (2 * vcounter);

	for (int i = 0; i < screen_width; i++) {
		f[idx1 + i] = convert_to_rgba(line[i].colour);
	}

	std::copy(f.begin() + idx1, f.begin() + idx1 + screen_width, f.begin() + idx1 + screen_width);
}

static const auto& rgba_lut() {
	static auto lut = []{
		std::array<std::array<uint32_t, 32768>, 16> t{};
		for (int b = 0; b < 16; b++) {
			for (int c = 0; c < 32768; c++) {
				Byte r5 = c & 0x1F;
				Byte g5 = (c >> 5) & 0x1F;
				Byte b5 = (c >> 10) & 0x1F;

				r5 = (r5 * (b + 1)) >> 4;
				g5 = (g5 * (b + 1)) >> 4;
				b5 = (b5 * (b + 1)) >> 4;

				Byte r8 = (r5 << 3) | (r5 >> 2);
				Byte g8 = (g5 << 3) | (g5 >> 2);
				Byte b8 = (b5 << 3) | (b5 >> 2);

				uint32_t rgba = (r8 << 24) | (g8 << 16) | (b8 << 8) | 0xFF;

				t[b][c] = rgba;
			}
		}
		return t;
	}();
	return lut;
}

uint32_t PPU::convert_to_rgba(uint16_t colour) {
	return rgba_lut()[brightness][colour];
}

void PPU::render_scanline() {
	fetch_objects();

	bool any_window_used =
		bg1.windows_on_subscreen || bg1.windows_on_main_screen ||
		bg2.windows_on_subscreen || bg2.windows_on_main_screen ||
		bg3.windows_on_subscreen || bg3.windows_on_main_screen ||
		bg4.windows_on_subscreen || bg4.windows_on_main_screen ||
		obj.windows_on_subscreen || obj.windows_on_main_screen;

	if (any_window_used) {
		for (int x = 0; x < 512; x ++) {
			int screen_x = hires_mode ? x : (x / 2);
			window1_dots[x] = (screen_x >= window1.left_position) && (screen_x <= window1.right_position);
			window2_dots[x] = (screen_x >= window2.left_position) && (screen_x <= window2.right_position);
		}
	}
	if (bg1.main_screen || bg1.sub_screen) { render_bg_scanline(bg1); }
	if constexpr (DEBUG_WINDOW) {
		if (!forced_blank) {
			add_to_framebuffer(bg1.framebuffer, bg1.main_scanline);
		}
	}

	if (bg2.main_screen || bg2.sub_screen) {
		if (bg_mode != 6) {
			render_bg_scanline(bg2);
			if constexpr (DEBUG_WINDOW) {
				if (!forced_blank) {
					add_to_framebuffer(bg2.framebuffer, bg2.main_scanline);
				}
			}
		}
	}

	if (bg3.main_screen || bg3.sub_screen) {
		if (bg_mode != 3 && bg_mode != 5 && bg_mode != 7) {
			render_bg_scanline(bg3);
			if constexpr (DEBUG_WINDOW) {
				if (!forced_blank) {
					add_to_framebuffer(bg3.framebuffer, bg3.main_scanline);
				}
			}
		}
	}

	if (bg4.main_screen || bg4.sub_screen) {
		if (bg_mode == 0) {
			render_bg_scanline(bg4);
			if constexpr (DEBUG_WINDOW) {
				if (!forced_blank) {
					add_to_framebuffer(bg4.framebuffer, bg4.main_scanline);
				}
			}
		}
	}

	if (obj.main_screen || obj.sub_screen) {
		render_obj_scanline(obj);
		if constexpr (DEBUG_WINDOW) {
			if (!forced_blank) {
				add_to_framebuffer(obj.framebuffer, obj.main_scanline);
			}
		}
	}

	std::array<Pixel, 512> final_scanline;
	composite(final_scanline);
	
	int idx1 = screen_width * (2 * vcounter);
	int idx2 = screen_width * ((2 * vcounter) + 1);
	
	if (forced_blank) {
		for (int i = 0; i < screen_width; i++) {
			framebuffer[idx1 + i] = 0x000000FF;
			framebuffer[idx2 + i] = 0x000000FF;
		}

		return;
	}

	for (const auto& px : final_scanline) {
		uint32_t rgba = convert_to_rgba(px.colour);

		framebuffer[idx1] = rgba;
		framebuffer[idx2] = rgba;

		idx1++;
		idx2++;
	}
}

void PPU::call_irq() {
	if (cpu) {
		cpu->signal_irq();
	}
}

void PPU::call_nmi() {
	if (cpu) {
		cpu->signal_nmi_start();
	}
}

bool PPU::frame_ended() {
	return vcounter >= 262;
}

void PPU::next_frame() {
	vcounter = 0;

	if (screen_interlacing) {
		field = !field;
	}

	update_vblank();
}

void PPU::enter_hblank() {
	hvbjoy = hvbjoy | (0b1 << 6);
	if (cpu) {
		cpu->set_hvbjoy_flag(0b1 << 6, true);
	}
}

void PPU::leave_hblank() {
	hvbjoy = hvbjoy & ~(0b1 << 6);
	if (cpu) {
		cpu->set_hvbjoy_flag(0b1 << 6, false);
	}
}

void PPU::enter_vblank() {
	frame_finished = true;
	hvbjoy = hvbjoy | (0b1 << 7);
	if (cpu) {
		cpu->set_hvbjoy_flag(0b1 << 7, true);
	}
	
	if (!forced_blank) {
		oam.oamadd = oam.reload << 1;
	}
	
	call_nmi();
}

void PPU::leave_vblank() {
	hvbjoy = hvbjoy & ~(0b1 << 7);
	if (cpu) {
		cpu->set_hvbjoy_flag(0b1 << 7, false);
		//cpu->signal_nmi_end();
	}

	if (!forced_blank) {
		range_over = false;
		time_over = false;
	}
}

void PPU::update_hblank() {
	bool old_hblank = hblank;
	hblank = (hcounter >= HBLANK_DOTS);

	if (!old_hblank && hblank) {
		enter_hblank();
	}
	if (old_hblank && !hblank) {
		leave_hblank();
	}
}

void PPU::update_vblank() {
	vblank_start = (overscan_mode ? 240 : 225);

	bool old_vblank = vblank;
	vblank = (vcounter >= vblank_start);

	if (!old_vblank && vblank) {
		enter_vblank();
	}
	if (old_vblank && !vblank) {
		leave_vblank();
	}
}

void PPU::end_scanline() {

	vcounter += 1;

	update_vblank();

	if (frame_ended()) {
		next_frame();
		return;
	}

	int visible_lines = overscan_mode ? overscan_vcount : no_overscan_vcount;
	if (vcounter < visible_lines) {
		render_scanline();
	}
}

void PPU::tick_component() {
	// MAIN PPU STUFF

	hcounter += 1;

	if (hcounter == DOTS_PER_LINE) {
		hcounter = 0;
		end_scanline();
	}

	if (hcounter == CPU_PAUSE) {
		bus->wram_refresh_pause();
	}

	if (hcounter == HDMA_INIT_DOT && vcounter == 0) {
		if (hdma_transfer_lines != 0) {
			//std::cout << "Did HDMA on " << (int)(hdma_transfer_lines) << " lines.\n";
		}
		dma->hdma_init();
		hdma_transfer_lines = 0;
	}

	bool is_hdma_transfer_line = vcounter < vblank_start;
	if (hcounter == HDMA_TRANSFER_DOT && is_hdma_transfer_line) {
		dma->hdma_transfer();
		hdma_transfer_lines++;
	}

	update_hblank();

	bool condition_now =
	    (irq_mode == 1 && hcounter == 4 + h_time_target) ||
	    (irq_mode == 2 && vcounter == v_time_target && hcounter == 3) ||
	    (irq_mode == 3 && vcounter == v_time_target && hcounter == 4 + h_time_target);

	if (condition_now && !irq_condition_met) {
	    call_irq();
	}
	irq_condition_met = condition_now;

	cycle += 4;
};

// Moved here to avoid circular dependency

Byte PPU::communication_read(SNESAddress addr) {
	Byte fetched = bus->get_open_bus();
	// OAM
	if (addr.offset == OAMDATAREAD_ADDRESS) {
		if (oam.oamadd < 0x200) {
			fetched = oam.data[oam.oamadd];
		} else {
			fetched = oam.data[0x200 + ((oam.oamadd - 0x200) & 0x1F)];
		}
		oam.oamadd = (oam.oamadd + 1) & 0x3FF;
	}

	// CGRAM
	if (addr.offset == CGDATAREAD_ADDRESS) {
		if (cgram.cgram_byte == 0) {
			fetched = get_lo(cgram.data[cgram.cgram_address]);
		} else {
			fetched = get_hi(cgram.data[cgram.cgram_address]);
			cgram.cgram_address++;
		}
		cgram.cgram_byte = !cgram.cgram_byte;
	}

	// VRAM
	if (addr.offset == VMDATALREAD_ADDRESS) {
		fetched = get_lo(vram.vram_latch);
		if (vram.address_increment_mode == 0) {
			vram.vram_latch = vram.data[remap_vmadd(vram.vmadd)];
			vram.vmadd = (vram.vmadd + vram.address_increment) & 0x7FFF;
		}
	}
	if (addr.offset == VMDATAHREAD_ADDRESS) {
		fetched = get_hi(vram.vram_latch);
		if (vram.address_increment_mode == 1) {
			vram.vram_latch = vram.data[remap_vmadd(vram.vmadd)];
			vram.vmadd = (vram.vmadd + vram.address_increment) & 0x7FFF;
		}
	}

	// Multiplication result
	if (addr.offset == MPYH_ADDRESS || addr.offset == MPYM_ADDRESS || addr.offset == MPYL_ADDRESS) {
		mode7.mpy = mode7.m7a * mode7.last_m7b;
		if (addr.offset == MPYL_ADDRESS) {
			fetched = (mode7.mpy >> 0) & 0xFF;
		}
		if (addr.offset == MPYM_ADDRESS) {
			fetched = (mode7.mpy >> 8) & 0xFF;
		}
		if (addr.offset == MPYH_ADDRESS) {
			fetched = (mode7.mpy >> 16) & 0xFF;
		}
	}

	// H/V Counters

	if (addr.offset == SLHV_ADDRESS) {
		latch_hv_counters();
	}
	if (addr.offset == OPHCT_ADDRESS) {
		if (ophct_byte == 0) {
			fetched = get_lo(ophct);
		} else {
			fetched = get_hi (ophct);
		}
		ophct_byte = !ophct_byte;
	}
	if (addr.offset == OPVCT_ADDRESS) {
		if (opvct_byte == 0) {
			fetched = get_lo(opvct);
		} else {
			fetched = get_hi (opvct);
		}
		opvct_byte = !opvct_byte;
	}

	// Status
	if (addr.offset == STAT77_ADDRESS) {
	    fetched = (time_over << 7) | (range_over << 6) | (master_slave_mode << 5) | (ppu1_version & 0xFF);
	    /*std::cout << "[DIAG] STAT77 read at vcounter=" << std::dec << vcounter
	              << " value=0x" << std::hex << (int)fetched
	              << " (time_over=" << time_over << " range_over=" << range_over << ")\n" << std::dec;*/
	}
	if (addr.offset == STAT78_ADDRESS) {
	    fetched = (field << 7) | (counter_latch << 5) | (region << 4) | (ppu2_version & 0xFF);
	    counter_latch = false;
	    ophct_byte = false;
	    opvct_byte = false;
	}

	return fetched;
}