#include "dijkstra.h"

Node graph[MAX_NODES] = {0};
int num_nodes = 0, num_edges = 0;
int start_node = 0, end_node = 0;

void add_edge(int u, int v, int w) {
    // Fix 6: guard against malloc failure
    Edge* new_edge = (Edge*)malloc(sizeof(Edge));
    if (!new_edge) {
        fprintf(stderr, "Error: malloc failed while adding edge.\n");
        return;
    }
    new_edge->to = v;
    new_edge->weight = w;
    new_edge->next = graph[u].head;
    graph[u].head = new_edge;
}

void dijkstra(int start, int end, int* dist, int* parent) {
    bool visited[MAX_NODES] = {false};
    for (int i = 0; i < num_nodes; i++) {
        dist[i] = INF;
        parent[i] = -1;
    }
    dist[start] = 0;

    for (int i = 0; i < num_nodes; i++) {
        int u = -1;
        int min_dist = INF;
        for (int j = 0; j < num_nodes; j++) {
            if (!visited[j] && dist[j] < min_dist) {
                min_dist = dist[j];
                u = j;
            }
        }

        if (u == -1) break;
        visited[u] = true;
        if (u == end) break;

        for (Edge* e = graph[u].head; e != NULL; e = e->next) {
            int v = e->to;
            int w = e->weight;
            if (dist[u] != INF && dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                parent[v] = u;
            }
        }
    }
}

void print_path(int* parent, int curr) {
    if (parent[curr] == -1) {
        printf("%d", curr);
        return;
    }
    print_path(parent, parent[curr]);
    printf(" -> %d", curr);
}

void free_graph() {
    for (int i = 0; i < MAX_NODES; i++) {
        Edge* curr = graph[i].head;
        while (curr) {
            Edge* temp = curr;
            curr = curr->next;
            free(temp);
        }
        graph[i].head = NULL;
    }
}

bool load_graph(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: cannot open file '%s'.\n", filename);
        return false;
    }

    // Fix 6: detect malformed/truncated header
    if (fscanf(file, "%d %d", &num_nodes, &num_edges) != 2) {
        fprintf(stderr, "Error: failed to read num_nodes and num_edges.\n");
        fclose(file);
        return false;
    }

    // Fix 1: reject graphs that exceed MAX_NODES or are empty
    if (num_nodes <= 0 || num_nodes > MAX_NODES) {
        fprintf(stderr, "Error: num_nodes=%d is out of valid range [1, %d].\n",
                num_nodes, MAX_NODES);
        fclose(file);
        return false;
    }

    // Fix 6: reject nonsensical edge count
    if (num_edges < 0) {
        fprintf(stderr, "Error: num_edges=%d is negative.\n", num_edges);
        fclose(file);
        return false;
    }

    for (int i = 0; i < num_edges; i++) {
        int u, v, w;

        // Fix 6: detect unexpected EOF or malformed edge lines
        if (fscanf(file, "%d %d %d", &u, &v, &w) != 3) {
            fprintf(stderr, "Error: malformed or incomplete edge data at edge %d.\n", i);
            // Fix 3: free any edges already allocated before returning
            free_graph();
            fclose(file);
            return false;
        }

        // Fix 3 (original): reject negative weights
        if (w < 0) {
            fprintf(stderr, "Error: negative weight %d on edge (%d->%d) is not allowed.\n",
                    w, u, v);
            // Fix 3: free previously allocated edges
            free_graph();
            fclose(file);
            return false;
        }

        // Fix 2: validate vertex indices before calling add_edge
        if (u < 0 || u >= num_nodes || v < 0 || v >= num_nodes) {
            fprintf(stderr, "Error: vertex index out of range in edge (%d->%d); "
                            "valid range is [0, %d).\n", u, v, num_nodes);
            // Fix 3: free previously allocated edges
            free_graph();
            fclose(file);
            return false;
        }

        add_edge(u, v, w);
    }

    // Fix 5: read and validate start/end nodes
    if (fscanf(file, "%d %d", &start_node, &end_node) != 2) {
        fprintf(stderr, "Error: failed to read start_node and end_node.\n");
        free_graph();
        fclose(file);
        return false;
    }

    if (start_node < 0 || start_node >= num_nodes) {
        fprintf(stderr, "Error: start_node=%d is out of valid range [0, %d).\n",
                start_node, num_nodes);
        free_graph();
        fclose(file);
        return false;
    }

    if (end_node < 0 || end_node >= num_nodes) {
        fprintf(stderr, "Error: end_node=%d is out of valid range [0, %d).\n",
                end_node, num_nodes);
        free_graph();
        fclose(file);
        return false;
    }

    fclose(file);
    return true;
}

// Main for Milestone 1
#ifdef MILESTONE1_MAIN
int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    if (!load_graph(argv[1])) {
        fprintf(stderr, "Error loading graph from %s\n", argv[1]);
        return 1;
    }

    int dist[MAX_NODES], parent[MAX_NODES];
    dijkstra(start_node, end_node, dist, parent);

    if (start_node == end_node) {
        printf("%d\n0\n", start_node);
    } else if (dist[end_node] == INF) {
        printf("No path found\n");
    } else {
        print_path(parent, end_node);
        printf("\n%d\n", dist[end_node]);
    }

    free_graph();
    return 0;
}
#endif