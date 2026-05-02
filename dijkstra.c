#include <stdio.h>
#include <limits.h>
#include <string.h>
#include "dijkstra.h"
#include "graph.h"

#define MAX_NODES 100

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

// Build path from prev[] (reversed: dst -> src)
int extractPath(int *prev, int src, int dst, int *path, int *pathLen) {
    if (dst != src && prev[dst] == -1)
        return 0;

    int current = dst;
    int len = 0;

    while (current != -1) {
        path[len++] = current;
        if (current == src) break;
        current = prev[current];
    }

    *pathLen = len;
    return 1;
}

// Reverse path to correct order (src -> dst)
void reversePath(int *path, int pathLen) {
    for (int i = 0; i < pathLen / 2; i++) {
        int tmp = path[i];
        path[i] = path[pathLen - i - 1];
        path[pathLen - i - 1] = tmp;
    }
}

// Get next node in path
int getNextNodeInPath(int *path, int pathLen, int currentIndex) {
    if (currentIndex < 0 || currentIndex >= pathLen - 1)
        return -1;

    return path[currentIndex + 1];
}