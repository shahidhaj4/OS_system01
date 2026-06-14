#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>

#define MAX_NODES 100
#define INF INT_MAX

typedef struct Edge {
    int to;
    int weight;
    struct Edge* next;
} Edge;

typedef struct {
    Edge* head;
} Node;

// Shared state and functions
extern Node graph[MAX_NODES];
extern int num_nodes, num_edges;
extern int start_node, end_node;

void add_edge(int u, int v, int w);
void dijkstra(int start, int end, int* dist, int* parent);
void print_path(int* parent, int curr);
void free_graph();
bool load_graph(const char* filename);

#endif