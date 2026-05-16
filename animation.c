#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "animation.h"
#include "file_reader.h"
#include "graph.h"
#include "dijkstra.h"

#define MAX_NODES 100

extern Graph g_graph;

/* ---------------- EDGE WEIGHT UTILITY ---------------- */

int get_edge_weight_from_data(const GraphData* data, int src, int dst) {
    if (!data) return 1;

    for (int i = 0; i < data->m; i++) {
        if (data->edges[i].src == src &&
            data->edges[i].dst == dst) {
            return data->edges[i].weight;
        }
    }

    return 1;
}

/* ---------------- AUTONOMOUS CHILD PROCESS ---------------- */

static void run_autonomous_child(
    int traveler_idx,
    int src,
    int dest,
    int write_fd,
    const GraphData* data
) {
    int dist[MAX_NODES];
    int prev[MAX_NODES];

    int path[MAX_NODES];
    int path_len = 0;

    /* Child calculates shortest path independently */
    dijkstra(&g_graph, src, dist, prev);

    /* Extract path */
    if (!extractPath(prev, src, dest, path, &path_len)) {

        IPCOverPipeMessage msg;

        msg.traveler_index = traveler_idx;
        msg.arrived_node = src;
        msg.next_node = -1;
        msg.is_finished = 1;

        write(write_fd, &msg, sizeof(msg));

        close(write_fd);
        _exit(0);
    }

    reversePath(path, path_len);

    /* Travel through path */
    for (int i = 0; i < path_len; i++) {

        IPCOverPipeMessage msg;

        msg.traveler_index = traveler_idx;
        msg.arrived_node = path[i];

        /* Destination reached */
        if (i == path_len - 1) {

            msg.next_node = -1;
            msg.is_finished = 1;

            write(write_fd, &msg, sizeof(msg));
            break;
        }

        /* Normal movement update */
        msg.next_node = path[i + 1];
        msg.is_finished = 0;

        write(write_fd, &msg, sizeof(msg));

        int weight = get_edge_weight_from_data(
            data,
            path[i],
            path[i + 1]
        );

        /* Simulated movement delay */
        struct timespec delay;

        /* Simulated movement delay */
        delay.tv_sec = 0;
        delay.tv_nsec = weight * 300000000L;
        nanosleep(&delay, NULL);

        /* Pause at node */
        delay.tv_sec = 1;
        delay.tv_nsec = 0;
        nanosleep(&delay, NULL);
    }

    close(write_fd);
    _exit(0);
}



void spawn_traveler_processes(
    Traveler* travelers,
    int num_travelers,
    const GraphData* data
) {
    for (int i = 0; i < num_travelers; i++) {

        /* Create pipe */
        if (pipe(travelers[i].pipe_fd) < 0) {
            perror("pipe failed");
            exit(EXIT_FAILURE);
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }



        if (pid == 0) {

            close(travelers[i].pipe_fd[0]);

            run_autonomous_child(
                i,
                travelers[i].src_node,
                travelers[i].dest_node,
                travelers[i].pipe_fd[1],
                data
            );
        }



        travelers[i].child_pid = pid;
        travelers[i].is_active = 1;

        close(travelers[i].pipe_fd[1]);

        /* Non-blocking pipe */
        int flags = fcntl(
            travelers[i].pipe_fd[0],
            F_GETFL,
            0
        );

        fcntl(
            travelers[i].pipe_fd[0],
            F_SETFL,
            flags | O_NONBLOCK
        );

        /* Initialize traveler state */
        travelers[i].current_node = travelers[i].src_node;
        travelers[i].next_node = -1;

        travelers[i].current_path_index = 0;
        travelers[i].current_jump_step = 0;

        travelers[i].time_counter = 0;
        travelers[i].is_waiting_at_node = 0;

        travelers[i].current_x = 0.0f;
        travelers[i].current_y = 0.0f;
    }
}



void update_all_travelers_from_pipes(
    Traveler* travelers,
    int num_travelers
) {
    for (int i = 0; i < num_travelers; i++) {

        if (!travelers[i].is_active)
            continue;

        IPCOverPipeMessage msg;

        ssize_t bytes_read = read(
            travelers[i].pipe_fd[0],
            &msg,
            sizeof(msg)
        );

        if (bytes_read != sizeof(msg))
            continue;



        travelers[i].current_node = msg.arrived_node;
        travelers[i].next_node = msg.next_node;

        travelers[i].current_path_index++;

        travelers[i].is_waiting_at_node = 1;
        travelers[i].time_counter = 0;

        /*
         * GUI rendering system can now use:
         * current_node
         * next_node
         * to animate movement and update colors
         */



        if (msg.next_node == -1) {

            printf(
                "[PID=%d] arrived at node %d | DESTINATION\n",
                travelers[i].child_pid,
                msg.arrived_node
            );

            printf(
                "[PID=%d] finished\n",
                travelers[i].child_pid
            );

            fflush(stdout);

            travelers[i].is_active = 0;

            close(travelers[i].pipe_fd[0]);
        }
        else {

            printf(
                "[PID=%d] arrived at node %d | next node: %d\n",
                travelers[i].child_pid,
                msg.arrived_node,
                msg.next_node
            );

            fflush(stdout);
        }
    }
}



void wait_for_all_children(
    Traveler* travelers,
    int num_travelers
) {
    for (int i = 0; i < num_travelers; i++) {

        if (travelers[i].child_pid > 0) {

            waitpid(
                travelers[i].child_pid,
                NULL,
                0
            );
        }
    }
}
void update_traveler_animation(
    Traveler* traveler,
    const GraphData* data
) {
    if (!traveler || !data) return;

    if (!traveler->is_active) return;

    if (traveler->next_node < 0) return;

    int weight = get_edge_weight_from_data(
        data,
        traveler->current_node,
        traveler->next_node
    );

    if (weight <= 0) {
        weight = 1;
    }

    /*
     * 60 FPS animation.
     * Higher edge weight means slower movement.
     */
    float step = 1.0f / (weight * 60.0f);

    traveler->current_x += step;

    if (traveler->current_x >= 1.0f) {
        traveler->current_x = 0.0f;
    }
}