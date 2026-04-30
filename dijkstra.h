#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "graph.h"


void dijkstra(Graph *g, int src, int *dist, int *prev);


void printPath(int *prev, int src, int node);


int buildPathMask(int *prev, int src, int dst, int n, int *onPath);

#endif /* DIJKSTRA_H */