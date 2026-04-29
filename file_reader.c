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
        printf("Error: Memory allocation failed for GraphData struct\n");
        fclose(f);
        return NULL;
    }

    // 1. Read number of nodes (N) and number of edges (M) from the first line
    if (fscanf(f, "%d %d", &data->n, &data->m) != 2) {
        printf("Error: Invalid file format (N or M missing)\n");
        fclose(f);
        free(data);
        return NULL;
    }

    // Milestone 2 Requirement: Max 15 nodes for GUI clarity
    if (data->n <= 0 || data->n > 15 || data->m < 0) {
        printf("Error: Validation failed. N must be between 1 and 15 nodes.\n");
        fclose(f);
        free(data);
        return NULL;
    }

    //  FIX: Safe allocation for edges
    data->edges = NULL;
    if (data->m > 0) {
        data->edges = (EdgeInput*)malloc(data->m * sizeof(EdgeInput));
        if (!data->edges) {
            printf("Error: Memory allocation failed for edges array\n");
            fclose(f);
            free(data);
            return NULL;
        }
    }

    // 3. Loop to read all M edges (src, dst, weight)
    for (int i = 0; i < data->m; i++) {
        if (fscanf(f, "%d %d %d", 
                   &data->edges[i].src, 
                   &data->edges[i].dst, 
                   &data->edges[i].weight) != 3) {
            printf("Error: Invalid edge format at line %d\n", i + 2);
            fclose(f);
            free(data->edges);
            free(data);
            return NULL;
        }

        // Project requirement: Reject negative weights
        if (data->edges[i].weight < 0) {
            printf("Error: Negative weight found at line %d\n", i + 2);
            fclose(f);
            free(data->edges);
            free(data);
            return NULL;
        }

        // Bound Checking: Ensure node indices are within range [0, N-1]
        if (data->edges[i].src < 0 || data->edges[i].src >= data->n ||
            data->edges[i].dst < 0 || data->edges[i].dst >= data->n) {
            printf("Error: Node index out of bounds at line %d\n", i + 2);
            fclose(f);
            free(data->edges);
            free(data);
            return NULL;
        }
    }

    // 4. Read the start and end nodes for Dijkstra (the last line)
    if (fscanf(f, "%d %d", &data->start, &data->end) != 2) {
        printf("Error: Missing source/destination query at end of file\n");
        fclose(f);
        free(data->edges);
        free(data);
        return NULL;
    }

    // Validate query nodes
    if (data->start < 0 || data->start >= data->n ||
        data->end < 0 || data->end >= data->n) {
        printf("Error: Query nodes out of bounds\n");
        fclose(f);
        free(data->edges);
        free(data);
        return NULL;
    }

    // Close the file and return the populated structure
    fclose(f);
    return data;
}

/**
 * free_graph_data(): Safely releases all allocated memory.
 */
void free_graph_data(GraphData* data) {
    if (!data) return;
    free(data->edges); // Safe even if NULL
    free(data);
}