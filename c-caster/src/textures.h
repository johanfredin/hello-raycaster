#ifndef TEXTURES_H
#define TEXTURES_H

#include <stdint.h>
#include "constants.h"
#include "upng.h"

extern const char *textureFileNames[NUM_TEXTURES];

typedef struct texture_t {
    upng_t *upng;
    uint16_t width;
    uint16_t height;
    uint32_t *textureBuffer;
} texture_t;

extern texture_t wallTextures[NUM_TEXTURES];

void loadWallTextures(void);

#endif