#include <stdio.h>
#include <limits.h>
#include <string.h>
#include "dijkstra.h"
#include "graph.h"
#include "raylib.h"
#include <math.h>
#define MAX_NODES 15


void dijkstra(Graph *g, int src, int *dist, int *prev) {
    int visited[MAX_NODES] = {0};


    for (int i = 0; i < g->n; i++) {
        dist[i] = INT_MAX;
        prev[i] = -1;
    }
    dist[src] = 0;

    for (int iter = 0; iter < g->n; iter++) {

        int u = -1;
        for (int i = 0; i < g->n; i++) {
            if (!visited[i] && dist[i] != INT_MAX) {
                if (u == -1 || dist[i] < dist[u])
                    u = i;
            }
        }
        if (u == -1) break;

        visited[u] = 1;


        for (Edge *e = g->adj[u]; e != NULL; e = e->next) {
            int v = e->dest;
            int w = e->weight;
            if (!visited[v] && dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                prev[v] = u;
            }
        }
    }
}


void printPath(int *prev, int src, int node) {
    if (node == src) {
        printf("%d", src);
        return;
    }
    printPath(prev, src, prev[node]);
    printf(" -> %d", node);
}


static void markPath(int *prev, int src, int node, int *onPath) {
    onPath[node] = 1;
    if (node == src) return;
    markPath(prev, src, prev[node], onPath);
}


int buildPathMask(int *prev, int src, int dst, int n, int *onPath) {
    memset(onPath, 0, n * sizeof(int));


    if (dst != src && prev[dst] == -1)
        return 0;

    markPath(prev, src, dst, onPath);
    return 1;
}



// ─── GUI: ───
#define PATH_EDGE_COLOR    YELLOW       /* highlighted edge color          */
#define PATH_EDGE_THICK    4.0f         /* highlighted edge line thickness */
#define PATH_NODE_COLOR    ORANGE       /* highlighted node fill color     */
#define PATH_NODE_BORDER   RED          /* highlighted node border color   */
#define INFO_BG_COLOR      ((Color){0, 0, 0, 180})  /* semi-transparent black */
#define INFO_TEXT_COLOR    WHITE


static void drawArrow(float x1, float y1, float x2, float y2,
                      Color color, float thick) {

    float dx = x2 - x1, dy = y2 - y1;
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 1.0f) return;
    float ux = dx / len, uy = dy / len;

    float startX = x1 + ux * 20.0f;
    float endX   = x2 - ux * 20.0f;
    float startY = y1 + uy * 20.0f;
    float endY   = y2 - uy * 20.0f;

    DrawLineEx((Vector2){startX, startY}, (Vector2){endX, endY}, thick, color);


    float headLen = 12.0f, headHalf = 6.0f;
    float perpX = -uy, perpY = ux;
    Vector2 tip  = {endX, endY};
    Vector2 left = {endX - ux * headLen + perpX * headHalf,
                    endY - uy * headLen + perpY * headHalf};
    Vector2 right = {endX - ux * headLen - perpX * headHalf,
                     endY - uy * headLen - perpY * headHalf};
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
                if (onPath[v] && prev[v] == u) {
                    // This edge (u->v) is on the shortest path
                    drawArrow(x[u], y[u], x[v], y[v],
                              PATH_EDGE_COLOR, PATH_EDGE_THICK);
                }
            }
        }
    }


    if (pathExists) {
        for (int i = 0; i < g->n; i++) {
            if (!onPath[i]) continue;

            DrawCircle((int)x[i], (int)y[i], radius, PATH_NODE_COLOR);

            DrawCircleLines((int)x[i], (int)y[i], radius, PATH_NODE_BORDER);

            char label[8];
            snprintf(label, sizeof(label), "%d", i);
            int textW = MeasureText(label, 18);
            DrawText(label,
                     (int)(x[i] - textW / 2.0f),
                     (int)(y[i] - 9),
                     18, BLACK);
        }
    }


    int panelX = 10, panelY = 10, panelW = 280, panelH = 70;
    DrawRectangle(panelX, panelY, panelW, panelH, INFO_BG_COLOR);
    DrawRectangleLines(panelX, panelY, panelW, panelH, GRAY);

    if (src == dst) {

        char line1[64];
        snprintf(line1, sizeof(line1), "Path: %d  (source = destination)", src);
        DrawText(line1, panelX + 8, panelY + 10, 16, INFO_TEXT_COLOR);
        DrawText("Total weight: 0", panelX + 8, panelY + 34, 16, INFO_TEXT_COLOR);

    } else if (!pathExists || dist[dst] == INT_MAX) {
        DrawText("No path found", panelX + 8, panelY + 10, 18, RED);
        char query[64];
        snprintf(query, sizeof(query), "Query: %d -> %d", src, dst);
        DrawText(query, panelX + 8, panelY + 36, 16, LIGHTGRAY);

    } else {
        char query[64], weight[64];
        snprintf(query,  sizeof(query),  "Shortest path: %d -> %d", src, dst);
        snprintf(weight, sizeof(weight), "Total weight: %d", dist[dst]);
        DrawText(query,  panelX + 8, panelY + 10, 16, INFO_TEXT_COLOR);
        DrawText(weight, panelX + 8, panelY + 34, 18, YELLOW);
    }
}
