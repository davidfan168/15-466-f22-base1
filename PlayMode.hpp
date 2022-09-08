#include "PPU466.hpp"
#include "Mode.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	void tick();
	void spawn_block();
	void rotate_all();
	void move_down();
	bool check_full();

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

	// (0, 0) denotes the bottom left corner
	bool current_block[2][3];
	int color_index;

	// y coordinate of bottom-right position of new block 
	// relative to grid
	int block_position = 10;
	const int x_offset = 4;

	bool has_block = false;

	// grid of position
	bool positions[10][10] = {{false}};

	//some weird background animation:
	float background_fade = 0.0f;

	glm::vec2 player_at = glm::vec2(0.0f);

	//----- drawing handled by PPU466 -----

	PPU466 ppu;
};
