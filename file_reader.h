#ifndef FILE_READER_H
#define FILE_READER_H

#include <stdio.h>
#include <stdlib.h>


typedef struct {
    int src;    // Source node index
    int dst;    // Destination node index
    int weight; // Edge weight ( must be non-negative )
} EdgeInput;


typedef struct {
    int n;             // Total number of nodes (N), Max: 15
    int m;             // Total number of edges (M)
    EdgeInput* edges;  // Array to store all M edges
    int start;         // Source node for Dijkstra algorithm
    int end;           // Destination node for Dijkstra algorithm
} GraphData;


GraphData* read_file(const char* filename);


void free_graph_data(GraphData* data);

#endif