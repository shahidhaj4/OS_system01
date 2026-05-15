#ifndef ANIMATION_H
#define ANIMATION_H

#include <sys/types.h>
#include "file_reader.h"

/**
 * Structure to hold and track each individual traveler's state concurrently.
 * This enables the parent process to animate multiple passengers simultaneously
 * inside the main visual GUI loop without blocking execution.
 */
typedef struct {
    pid_t child_pid;          // Process ID of the child process representing this traveler
    int src_node;             // Starting node ID
    int dest_node;            // Final destination node ID
    int* path;                // Array holding the calculated shortest path structure
    int path_size;            // Total number of nodes in the path array
    int current_path_index;   // Current index within the path array
    int current_jump_step;    // Tracks the step count along the current edge (up to Weight W)
    int time_counter;         // Millisecond accumulator used for non-blocking frame timing
    int is_waiting_at_node;   // Flag: 1 if currently experiencing the 1-second node delay, 0 otherwise
    int is_active;            // Flag: 1 if traveler is still moving, 0 if arrived/terminated
    float current_x;          // Visual X-coordinate hook for the rendering logic
    float current_y;          // Visual Y-coordinate hook for the rendering logic
    unsigned char color[4];   // RGBA color array to fulfill the unique color requirement
} Traveler;

/* ---------------- FUNCTION PROTOTYPES ---------------- */

/**
 * Retrieves the weight of a directed edge between two given nodes.
 */
int get_edge_weight(const GraphData* data, int src, int dst);

/**
 * Milestone 4 Requirement: Spawns a dedicated child process for each traveler using fork().
 */
void spawn_traveler_processes(Traveler* travelers, int num_travelers);

/**
 * Progresses the animation state of a single traveler frame-by-frame (Non-blocking).
 */
void update_traveler_animation(Traveler* traveler, const GraphData* data);

/**
 * Parent process cleanup routine: reaps all signaled child processes using waitpid().
 */
void wait_for_all_children(Traveler* travelers, int num_travelers);

#endif // ANIMATION_H