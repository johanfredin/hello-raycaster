#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>

typedef struct player_t {
	float x;
	float y;
	float walkSpeed;
	float turnSpeed;
	float rotationAngle;
	uint8_t width;
	uint8_t height;
	int8_t turnDirection: 4; // -1 for left +1 for right 
	int8_t walkDirection: 4; // -1 for left +1 for right
} player_t;

extern player_t player;

void movePlayer(float deltaTime);
void renderPlayer(void);

#endif