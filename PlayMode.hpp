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
	void rotate_block();
	void check_block_collision();

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

	bool current_block[3][3];

	struct position {
		uint8_t x = 0;
		uint8_t y = 0;
	} block_position;

	bool has_block = false;

	bool positions[10][10] = {{false}};

	//some weird background animation:
	float background_fade = 0.0f;

	glm::vec2 player_at = glm::vec2(0.0f);

	//----- drawing handled by PPU466 -----

	PPU466 ppu;
};
