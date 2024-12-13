#ifndef CONSTANTS_H
#define CONSTANTS_H


#define PI 3.14159265
#define TWO_PI 6.28318530

#define TILE_SIZE 64
#define MAP_NUM_ROWS 13
#define MAP_NUM_COLS 20

#define FALSE 0
#define TRUE 1
#define WINDOW_WIDTH (MAP_NUM_COLS * TILE_SIZE)
#define WINDOW_HEIGHT (MAP_NUM_ROWS * TILE_SIZE)

#define FOV (60 * (PI / 180))
#define NUM_RAYS WINDOW_WIDTH

#define MINIMAP_SCALE_FACTOR 1.0

#define FPS 30
#define FRAME_TIME_LENGTH  (1000 / FPS)

#define SDL_DEFAULT_DRIVER -1

#define DEG_TO_RAD(deg) (deg) * (PI / 180)

#endif