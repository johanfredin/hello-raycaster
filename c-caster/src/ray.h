#ifndef RAY_H
#define RAY_H

#include <stdint.h>
#include <stdbool.h>
#include "defs.h"

#define NUM_RAYS WINDOW_WIDTH

typedef struct ray_t {
	float rayAngle;
	float wallHitX;
	float wallHitY;
	float distance;
	bool wasHitVertical;
	uint8_t wallHitContent;		
} ray_t;

extern ray_t rays[NUM_RAYS];

void castAllRays(void);
void renderRays(void);

#endif