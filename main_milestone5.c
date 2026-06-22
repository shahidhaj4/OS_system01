#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "raylib.h"
#include "graph.h"
#include "file_reader.h"
#include "dijkstra.h"
#include "drawing.h"

#define SCREEN_W 900
#define SCREEN_H 700
#define NODE_RADIUS 22
#define EDGE_STEP_TIME 0.3f
#define MAX_TRAVELERS_LOCAL 10

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

    int current_node;
    int next_node;

    int finished;
    int total_weight;

    TravelerState state;

    float edge_elapsed;

    pid_t child_pid;
    int pipe_fd[2];
} M5Traveler;

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

static void compute_positions(int n, float* x, float* y) {
    for (int i = 0; i < n; i++) {
        float angle = 2.0f * PI * i / n;
        x[i] = SCREEN_W / 2.0f + cosf(angle) * 230.0f;
        y[i] = SCREEN_H / 2.0f + sinf(angle) * 230.0f;
    }
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

static void send_pipe_message(int fd,
                              int traveler_index,
                              int current_node,
                              int next_node,
                              int is_finished,
                              TravelerState state) {
    PipeMessage msg;

    msg.traveler_index = traveler_index;
    msg.pid = getpid();
    msg.current_node = current_node;
    msg.next_node = next_node;
    msg.is_finished = is_finished;
    msg.state = state;

    write(fd, &msg, sizeof(msg));
}

static void child_process_run(int traveler_index, Graph* g, int src, int dst, int write_fd) {
    int path[MAX_NODES];
    int path_len = 0;
    int total_weight = 0;

    if (!calculate_path(g, src, dst, path, &path_len, &total_weight)) {
        send_pipe_message(write_fd, traveler_index, src, -1, 1, FINISHED);
        close(write_fd);
        _exit(0);
    }

    for (int i = 0; i < path_len; i++) {
        int current = path[i];
        int next = (i + 1 < path_len) ? path[i + 1] : -1;

        if (next < 0) {
            send_pipe_message(write_fd, traveler_index, current, -1, 1, FINISHED);
            break;
        }

        send_pipe_message(write_fd, traveler_index, current, next, 0, MOVING);

        long total_nsec = edge_weight(g, current, next) * 300000000L;

        struct timespec delay;
        delay.tv_sec = total_nsec / 1000000000L;
        delay.tv_nsec = total_nsec % 1000000000L;

        nanosleep(&delay, NULL);
    }

    close(write_fd);
    _exit(0);
}

static void init_traveler(M5Traveler* t, int src, int dst) {
    memset(t, 0, sizeof(*t));

    t->src = src;
    t->dst = dst;
    t->current_node = src;
    t->next_node = -1;
    t->finished = 0;
    t->total_weight = 0;
    t->state = MOVING;
    t->edge_elapsed = 0.0f;
    t->child_pid = -1;
    t->pipe_fd[0] = -1;
    t->pipe_fd[1] = -1;
}

static void spawn_ipc_child(Graph* g, M5Traveler* t, int index) {
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
        child_process_run(index, g, t->src, t->dst, t->pipe_fd[1]);
    }

    close(t->pipe_fd[1]);
    t->pipe_fd[1] = -1;
    t->child_pid = pid;
}

static void log_pipe_message(const PipeMessage* msg) {
    char line[128];

    if (msg->is_finished || msg->next_node < 0) {
        snprintf(line,
                 sizeof(line),
                 "[PID=%d] arrived at node %d | DESTINATION",
                 (int)msg->pid,
                 msg->current_node);

        printf("%s\n", line);
        addLogLine(line);

        snprintf(line,
                 sizeof(line),
                 "[PID=%d] finished",
                 (int)msg->pid);

        printf("%s\n", line);
        addLogLine(line);
    } else {
        snprintf(line,
                 sizeof(line),
                 "[PID=%d] arrived at node %d | next node: %d",
                 (int)msg->pid,
                 msg->current_node,
                 msg->next_node);

        printf("%s\n", line);
        addLogLine(line);
    }

    fflush(stdout);
}

static void receive_pipe_updates(M5Traveler* travelers, int traveler_count) {
    for (int i = 0; i < traveler_count; i++) {
        if (travelers[i].pipe_fd[0] < 0) continue;

        while (1) {
            PipeMessage msg;
            ssize_t n = read(travelers[i].pipe_fd[0], &msg, sizeof(msg));

            if (n == sizeof(msg)) {
                M5Traveler* t = &travelers[msg.traveler_index];

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

static void update_animation(Graph* g, M5Traveler* travelers, int traveler_count, float dt) {
    for (int i = 0; i < traveler_count; i++) {
        M5Traveler* t = &travelers[i];

        if (!t->finished && t->state == MOVING && t->next_node >= 0) {
            float duration = edge_weight(g, t->current_node, t->next_node) * EDGE_STEP_TIME;
            t->edge_elapsed += dt;

            if (t->edge_elapsed > duration) {
                t->edge_elapsed = duration;
            }
        }
    }
}

static void draw_all_travelers(Graph* g, M5Traveler* travelers, int count, float* x, float* y) {
    for (int i = 0; i < count; i++) {
        M5Traveler* t = &travelers[i];

        int current = t->current_node;
        if (current < 0 || current >= g->n) continue;

        float px = x[current];
        float py = y[current];

        if (t->state == MOVING && t->next_node >= 0 && t->next_node < g->n) {
            float duration = edge_weight(g, current, t->next_node) * EDGE_STEP_TIME;
            float progress = duration > 0.0f ? t->edge_elapsed / duration : 1.0f;

            if (progress < 0.0f) progress = 0.0f;
            if (progress > 1.0f) progress = 1.0f;

            px = x[current] + (x[t->next_node] - x[current]) * progress;
            py = y[current] + (y[t->next_node] - y[current]) * progress;
        }

        drawTraveler(i,
                     px,
                     py,
                     t->dst,
                     t->finished,
                     t->total_weight,
                     stationNames,
                     t->state,
                     0);
    }
}

static void cleanup_children(M5Traveler* travelers, int traveler_count) {
    for (int i = 0; i < traveler_count; i++) {
        if (travelers[i].pipe_fd[0] >= 0) {
            close(travelers[i].pipe_fd[0]);
            travelers[i].pipe_fd[0] = -1;
        }

        if (travelers[i].child_pid > 0) {
            waitpid(travelers[i].child_pid, NULL, 0);
            travelers[i].child_pid = 0;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file_name>\n", argv[0]);
        return 1;
    }

    GraphData* data = read_file(argv[1]);
    if (!data) return 1;

    Graph* g = build_graph_from_data(data);
    if (!g) {
        free_graph_data(data);
        return 1;
    }

    int traveler_count = data->traveler_count;

    if (traveler_count <= 0 || traveler_count > MAX_TRAVELERS_LOCAL) {
        fprintf(stderr, "Error: invalid traveler count. Max is %d\n", MAX_TRAVELERS_LOCAL);
        freeGraph(g);
        free_graph_data(data);
        return 1;
    }

    M5Traveler travelers[MAX_TRAVELERS_LOCAL];

    for (int i = 0; i < traveler_count; i++) {
        init_traveler(&travelers[i], data->travelers[i].src, data->travelers[i].dst);

        int path[MAX_NODES];
        int path_len = 0;
        int total_weight = 0;

        calculate_path(g,
                       travelers[i].src,
                       travelers[i].dst,
                       path,
                       &path_len,
                       &total_weight);

        travelers[i].total_weight = total_weight;
    }

    float x[MAX_NODES];
    float y[MAX_NODES];

    compute_positions(g->n, x, y);

    InitWindow(SCREEN_W, SCREEN_H, "Milestone 5 - IPC Pipes");
    SetTargetFPS(60);

    int started = 0;
    int playing = 0;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        if (IsKeyPressed(KEY_SPACE)) {
            if (!started) {
                for (int i = 0; i < traveler_count; i++) {
                    spawn_ipc_child(g, &travelers[i], i);
                }

                started = 1;
                playing = 1;
            } else {
                playing = !playing;
            }
        }

        if (started && playing) {
            receive_pipe_updates(travelers, traveler_count);
            update_animation(g, travelers, traveler_count, dt);
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText("Milestone 5 - IPC Pipes", 20, 20, 24, BLACK);
        DrawText("Children calculate paths and send updates to parent through pipes",
                 20, 55, 18, DARKGRAY);

        if (!started) {
            DrawText("Press SPACE to start", 20, 85, 20, DARKGREEN);
        } else {
            DrawText(playing ? "Running... SPACE to stop" : "Stopped... SPACE to play",
                     20, 85, 20, playing ? RED : DARKGREEN);
        }

        drawGraph(g, x, y, NODE_RADIUS, stationNames);

        if (started) {
            draw_all_travelers(g, travelers, traveler_count, x, y);
            drawLogPanel();
        }

        EndDrawing();
    }

    cleanup_children(travelers, traveler_count);

    CloseWindow();

    freeGraph(g);
    free_graph_data(data);

    return 0;
}
