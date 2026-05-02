#include <stdio.h>
#include <unistd.h>
#include "animation.h"
#include "file_reader.h"

// Static variable to manage animation state (Play/Stop)
static int is_running = 1;

/**
 * Get edge weight between two nodes (DIRECTED GRAPH).
 * Added 'const' to address the IDE warning.
 */
int get_edge_weight(const GraphData* data, int src, int dst) {
    for (int i = 0; i < data->m; i++) {
        // Check if current edge matches source and destination
        if (data->edges[i].src == src && data->edges[i].dst == dst) {
            return data->edges[i].weight;
        }
    }
    return 1; // Default weight if not found
}

/**
 * Moves the entity along a single edge.
 * Requirement: Split edge weight W into W jumps, each taking 300ms.
 */
void move_on_edge(int weight) {
    int steps = (weight > 0) ? weight : 1;

    for (int i = 1; i <= steps; i++) {
        if (!is_running) break; // Check if animation was stopped

        printf("  -> Animation: Step %d of %d (300ms jump)\n", i, steps);

        // Integration: Update visual position here
        usleep(300 * 1000);
    }
}

/**
 * Handles waiting behavior at nodes.
 * Requirement: 1-second full stop at intermediate nodes.
 */
void wait_at_node(int node_id) {
    printf("  -> Node %d: Waiting for 1 second\n", node_id);
    usleep(1000 * 1000);
}

/**
 * Main animation logic.
 * Added 'const' to 'path' to address the IDE warning.
 */
void run_animation_logic(const int* path, int path_size, const GraphData* data) {
    printf("\n[ANIMATION START]\n");

    for (int i = 0; i < path_size; i++) {
        int current_node = path[i];

        // 1. Wait at intermediate nodes (1 second)
        if (i > 0 && i < path_size - 1) {
            wait_at_node(current_node);
        }

        // 2. Move to the next node if it exists
        if (i < path_size - 1) {
            int next_node = path[i + 1];

            // Using your real function to get the actual weight
            int weight = get_edge_weight(data, current_node, next_node);

            printf("Moving from Node %d to Node %d (Weight: %d)\n", current_node, next_node, weight);
            move_on_edge(weight);
        }
    }

    printf("[ANIMATION END]\n");
}