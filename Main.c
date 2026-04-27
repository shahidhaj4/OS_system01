#include "library.h"
#include <stdio.h>
#include <limits.h>
#include "graph.h"
#include "file_reader.h"
#include "dijkstra.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <graph_file>\n", argv[0]);
        return 1;
    }
    int src, dst;
    Graph *g = readGraph(argv[1], &src, &dst);
    if (!g) return 1;

    // Edge case: source == destination
    if (src == dst) {
        printf("%d\n0\n", src);
        freeGraph(g);
        return 0;
    }

    int dist[MAX_NODES];
    int prev[MAX_NODES];
    dijkstra(g, src, dist, prev);

    // Check if destination is reachable
    if (dist[dst] == INT_MAX) {
        printf("No path found\n");
    } else {
        printPath(prev, src, dst);
        printf("\n%d\n", dist[dst]);
    }

    freeGraph(g);
    return 0;
