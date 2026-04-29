#ifndef FILE_READER_H
#define FILE_READER_H

#include <stdio.h>
#include <stdlib.h>

/**
 * Structure to represent a single edge read from the file.
 * This is used to temporarily store edge data before graph construction.
 */
typedef struct {
    int src;    // Source node index
    int dst;    // Destination node index
    int weight; // Edge weight (must be non-negative)
} EdgeInput;

/**
 * Main data structure to be passed to Student 3 (Graph Construction) and Student 1 (GUI).
 * Milestone 2 Constraint: Maximum of 15 nodes for clear visualization.
 */
typedef struct {
    int n;             // Total number of nodes (N), Max: 15
    int m;             // Total number of edges (M)
    EdgeInput* edges;  // Array to store all M edges
    int start;         // Source node for Dijkstra algorithm
    int end;           // Destination node for Dijkstra algorithm
} GraphData;

/**
 * Reads the graph data and query from a text file.
 * Includes validation for file integrity, negative weights, and node limits.
 * * @param filename: Path to the input file.
 * @return: Pointer to GraphData structure, or NULL if an error occurs.
 */
GraphData* read_file(const char* filename);

/**
 * Frees all memory allocated for the GraphData structure.
 * Essential for preventing memory leaks (Standard in Operating Systems projects).
 * * @param data: Pointer to the GraphData structure to be freed.
 */
void free_graph_data(GraphData* data);

#endif