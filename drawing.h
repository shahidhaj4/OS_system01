#ifndef DRAWING_H
#define DRAWING_H

#include "graph.h"
#include "raylib.h"

#ifndef NODE_R
#define NODE_R 22
#endif

#define MAX_TRAVELERS 10

extern const Color TRAVELER_COLORS[MAX_TRAVELERS];

void drawEntity(float px, float py, Color color, int travelerIndex);
void drawArrivalMessage(int travelerIndex, int dst, int totalWeight,
                        const char *stationNames[]);

void drawTraveler(int travelerIndex, float px, float py,
                  int dst, int arrived, int totalWeight,
                  const char *stationNames[]);

#endif /* DRAWING_H */
