#include "player.h"

#include <math.h>
#include <stdint.h>
#include "defs.h"
#include "graphics.h"
#include "map.h"
#include "utils.h"

player_t player = {
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

void movePlayer(float deltaTime) {
	player.rotationAngle += player.turnDirection * player.turnSpeed * deltaTime;
	normalizeAngle(&player.rotationAngle);
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

void renderMapPlayer(void) {
	drawRect(
		MINIMAP_SCALE_FACTOR * player.x,
		MINIMAP_SCALE_FACTOR * player.y,
		player.width,
		player.height,
        0xFFFFFFFF
    );
}