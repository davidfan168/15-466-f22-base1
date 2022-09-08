#include "PlayMode.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

#include <random>

#include "data_path.hpp"
#include "read_write_chunk.hpp"
#include <iostream>
#include <fstream> 

PlayMode::PlayMode() {

	{ // import tile (just one tile)
		std::string magic = "tile";
		for (int index = 1; index < 5; index++) {
			std::string sprite_path = data_path("sprite" + std::to_string(index));
			PPU466::Tile tile;
			std::filebuf fb;
			if (fb.open (sprite_path,std::ios::in))
			{
				std::istream istr(&fb);
				std::vector<uint8_t> tile_info;
				read_chunk(istr, magic, &tile_info);
				for (int i = 0; i < 8; i++) {
					tile.bit0[i] = tile_info[i];
					tile.bit1[i] = tile_info[i];
				}
				fb.close();
			}

			ppu.tile_table[index] = tile;
		}

		// Empty tiles
		PPU466::Tile black_tile;
		for (int i = 0; i < 8; i++) {
			black_tile.bit0[i] = 0xff;
			black_tile.bit1[i] = 0xff;
		}
		ppu.tile_table[6] = black_tile;

		PPU466::Tile white_tile;
		for (int i = 0; i < 8; i++) {
			white_tile.bit0[i] = 0;
			white_tile.bit1[i] = 0;
		}
		ppu.tile_table[7] = white_tile;
	}

	{
		std::string palette_path = data_path("palette");
		std::string magic = "pale";
		std::vector<char> palette_information;

		std::filebuf fb;
		if (fb.open (palette_path,std::ios::in))
		{
			std::istream istr(&fb);
			read_chunk(istr, magic, &palette_information);
			fb.close();
		}

		// we should import 7 palettes, 14 colors
		assert(palette_information.size() == 2 * 7 * 4);

		for (int i = 0; i < 7; i++) {
			//makes the outside of tiles 0-16 solid:
			glm::u8vec4 c1 = glm::u8vec4(palette_information[8*i], 
										 palette_information[8*i+1], 
										 palette_information[8*i+2], 
										 palette_information[8*i+3]);
			glm::u8vec4 c2 = glm::u8vec4(palette_information[8*i+4],
										 palette_information[8*i+5],
										 palette_information[8*i+6],
										 palette_information[8*i+7]);
			// I have more colors than I need, wasting some space here
			ppu.palette_table[i] = {
				c1, c2, c1, c2
			};
		}

		glm::u8vec4 c1 = glm::u8vec4(0, 0, 0, 0);
		glm::u8vec4 c2 = glm::u8vec4(255, 255, 255, 255);
		// I have more colors than I need, wasting some space here
		ppu.palette_table[7] = {
			c2, c2, c1, c1
		};
	}

}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.pressed = true;
			return true;
		}
	}

	return false;
}

void PlayMode::rotate_all() {
	// transpose
	for (int i = 0; i < 10; i++) {
		for (int j = i; j < 10; j++) {
			if (i != j) {
				std::swap(positions[i][j], positions[j][i]);
			}
		}
	}

	// swap columns
	for (int j = 0; j < 10; j++) {
		for (int i = 0; i < 5; i++) {
			std::swap(positions[j][i], positions[j][9-i]);
		}
	}
}

bool PlayMode::check_full(){
	// check row
	for (int y = 0; y < 10; y++) {
		bool all_full = true;
		for (int x = 0; x < 10; x++) {
			if (positions[y][x] == false) {
				all_full = false;
			}
		}

		if (all_full) {
			for (int i = y; i < 9; i++) {
				for (int x = 0; x < 10; x++) {
					positions[i][x] = positions[i+1][x];
				}
			}
			for (int x = 0; x < 10; x++) {
				positions[9][x] = false;
			}
			check_full();
			return true;
		}
	}

	// check column
	for (int x = 0; x < 10; x++) {
		bool all_full = true;
		for (int y = 0; y < 10; y++) {
			if (positions[y][x] == false) {
				all_full = false;
			}
		}

		if (all_full) {
			for (int i = x; i < 9; i++) {
				for (int y = 0; y < 10; y++) {
					positions[y][i] = positions[y][i+1];
				}
			}
			for (int y = 0; y < 10; y++) {
				positions[y][9] = false;
			}
			check_full();
			return true;
		}
	}
	return false;
}

void PlayMode::spawn_block(){
	// https://stackoverflow.com/questions/13445688/how-to-generate-a-random-number-in-c
	std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist6(0,5);

	color_index = dist6(rng);
	switch (color_index) {
		case 0:
		// J block
			current_block[0][0] = true;
			current_block[0][1] = true;
			current_block[0][2] = true;
			current_block[1][0] = true;
			current_block[1][1] = false;
			current_block[1][2] = false;
			break;
		case 1:
		// L block
			current_block[0][0] = true;
			current_block[0][1] = true;
			current_block[0][2] = true;
			current_block[1][0] = false;
			current_block[1][1] = false;
			current_block[1][2] = true;
			break;
		case 2:
		// O block
			current_block[0][0] = true;
			current_block[0][1] = true;
			current_block[0][2] = false;
			current_block[1][0] = true;
			current_block[1][1] = true;
			current_block[1][2] = false;
			break;
		case 3:
		// S block
			current_block[0][0] = true;
			current_block[0][1] = true;
			current_block[0][2] = false;
			current_block[1][0] = false;
			current_block[1][1] = true;
			current_block[1][2] = true;
			break;
		case 4:
		// T block
			current_block[0][0] = true;
			current_block[0][1] = true;
			current_block[0][2] = true;
			current_block[1][0] = false;
			current_block[1][1] = true;
			current_block[1][2] = false;
			break;
		case 5:
		// Z block
			current_block[0][0] = false;
			current_block[0][1] = true;
			current_block[0][2] = true;
			current_block[1][0] = true;
			current_block[1][1] = true;
			current_block[1][2] = false;
			break;
		default:
			break;
	}

	block_position = 10;
	has_block = true;
	
}

void PlayMode::move_down(){

	if (block_position == 0) {
		for (int y = 0; y < 2; y++) {
			for (int x = 0; x < 3; x++) {
				if (current_block[y][x] == true) {
					positions[block_position+y][x_offset + x] = true;
				}
			}
		}
		has_block = false;
		return;
	}

	for (int x = 0; x < 3; x++) {
		// check if block is in another block
		if (current_block[0][x] == true
			&& block_position - 1 <=10 
			&& positions[block_position - 1][x_offset + x] == true) {
			
			for (int y = 0; y < 2; y++) {
				for (int x = 0; x < 3; x++) {
					if (current_block[y][x] == true) {
						positions[block_position+y][x_offset + x] = true;
					}
				}
			}
			has_block = false;
			return;
		}
	}

	block_position -= 1;
}

void PlayMode::tick() {
	// if we have a row or column that's full, remove it and return
	if (check_full()) {
		return;
	}

	if (!has_block) {
		spawn_block();
	}
	else {
		move_down();
	}
}

void PlayMode::update(float elapsed) {
	if (left.pressed) {
		rotate_all();
		rotate_all();
		rotate_all();
	}

	if (right.pressed) {
		rotate_all();
	}

	if (down.pressed) {
		tick();
	}

	left.pressed = false;
	right.pressed = false;
	down.pressed = false;


	static float interval = 0.8f;
	static float time_passed = 0.0f;
	time_passed += elapsed;
	if (time_passed > interval) {
		time_passed = 0;
		tick();
	}
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//--- set ppu state based on game state ---

	// ppu.background_color = glm::u8vec4(
	// 	0xff, 0xff, 0xff, 0xff
	// );

	const uint16_t black_tile = (7 << 8) + 6;
	const uint16_t white_tile = (7 << 8) + 7;

	for (uint32_t y = 0; y < PPU466::BackgroundHeight; ++y) {
		for (uint32_t x = 0; x < PPU466::BackgroundWidth; ++x) {
			ppu.background[x+PPU466::BackgroundWidth*y] = black_tile;
		}
	}

	for (uint32_t y = 6; y < PPU466::BackgroundHeight - 34; ++y) {
		for (uint32_t x = 6; x < PPU466::BackgroundWidth - 38; ++x) {
			ppu.background[x+PPU466::BackgroundWidth*y] = white_tile;
		}
	}

	for (int y = 0; y < 10; y++) {
		for (int x = 0; x < 10; x++) {
			if (positions[y][x] == true) {
				ppu.background[(2*x+6)+PPU466::BackgroundWidth*(2*y+6)] = black_tile;
				ppu.background[(2*x+1+6)+PPU466::BackgroundWidth*(2*y+6)] = black_tile;
				ppu.background[(2*x+6)+PPU466::BackgroundWidth*(2*y+1+6)] = black_tile;
				ppu.background[(2*x+1+6)+PPU466::BackgroundWidth*(2*y+1+6)] = black_tile;
			}
		}
	}

	//tetris sprite:
	if (has_block) {
		int n = 0;
		for (int y = 0; y < 2; y++) {
		for (int x = 0; x < 3; x++) {
			if (current_block[y][x] == true) {
				ppu.sprites[n].x = int8_t((6+2*(x+x_offset)) * 8);
				ppu.sprites[n].y = int8_t((2*(block_position+y) + 6) * 8);
				ppu.sprites[n].index = 4;
				ppu.sprites[n].attributes = color_index;
				ppu.sprites[n+1].x = int8_t((7+2*(x+x_offset)) * 8);
				ppu.sprites[n+1].y = int8_t((2*(block_position+y) + 7) * 8);
				ppu.sprites[n+1].index = 2;
				ppu.sprites[n+1].attributes = color_index;
				ppu.sprites[n+2].x = int8_t((6+2*(x+x_offset)) * 8);
				ppu.sprites[n+2].y = int8_t((2*(block_position+y) + 7) * 8);
				ppu.sprites[n+2].index = 3;
				ppu.sprites[n+2].attributes = color_index;
				ppu.sprites[n+3].x = int8_t((7+2*(x+x_offset)) * 8);
				ppu.sprites[n+3].y = int8_t((2*(block_position+y) + 6) * 8);
				ppu.sprites[n+3].index = 1;
				ppu.sprites[n+3].attributes = color_index;
				n += 4;
			}
		}
		}
	}

	//--- actually draw ---
	ppu.draw(drawable_size);
}
