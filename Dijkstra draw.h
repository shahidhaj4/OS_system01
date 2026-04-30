#ifndef DIJKSTRA_DRAW_H
#define DIJKSTRA_DRAW_H

#include "graph.h"


void drawDijkstraResult(Graph *g, int *prev, int *dist,
                        int src, int dst,
                        float *x, float *y, float radius);

#endif /* DIJKSTRA_DRAW_H */