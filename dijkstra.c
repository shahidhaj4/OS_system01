#include <stdio.h>
#include <limits.h>
#include "dijkstra.h"
#include "graph.h"

#define MAX_NODES 100

/* ─── Dijkstra ────────────────────────────────────────────────── */

/*
 * Simple O(N^2) Dijkstra using arrays (no fancy heap needed for <=100 nodes).
 *
 * dist[i]   = shortest distance from src to node i (INT_MAX if unreachable)
 * prev[i]   = previous node on the shortest path to i (-1 if none)
 * visited[i] = 1 if node i has been finalized
 */
void dijkstra(Graph *g, int src, int *dist, int *prev) {
    int visited[MAX_NODES] = {0};

    // Initialize distances to infinity
    for (int i = 0; i < g->n; i++) {
        dist[i] = INT_MAX;
        prev[i] = -1;
    }
    dist[src] = 0;

    for (int iter = 0; iter < g->n; iter++) {
        // Pick the unvisited node with smallest tentative distance
        int u = -1;
        for (int i = 0; i < g->n; i++) {
            if (!visited[i] && dist[i] != INT_MAX) {
                if (u == -1 || dist[i] < dist[u])
                    u = i;
            }
        }
        if (u == -1) break; // all remaining nodes are unreachable

        visited[u] = 1;

        // Relax all edges going out from u
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

/* ─── Path Printing ───────────────────────────────────────────── */

/*
 * Recursively prints the path from src to node using prev[].
 * Output format: 0 -> 2 -> 1 -> 3
 */
void printPath(int *prev, int src, int node) {
    if (node == src) {
        printf("%d", src);
        return;
    }
    printPath(prev, src, prev[node]);
    printf(" -> %d", node);
}
