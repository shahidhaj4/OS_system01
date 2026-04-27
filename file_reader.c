#include "file_reader.h"

GraphData* read_file(const char* filename) {
    // Attempt to open the file for reading
    FILE* f = fopen(filename, "r");
    if (!f) {
        printf("File error: Could not open file %s\n", filename);
        return NULL;
    }

    // Allocate memory for the main GraphData structure
    GraphData* data = (GraphData*)malloc(sizeof(GraphData));
    if (!data) {
        printf("Memory allocation failed for GraphData struct\n");
        fclose(f);
        return NULL;
    }

    // 1. Read number of nodes (N) and number of edges (M) from the first line
    if (fscanf(f, "%d %d", &data->n, &data->m) != 2) {
        printf("Invalid file format: N or M missing\n");
        fclose(f);
        free(data);
        return NULL;
    }

    // Basic validation for graph size
    if (data->n <= 0 || data->m < 0) {
        printf("Invalid graph size\n");
        fclose(f);
        free(data);
        return NULL;
    }

    // 2. Allocate memory for the edges array based on M
    data->edges = (EdgeInput*)malloc(data->m * sizeof(EdgeInput));
    if (data->m > 0 && !data->edges) {
        printf("Memory allocation failed for edges array\n");
        fclose(f);
        free(data);
        return NULL;
    }

    // 3. Loop to read all M edges (src, dst, weight)
    for (int i = 0; i < data->m; i++) {
        if (fscanf(f, "%d %d %d", 
                   &data->edges[i].src, 
                   &data->edges[i].dst, 
                   &data->edges[i].weight) != 3) {
            printf("Invalid edge format at line %d\n", i + 2);
            free(data->edges);
            free(data);
            fclose(f);
            return NULL;
        }

        // Project requirement: Reject negative weights
        if (data->edges[i].weight < 0) {
            printf("Invalid input (negative weight)\n");
            free(data->edges);
            free(data);
            fclose(f);
            return NULL;
        }

        // Ensure node indices are within valid range [0, N-1]
        if (data->edges[i].src < 0 || data->edges[i].src >= data->n ||
            data->edges[i].dst < 0 || data->edges[i].dst >= data->n) {
            printf("Invalid node index in edges list\n");
            free(data->edges);
            free(data);
            fclose(f);
            return NULL;
        }
    }

    // 4. Read the start and end nodes for Dijkstra (the last line)
    if (fscanf(f, "%d %d", &data->start, &data->end) != 2) {
        printf("Invalid query format at the end of file\n");
        free(data->edges);
        free(data);
        fclose(f);
        return NULL;
    }

    // Validate query nodes
    if (data->start < 0 || data->start >= data->n ||
        data->end < 0 || data->end >= data->n) {
        printf("Invalid query nodes\n");
        free(data->edges);
        free(data);
        fclose(f);
        return NULL;
    }

    // Close the file and return the populated structure
    fclose(f);
    return data;
}

// Function to safely free all allocated memory to prevent memory leaks
void free_graph_data(GraphData* data) {
    if (data) {
        if (data->edges) {
            free(data->edges);
        }
        free(data);
    }
}