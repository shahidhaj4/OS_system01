#ifndef ANIMATION_H
#define ANIMATION_H

#include "file_reader.h"

/*
 * Prototypes for Milestone 3 - Animation Logic.
 * Using 'const' ensures the data is read-only for safety.
 */

// Finds the weight for a specific edge in the graph
int get_edge_weight(const GraphData* data, int src, int dst);

// Handles the logic for moving along an edge with 300ms intervals
void move_on_edge(int weight);

// Handles the 1-second wait at each intermediate node
void wait_at_node(int node_id);

// Orchestrates the movement through the entire path provided by Ghada
void run_animation_logic(const int* path, int path_size, const GraphData* data);

#endif // ANIMATION_H