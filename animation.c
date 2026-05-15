#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "animation.h"
#include "file_reader.h"


int get_edge_weight_from_data(const GraphData* data, int src, int dst) {
    if (!data) return 1;

    for (int i = 0; i < data->m; i++) {
        if (data->edges[i].src == src &&
            data->edges[i].dst == dst) {
            return data->edges[i].weight;
        }
    }

    return 1; // Fallback default weight
}


static void handle_sigterm(int sig) {
    (void)sig;
    printf("[%d] terminated\n", getpid());
    fflush(stdout);
    _exit(0);
}


void spawn_traveler_processes(Traveler* travelers, int num_travelers) {
    for (int i = 0; i < num_travelers; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0) {

            signal(SIGTERM, handle_sigterm);

            printf("[%d] started\n", getpid());
            fflush(stdout);

            while (1) {
                pause(); // Passive waiting
            }
        }
        else {

            travelers[i].child_pid = pid;
            travelers[i].is_active = 1;
        }
    }
}

void update_traveler_animation(Traveler* traveler, const GraphData* data) {
    if (!traveler || !traveler->is_active)
        return;

    if (traveler->current_path_index >= traveler->path_size - 1) {
        traveler->is_active = 0;

        if (traveler->child_pid > 0) {
            printf("[PARENT] Traveler finished -> killing PID %d\n", traveler->child_pid);
            kill(traveler->child_pid, SIGTERM);
        }
        return;
    }

    if (traveler->is_waiting_at_node) {
        traveler->time_counter += 16; // ~60 FPS update frame

        if (traveler->time_counter >= 1000) { // 1 second total wait
            traveler->is_waiting_at_node = 0;
            traveler->time_counter = 0;
        }
        return;
    }

    int u = traveler->path[traveler->current_path_index];
    int v = traveler->path[traveler->current_path_index + 1];

    int weight = get_edge_weight_from_data(data, u, v);

    traveler->time_counter += 16;

    if (traveler->time_counter >= 300) {
        traveler->time_counter = 0;
        traveler->current_jump_step++;

        if (traveler->current_jump_step >= weight) {
            traveler->current_jump_step = 0;
            traveler->current_path_index++;

            if (traveler->current_path_index < traveler->path_size - 1) {
                traveler->is_waiting_at_node = 1;
                traveler->time_counter = 0;
            }
        }
    }


    float t = (float)traveler->current_jump_step / (float)weight;


    traveler->current_x = t;
    traveler->current_y = 0.0f;
}




void wait_for_all_children(Traveler* travelers, int num_travelers) {
    for (int i = 0; i < num_travelers; i++) {
        if (travelers[i].child_pid > 0) {
            waitpid(travelers[i].child_pid, NULL, 0);
        }
    }
    printf("[PARENT] All child processes reaped.\n");
}