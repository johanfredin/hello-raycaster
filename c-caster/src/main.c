#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <stdint.h>
#include "defs.h"
#include "graphics.h"
#include "textures.h"
#include <stdbool.h>
#include "map.h"

typedef struct Player {
	float x;
	float y;
	float walkSpeed;
	float turnSpeed;
	float rotationAngle;
	uint8_t width;
	uint8_t height;
	int8_t turnDirection; // -1 for left +1 for right 
	int8_t walkDirection; // -1 for left +1 for right
} Player;

typedef struct Ray {
	float rayAngle;
	float wallHitX;
	float wallHitY;
	float distance;
	bool wasHitVertical;
	uint8_t wallHitContent;		
} Ray;

static bool isGameRunning = false;
static uint32_t ticksLastFrame;
static Player player;
static Ray rays[NUM_RAYS];

static float distanceBetweenPoints(float x1, float y1, float x2, float y2) {
	return sqrtf(((x2 - x1) * (x2 - x1)) + ((y2 - y1) * (y2 - y1))); 
}


static void setup(void) {
	player = (Player) {
		WINDOW_WIDTH >> 1,
		WINDOW_HEIGHT >> 1,
		100,
		DEG_TO_RAD(90),
		PI / 2,
		0,
		PI / 2,
		0,
		0,
	};
	
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

static void movePlayer(float deltaTime) {
	player.rotationAngle += player.turnDirection * player.turnSpeed * deltaTime;
	const float moveStep = player.walkDirection * player.walkSpeed * deltaTime;
	const float newPlayerX = player.x + cosf(player.rotationAngle) * moveStep;
	const float newPlayerY = player.y + sinf(player.rotationAngle) * moveStep;

	// Check collision
	if (mapHasWallAt(newPlayerX, newPlayerY)) {
		return;
	}
	
	// Update position if no collision
	player.x = newPlayerX;
	player.y = newPlayerY;
}

static inline void normalizeAngle(float *angle) {
	*angle = remainderf(*angle, TWO_PI);
	if (*angle < 0) {
		*angle = TWO_PI + *angle;
	}
}

static inline void castRay(float rayAngle, int stripId) {
	normalizeAngle(&rayAngle);
	const bool isRayFacingDown = rayAngle > 0 && rayAngle < PI;
    const bool isRayFacingUp = !isRayFacingDown;
    const bool isRayFacingRight = rayAngle < 0.5 * PI || rayAngle > 1.5 * PI;
    const bool isRayFacingLeft = !isRayFacingRight;
    
    float xintercept, yintercept;
    float xstep, ystep;

    ///////////////////////////////////////////
    // HORIZONTAL RAY-GRID INTERSECTION CODE
    ///////////////////////////////////////////
    bool foundHorzWallHit = false;
    uint8_t horzWallContent = 0;
	float horzWallHitX = 0;
    float horzWallHitY = 0;
    

    // Find the y-coordinate of the closest horizontal grid intersection
    yintercept = floorf(player.y / TILE_SIZE) * TILE_SIZE;
    yintercept += isRayFacingDown ? TILE_SIZE : 0;

    // Find the x-coordinate of the closest horizontal grid intersection
    xintercept = player.x + (yintercept - player.y) / tanf(rayAngle);

    // Calculate the increment xstep and ystep
    ystep = TILE_SIZE;
    ystep *= isRayFacingUp ? -1 : 1;

    xstep = TILE_SIZE / tanf(rayAngle);
    xstep *= (isRayFacingLeft && xstep > 0) ? -1 : 1;
    xstep *= (isRayFacingRight && xstep < 0) ? -1 : 1;

    float nextHorzTouchX = xintercept;
    float nextHorzTouchY = yintercept;

    // Increment xstep and ystep until we find a wall
    while (nextHorzTouchX >= 0 && nextHorzTouchX <= (MAP_NUM_COLS * TILE_SIZE) && nextHorzTouchY >= 0 && nextHorzTouchY <= (MAP_NUM_ROWS * TILE_SIZE)) {
        float xToCheck = nextHorzTouchX;
        float yToCheck = nextHorzTouchY + (isRayFacingUp ? -1 : 0);
        
        if (mapHasWallAt(xToCheck, yToCheck)) {
            // found a wall hit
            horzWallHitX = nextHorzTouchX;
            horzWallHitY = nextHorzTouchY;
            horzWallContent = map[(uint8_t)floorf(yToCheck / TILE_SIZE)][(uint8_t)floorf(xToCheck / TILE_SIZE)];
            foundHorzWallHit = true;
            break;
        } else {
            nextHorzTouchX += xstep;
            nextHorzTouchY += ystep;
        }
    }
    
    ///////////////////////////////////////////
    // VERTICAL RAY-GRID INTERSECTION CODE
    ///////////////////////////////////////////
    bool foundVertWallHit = false;
    uint8_t vertWallContent = 0;
	float vertWallHitX = 0;
    float vertWallHitY = 0;

    // Find the x-coordinate of the closest horizontal grid intersection
    xintercept = floor(player.x / TILE_SIZE) * TILE_SIZE;
    xintercept += isRayFacingRight ? TILE_SIZE : 0;

    // Find the y-coordinate of the closest horizontal grid intersection
    yintercept = player.y + (xintercept - player.x) * tanf(rayAngle);

    // Calculate the increment xstep and ystep
    xstep = TILE_SIZE;
    xstep *= isRayFacingLeft ? -1 : 1;

    ystep = TILE_SIZE * tanf(rayAngle);
    ystep *= (isRayFacingUp && ystep > 0) ? -1 : 1;
    ystep *= (isRayFacingDown && ystep < 0) ? -1 : 1;

    float nextVertTouchX = xintercept;
    float nextVertTouchY = yintercept;

    // Increment xstep and ystep until we find a wall
    while (nextVertTouchX >= 0 && nextVertTouchX <= (MAP_NUM_COLS * TILE_SIZE) && nextVertTouchY >= 0 && nextVertTouchY <= (MAP_NUM_ROWS * TILE_SIZE)) {
        float xToCheck = nextVertTouchX + (isRayFacingLeft ? -1 : 0);
        float yToCheck = nextVertTouchY;
        
        if (mapHasWallAt(xToCheck, yToCheck)) {
            // found a wall hit
            vertWallHitX = nextVertTouchX;
            vertWallHitY = nextVertTouchY;
            vertWallContent = map[(uint8_t)floorf(yToCheck / TILE_SIZE)][(uint8_t)floorf(xToCheck / TILE_SIZE)];
            foundVertWallHit = true;
            break;
        } else {
            nextVertTouchX += xstep;
            nextVertTouchY += ystep;
        }
    }

    // Calculate both horizontal and vertical hit distances and choose the smallest one
    float horzHitDistance = foundHorzWallHit ? distanceBetweenPoints(player.x, player.y, horzWallHitX, horzWallHitY) : FLT_MAX;
    float vertHitDistance = foundVertWallHit ? distanceBetweenPoints(player.x, player.y, vertWallHitX, vertWallHitY) : FLT_MAX;

    if (vertHitDistance < horzHitDistance) {
        rays[stripId].distance = vertHitDistance;
        rays[stripId].wallHitX = vertWallHitX;
        rays[stripId].wallHitY = vertWallHitY;
        rays[stripId].wallHitContent = vertWallContent;
        rays[stripId].wasHitVertical = true;
    } else {
        rays[stripId].distance = horzHitDistance;
        rays[stripId].wallHitX = horzWallHitX;
        rays[stripId].wallHitY = horzWallHitY;
        rays[stripId].wallHitContent = horzWallContent;
        rays[stripId].wasHitVertical = false;
    }
    rays[stripId].rayAngle = rayAngle;
}

static void castAllRays() {
	// Start first ray subtracting half of our FOV
	int halfnrays = NUM_RAYS >> 1;
	for (int col = 0; col < NUM_RAYS; col++) {
		float rayAngle = player.rotationAngle + atanf((col - halfnrays) / DIST_PROJ_PLANE);
		castRay(rayAngle, col);
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

static void renderRays(void) {
	// SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	// for (int i = 0; i < NUM_RAYS; i++) {
	// 	SDL_RenderDrawLine(
	// 		renderer, 
	// 	MINIMAP_SCALE_FACTOR * player.x, 
	// 	MINIMAP_SCALE_FACTOR * player.y, 
	// 	MINIMAP_SCALE_FACTOR * rays[i].wallHitX, 
	// 	MINIMAP_SCALE_FACTOR * rays[i].wallHitY);
	// }
}

static void renderPlayer(void) {
	// SDL_SetRenderDrawColor(renderer, 255, 200, 255, 255);
	// SDL_Rect playerRectangle = {
	// 	MINIMAP_SCALE_FACTOR * player.x,
	// 	MINIMAP_SCALE_FACTOR * player.y,
	// 	MINIMAP_SCALE_FACTOR * player.width,
	// 	MINIMAP_SCALE_FACTOR * player.height
	// };
	// SDL_RenderFillRect(renderer, &playerRectangle);
	// SDL_RenderDrawLine(renderer, 
	// 	MINIMAP_SCALE_FACTOR * player.x,
	// 	MINIMAP_SCALE_FACTOR * player.y,
	// 	MINIMAP_SCALE_FACTOR * player.x + cosf(player.rotationAngle) * 40,
	// 	MINIMAP_SCALE_FACTOR * player.y + sinf(player.rotationAngle) * 40
	// );
}

static void renderWallProjection(void) {
	for (int x = 0; x < NUM_RAYS; x++) {
		// Calculate perpendicular distance to avoid fisheye effect
		float perpDistance = rays[x].distance * cosf(rays[x].rayAngle - player.rotationAngle);

		float projectedWallHeight = (TILE_SIZE / perpDistance) * DIST_PROJ_PLANE;

		int wallStripHeight = (int) projectedWallHeight;

		// Calculate wall top pixel. If less than 0 (meaning we are super close) set to 0
		int wallTopPixel = (WINDOW_HEIGHT >> 1) - (wallStripHeight >> 1);
		if (wallTopPixel < 0) {
			wallTopPixel = 0;
		} else {
			// Draw any ceiling in view
			for (int y = 0; y < wallTopPixel; y++) {
				drawPixel(x, y, 0xFF333333);
			}
		}

		// Calculate wall bottom pixel. If larger than window height (meaning we are super close)
		// set to window height
		int wallBottomPixel = (WINDOW_HEIGHT >> 1) + (wallStripHeight >> 1);
		if (wallBottomPixel > WINDOW_HEIGHT) {
			wallBottomPixel = WINDOW_HEIGHT;
		} else {
			// Draw any floor in view
			for (int y = wallBottomPixel; y < WINDOW_HEIGHT; y++) {
				drawPixel(x, y, 0xFF777777);
			}
		}

		int textureOffsetX;

		// Calculaye texture offset x
		if(rays[x].wasHitVertical) {
			// Perform offset for the vertical hit
			textureOffsetX = (int) rays[x].wallHitY % TILE_SIZE;
		} else {
			// Perform offset for the horisontal hit
			textureOffsetX = (int) rays[x].wallHitX % TILE_SIZE;
		}
		
		// Get the correct texture id number from the map content
		uint8_t texNum = rays[x].wallHitContent - 1;

		const texture_t *wallTexture = getTextureAt(texNum);

		// Draw the vertical strip (e.g wall slice)
		for (int y = wallTopPixel; y < wallBottomPixel; y++) {
			// Calculate textureOffsetY, multiply by texture width / wallStrip height to translate texture to height of wall strip on the screen
			int distanceFromTop = (y + (wallStripHeight >> 1)) - (WINDOW_HEIGHT >> 1);
			int textureOffsetY = distanceFromTop * ((float) wallTexture->height / wallStripHeight);

			// set the color of the wall based on the color from the texture
			uint32_t texelColor = wallTextures[texNum].textureBuffer[(wallTexture->width * textureOffsetY) + textureOffsetX];
			drawPixel(x, y, texelColor);
		}
	}
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

	return 0;
}