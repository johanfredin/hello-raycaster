#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <stdint.h>
#include "defs.h"
#include "graphics.h"
#include "player.h"
#include "ray.h"
#include "textures.h"
#include <stdbool.h>
#include "map.h"
#include "wall.h"


static bool isGameRunning = false;
static uint32_t ticksLastFrame;

static void setup(void) {
	loadWallTextures();
}

static void processInput(void) {
	SDL_Event event;
	SDL_PollEvent(&event);

	switch(event.type) {
		case SDL_QUIT: {
			isGameRunning = false;
			break;
		}
		case SDL_KEYDOWN: {
			const SDL_Keycode sym = event.key.keysym.sym;	
			if (sym == SDLK_UP) {
				player.walkDirection = 1;
			}
			if (sym == SDLK_DOWN) {
				player.walkDirection = -1;
			}
			if (sym == SDLK_RIGHT) {
				player.turnDirection = 1;
			}
			if (sym == SDLK_LEFT) {
				player.turnDirection = -1;
			}
			break;
		}
		case SDL_KEYUP: {
			const SDL_Keycode sym = event.key.keysym.sym;	
			if (event.key.keysym.sym == SDLK_ESCAPE) {
				isGameRunning = false;
			}
			if (sym == SDLK_UP || sym == SDLK_DOWN) {
				player.walkDirection = 0;
			}
			if (sym == SDLK_RIGHT || sym == SDLK_LEFT) {
				player.turnDirection = 0;
			}
			break;
		}

	}
}


static void update(void) {
	// Wait some time until the reach the target frame time in milliseconds
	int time_to_wait = FRAME_TIME_LENGTH - (SDL_GetTicks() - ticksLastFrame);

	// Only delay execution if we are running too fast
	if (time_to_wait > 0 && time_to_wait <= FRAME_TIME_LENGTH) {
		SDL_Delay(time_to_wait);
	}
	float deltaTime = (SDL_GetTicks() - ticksLastFrame) / 1000.0f;
	ticksLastFrame = SDL_GetTicks();

	// Update all game objects
	movePlayer(deltaTime);
	castAllRays();
}

static void render(void) {
	clearColorBuffer(0xFF000000);
	renderWallProjection();
	
	renderMap();
	renderRays();
	renderPlayer();

	renderColorBuffer();
}


static void releaseResources(void) {
	freeWallTextures();
	destroyWindow();
	SDL_Quit();
}

int main() {
	isGameRunning = initializeWindow();
	setup();
	while (isGameRunning) {
		processInput();
		update();
		render();
	}
	releaseResources();
	return EXIT_SUCCESS;
}