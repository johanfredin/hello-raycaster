#ifndef GRAPHICS_H 
#define GRAPHICS_H

#include <stdbool.h>
#include <stdint.h>

typedef uint32_t color_t; 

bool initializeWindow(void);
void renderColorBuffer(void);
void clearColorBuffer(color_t clearColor);
void destroyWindow(void);
void drawPixel(int x, int y, color_t color);
void drawRect(int x, int y, int w, int h, color_t color);

void drawLine(int x0, int y0, int x1, int y1, color_t color);

#endif