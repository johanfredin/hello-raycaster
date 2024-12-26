#include "sprite.h"
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include "defs.h"
#include "graphics.h"
#include "player.h"
#include "ray.h"
#include "textures.h"
#include "upng.h"
#include "utils.h"

#define NUM_SPRITES 3
#define TEXTURE_BARREL 9

static const float HALF_FOV = FOV / 2;

static sprite_t sprites[NUM_SPRITES] = {
    {.x = 640, .y = 630, .textureIndex = TEXTURE_BARREL},
    {.x = 250, .y = 600, .textureIndex = 11},
    {.x = 300, .y = 400, .textureIndex = 12},
};

static inline bool isWithinWindowBounds(int x, int y) {
    return x > 0 & x < WINDOW_WIDTH & y > 0 & y < WINDOW_HEIGHT;
}

void renderMapSprites(void) {
    for (int i = 0; i < NUM_SPRITES; i++) {
        drawRect(
            sprites[i].x * MINIMAP_SCALE_FACTOR,
            sprites[i].y * MINIMAP_SCALE_FACTOR,
            2,
            2,
            (sprites[i].visible) ? 0xFF00FFFF : 0xFF444444
        );
    }
}

static int compareSpriteDistance(const void *elem1, const void *elem2) {
	sprite_t *s1 = (sprite_t *)elem1;
	sprite_t *s2 = (sprite_t *)elem2;
    return s2->distance - s1->distance;
}

void renderSpriteProjection(void) {
    sprite_t visibleSprites[NUM_SPRITES];
    uint8_t numVisibleSprites = 0;

    // Find sprites that are visible (inside our FOV)
    for (int i = 0; i < NUM_SPRITES; i++) {
        float angleSpritePlayer = player.rotationAngle - atan2f(sprites[i].y - player.y, sprites[i].x - player.x);

        // Make sure the angle is always between 0 and 180 degrees
        if (angleSpritePlayer > PI) {
            angleSpritePlayer -= TWO_PI;
        }
        if (angleSpritePlayer < -PI) {
            angleSpritePlayer += TWO_PI;
        }
            
        angleSpritePlayer = fabsf(angleSpritePlayer);

        // If sprite angle is less than half the FOV plus a small error margin
		const float EPSILON = 0.2;
        if (angleSpritePlayer < HALF_FOV + EPSILON) {
            sprites[i].visible = true;
            sprites[i].angle = angleSpritePlayer;
            sprites[i].distance = distanceBetweenPoints(sprites[i].x, sprites[i].y, player.x, player.y);
			visibleSprites[numVisibleSprites] = sprites[i];
            numVisibleSprites++;
        } else {
            sprites[i].visible = false;
        }
    }

    // Sort the sprites based on distance
    qsort(visibleSprites, numVisibleSprites, sizeof(sprite_t), compareSpriteDistance);

    // Draw the visible sprites
    for (int i = 0; i < numVisibleSprites; i++) {
        sprite_t sprite = visibleSprites[i];

        // Calculate perpendicular distance to avoid fisheye effect
		const float perpDistance = sprite.distance * cosf(sprite.angle);

        // Calculate the projected sprite height and width (the same, as sprites are squared)
        float spriteHeight = (TILE_SIZE / perpDistance) * DIST_PROJ_PLANE;
        float spriteWidth = spriteHeight;

        float spriteTopY = ((float) WINDOW_HEIGHT / 2) - (spriteHeight / 2);
        if (spriteTopY < 0) {
            spriteTopY = 0;
        }

        float spriteBottomY = ((float) WINDOW_HEIGHT / 2) + (spriteHeight / 2);
        if (spriteBottomY > WINDOW_HEIGHT) {
            spriteBottomY = WINDOW_HEIGHT;
        }

        // Calculate the sprite x position in the projection plane
        float spriteAngle = atan2f(sprite.y - player.y, sprite.x - player.x) - player.rotationAngle;
        float spriteScreenPosX = tanf(spriteAngle) * DIST_PROJ_PLANE;

        float spriteLeftX = ((float) WINDOW_WIDTH / 2) + spriteScreenPosX - (spriteWidth / 2);
        float spriteRightX = spriteLeftX + spriteWidth;

        // Query the width and the height of the texture
        int textureWidth = upng_get_width(textures[sprite.textureIndex]);
		int textureHeight = upng_get_height(textures[sprite.textureIndex]);

		for (int x = spriteLeftX; x < spriteRightX; x++) {
			float texelWidth = (textureWidth / spriteWidth);
			int textureOffsetX = (x - spriteLeftX) * texelWidth;

			for (int y = spriteTopY; y < spriteBottomY; y++) {
				if (x > 0 && x < WINDOW_WIDTH && y > 0 && y < WINDOW_HEIGHT) {
					int distanceFromTop = y + (spriteHeight / 2) - ((float)WINDOW_HEIGHT / 2);
					int textureOffsetY = distanceFromTop * (textureHeight / spriteHeight);

					color_t *spriteTextureBuffer = (color_t *)upng_get_buffer(textures[sprite.textureIndex]);
					color_t texelColor = spriteTextureBuffer[(textureWidth * textureOffsetY) + textureOffsetX];

					if (sprite.distance < rays[x].distance && texelColor != 0xFFFF00FF) {
						drawPixel(x, y, texelColor);
					}
				}
			}
		}
	}   
}