#ifndef SPRITE_H
#define SPRITE_H

#include <stdbool.h>
#include <stdint.h>

typedef struct sprite_t {
	float x;
	float y;
	float distance;
	float angle;
	bool visible;
	uint8_t textureIndex;
} sprite_t;

void renderSpriteProjection(void);
void renderMapSprites(void);

#endif