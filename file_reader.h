#ifndef FILE_READER_H
#define FILE_READER_H

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int src;
    int dst;
    int weight;
} EdgeInput;

typedef struct {
    int src;
    int dst;
} TravelerInput;

typedef struct {
    int n;
    int m;
    EdgeInput* edges;

    int traveler_count;
    TravelerInput* travelers;
} GraphData;

GraphData* read_file(const char* filename);
void free_graph_data(GraphData* data);

#endif