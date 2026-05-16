#ifndef DRAWING_H
#define DRAWING_H

#include "graph.h"
#include "raylib.h"
#include "animation.h"

#ifndef NODE_R
#define NODE_R 22
#endif

#define MAX_TRAVELERS   10
#define MAX_LOG_LINES   12
#define MAX_LOG_LINE_LEN 128

extern const Color TRAVELER_COLORS[MAX_TRAVELERS];


void addLogLine(const char *line);

void drawLogPanel(void);

void drawEntity(float px, float py, Color color, int travelerIndex);
void drawArrivalMessage(int travelerIndex, int dst, int totalWeight,
                        const char *stationNames[]);

void drawTraveler(int travelerIndex, float px, float py,
                  int dst, int arrived, int totalWeight,
                  const char *stationNames[]);

void drawGraph(Graph *g, float *x, float *y,
               float radius, const char *stationNames[]);

#endif /* DRAWING_H */
