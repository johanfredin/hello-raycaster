#include "textures.h"
#include <stdio.h>
#include <stdlib.h>
#include "constants.h"
#include "upng.h"

static const char *textureFileNames[NUM_TEXTURES] = {
    "./images/redbrick.png",
    "./images/purplestone.png",
    "./images/mossystone.png",
    "./images/graystone.png",
    "./images/colorstone.png",
    "./images/bluestone.png",
    "./images/wood.png",
    "./images/eagle.png",
    "./images/pikuma.png"
};

texture_t wallTextures[NUM_TEXTURES];

void loadWallTextures(void) {
    for (int i = 0; i < NUM_TEXTURES; i++) {
        upng_t *upng = upng_new_from_file(textureFileNames[i]);
        if(upng == NULL) {
            fprintf(stderr, "Could not load png=%s\n", textureFileNames[i]);
            exit(1);
        }
        upng_decode(upng);
        if (upng_get_error(upng) == UPNG_EOK) {
            wallTextures[i].upng = upng;
            wallTextures[i].width = upng_get_width(upng);
            wallTextures[i].height = upng_get_height(upng);
            wallTextures[i].textureBuffer = (uint32_t *) upng_get_buffer(upng);
            printf("png=%s decoded\n", textureFileNames[i]);
        } else {
            fprintf(stderr, "Could not decode png=%s\n", textureFileNames[i]);
            exit(1);
        }
    }
}

texture_t *getTextureAt(int i) {
    return &wallTextures[i];
}

void freeWallTextures(void) {
    for (int i = 0; i < NUM_TEXTURES; i++) {
        upng_free(wallTextures[i].upng);
    }
}