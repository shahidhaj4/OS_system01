#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "dijkstra.h"
#include "file_reader.h"
#include "graph.h"

static Graph* build_graph_from_data(const GraphData* data) {
    Graph* g = create_graph(data->n, data->m);
    if (!g) return NULL;

    for (int i = 0; i < data->m; i++) {
        add_edge(g, data->edges[i].src, data->edges[i].dst, data->edges[i].weight);
    }

    return g;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <graph_file>\n", argv[0]);
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

    int dist[MAX_NODES];
    int prev[MAX_NODES];
    int path[MAX_NODES];
    int path_len = 0;

    dijkstra(g, src, dist, prev);

    if (!extractPath(prev, src, dst, path, &path_len) || dist[dst] == INT_MAX) {
        printf("No path found\n");
    } else {
        reversePath(path, path_len);
        for (int i = 0; i < path_len; i++) {
            if (i > 0) printf(" -> ");
            printf("%d", path[i]);
        }
        printf("\n%d\n", dist[dst]);
    }

    freeGraph(g);
    free_graph_data(data);
    return 0;
}
