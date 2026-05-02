#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "graph.h"


void dijkstra(Graph *g, int src, int *dist, int *prev);


void printPath(int *prev, int src, int node);


int buildPathMask(int *prev, int src, int dst, int n, int *onPath);

// Extract path into array (dst -> src)
int extractPath(int *prev, int src, int dst, int *path, int *pathLen);

// Reverse path to (src -> dst)
void reversePath(int *path, int pathLen);

// Step-by-step traversal
int getNextNodeInPath(int *path, int pathLen, int currentIndex);

#endif /* DIJKSTRA_H */