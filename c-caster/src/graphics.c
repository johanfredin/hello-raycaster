#include "graphics.h"
#include <math.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include "defs.h"

#define FULL_SCREEN 1

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static color_t *colorBuffer = NULL;
static SDL_Texture *colorBufferTexture = NULL;

bool initializeWindow(void) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "Error initializing SDL\n");
		return false;
	}

	SDL_DisplayMode displayMode;
	SDL_GetCurrentDisplayMode(0, &displayMode);

	const int w = FULL_SCREEN ? displayMode.w : WINDOW_WIDTH;
	const int h = FULL_SCREEN ? displayMode.h : WINDOW_HEIGHT;

	window = SDL_CreateWindow(
		"JayCaster", 
		SDL_WINDOWPOS_CENTERED, 
		SDL_WINDOWPOS_CENTERED, 
		w, 
		h, 
		SDL_WINDOW_BORDERLESS
	);
	if (!window) {
		fprintf(stderr, "Error creating sdl window SDL\n");
		return false;
	}

	renderer = SDL_CreateRenderer(window, SDL_DEFAULT_DRIVER, 0);
	if (!renderer) {
		fprintf(stderr, "Error creating sdl renderer SDL\n");
		return false;
	}

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Allocate the total amount of bytes to hold our color buffer
	colorBuffer = (color_t *)calloc(WINDOW_WIDTH * WINDOW_HEIGHT, sizeof(color_t));
	colorBufferTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, WINDOW_WIDTH, WINDOW_HEIGHT);
	return true;
}

void renderColorBuffer(void) {
	// Pitch = the amount of bytes per row
	SDL_UpdateTexture(
        colorBufferTexture, 
        NULL, 
        colorBuffer, 
        (int) WINDOW_WIDTH * sizeof(color_t)
    );
	SDL_RenderCopy(renderer, colorBufferTexture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void clearColorBuffer(color_t clearColor) {
	for (int i = 0; i < (WINDOW_WIDTH * WINDOW_HEIGHT); i++) {
		colorBuffer[i] = clearColor;
	}
}

void destroyWindow(void) {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_DestroyTexture(colorBufferTexture);
	free(colorBuffer);
	SDL_Quit();
}

void drawPixel(int x, int y, color_t color) {
    colorBuffer[(WINDOW_WIDTH * y) + x] = color;
}

void drawRect(int x, int y, int width, int height, color_t color) {
    for (int i = x; i <= (x + width); i++) {
        for (int j = y; j <= (y + height); j++) {
            drawPixel(i, j, color);
        }
    }
}

void drawLine(int x0, int y0, int x1, int y1, color_t color) {
    int deltaX = (x1 - x0);
    int deltaY = (y1 - y0);

    int longestSideLength = (abs(deltaX) >= abs(deltaY)) ? abs(deltaX) : abs(deltaY);

    float xIncrement = deltaX / (float)longestSideLength;
    float yIncrement = deltaY / (float)longestSideLength;

    float currentX = x0;
    float currentY = y0;

    for (int i = 0; i < longestSideLength; i++) {
        drawPixel(round(currentX), round(currentY), color);
        currentX += xIncrement;
        currentY += yIncrement;
    }
}

void changeColorIntensity(color_t *color, float factor) {
	color_t a = (*color & 0xFF000000);
	color_t r = (*color & 0x00FF0000) * factor;
	color_t g = (*color & 0x0000FF00) * factor;
	color_t b = (*color & 0x000000FF) * factor;

	*color = a | (r & 0x00FF0000) | (g & 0x0000FF00) | (b & 0x000000FF);
}