#ifndef OS_SYSTEM01_GRAPH_H
#define OS_SYSTEM01_GRAPH_H


#define MAX_NODES 100

// One edge in the adjacency list
typedef struct Edge {
    int dest;           // destination node
    int weight;         // edge weight
    struct Edge *next;  // next edge in the list
} Edge;

// The graph itself
typedef struct {
    int n;              // number of nodes
    int m;              // number of edges
    Edge *adj[MAX_NODES]; // adjacency list (one list per node)
} Graph;

/* Function prototypes */
Graph* create_graph(int n, int m);
void add_edge(Graph *g, int src, int dest, int weight);
Graph* readGraph(char *filename, int *src, int *dst);
void freeGraph(Graph *g);
int get_num_nodes(Graph *g);
void print_neighbors(Graph *g, int node);
int get_edge_weight(Graph *g, int src, int dest);
void traverse_all_edges(Graph *g);


#endif //OS_SYSTEM01_GRAPH_H