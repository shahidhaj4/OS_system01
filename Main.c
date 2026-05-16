#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

#include "raylib.h"
#include "graph.h"
#include "file_reader.h"
#include "dijkstra.h"
#include "animation.h"
#include "drawing.h"

#define SCREEN_W 900
#define SCREEN_H 700
#define NODE_R 22

#ifndef MAX_TRAVELERS
#define MAX_TRAVELERS 10
#endif

const char *stationNames[MAX_NODES] = {
    "Tel Aviv", "Haifa", "Jerusalem", "Beer Sheva", "Nazareth",
    "Eilat", "Netanya", "Ashdod", "Tiberias", "Acre",
    "Rishon LeZion", "Herzliya", "Petah Tikva", "Ramla", "Lod"
};

/*
 * Global graph used by animation.c.
 * animation.c has: extern Graph g_graph;
 * Each child process uses this graph to calculate Dijkstra independently.
 */
Graph g_graph;

static int calculate_total_weight(Graph *g, int *path, int path_size) {
    int total = 0;

    for (int i = 0; i < path_size - 1; i++) {
        int u = path[i];
        int v = path[i + 1];

        int weight = get_edge_weight(g, u, v);
        if (weight > 0) {
            total += weight;
        }
    }

    return total;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <graph_file>\n", argv[0]);
        return 1;
    }

    /* Read Milestone 5 input file */
    GraphData *data = read_file(argv[1]);
    if (!data) {
        return 1;
    }

    /* Build graph from GraphData */
    Graph *g = create_graph(data->n, data->m);
    if (!g) {
        free_graph_data(data);
        return 1;
    }

    for (int i = 0; i < data->m; i++) {
        add_edge(g,
                 data->edges[i].src,
                 data->edges[i].dst,
                 data->edges[i].weight);
    }

    /*
     * Copy graph into global graph.
     * This is needed because child processes in animation.c use g_graph.
     */
    g_graph = *g;

    int num_travelers = data->traveler_count;

    if (num_travelers <= 0 || num_travelers > MAX_TRAVELERS) {
        printf("Error: invalid number of travelers. Max is %d\n", MAX_TRAVELERS);
        freeGraph(g);
        free_graph_data(data);
        return 1;
    }

    Traveler travelers[MAX_TRAVELERS];
    int totalWeights[MAX_TRAVELERS];

    /*
     * Milestone 5:
     * The parent does NOT calculate Dijkstra paths.
     * Each child process calculates its own path independently.
     */
    for (int i = 0; i < num_travelers; i++) {
        travelers[i].src_node = data->travelers[i].src;
        travelers[i].dest_node = data->travelers[i].dst;

        travelers[i].path = NULL;
        travelers[i].path_size = 0;

        travelers[i].current_node = travelers[i].src_node;
        travelers[i].next_node = -1;

        travelers[i].current_path_index = 0;
        travelers[i].current_jump_step = 0;

        travelers[i].time_counter = 0;
        travelers[i].is_waiting_at_node = 0;

        travelers[i].is_active = 1;

        travelers[i].current_x = 0.0f;
        travelers[i].current_y = 0.0f;

        travelers[i].child_pid = -1;

        totalWeights[i] = 0;
    }

    /* Create child process for each traveler */
    spawn_traveler_processes(travelers, num_travelers, data);

    /* Calculate node positions */
    float x[MAX_NODES];
    float y[MAX_NODES];

    for (int i = 0; i < g->n; i++) {
        float angle = 2.0f * PI * i / g->n;
        x[i] = SCREEN_W / 2.0f + cosf(angle) * 230.0f;
        y[i] = SCREEN_H / 2.0f + sinf(angle) * 230.0f;
    }

    InitWindow(SCREEN_W, SCREEN_H, "Milestone 5 - IPC Travelers");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        /*
         * Parent receives updates from all child processes.
         * The parent also prints the required terminal logs.
         */
        update_all_travelers_from_pipes(travelers, num_travelers);

        for (int i = 0; i < num_travelers; i++) {
            update_traveler_animation(&travelers[i], data);
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText("Milestone 5 - IPC Travelers", 20, 20, 24, BLACK);
        DrawText("Children calculate paths and send updates to parent using pipes",
                 20, 55, 18, DARKGRAY);

        /* Draw edges */
        for (int u = 0; u < g->n; u++) {
            for (Edge *e = g->adj[u]; e != NULL; e = e->next) {
                int v = e->dest;

                DrawLineEx((Vector2){x[u], y[u]},
                           (Vector2){x[v], y[v]},
                           4,
                           BLACK);

                float dx = x[v] - x[u];
                float dy = y[v] - y[u];
                float len = sqrtf(dx * dx + dy * dy);

                if (len > 0) {
                    float ux = dx / len;
                    float uy = dy / len;

                    Vector2 tip = {
                        x[v] - ux * NODE_R,
                        y[v] - uy * NODE_R
                    };

                    Vector2 left = {
                        tip.x - ux * 12 - uy * 6,
                        tip.y - uy * 12 + ux * 6
                    };

                    Vector2 right = {
                        tip.x - ux * 12 + uy * 6,
                        tip.y - uy * 12 - ux * 6
                    };

                    DrawTriangle(tip, left, right, DARKGRAY);
                }

                int midX = (int)((x[u] + x[v]) / 2);
                int midY = (int)((y[u] + y[v]) / 2);

                char weightText[16];
                snprintf(weightText, sizeof(weightText), "%d", e->weight);
                DrawText(weightText, midX, midY, 18, BLUE);
            }
        }

        /* Draw nodes */
        for (int i = 0; i < g->n; i++) {
            DrawCircle((int)x[i], (int)y[i], NODE_R, ORANGE);
            DrawCircleLines((int)x[i], (int)y[i], NODE_R, BROWN);

            char label[64];
            snprintf(label, sizeof(label), "%d (%s)", i, stationNames[i]);

            int textW = MeasureText(label, 18);
            int textX = (int)(x[i] - textW / 2.0f);
            int textY = (int)(y[i] + NODE_R + 8);

            DrawRectangle(textX - 4,
                          textY - 4,
                          textW + 8,
                          24,
                          (Color){255, 255, 255, 220});

            DrawRectangleLines(textX - 4,
                               textY - 4,
                               textW + 8,
                               24,
                               GRAY);

            DrawText(label, textX, textY, 18, BLACK);
        }

        /*
         * Draw all travelers using IPC state:
         * current_node and next_node are updated by pipe messages.
         */
        for (int i = 0; i < num_travelers; i++) {
            int curr = travelers[i].current_node;

            if (curr < 0 || curr >= g->n) {
                continue;
            }

            float px = x[curr];
            float py = y[curr];

            if (travelers[i].is_active &&
                travelers[i].next_node >= 0 &&
                travelers[i].next_node < g->n) {

                int next = travelers[i].next_node;
                float t = travelers[i].current_x;

                px = x[curr] + (x[next] - x[curr]) * t;
                py = y[curr] + (y[next] - y[curr]) * t;
            }

            int arrived = travelers[i].is_active ? 0 : 1;

            drawTraveler(i,
                         px,
                         py,
                         travelers[i].dest_node,
                         arrived,
                         totalWeights[i],
                         stationNames);
        }

        drawLogPanel();

        EndDrawing();
    }

    wait_for_all_children(travelers, num_travelers);

    for (int i = 0; i < num_travelers; i++) {
        if (travelers[i].path) {
            free(travelers[i].path);
        }
    }

    CloseWindow();

    freeGraph(g);
    free_graph_data(data);

    return 0;
}