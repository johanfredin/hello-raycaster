#include "ray.h"
#include "defs.h"
#include "graphics.h"
#include "player.h"
#include <math.h>
#include "map.h"
#include <float.h>

static const float HALF_PI = 0.5 * PI;
static const float ONE_POINT_FIVE_PI = 1.5 * PI; 

ray_t rays[NUM_RAYS];

static inline float distanceBetweenPoints(float x1, float y1, float x2, float y2) {
	return sqrtf(((x2 - x1) * (x2 - x1)) + ((y2 - y1) * (y2 - y1))); 
}

static inline void normalizeAngle(float *angle) {
	*angle = remainderf(*angle, TWO_PI);
	if (*angle < 0) {
		*angle = TWO_PI + *angle;
	}
}

static inline bool isRayFacingDown(float *angle) {
    return *angle > 0 & *angle < PI;
}

static inline bool isRayFacingUp(float *angle) {
    return !isRayFacingDown(angle);
}

static inline bool isRayFacingRight(float *angle) {
    return (*angle < HALF_PI) | (*angle > ONE_POINT_FIVE_PI);
}

static inline bool isRayFacingLeft(float *angle) {
    return !isRayFacingRight(angle);
}

static void castRay(float rayAngle, int stripId) {
	normalizeAngle(&rayAngle);
    
    float xintercept, yintercept;
    float xstep, ystep;

    const bool isFacingUp = isRayFacingUp(&rayAngle);
    const bool isFacingDown = !isFacingUp;
    const bool isFacingRight = isRayFacingRight(&rayAngle);
    const bool isFacingLeft = !isFacingRight;

    ///////////////////////////////////////////
    // HORIZONTAL RAY-GRID INTERSECTION CODE
    ///////////////////////////////////////////
    bool foundHorzWallHit = false;
    uint8_t horzWallContent = 0;
	float horzWallHitX = 0;
    float horzWallHitY = 0;

    // Find the y-coordinate of the closest horizontal grid intersection
    yintercept = floorf(player.y / TILE_SIZE) * TILE_SIZE;
    if (isFacingDown) {
        yintercept += TILE_SIZE;
    }

    // Find the x-coordinate of the closest horizontal grid intersection
    xintercept = player.x + (yintercept - player.y) / tanf(rayAngle);

    // Calculate the increment xstep and ystep
    ystep = isFacingUp ? -TILE_SIZE : +TILE_SIZE;

    xstep = TILE_SIZE / tanf(rayAngle);
    if ((isFacingLeft) & (xstep > 0)) {
        xstep = -xstep;
    } else if ((isFacingRight) & (xstep < 0)) {
        xstep = -xstep;
    }

    float nextHorzTouchX = xintercept;
    float nextHorzTouchY = yintercept;

    // Increment xstep and ystep until we find a wall
    while (isInsideMap(nextHorzTouchX, nextHorzTouchY)) {
        float xToCheck = nextHorzTouchX;
        float yToCheck = nextHorzTouchY;
        if (isFacingUp) {
            yToCheck--;
        }
        
        if (mapHasWallAt(xToCheck, yToCheck)) {
            // found a wall hit
            horzWallHitX = nextHorzTouchX;
            horzWallHitY = nextHorzTouchY;
            horzWallContent = getMapAt((uint8_t)floorf(yToCheck / TILE_SIZE), (uint8_t)floorf(xToCheck / TILE_SIZE));
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
    xintercept += isFacingRight ? TILE_SIZE : 0;

    // Find the y-coordinate of the closest horizontal grid intersection
    yintercept = player.y + (xintercept - player.x) * tanf(rayAngle);

    // Calculate the increment xstep and ystep
    xstep = TILE_SIZE;
    xstep *= isFacingLeft ? -1 : 1;

    ystep = TILE_SIZE * tanf(rayAngle);
    ystep *= (isFacingUp && ystep > 0) ? -1 : 1;
    ystep *= (isFacingDown && ystep < 0) ? -1 : 1;

    float nextVertTouchX = xintercept;
    float nextVertTouchY = yintercept;

    // Increment xstep and ystep until we find a wall
	while (isInsideMap(nextVertTouchX, nextVertTouchY)) {
        float xToCheck = nextVertTouchX + (isFacingLeft ? -1 : 0);
        float yToCheck = nextVertTouchY;
        
        if (mapHasWallAt(xToCheck, yToCheck)) {
            // found a wall hit
            vertWallHitX = nextVertTouchX;
            vertWallHitY = nextVertTouchY;
            vertWallContent = getMapAt((uint8_t) floorf (yToCheck / TILE_SIZE), (uint8_t) floorf(xToCheck / TILE_SIZE));
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

void castAllRays(void) {
	// Start first ray subtracting half of our FOV
	int halfnrays = NUM_RAYS >> 1;
	for (int col = 0; col < NUM_RAYS; col++) {
		float rayAngle = player.rotationAngle + atanf((col - halfnrays) / DIST_PROJ_PLANE);
		castRay(rayAngle, col);
	}
}

void renderMapRays(void) {
	for (int i = 0; i < NUM_RAYS; i++) {
        drawLine(
    		MINIMAP_SCALE_FACTOR * player.x, 
	    	MINIMAP_SCALE_FACTOR * player.y, 
		    MINIMAP_SCALE_FACTOR * rays[i].wallHitX, 
		    MINIMAP_SCALE_FACTOR * rays[i].wallHitY,
            0xFF0000FF
        );
	}
}