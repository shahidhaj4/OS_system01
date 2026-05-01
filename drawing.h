#ifndef DRAWING_H
#define DRAWING_H

#include "graph.h"
#include "raylib.h"

#ifndef NODE_R
#define NODE_R 22
#endif



void drawEntity(float px, float py);

void drawArrivalMessage(int dst, int totalWeight, const char *stationNames[]);

void drawAnimationFrame(float px, float py, int dst, int arrived,
                        int totalWeight, const char *stationNames[]);

#endif /* DRAWING_H */
