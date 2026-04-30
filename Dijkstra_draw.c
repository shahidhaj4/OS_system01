#include <stdio.h>
#include <limits.h>
#include "dijkstra_draw.h"
#include "dijkstra.h"
#include "graph.h"
#include "raylib.h"
#include <math.h>
#define MAX_NODES 100

static void drawArrow(float x1, float y1, float x2, float y2,
                      Color color, float thick) {
    float dx = x2 - x1, dy = y2 - y1;
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 1.0f) return;
    float ux = dx / len, uy = dy / len;
    float startX = x1 + ux * 20.0f, startY = y1 + uy * 20.0f;
    float endX   = x2 - ux * 20.0f, endY   = y2 - uy * 20.0f;
    DrawLineEx((Vector2){startX, startY}, (Vector2){endX, endY}, thick, color);
    float headLen = 12.0f, headHalf = 6.0f;
    float perpX = -uy, perpY = ux;
    Vector2 tip   = {endX, endY};
    Vector2 left  = {endX - ux*headLen + perpX*headHalf, endY - uy*headLen + perpY*headHalf};
    Vector2 right = {endX - ux*headLen - perpX*headHalf, endY - uy*headLen - perpY*headHalf};
    DrawTriangle(tip, left, right, color);
}

void drawDijkstraResult(Graph *g, int *prev, int *dist,
                        int src, int dst,
                        float *x, float *y, float radius) {
    int onPath[MAX_NODES];
    int pathExists = buildPathMask(prev, src, dst, g->n, onPath);

    if (pathExists) {
        for (int u = 0; u < g->n; u++) {
            if (!onPath[u]) continue;
            for (Edge *e = g->adj[u]; e != NULL; e = e->next) {
                int v = e->dest;
                if (onPath[v] && prev[v] == u)
                    drawArrow(x[u], y[u], x[v], y[v], RED, 6.0f);
            }
        }
        for (int i = 0; i < g->n; i++) {
            if (!onPath[i]) continue;
            DrawCircle((int)x[i], (int)y[i], radius, ORANGE);
            DrawCircleLines((int)x[i], (int)y[i], radius, RED);
            char label[8];
            snprintf(label, sizeof(label), "%d", i);
            int textW = MeasureText(label, 18);
            DrawText(label, (int)(x[i] - textW/2.0f), (int)(y[i] - 9), 18, BLACK);
        }
    }

    Color infoBg = {0, 0, 0, 180};

    int boxX = 10;
    int boxY = 80;
    int boxW = 280;
    int boxH = 70;

    DrawRectangle(boxX, boxY, boxW, boxH, infoBg);
    DrawRectangleLines(boxX, boxY, boxW, boxH, GRAY);

    if (src == dst) {
        char line1[64];
        snprintf(line1, sizeof(line1), "Path: %d  (source = destination)", src);
        DrawText(line1, boxX + 8, boxY + 10, 16, WHITE);
        DrawText("Total weight: 0", boxX + 8, boxY + 34, 16, WHITE);
    } else if (!pathExists || dist[dst] == INT_MAX) {
        DrawText("No path found", boxX + 8, boxY + 10, 18, RED);

        char query[64];
        snprintf(query, sizeof(query), "Query: %d -> %d", src, dst);
        DrawText(query, boxX + 8, boxY + 36, 16, LIGHTGRAY);
    } else {
        char query[64], weight[64];

        snprintf(query, sizeof(query), "Shortest path: %d -> %d", src, dst);
        snprintf(weight, sizeof(weight), "Total weight: %d", dist[dst]);

        DrawText(query, boxX + 8, boxY + 10, 16, WHITE);
        DrawText(weight, boxX + 8, boxY + 34, 18, YELLOW);
    }
}