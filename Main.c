#include "library.h"
#include <stdio.h>
#include <limits.h>
#include "graph.h"
#include "file_reader.h"
#include "dijkstra.h"
#include "raylib.h"
#include <math.h>
#include "Dijkstra_draw.h"

#define SCREEN_W 900
#define SCREEN_H 700
#define NODE_R 22
const char *stationNames[MAX_NODES] = {
    "Tel Aviv",
    "Haifa",
    "Jerusalem",
    "Beer Sheva",
    "Nazareth",
    "Eilat",
    "Netanya",
    "Ashdod",
    "Tiberias",
    "Acre",
    "Rishon LeZion",
    "Herzliya",
    "Petah Tikva",
    "Ramla",
    "Lod"
};
void drawDijkstraResult(Graph *g, int *prev, int *dist,
                        int src, int dst,
                        float *x, float *y, float radius);

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

                // رسم الخط
                DrawLineEx((Vector2){x[u], y[u]}, (Vector2){x[v], y[v]}, 4, BLACK);

                // رسم السهم (arrow)
                float dx = x[v] - x[u];
                float dy = y[v] - y[u];
                float len = sqrtf(dx * dx + dy * dy);

                if (len > 0) {
                    float ux = dx / len;
                    float uy = dy / len;

                    Vector2 tip = {x[v] - ux * NODE_R, y[v] - uy * NODE_R};
                    Vector2 left = {tip.x - ux * 12 - uy * 6, tip.y - uy * 12 + ux * 6};
                    Vector2 right = {tip.x - ux * 12 + uy * 6, tip.y - uy * 12 - ux * 6};

                    DrawTriangle(tip, left, right, DARKGRAY);
                }

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

            char label[64];
            snprintf(label, sizeof(label), "%d (%s)", i, stationNames[i]);

            int textW = MeasureText(label, 18);
            float centerX = SCREEN_W / 2.0f;
            float centerY = SCREEN_H / 2.0f;

            int textX = (int)x[i];
            int textY = (int)y[i];


            if (x[i] > centerX + 20) {
                textX = (int)(x[i] + NODE_R + 5);
                textY = (int)(y[i] - 8);
            }

            else if (x[i] < centerX - 20) {
                textX = (int)(x[i] - textW - NODE_R - 5);
                textY = (int)(y[i] - 8);
            }

            else if (y[i] < centerY) {
                textX = (int)(x[i] - textW / 2);
                textY = (int)(y[i] - NODE_R - 20);
            }

            else {
                textX = (int)(x[i] - textW / 2);
                textY = (int)(y[i] + NODE_R + 10);
            }

           // DrawText(label, textX, textY, 18, BLACK);
            int padding = 4;
            int textH = 18;

            DrawRectangle(textX - padding,
                          textY - padding,
                          textW + padding * 2,
                          textH + padding * 2,
                          (Color){255, 255, 255, 220});

            DrawRectangleLines(textX - padding,
                               textY - padding,
                               textW + padding * 2,
                               textH + padding * 2,
                               GRAY);

            DrawText(label, textX + 1, textY + 1, 18, DARKGRAY); // shadow
            DrawText(label, textX, textY, 18, BLACK);
        }

        drawDijkstraResult(g, prev, dist, src, dst, x, y, (float)NODE_R);

        EndDrawing();
    }

    CloseWindow();

    freeGraph(g);
    return 0;
}
