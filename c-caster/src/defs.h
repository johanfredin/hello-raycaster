#ifndef DEFS_H
#define DEFS_H


#define PI 3.14159265
#define TWO_PI 6.28318530

#define TILE_SIZE 64

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 800

#define FOV (60 * (PI / 180))

#define DIST_PROJ_PLANE ((WINDOW_WIDTH >> 1) / tanf(FOV / 2))

#define MINIMAP_SCALE_FACTOR 0.2

#define FPS 30
#define FRAME_TIME_LENGTH  (1000 / FPS)

#define SDL_DEFAULT_DRIVER -1

#define DEG_TO_RAD(deg) (deg) * (PI / 180)

#define NUM_TEXTURES 14

#endif