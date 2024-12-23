#ifndef TEXTURES_H
#define TEXTURES_H

#include <stdint.h>
#include "defs.h"
#include "upng.h"

typedef struct texture_t {
    upng_t *upng;
    uint16_t width;
    uint16_t height;
    uint32_t *textureBuffer;
} texture_t;

extern texture_t wallTextures[NUM_TEXTURES];

void loadWallTextures(void);

void freeWallTextures(void);

texture_t *getTextureAt(int i);

#endif