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
static uint32_t *colorBuffer = NULL;
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
	colorBuffer = (uint32_t *)calloc(WINDOW_WIDTH * WINDOW_HEIGHT, sizeof(uint32_t));
	colorBufferTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, WINDOW_WIDTH, WINDOW_HEIGHT);
	return true;
}

void renderColorBuffer(void) {
	// Pitch = the amount of bytes per row
	SDL_UpdateTexture(
        colorBufferTexture, 
        NULL, 
        colorBuffer, 
        (int) WINDOW_WIDTH * sizeof(uint32_t)
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

void drawRect(int x, int y, int w, int h, color_t color) {
    for (int i = x; i <= (x + w); i++) {
        for (int j = y; j < (y + h); j++) {
            drawPixel(i, j, color);
        }
    }
}

void drawLine(int x0, int y0, int x1, int y1, color_t color) {
	const int deltaX = (x1 - x0);
	const int deltaY = (y1 - y0);
	const int absDx = abs(deltaX);
	const int absDy = abs(deltaY);
	const int sideLength = absDx > absDy ? absDx : absDy;

	// Find out how much we should increment in x and y each step
	const float xInc = deltaX / (float) sideLength;
	const float yInc = deltaY / (float) sideLength;

	float currentX = x0;
	float currentY = y0;
	for (int i = 0; i <= sideLength; i++) {
		drawPixel(roundf(currentX), roundf(currentY), color);
		currentX += xInc;
		currentY += yInc;
	}
}