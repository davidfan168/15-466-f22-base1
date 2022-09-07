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
					tile.bit1[i] = 0;
				}
				fb.close();
			}
			// for (int i = 0; i < 8; i++) {
			// 	std::cout << std::to_string(tile.bit0[i]) << ", ";
			// }
			// std::cout << "\n";
			ppu.tile_table[index] = tile;
		}
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
			ppu.palette_table[i] = {
				c1, c2, c1, c2
			};
		}
	}
	ppu.palette_table[7] = {
		glm::u8vec4(0xff, 0xff, 0xff, 0xff),
		glm::u8vec4(0xff, 0xff, 0x00, 0xff),
		glm::u8vec4(0xff, 0xff, 0xff, 0xff),
		glm::u8vec4(0xff, 0xff, 0xff, 0xff),
	};

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

void PlayMode::check_block_collision() {

}

void PlayMode::rotate_block() {

}

void PlayMode::spawn_block(){

}

void PlayMode::tick() {
	if (!has_block) {
		spawn_block();
	}
	else {

	}
}

void PlayMode::update(float elapsed) {
	float PlayerSpeed = 100.0f;
	if (left.pressed) player_at.x -= PlayerSpeed * elapsed;
	if (right.pressed) player_at.x += PlayerSpeed * elapsed;
	left.pressed = false;
	right.pressed = false;


	static float interval = 3.0f;
	static float time_passed = 0.0f;
	if (time_passed > interval) {
		time_passed = 0;
		tick();
	}
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//--- set ppu state based on game state ---

	ppu.background_color = glm::u8vec4(
		0xff, 0xff, 0xff, 0xff
	);

	//tilemap gets recomputed every frame as some weird plasma thing:
	//NOTE: don't do this in your game! actually make a map or something :-)
	for (uint32_t y = 0; y < PPU466::BackgroundHeight; ++y) {
		for (uint32_t x = 0; x < PPU466::BackgroundWidth; ++x) {
			ppu.background[x+PPU466::BackgroundWidth*y] = 0;
		}
	}

	//player sprite:
	ppu.sprites[0].x = int8_t(player_at.x);
	ppu.sprites[0].y = int8_t(player_at.y);
	ppu.sprites[0].index = 1;
	ppu.sprites[0].attributes = 1;

	//--- actually draw ---
	ppu.draw(drawable_size);
}
