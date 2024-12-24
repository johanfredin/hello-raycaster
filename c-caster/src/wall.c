

#include "wall.h"
#include "graphics.h"
#include "player.h"
#include <math.h>
#include "ray.h"
#include "textures.h"
#include "upng.h"


void renderWallProjection(void) {
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
		
		// Query the width and height from the upng
		upng_t *texture = textures[texNum];
		int textureWidth = upng_get_width(texture);
		int textureHeight = upng_get_height(texture);

		// Draw the vertical strip (e.g wall slice)
		for (int y = wallTopPixel; y < wallBottomPixel; y++) {
			// Calculate textureOffsetY, multiply by texture width / wallStrip height to translate texture to height of wall strip on the screen
			int distanceFromTop = (y + (wallStripHeight >> 1)) - (WINDOW_HEIGHT >> 1);
			int textureOffsetY = distanceFromTop * ((float) textureHeight / wallStripHeight);

			// set the color of the wall based on the color from the texture
			color_t *wallTextureBuffer = (color_t*) upng_get_buffer(texture);
			color_t texelColor = wallTextureBuffer[(textureWidth * textureOffsetY) + textureOffsetX];
			if(rays[x].wasHitVertical) {
				changeColorIntensity(&texelColor, 0.7f);
			}
			drawPixel(x, y, texelColor);
		}
	}
}
