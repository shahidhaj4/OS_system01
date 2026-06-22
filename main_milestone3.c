#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

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
} AnimatedTraveler;

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

static int calculate_path(Graph* g, int src, int dst, AnimatedTraveler* t) {
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

static void init_traveler(AnimatedTraveler* t, Graph* g, int src, int dst) {
    t->src = src;
    t->dst = dst;
    t->path_index = 0;
    t->current_node = src;
    t->next_node = -1;
    t->finished = 0;
    t->state = MOVING;
    t->edge_elapsed = 0.0f;
    t->wait_elapsed = 0.0f;

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

static void update_traveler(Graph* g, AnimatedTraveler* t, float dt, int playing) {
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
            } else {
                t->next_node = -1;
                t->state = INSIDE_NODE;
            }
        }
    }
}

static void draw_animated_traveler(Graph* g, AnimatedTraveler* t, float* x, float* y) {
    int current = t->current_node;
    if (current < 0 || current >= g->n) return;

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

    drawTraveler(0, px, py, t->dst, t->finished, t->total_weight,
                 stationNames, t->state, 0);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file_name>\n", argv[0]);
        return 1;
    }

    int src = -1;
    int dst = -1;

    GraphData* data = read_single_query_file(argv[1], &src, &dst);
    if (!data) return 1;

    Graph* g = build_graph_from_data(data);
    if (!g) {
        free_graph_data(data);
        return 1;
    }

    AnimatedTraveler traveler;
    init_traveler(&traveler, g, src, dst);

    float x[MAX_NODES];
    float y[MAX_NODES];
    compute_positions(g->n, x, y);

    InitWindow(SCREEN_W, SCREEN_H, "Milestone 3 - Single Traveler Animation");
    SetTargetFPS(60);

    int playing = 0;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        if (IsKeyPressed(KEY_SPACE)) {
            playing = !playing;
        }

        update_traveler(g, &traveler, dt, playing);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText("Milestone 3 - Animation", 20, 20, 24, BLACK);
        DrawText(playing ? "SPACE: stop" : "SPACE: play", 20, 55, 18, DARKGRAY);

        drawGraph(g, x, y, NODE_RADIUS, stationNames);
        draw_animated_traveler(g, &traveler, x, y);

        EndDrawing();
    }

    CloseWindow();

    freeGraph(g);
    free_graph_data(data);

    return 0;
}
