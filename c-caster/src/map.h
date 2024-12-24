#ifndef MAP_H
#define MAP_H

#include <stdbool.h>

#define MAP_NUM_ROWS 13
#define MAP_NUM_COLS 20

bool mapHasWallAt(float x, float y);
void renderMapGrid(void);
int getMapAt(int x, int y);
bool isInsideMap(float x, float y);

#endif