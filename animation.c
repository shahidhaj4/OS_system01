#include "animation.h"
#include "sync.h"
#include "graph.h"
#include "dijkstra.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

/* External graph structure used for path calculations */
extern Graph g_graph;

/* -------------------------------------------------------------------- *
 *                         IMPLEMENTATION                               *
 * -------------------------------------------------------------------- */

int get_edge_weight_from_data(const GraphData* data, int src, int dst) {
    if (!data) return 1;
    for (int i = 0; i < data->m; i++) {
        if ((data->edges[i].src == src && data->edges[i].dst == dst) ||
            (data->edges[i].src == dst && data->edges[i].dst == src)) {
            return data->edges[i].weight;
        }
    }
    return 1; /* Default weight if edge is not found */
}

/* ------------------ AUTONOMOUS CHILD PROCESS LOGIC ------------------ */

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

    /* Calculate the shortest path independently for each traveler */
    dijkstra(&g_graph, src, dist, prev);

    if (!extractPath(prev, src, dest, path, &path_len)) {
        IPCOverPipeMessage msg;
        msg.traveler_index = traveler_idx;
        msg.arrived_node = src;
        msg.next_node = -1;
        msg.is_finished = 1;
        msg.state = FINISHED; /* Handle case where no path is found */
        write(write_fd, &msg, sizeof(msg));
        close(write_fd);
        _exit(0);
    }

    reversePath(path, path_len);

    /* Systematically traverse through the nodes of the calculated path */
    for (int i = 0; i < path_len; i++) {
        IPCOverPipeMessage msg;
        msg.traveler_index = traveler_idx;
        msg.arrived_node = path[i];
        msg.next_node = (i == path_len - 1) ? -1 : path[i + 1];
        msg.is_finished = (i == path_len - 1) ? 1 : 0;

        /* 🟡 1. [State: WAITING] - Inform parent about waiting outside the node before locking */
        msg.state = WAITING;
        write(write_fd, &msg, sizeof(msg));

        lock_node(path[i]);

        /* 🟢 2. [State: INSIDE_NODE] - Successfully acquired lock and entered node, update state */
        msg.state = INSIDE_NODE;
        write(write_fd, &msg, sizeof(msg));

        /* Mandatory 1-second stay inside the node (critical section) */
        sleep(1);

        unlock_node(path[i]);

        /* If this is the destination node, declare finished and break loop */
        if (i == path_len - 1) {
            msg.state = FINISHED;
            write(write_fd, &msg, sizeof(msg));
            break;
        }

        /* 🔵 3. [State: MOVING] - Inform parent that child is moving on the edge toward the next node */
        msg.state = MOVING;
        write(write_fd, &msg, sizeof(msg));

        /* Simulate actual travel delay based on the edge weight */
        int weight = get_edge_weight_from_data(data, path[i], path[i + 1]);
        struct timespec delay;
        delay.tv_sec = 0;
        delay.tv_nsec = weight * 300000000L;
        nanosleep(&delay, NULL);
    }

    close(write_fd);
    _exit(0);
}

/* ------------------ SPAWN PROCESSES LOGIC ------------------ */

void spawn_traveler_processes(Traveler* travelers, int num_travelers, const GraphData* data) {
    for (int i = 0; i < num_travelers; i++) {
        /* Create pipe for each traveler */
        if (pipe(travelers[i].pipe_fd) < 0) {
            perror("Failed to create pipe");
            exit(1);
        }

        /* Set the read end of the pipe to non-blocking for the parent process */
        int flags = fcntl(travelers[i].pipe_fd[0], F_GETFL, 0);
        fcntl(travelers[i].pipe_fd[0], F_SETFL, flags | O_NONBLOCK);

        pid_t pid = fork();
        if (pid < 0) {
            perror("Failed to fork process");
            exit(1);
        }

        if (pid == 0) {
            /* Child process: close read end and run the simulation */
            close(travelers[i].pipe_fd[0]);
            run_autonomous_child(i, travelers[i].src_node, travelers[i].dest_node, travelers[i].pipe_fd[1], data);
        } else {
            /* Parent process: close write end, save child PID, and initialize state */
            close(travelers[i].pipe_fd[1]);
            travelers[i].child_pid = pid;
            travelers[i].is_active = 1;
            travelers[i].state = MOVING; /* Initial state */
        }
    }
}

/* ------------------ PARENT RECEIVER LOGIC ------------------ */

void update_all_travelers_from_pipes(Traveler* travelers, int num_travelers) {
    for (int i = 0; i < num_travelers; i++) {
        if (!travelers[i].is_active) continue;

        IPCOverPipeMessage msg;
        /* Non-blocking read from pipe for the current traveler */
        ssize_t bytes_read = read(travelers[i].pipe_fd[0], &msg, sizeof(msg));

        if (bytes_read == sizeof(msg)) {
            travelers[i].current_node = msg.arrived_node;
            travelers[i].next_node = msg.next_node;
            
            /* 💡 Receive state update from pipe and store it for GUI rendering (colors & offsets) */
            travelers[i].state = msg.state;

            /* If child declares finished, close read end and deactivate traveler */
            if (msg.state == FINISHED) {
                travelers[i].is_active = 0;
                close(travelers[i].pipe_fd[0]);
            }
        }
    }
}

/* ------------------ ANIMATION COORDINATES LOGIC ------------------ */

void update_traveler_animation(Traveler* traveler, const GraphData* data) {
    if (!traveler || !traveler->is_active) return;

    /* This function can be used to implement interpolation/jumps logic */
}

/* ------------------ CLEANUP & WAIT LOGIC ------------------ */

void wait_for_all_children(Traveler* travelers, int num_travelers) {
    for (int i = 0; i < num_travelers; i++) {
        if (travelers[i].child_pid > 0) {
            waitpid(travelers[i].child_pid, NULL, 0);
            travelers[i].child_pid = 0;
        }
    }
}