#include <SDL2/SDL.h>
#include <SDL2/SDL_blendmode.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <math.h>
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <stdint.h>
#include "constants.h"

const uint8_t map[MAP_NUM_ROWS][MAP_NUM_COLS] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ,1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

typedef struct Player {
	float x;
	float y;
	uint16_t width;
	uint16_t height;
	float turnDirection; // -1 for left +1 for right 
	float walkDirection; // -1 for left +1 for right
	float rotationAngle;
	float walkSpeed;
	float turnSpeed;
} Player;

static Player player;
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

static int isGameRunning = FALSE;


static uint32_t ticksLastFrame;


static int initializeWindow(void) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "Error initializing SDL\n");
		return FALSE;
	}
	window = SDL_CreateWindow(
		"JayCaster", 
		SDL_WINDOWPOS_CENTERED, 
		SDL_WINDOWPOS_CENTERED, 
		WINDOW_WIDTH, 
		WINDOW_HEIGHT, 
		SDL_WINDOW_BORDERLESS
	);
	if (!window) {
		fprintf(stderr, "Error creating sdl window SDL\n");
		return FALSE;
	}

	renderer = SDL_CreateRenderer(window, SDL_DEFAULT_DRIVER, 0);
	if (!renderer) {
		fprintf(stderr, "Error creating sdl renderer SDL\n");
		return FALSE;
	}

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	return TRUE;
}	

static void setup(void) {
	player = (Player) {
		WINDOW_WIDTH >> 1,
		WINDOW_HEIGHT >> 1,
		5,
		5,
		0,
		0,
		PI / 2,
		50,
		DEG_TO_RAD(45)
	};
}

static void processInput(void) {
	SDL_Event event;
	SDL_PollEvent(&event);

	switch(event.type) {
		case SDL_QUIT: {
			isGameRunning = FALSE;
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
				isGameRunning = FALSE;
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

static void movePlayer(float deltaTime) {
	player.rotationAngle += player.turnDirection * player.turnSpeed * deltaTime;
	const float moveStep = player.walkDirection * player.walkSpeed * deltaTime;
	const float newPlayerX = player.x + cosf(player.rotationAngle) * moveStep;
	const float newPlayerY = player.y + sinf(player.rotationAngle) * moveStep;

	player.x = newPlayerX;
	player.y = newPlayerY;
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
}

static void renderRays(void) {

}

static void renderPlayer(void) {
	SDL_SetRenderDrawColor(renderer, 255, 200, 255, 255);
	SDL_Rect playerRectangle = {
		MINIMAP_SCALE_FACTOR * player.x,
		MINIMAP_SCALE_FACTOR * player.y,
		MINIMAP_SCALE_FACTOR * player.width,
		MINIMAP_SCALE_FACTOR * player.height
	};
	SDL_RenderFillRect(renderer, &playerRectangle);
	SDL_RenderDrawLine(renderer, 
		MINIMAP_SCALE_FACTOR *player.x,
		MINIMAP_SCALE_FACTOR *player.y,
		MINIMAP_SCALE_FACTOR *player.x + cosf(player.rotationAngle) * 40,
		MINIMAP_SCALE_FACTOR *player.y + sinf(player.rotationAngle) * 40
	);
}

static void renderMap(void) {
	for (int i = 0; i < MAP_NUM_ROWS; i++) {
		for (int j = 0; j < MAP_NUM_COLS; j++) {
			int tileX = j * TILE_SIZE;
			int tileY = i * TILE_SIZE;
			uint8_t tileColor = map[i][j] & 1 ? 0xFF : 0x00;
			SDL_SetRenderDrawColor(renderer, tileColor, tileColor, tileColor, 0xFF);
			SDL_Rect mapTileRect = {
				MINIMAP_SCALE_FACTOR * tileX,
				MINIMAP_SCALE_FACTOR * tileY,
				MINIMAP_SCALE_FACTOR * TILE_SIZE,
				MINIMAP_SCALE_FACTOR * TILE_SIZE
			};
			SDL_RenderFillRect(renderer, &mapTileRect);	
		}
	}
}

static void render(void) {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	// Render game objects
	renderMap();
	renderRays();
	renderPlayer();

	SDL_RenderPresent(renderer);
}


static void destroyWindow(void) {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
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

	destroyWindow();

	return 0;
}