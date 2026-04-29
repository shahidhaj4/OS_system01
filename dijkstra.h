#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "graph.h"

void dijkstra(Graph *g, int src, int *dist, int *prev);
void printPath(int *prev, int src, int node);
int buildPathMask(int *prev, int src, int dst, int n, int *onPath);
void drawDijkstraResult(Graph *g, int *prev, int *dist,
                        int src, int dst,
                        float *x, float *y, float radius);

#endif /* DIJKSTRA_H */
