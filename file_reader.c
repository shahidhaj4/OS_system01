#include "file_reader.h"

GraphData* read_file(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        printf("File error: Could not open file %s\n", filename);
        return NULL;
    }

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
    if (fscanf(f, "%d", &data->traveler_count) != 1) {
        printf("Error: Missing traveler count\n");
        fclose(f);
        free(data->edges);
        free(data);
        return NULL;
    }

    if (data->traveler_count <= 0) {
        printf("Error: Invalid traveler count\n");
        fclose(f);
        free(data->edges);
        free(data);
        return NULL;
    }

    data->travelers = malloc(data->traveler_count * sizeof(TravelerInput));
    if (!data->travelers) {
        printf("Error: Memory allocation failed for travelers\n");
        fclose(f);
        free(data->edges);
        free(data);
        return NULL;
    }

    for (int i = 0; i < data->traveler_count; i++) {
        if (fscanf(f, "%d %d", &data->travelers[i].src, &data->travelers[i].dst) != 2) {
            printf("Error: Invalid traveler format\n");
            fclose(f);
            free(data->travelers);
            free(data->edges);
            free(data);
            return NULL;
        }

        if (data->travelers[i].src < 0 || data->travelers[i].src >= data->n ||
            data->travelers[i].dst < 0 || data->travelers[i].dst >= data->n) {
            printf("Error: Traveler node out of bounds\n");
            fclose(f);
            free(data->travelers);
            free(data->edges);
            free(data);
            return NULL;
            }
    }

    // Close the file and return the populated structure
    fclose(f);
    return data;
}


void free_graph_data(GraphData* data) {
    if (!data) return;
    free(data->edges);
    free(data->travelers);
    free(data);

}