#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "dijkstra.h"
#include "Dijkstra_draw.h"
#include "drawing.h"
#include "file_reader.h"
#include "graph.h"
#include "raylib.h"
#include "sync.h"

#ifndef MILESTONE
#define MILESTONE 6
#endif

#define SCREEN_W 900
#define SCREEN_H 700
#define NODE_RADIUS 22
#define MAX_TRAVELERS_LOCAL 10
#define EDGE_STEP_SECONDS 0.3f
#define NODE_WAIT_SECONDS 1.0f

const char *stationNames[MAX_NODES] = {
    "Tel Aviv", "Haifa", "Jerusalem", "Beer Sheva", "Nazareth",
    "Eilat", "Netanya", "Ashdod", "Tiberias", "Acre",
    "Rishon LeZion", "Herzliya", "Petah Tikva", "Ramla", "Lod"
};

typedef struct {
    int traveler_index;
    pid_t pid;
    int current_node;
    int next_node;
    int is_finished;
    TravelerState state;
} PipeMessage;

typedef struct {
    int src;
    int dst;
    int path[MAX_NODES];
    int path_len;
    int path_index;
    int current_node;
    int next_node;
    int finished;
    int total_weight;
    TravelerState state;
    float edge_elapsed;
    float wait_elapsed;
    pid_t child_pid;
    int pipe_fd[2];
} SimTraveler;

static Graph* build_graph_from_data(const GraphData* data) {
    Graph* g = create_graph(data->n, data->m);
    if (!g) return NULL;

    for (int i = 0; i < data->m; i++) {
        add_edge(g, data->edges[i].src, data->edges[i].dst, data->edges[i].weight);
    }

    return g;
}

static int edge_weight(Graph* g, int src, int dst) {
    int w = get_edge_weight(g, src, dst);
    return w > 0 ? w : 1;
}

static int calculate_path(Graph* g, int src, int dst, int* path, int* path_len, int* total_weight) {
    int dist[MAX_NODES];
    int prev[MAX_NODES];

    dijkstra(g, src, dist, prev);
    if (!extractPath(prev, src, dst, path, path_len) || dist[dst] == INT_MAX) {
        *path_len = 0;
        *total_weight = 0;
        return 0;
    }

    reversePath(path, *path_len);
    *total_weight = dist[dst];
    return 1;
}

static void compute_positions(int n, float* x, float* y) {
    for (int i = 0; i < n; i++) {
        float angle = 2.0f * PI * i / n;
        x[i] = SCREEN_W / 2.0f + cosf(angle) * 230.0f;
        y[i] = SCREEN_H / 2.0f + sinf(angle) * 230.0f;
    }
}

static void init_traveler(SimTraveler* t, int src, int dst) {
    memset(t, 0, sizeof(*t));
    t->src = src;
    t->dst = dst;
    t->current_node = src;
    t->next_node = -1;
    t->state = MOVING;
    t->child_pid = -1;
    t->pipe_fd[0] = -1;
    t->pipe_fd[1] = -1;
}

static void start_parent_driven_traveler(Graph* g, SimTraveler* t) {
    if (!calculate_path(g, t->src, t->dst, t->path, &t->path_len, &t->total_weight)) {
        t->finished = 1;
        t->state = FINISHED;
        return;
    }

    t->path_index = 0;
    t->current_node = t->path[0];
    t->next_node = t->path_len > 1 ? t->path[1] : -1;
    t->finished = t->path_len <= 1;
    t->state = t->finished ? FINISHED : MOVING;
}

static void update_parent_driven_traveler(Graph* g, SimTraveler* t, float dt, int playing) {
    if (!playing || t->finished || t->path_len == 0) return;

    if (t->state == INSIDE_NODE) {
        t->wait_elapsed += dt;

        if (t->wait_elapsed >= NODE_WAIT_SECONDS) {
            t->wait_elapsed = 0.0f;
            t->next_node = t->path_index + 1 < t->path_len ? t->path[t->path_index + 1] : -1;
            t->state = t->next_node >= 0 ? MOVING : FINISHED;
        }

        return;
    }

    if (t->state == MOVING && t->next_node >= 0) {
        float duration = edge_weight(g, t->current_node, t->next_node) * EDGE_STEP_SECONDS;
        t->edge_elapsed += dt;

        if (t->edge_elapsed >= duration) {
            t->edge_elapsed = 0.0f;
            t->path_index++;
            t->current_node = t->path[t->path_index];

            if (t->path_index == t->path_len - 1) {
                t->next_node = -1;
                t->finished = 1;
                t->state = FINISHED;

                if (t->child_pid > 0) {
                    kill(t->child_pid, SIGTERM);
                    waitpid(t->child_pid, NULL, 0);
                    t->child_pid = 0;
                }
            } else {
                t->next_node = -1;
                t->state = INSIDE_NODE;
            }
        }
    }
}

static void send_pipe_message(int fd, int traveler_index, int current, int next, int finished, TravelerState state) {
    PipeMessage msg;
    msg.traveler_index = traveler_index;
    msg.pid = getpid();
    msg.current_node = current;
    msg.next_node = next;
    msg.is_finished = finished;
    msg.state = state;
    write(fd, &msg, sizeof(msg));
}

static void child_drive_path(int traveler_index, Graph* g, int src, int dst, int fd, int use_locks) {
    int path[MAX_NODES];
    int path_len = 0;
    int total = 0;

    if (!calculate_path(g, src, dst, path, &path_len, &total)) {
        send_pipe_message(fd, traveler_index, src, -1, 1, FINISHED);
        close(fd);
        _exit(0);
    }

    for (int i = 0; i < path_len; i++) {
        int current = path[i];
        int next = i + 1 < path_len ? path[i + 1] : -1;

        if (use_locks) {
            send_pipe_message(fd, traveler_index, current, next, 0, WAITING);
            lock_node(current);
            send_pipe_message(fd, traveler_index, current, next, 0, INSIDE_NODE);
            sleep(1);
            unlock_node(current);
        }

        if (next < 0) {
            send_pipe_message(fd, traveler_index, current, -1, 1, FINISHED);
            break;
        }

        send_pipe_message(fd, traveler_index, current, next, 0, MOVING);

        struct timespec delay;
        long total_nsec = edge_weight(g, current, next) * 300000000L;
        delay.tv_sec = total_nsec / 1000000000L;
        delay.tv_nsec = total_nsec % 1000000000L;
        nanosleep(&delay, NULL);
    }

    close(fd);
    _exit(0);
}

static void spawn_sleeping_child(SimTraveler* t) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        printf("[%d] started\n", (int)getpid());
        fflush(stdout);

        for (;;) {
            pause();
        }
    }

    t->child_pid = pid;
}

static void spawn_ipc_child(Graph* g, SimTraveler* t, int index, int use_locks) {
    if (pipe(t->pipe_fd) < 0) {
        perror("pipe");
        exit(1);
    }

    int flags = fcntl(t->pipe_fd[0], F_GETFL, 0);
    fcntl(t->pipe_fd[0], F_SETFL, flags | O_NONBLOCK);

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        close(t->pipe_fd[0]);
        child_drive_path(index, g, t->src, t->dst, t->pipe_fd[1], use_locks);
    }

    close(t->pipe_fd[1]);
    t->pipe_fd[1] = -1;
    t->child_pid = pid;
}

static void set_children_paused(SimTraveler* travelers, int count, int paused) {
    for (int i = 0; i < count; i++) {
        if (travelers[i].child_pid > 0 && !travelers[i].finished) {
            kill(travelers[i].child_pid, paused ? SIGSTOP : SIGCONT);
        }
    }
}

static void log_pipe_message(const PipeMessage* msg) {
    char line[128];

    if (msg->is_finished || msg->next_node < 0) {
        snprintf(line, sizeof(line), "[PID=%d] arrived at node %d | DESTINATION",
                 (int)msg->pid, msg->current_node);
        printf("%s\n", line);
        addLogLine(line);

        snprintf(line, sizeof(line), "[PID=%d] finished", (int)msg->pid);
        printf("%s\n", line);
        addLogLine(line);
    } else if (msg->state == MOVING) {
        snprintf(line, sizeof(line), "[PID=%d] arrived at node %d | next node: %d",
                 (int)msg->pid, msg->current_node, msg->next_node);
        printf("%s\n", line);
        addLogLine(line);
    }

    fflush(stdout);
}

static void receive_pipe_updates(SimTraveler* travelers, int count) {
    for (int i = 0; i < count; i++) {
        if (travelers[i].pipe_fd[0] < 0) continue;

        for (;;) {
            PipeMessage msg;
            ssize_t n = read(travelers[i].pipe_fd[0], &msg, sizeof(msg));

            if (n == sizeof(msg)) {
                SimTraveler* t = &travelers[msg.traveler_index];

                t->current_node = msg.current_node;
                t->next_node = msg.next_node;
                t->state = msg.state;
                t->edge_elapsed = 0.0f;
                t->finished = msg.is_finished;

                log_pipe_message(&msg);

                if (msg.is_finished) {
                    close(t->pipe_fd[0]);
                    t->pipe_fd[0] = -1;
                    waitpid(t->child_pid, NULL, 0);
                    t->child_pid = 0;
                }
            } else if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                break;
            } else {
                break;
            }
        }
    }
}

static void update_ipc_animation(Graph* g, SimTraveler* travelers, int count, float dt) {
    for (int i = 0; i < count; i++) {
        SimTraveler* t = &travelers[i];

        if (!t->finished && t->state == MOVING && t->next_node >= 0) {
            float duration = edge_weight(g, t->current_node, t->next_node) * EDGE_STEP_SECONDS;
            t->edge_elapsed += dt;

            if (t->edge_elapsed > duration) {
                t->edge_elapsed = duration;
            }
        }
    }
}

static void cleanup_children(SimTraveler* travelers, int count) {
    for (int i = 0; i < count; i++) {
        if (travelers[i].pipe_fd[0] >= 0) {
            close(travelers[i].pipe_fd[0]);
            travelers[i].pipe_fd[0] = -1;
        }

        if (travelers[i].child_pid > 0) {
            kill(travelers[i].child_pid, SIGTERM);
            waitpid(travelers[i].child_pid, NULL, 0);
            travelers[i].child_pid = 0;
        }
    }
}

static void draw_travelers(Graph* g, SimTraveler* travelers, int count, float* x, float* y) {
    int waiter_slot[MAX_TRAVELERS_LOCAL] = {0};
    int wait_count[MAX_NODES] = {0};

    for (int i = 0; i < count; i++) {
        if (travelers[i].state == WAITING) {
            int node = travelers[i].current_node;

            if (node >= 0 && node < MAX_NODES) {
                waiter_slot[i] = wait_count[node]++;
            }
        }
    }

    for (int i = 0; i < count; i++) {
        SimTraveler* t = &travelers[i];
        int current = t->current_node;

        if (current < 0 || current >= g->n) continue;

        float px = x[current];
        float py = y[current];

        if (t->state == MOVING && t->next_node >= 0 && t->next_node < g->n) {
            float duration = edge_weight(g, current, t->next_node) * EDGE_STEP_SECONDS;
            float progress = duration > 0.0f ? t->edge_elapsed / duration : 1.0f;

            if (progress < 0.0f) progress = 0.0f;
            if (progress > 1.0f) progress = 1.0f;

            px = x[current] + (x[t->next_node] - x[current]) * progress;
            py = y[current] + (y[t->next_node] - y[current]) * progress;
        }

        drawTraveler(i, px, py, t->dst, t->finished, t->total_weight,
                     stationNames, t->state, waiter_slot[i]);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <graph_file>\n", argv[0]);
        return 1;
    }

    GraphData* data = NULL;
    int query_src = 0;
    int query_dst = 0;

    if (MILESTONE <= 3) {
        data = read_single_query_file(argv[1], &query_src, &query_dst);
    } else {
        data = read_file(argv[1]);
    }

    if (!data) return 1;

    Graph* g = build_graph_from_data(data);

    if (!g) {
        free_graph_data(data);
        return 1;
    }

    SimTraveler travelers[MAX_TRAVELERS_LOCAL];
    int traveler_count = MILESTONE <= 3 ? 1 : data->traveler_count;

    if (traveler_count > MAX_TRAVELERS_LOCAL) {
        fprintf(stderr, "Error: maximum travelers is %d\n", MAX_TRAVELERS_LOCAL);
        freeGraph(g);
        free_graph_data(data);
        return 1;
    }

    if (MILESTONE <= 3) {
        init_traveler(&travelers[0], query_src, query_dst);
        start_parent_driven_traveler(g, &travelers[0]);
    } else {
        for (int i = 0; i < traveler_count; i++) {
            init_traveler(&travelers[i], data->travelers[i].src, data->travelers[i].dst);

            if (MILESTONE == 4) {
                start_parent_driven_traveler(g, &travelers[i]);
            }
        }
    }

    if (MILESTONE == 6) {
        init_node_locks(g->n);
    }

    float x[MAX_NODES];
    float y[MAX_NODES];
    compute_positions(g->n, x, y);

    char title[64];
    snprintf(title, sizeof(title), "OS Graph Simulation - Milestone %d", MILESTONE);

    InitWindow(SCREEN_W, SCREEN_H, title);
    SetTargetFPS(60);

    int playing = 0;
    int started = 0;

    if (MILESTONE <= 2) {
        started = 1;
        playing = 1;
    }

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        if (MILESTONE == 3 && IsKeyPressed(KEY_SPACE)) {
            playing = !playing;
            started = 1;
        }

        if (MILESTONE == 4 && IsKeyPressed(KEY_SPACE)) {
            if (!started) {
                for (int i = 0; i < traveler_count; i++) {
                    spawn_sleeping_child(&travelers[i]);
                }

                started = 1;
                playing = 1;
            } else {
                playing = !playing;
                set_children_paused(travelers, traveler_count, !playing);
            }
        }

        if ((MILESTONE == 5 || MILESTONE == 6) && IsKeyPressed(KEY_SPACE)) {
            if (!started) {
                for (int i = 0; i < traveler_count; i++) {
                    int total = 0;
                    int path_len = 0;
                    int path[MAX_NODES];

                    calculate_path(g, travelers[i].src, travelers[i].dst, path, &path_len, &total);
                    travelers[i].total_weight = total;

                    spawn_ipc_child(g, &travelers[i], i, MILESTONE == 6);
                }

                started = 1;
                playing = 1;
            } else {
                playing = !playing;
                set_children_paused(travelers, traveler_count, !playing);
            }
        }

        if ((MILESTONE == 3 || MILESTONE == 4) && started) {
            for (int i = 0; i < traveler_count; i++) {
                update_parent_driven_traveler(g, &travelers[i], dt, playing);
            }
        } else if ((MILESTONE == 5 || MILESTONE == 6) && started && playing) {
            receive_pipe_updates(travelers, traveler_count);
            update_ipc_animation(g, travelers, traveler_count, dt);
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText(title, 20, 20, 24, BLACK);

        if (MILESTONE == 3) {
            DrawText(playing ? "SPACE: stop" : "SPACE: play", 20, 55, 18, DARKGRAY);
        }

        if (MILESTONE == 4) {
            DrawText("Parent animation + child processes", 20, 55, 18, DARKGRAY);
        }

        if (MILESTONE == 5) {
            DrawText("Children calculate paths and send node updates through pipes", 20, 55, 18, DARKGRAY);
        }

        if (MILESTONE == 6) {
            DrawText("Pipes + process-shared node locks", 20, 55, 18, DARKGRAY);
        }

        if (MILESTONE >= 4 && MILESTONE <= 6) {
            if (!started) {
                DrawText("Press SPACE to start", 20, 85, 20, DARKGREEN);
            } else {
                DrawText(playing ? "Running... SPACE to stop" : "Stopped... SPACE to play",
                         20,
                         85,
                         20,
                         playing ? RED : DARKGREEN);
            }
        }

        drawGraph(g, x, y, NODE_RADIUS, stationNames);

        if (MILESTONE == 3 || (MILESTONE >= 4 && started)) {
            draw_travelers(g, travelers, traveler_count, x, y);
        } else if (MILESTONE == 2) {
            int dist[MAX_NODES];
            int prev[MAX_NODES];

            dijkstra(g, query_src, dist, prev);
            drawDijkstraResult(g, prev, dist, query_src, query_dst, x, y, NODE_RADIUS);
        }

        if (MILESTONE >= 5 && started) {
            drawLogPanel();
        }

        EndDrawing();
    }

    cleanup_children(travelers, traveler_count);

    if (MILESTONE == 6) {
        destroy_node_locks(g->n);
    }

    CloseWindow();

    freeGraph(g);
    free_graph_data(data);

    return 0;
}