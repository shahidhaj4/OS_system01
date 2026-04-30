#include "library.h"
#include <stdio.h>
#include <limits.h>
#include "graph.h"
#include "file_reader.h"
#include "dijkstra.h"
#include "raylib.h"
#include <math.h>

#define SCREEN_W 900
#define SCREEN_H 700
#define NODE_R 22

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <graph_file>\n", argv[0]);
        return 1;
    }

    int src, dst;
    Graph *g = readGraph(argv[1], &src, &dst);
    if (!g) return 1;

    int dist[MAX_NODES];
    int prev[MAX_NODES];

    dijkstra(g, src, dist, prev);

    if (src == dst) {
        printf("%d\n0\n", src);
    } else if (dist[dst] == INT_MAX) {
        printf("No path found\n");
    } else {
        printPath(prev, src, dst);
        printf("\n%d\n", dist[dst]);
    }

    float x[MAX_NODES];
    float y[MAX_NODES];

    for (int i = 0; i < g->n; i++) {
        float angle = 2.0f * PI * i / g->n;
        x[i] = SCREEN_W / 2.0f + cosf(angle) * 230.0f;
        y[i] = SCREEN_H / 2.0f + sinf(angle) * 230.0f;
    }

    InitWindow(SCREEN_W, SCREEN_H, "Milestone 2 - Graph GUI");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText("Directed Weighted Graph", 20, 20, 24, BLACK);

        for (int u = 0; u < g->n; u++) {
            for (Edge *e = g->adj[u]; e != NULL; e = e->next) {
                int v = e->dest;

                DrawLine((int)x[u], (int)y[u], (int)x[v], (int)y[v], DARKGRAY);

                int midX = (int)((x[u] + x[v]) / 2);
                int midY = (int)((y[u] + y[v]) / 2);

                char weight[16];
                snprintf(weight, sizeof(weight), "%d", e->weight);
                DrawText(weight, midX, midY, 18, BLUE);
            }
        }

        for (int i = 0; i < g->n; i++) {
            DrawCircle((int)x[i], (int)y[i], NODE_R, SKYBLUE);
            DrawCircleLines((int)x[i], (int)y[i], NODE_R, DARKBLUE);

            char label[8];
            snprintf(label, sizeof(label), "%d", i);
            int textW = MeasureText(label, 20);
            DrawText(label, (int)(x[i] - textW / 2), (int)(y[i] - 10), 20, BLACK);
        }

        drawDijkstraResult(g, prev, dist, src, dst, x, y, NODE_R);

        EndDrawing();
    }

    CloseWindow();

    freeGraph(g);
    return 0;
}