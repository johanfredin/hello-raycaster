#include "textures.h"
#include <stdio.h>
#include <stdlib.h>
#include "defs.h"
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

upng_t *textures[NUM_TEXTURES];

void loadTextures(void) {
    for (int i = 0; i < NUM_TEXTURES; i++) {
        upng_t *upng = upng_new_from_file(textureFileNames[i]);
        if(upng == NULL) {
            fprintf(stderr, "Could not load png=%s\n", textureFileNames[i]);
            exit(EXIT_FAILURE);
        }
        upng_decode(upng);
        if (upng_get_error(upng) == UPNG_EOK) {
            textures[i] = upng;
            printf("png=%s decoded\n", textureFileNames[i]);
        } else {
            fprintf(stderr, "Could not decode png=%s\n", textureFileNames[i]);
            exit(EXIT_FAILURE);
        }
    }
}

void freeTextures(void) {
    for (int i = 0; i < NUM_TEXTURES; i++) {
        upng_free(textures[i]);
    }
}