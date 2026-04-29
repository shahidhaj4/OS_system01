#include <stdlib.h>
#include <stdio.h>
#include "graph.h"

// Create an empty graph
Graph* create_graph(int n, int m) {
    Graph *g = malloc(sizeof(Graph));
    if (g == NULL) return NULL;

    g->n = n;
    g->m = m;
    for (int i = 0; i < n; i++)
        g->adj[i] = NULL;
    return g;
}

// Add a directed edge src->dest with given weight
void add_edge(Graph *g, int src, int dest, int weight) {
    Edge *e = malloc(sizeof(Edge));
    if (e == NULL) return;

    e->dest   = dest;
    e->weight = weight;
    e->next   = g->adj[src];
    g->adj[src] = e;
}

Graph* readGraph(char *filename, int *src, int *dst) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) return NULL;

    int n, m;
    if (fscanf(file, "%d %d", src, dst) != 2) {
        fclose(file);
        return NULL;
    }

    if (fscanf(file, "%d %d", &n, &m) != 2) {
        fclose(file);
        return NULL;
    }

    Graph *g = create_graph(n, m);
    if (g == NULL) {
        fclose(file);
        return NULL;
    }

    for (int i = 0; i < m; i++) {
        int u, v, w;
        if (fscanf(file, "%d %d %d", &u, &v, &w) == 3) {
            add_edge(g, u, v, w);
        }
    }

    fclose(file);
    return g;
}

// Free all memory used by the graph
void freeGraph(Graph *g) {
    if (g == NULL) return;

    for (int i = 0; i < g->n; i++) {
        Edge *cur = g->adj[i];
        while (cur) {
            Edge *tmp = cur->next;
            free(cur);
            cur = tmp;
        }
    }
    free(g);
}