#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "raylib.h"
#include "graph.h"
#include "file_reader.h"
#include "drawing.h"

#define SCREEN_W 900
#define SCREEN_H 700
#define NODE_RADIUS 22

const char *stationNames[MAX_NODES] = {
    "Tel Aviv", "Haifa", "Jerusalem", "Beer Sheva", "Nazareth",
    "Eilat", "Netanya", "Ashdod", "Tiberias", "Acre",
    "Rishon LeZion", "Herzliya", "Petah Tikva", "Ramla", "Lod"
};

static Graph* build_graph_from_data(const GraphData* data) {
    Graph* g = create_graph(data->n, data->m);
    if (!g) return NULL;

    for (int i = 0; i < data->m; i++) {
        add_edge(g,
                 data->edges[i].src,
                 data->edges[i].dst,
                 data->edges[i].weight);
    }

    return g;
}

static void compute_positions(int n, float* x, float* y) {
    for (int i = 0; i < n; i++) {
        float angle = 2.0f * PI * i / n;
        x[i] = SCREEN_W / 2.0f + cosf(angle) * 230.0f;
        y[i] = SCREEN_H / 2.0f + sinf(angle) * 230.0f;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file_name>\n", argv[0]);
        return 1;
    }

    int src = -1;
    int dst = -1;

    GraphData* data = read_single_query_file(argv[1], &src, &dst);
    if (!data) {
        return 1;
    }

    Graph* g = build_graph_from_data(data);
    if (!g) {
        free_graph_data(data);
        return 1;
    }

    float x[MAX_NODES];
    float y[MAX_NODES];

    compute_positions(g->n, x, y);

    InitWindow(SCREEN_W, SCREEN_H, "Milestone 2 - Static Graph GUI");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText("Milestone 2 - Static Directed Weighted Graph", 20, 20, 24, BLACK);
        DrawText("Static GUI only: nodes, directed edges, weights and labels", 20, 55, 18, DARKGRAY);

        drawGraph(g, x, y, NODE_RADIUS, stationNames);

        EndDrawing();
    }

    CloseWindow();

    freeGraph(g);
    free_graph_data(data);

    return 0;
}
