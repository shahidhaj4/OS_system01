#include "file_reader.h"

static int read_int(FILE* f, int* out) {
    int ch;

    while ((ch = fgetc(f)) != EOF) {
        if (ch == '#') {
            while ((ch = fgetc(f)) != EOF && ch != '\n') {
            }
            continue;
        }

        if (ch == '-' || (ch >= '0' && ch <= '9')) {
            ungetc(ch, f);
            return fscanf(f, "%d", out) == 1;
        }
    }

    return 0;
}

static void free_partial(GraphData* data) {
    if (!data) return;
    free(data->edges);
    free(data->travelers);
    free(data);
}

static GraphData* alloc_graph_data(int n, int m) {
    GraphData* data = (GraphData*)calloc(1, sizeof(GraphData));
    if (!data) {
        printf("Error: Memory allocation failed for GraphData struct\n");
        return NULL;
    }

    data->n = n;
    data->m = m;

    if (data->n <= 0 || data->n > 15 || data->m < 0) {
        printf("Error: Validation failed. N must be between 1 and 15 nodes.\n");
        free(data);
        return NULL;
    }

    if (data->m > 0) {
        data->edges = (EdgeInput*)malloc(data->m * sizeof(EdgeInput));
        if (!data->edges) {
            printf("Error: Memory allocation failed for edges array\n");
            free(data);
            return NULL;
        }
    }

    return data;
}

static int read_edges(FILE* f, GraphData* data) {
    for (int i = 0; i < data->m; i++) {
        if (!read_int(f, &data->edges[i].src) ||
            !read_int(f, &data->edges[i].dst) ||
            !read_int(f, &data->edges[i].weight)) {
            printf("Error: Invalid edge format at edge %d\n", i + 1);
            return 0;
        }

        if (data->edges[i].weight < 0) {
            printf("Error: Negative weight found at edge %d\n", i + 1);
            return 0;
        }

        if (data->edges[i].src < 0 || data->edges[i].src >= data->n ||
            data->edges[i].dst < 0 || data->edges[i].dst >= data->n) {
            printf("Error: Node index out of bounds at edge %d\n", i + 1);
            return 0;
        }
    }

    return 1;
}

GraphData* read_file(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        printf("File error: Could not open file %s\n", filename);
        return NULL;
    }

    int n, m;
    if (!read_int(f, &n) || !read_int(f, &m)) {
        printf("Error: Invalid file format (N or M missing)\n");
        fclose(f);
        return NULL;
    }

    GraphData* data = alloc_graph_data(n, m);
    if (!data) {
        fclose(f);
        return NULL;
    }

    if (!read_edges(f, data)) {
        fclose(f);
        free_partial(data);
        return NULL;
    }

    if (!read_int(f, &data->traveler_count)) {
        printf("Error: Missing traveler count\n");
        fclose(f);
        free_partial(data);
        return NULL;
    }

    if (data->traveler_count <= 0) {
        printf("Error: Invalid traveler count\n");
        fclose(f);
        free_partial(data);
        return NULL;
    }

    data->travelers = malloc(data->traveler_count * sizeof(TravelerInput));
    if (!data->travelers) {
        printf("Error: Memory allocation failed for travelers array\n");
        fclose(f);
        free_partial(data);
        return NULL;
    }

    for (int i = 0; i < data->traveler_count; i++) {
        if (!read_int(f, &data->travelers[i].src) ||
            !read_int(f, &data->travelers[i].dst)) {
            printf("Error: Invalid traveler format\n");
            fclose(f);
            free_partial(data);
            return NULL;
        }

        if (data->travelers[i].src < 0 || data->travelers[i].src >= data->n ||
            data->travelers[i].dst < 0 || data->travelers[i].dst >= data->n) {
            printf("Error: Traveler node out of bounds\n");
            fclose(f);
            free_partial(data);
            return NULL;
        }
    }

    fclose(f);
    return data;
}

GraphData* read_single_query_file(const char* filename, int* src, int* dst) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        printf("File error: Could not open file %s\n", filename);
        return NULL;
    }

    int n, m;
    if (!read_int(f, &n) || !read_int(f, &m)) {
        printf("Error: Invalid file format (N or M missing)\n");
        fclose(f);
        return NULL;
    }

    GraphData* data = alloc_graph_data(n, m);
    if (!data) {
        fclose(f);
        return NULL;
    }

    if (!read_edges(f, data)) {
        fclose(f);
        free_partial(data);
        return NULL;
    }

    int rest[256];
    int rest_count = 0;
    while (rest_count < (int)(sizeof(rest) / sizeof(rest[0])) &&
           read_int(f, &rest[rest_count])) {
        rest_count++;
    }

    if (rest_count == 2) {
        *src = rest[0];
        *dst = rest[1];
    } else if (rest_count >= 3 && (rest_count - 1) % 2 == 0) {
        *src = rest[1];
        *dst = rest[2];
    } else {
        printf("Error: Missing source and destination query\n");
        fclose(f);
        free_partial(data);
        return NULL;
    }

    if (*src < 0 || *src >= data->n || *dst < 0 || *dst >= data->n) {
        printf("Error: Query node out of bounds\n");
        fclose(f);
        free_partial(data);
        return NULL;
    }

    fclose(f);
    return data;
}

void free_graph_data(GraphData* data) {
    if (!data) return;
    free(data->edges);
    free(data->travelers);
    free(data);
}
