#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "animation.h"
#include "file_reader.h"

/* ---------------- EDGE WEIGHT UTILITY ---------------- */

/**
 * Retrieves the weight of a directed edge between two given nodes.
 * @param data Pointer to the shared graph structure.
 * @param src Source node ID.
 * @param dst Destination node ID.
 * @return The weight W representing the number of animation jumps, defaults to 1.
 */
int get_edge_weight(const GraphData* data, int src, int dst) {
    if (!data) return 1;

    for (int i = 0; i < data->m; i++) {
        if (data->edges[i].src == src &&
            data->edges[i].dst == dst) {
            return data->edges[i].weight;
        }
    }

    return 1; // Fallback default weight
}

/* ---------------- MULTI-PROCESSING LOGIC (FORK) ---------------- */

/**
 * Signal handler for child processes to intercept SIGTERM.
 * Ensures clean exit reporting to stdout upon termination.
 */
static void handle_sigterm(int sig) {
    (void)sig; // Silence unused parameter compiler warning
    printf("[%d] terminated\n", getpid());
    fflush(stdout);
    _exit(0); // Safe exit within async-signal context
}

/**
 * Milestone 4 Requirement: Spawns a dedicated child process for each traveler.
 * Child processes print their status and yield CPU execution efficiently.
 * @param travelers Array holding the states of all passengers.
 * @param num_travelers Total number of active travelers.
 */
void spawn_traveler_processes(Traveler* travelers, int num_travelers) {

    for (int i = 0; i < num_travelers; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0) {
            /* ----------- CHILD PROCESS BEHAVIOR ----------- */
            // Register signal handler to catch termination signal from parent
            signal(SIGTERM, handle_sigterm);

            // Milestone 4 Requirement: Print PID started instantly
            printf("[%d] started\n", getpid());
            fflush(stdout);

            /* Efficient passive waiting: pause() puts the child process to sleep
               completely without spinning or consuming CPU cycles until a signal arrives. */
            while (1) {
                pause();
            }
        }
        else {
            /* ----------- PARENT PROCESS BEHAVIOR ----------- */
            // Retain the child's PID to manage lifecycle and signaling
            travelers[i].child_pid = pid;
            travelers[i].is_active = 1;
        }
    }
}

/* ---------------- FRAME-BASED SIMULATION LOOP ---------------- */

/**
 * Progresses the animation state of a single traveler frame-by-frame (Non-blocking).
 * Designed to handle multiple passengers concurrently inside the Raylib main GUI loop.
 * @param traveler Pointer to the specific passenger tracking structure.
 * @param data Pointer to the loaded graph map layout.
 */
void update_traveler_animation(Traveler* traveler, const GraphData* data) {
    if (!traveler || !traveler->is_active)
        return;

    /* ---------------- ARRIVAL & SIGNALING CHECK ---------------- */
    // Triggered when the traveler has processed all edges in their calculated path
    if (traveler->current_path_index >= traveler->path_size - 1) {
        traveler->is_active = 0;

        /* Milestone 4 Requirement: Parent intercepts terminal arrival,
           sending a SIGTERM signal to kill the passive child process. */
        if (traveler->child_pid > 0) {
            printf("[PARENT] Traveler finished -> killing PID %d\n", traveler->child_pid);
            kill(traveler->child_pid, SIGTERM);
        }
        return;
    }

    /* ---------------- INTERMEDIATE NODE WAIT ---------------- */
    // Milestone 3/4 Requirement: 1-second full stop halt at intermediate junction nodes
    if (traveler->is_waiting_at_node) {
        traveler->time_counter += 16; // Simulate frame step duration (~60 FPS -> 16.67ms)

        if (traveler->time_counter >= 1000) { // 1000ms threshold reached
            traveler->is_waiting_at_node = 0;
            traveler->time_counter = 0;
        }
        return; // Halt movement progression until waiting interval expires
    }

    /* ---------------- EDGE MOVEMENT CONCURRENCY ---------------- */
    int u = traveler->path[traveler->current_path_index];
    int v = traveler->path[traveler->current_path_index + 1];

    int weight = get_edge_weight(data, u, v);

    traveler->time_counter += 16; // Accumulate time intervals between frames

    // Milestone 3/4 Requirement: Each procedural step jump takes exactly 300ms
    if (traveler->time_counter >= 300) {
        traveler->time_counter = 0;
        traveler->current_jump_step++;

        /* TODO: Coordinate interpolation hook for Shahd's visual rendering logic.
           Example: float t = (float)traveler->current_jump_step / weight; */

        // Check if traveler has accomplished all W jumps along the current edge
        if (traveler->current_jump_step >= weight) {
            traveler->current_jump_step = 0;
            traveler->current_path_index++; // Move to next node layout index

            /* Evaluate next location status: Trigger wait period if not the final destination */
            if (traveler->current_path_index < traveler->path_size - 1) {
                traveler->is_waiting_at_node = 1;
                traveler->time_counter = 0;
            }
        }
    }
}

/* ---------------- PROCESS CLEANUP & REAPING ---------------- */

/**
 * Reaps all terminated child processes to ensure systematic memory reclamation.
 * @param travelers Array holding the passenger metrics and process handles.
 * @param num_travelers Number of tracked traveler objects.
 */
void wait_for_all_children(Traveler* travelers, int num_travelers) {
    for (int i = 0; i < num_travelers; i++) {
        if (travelers[i].child_pid > 0) {
            // Synchronously wait for child process state change to clear zombie entries
            waitpid(travelers[i].child_pid, NULL, 0);
        }
    }
    printf("[PARENT] All child processes reaped.\n");
}