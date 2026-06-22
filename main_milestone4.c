#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "raylib.h"
#include "graph.h"
#include "file_reader.h"
#include "dijkstra.h"
#include "drawing.h"

#define SCREEN_W 900
#define SCREEN_H 700
#define NODE_RADIUS 22
#define EDGE_STEP_TIME 0.3f
#define NODE_WAIT_TIME 1.0f
#define MAX_TRAVELERS_LOCAL 10

const char *stationNames[MAX_NODES] = {
    "Tel Aviv", "Haifa", "Jerusalem", "Beer Sheva", "Nazareth",
    "Eilat", "Netanya", "Ashdod", "Tiberias", "Acre",
    "Rishon LeZion", "Herzliya", "Petah Tikva", "Ramla", "Lod"
};

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
} M4Traveler;

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

static int calculate_path(Graph* g, int src, int dst, M4Traveler* t) {
    int dist[MAX_NODES];
    int prev[MAX_NODES];

    dijkstra(g, src, dist, prev);

    if (!extractPath(prev, src, dst, t->path, &t->path_len) || dist[dst] == INT_MAX) {
        t->path_len = 0;
        t->total_weight = 0;
        return 0;
    }

    reversePath(t->path, t->path_len);
    t->total_weight = dist[dst];
    return 1;
}

static void init_traveler(M4Traveler* t, Graph* g, int src, int dst) {
    t->src = src;
    t->dst = dst;
    t->path_index = 0;
    t->current_node = src;
    t->next_node = -1;
    t->finished = 0;
    t->state = MOVING;
    t->edge_elapsed = 0.0f;
    t->wait_elapsed = 0.0f;
    t->child_pid = -1;

    if (!calculate_path(g, src, dst, t)) {
        t->finished = 1;
        t->state = FINISHED;
        return;
    }

    t->current_node = t->path[0];

    if (t->path_len <= 1) {
        t->finished = 1;
        t->state = FINISHED;
        t->next_node = -1;
    } else {
        t->next_node = t->path[1];
        t->state = MOVING;
    }
}

static void spawn_child(M4Traveler* t) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        printf("[%d] started\n", (int)getpid());
        fflush(stdout);

        while (1) {
            pause();
        }
    }

    t->child_pid = pid;
}

static void update_traveler(Graph* g, M4Traveler* t, float dt, int playing) {
    if (!playing || t->finished || t->path_len == 0) return;

    if (t->state == INSIDE_NODE) {
        t->wait_elapsed += dt;

        if (t->wait_elapsed >= NODE_WAIT_TIME) {
            t->wait_elapsed = 0.0f;
            t->next_node = t->path_index + 1 < t->path_len ? t->path[t->path_index + 1] : -1;
            t->state = t->next_node >= 0 ? MOVING : FINISHED;
        }

        return;
    }

    if (t->state == MOVING && t->next_node >= 0) {
        float duration = edge_weight(g, t->current_node, t->next_node) * EDGE_STEP_TIME;
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

static void draw_all_travelers(Graph* g, M4Traveler* travelers, int count, float* x, float* y) {
    for (int i = 0; i < count; i++) {
        M4Traveler* t = &travelers[i];

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

        drawTraveler(i, px, py, t->dst, t->finished, t->total_weight,
                     stationNames, t->state, 0);
    }
}

static void cleanup_children(M4Traveler* travelers, int count) {
    for (int i = 0; i < count; i++) {
        if (travelers[i].child_pid > 0) {
            kill(travelers[i].child_pid, SIGTERM);
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

    M4Traveler travelers[MAX_TRAVELERS_LOCAL];

    for (int i = 0; i < traveler_count; i++) {
        init_traveler(&travelers[i], g, data->travelers[i].src, data->travelers[i].dst);
    }

    float x[MAX_NODES];
    float y[MAX_NODES];
    compute_positions(g->n, x, y);

    InitWindow(SCREEN_W, SCREEN_H, "Milestone 4 - Multiple Travelers + fork");
    SetTargetFPS(60);

    int started = 0;
    int playing = 0;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        if (IsKeyPressed(KEY_SPACE)) {
            if (!started) {
                for (int i = 0; i < traveler_count; i++) {
                    spawn_child(&travelers[i]);
                }
                started = 1;
                playing = 1;
            } else {
                playing = !playing;
            }
        }

        if (started) {
            for (int i = 0; i < traveler_count; i++) {
                update_traveler(g, &travelers[i], dt, playing);
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText("Milestone 4 - Multiple Travelers + fork", 20, 20, 24, BLACK);

        if (!started) {
            DrawText("Press SPACE to start", 20, 55, 18, DARKGREEN);
        } else {
            DrawText(playing ? "Running... SPACE to stop" : "Stopped... SPACE to play",
                     20, 55, 18, playing ? RED : DARKGREEN);
        }

        drawGraph(g, x, y, NODE_RADIUS, stationNames);

        if (started) {
            draw_all_travelers(g, travelers, traveler_count, x, y);
        }

        EndDrawing();
    }

    cleanup_children(travelers, traveler_count);

    CloseWindow();

    freeGraph(g);
    free_graph_data(data);

    return 0;
}

