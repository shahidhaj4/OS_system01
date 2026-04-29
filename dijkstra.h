#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "graph.h"

/*
 * Runs Dijkstra's algorithm from src on the given graph.
 *
 * Outputs:
 *   dist[i] = shortest distance from src to node i (INT_MAX if unreachable)
 *   prev[i] = previous node on the shortest path to i (-1 if none)
 */
void dijkstra(Graph *g, int src, int *dist, int *prev);

/*
 * Recursively prints the shortest path from src to node
 * using the prev[] array filled by dijkstra().
 *
 * Example output: 0 -> 2 -> 1 -> 3
 */
void printPath(int *prev, int src, int node);

#endif /* DIJKSTRA_H */
