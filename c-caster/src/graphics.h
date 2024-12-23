#ifndef GRAPHICS_H 
#define GRAPHICS_H

#include <stdbool.h>
#include <stdint.h>

bool initializeWindow(void);
void renderColorBuffer(void);
void clearColorBuffer(uint32_t clearColor);
void destroyWindow(void);
void drawPixel(int x, int y, uint32_t color);
void drawRect(int x, int y, int w, int h, uint32_t color);

#endif