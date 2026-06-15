#ifndef ANIMATION_H
#define ANIMATION_H

#include <sys/types.h>
#include "file_reader.h"


/* ---------------- TRAVELER STATE ---------------- */

typedef enum {
    MOVING,
    WAITING,
    INSIDE_NODE,
    FINISHED
} TravelerState;

/* ---------------- IPC MESSAGE STRUCTURE ---------------- */

typedef struct {
    int traveler_index;
    int arrived_node;
    int next_node;
    int is_finished;
    TravelerState state;
} IPCOverPipeMessage;

/* ---------------- TRAVELER STRUCTURE ---------------- */

typedef struct {

    /* Child process identifier */
    pid_t child_pid;

    /* Pipe descriptors:
       pipe_fd[0] -> read end
       pipe_fd[1] -> write end
    */
    int pipe_fd[2];

    /* Source and destination nodes */
    int src_node;
    int dest_node;

    /* Path information */
    int* path;
    int path_size;

    /* Current movement state */
    int current_node;
    int next_node;

    int current_path_index;
    int current_jump_step;

    /* Timing and animation control */
    int time_counter;
    int is_waiting_at_node;

    /* Active state flag */
    int is_active;

    TravelerState state;

    /* Rendering coordinates */
    float current_x;
    float current_y;

    /* Unique traveler color (RGBA) */
    unsigned char color[4];

} Traveler;

/* ---------------- FUNCTION DECLARATIONS ---------------- */

/* Returns edge weight between two nodes */
int get_edge_weight_from_data(
    const GraphData* data,
    int src,
    int dst
);

/* Creates all traveler child processes */
void spawn_traveler_processes(
    Traveler* travelers,
    int num_travelers,
    const GraphData* data
);

/* Reads incoming IPC pipe updates */
void update_all_travelers_from_pipes(
    Traveler* travelers,
    int num_travelers
);

/* Updates animation state for rendering */
void update_traveler_animation(
    Traveler* traveler,
    const GraphData* data
);

/* Wait for all child processes */
void wait_for_all_children(
    Traveler* travelers,
    int num_travelers
);

#endif // ANIMATION_H