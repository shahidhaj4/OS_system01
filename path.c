#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>

// Include your project headers
#include "graph.h"
#include "dijkstra.h"
#include "animation.h"
#include "drawing.h"
#include "dijkstra_draw.h"
#include "raylib.h"

#define SCREEN_W 900
#define SCREEN_H 700
#define NODE_R 22

const char *stationNames[MAX_NODES] = {
    "Tel Aviv", "Haifa", "Jerusalem", "Beer Sheva", "Nazareth",
    "Eilat", "Netanya", "Ashdod", "Tiberias", "Acre",
    "Rishon LeZion", "Herzliya", "Petah Tikva", "Ramla", "Lod"
};

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <graph_file>\n", argv[0]);
        return 1;
    }

    int src, dst;
    Graph *g = readGraph(argv[1], &src, &dst);
    if (!g) return 1;

    /* ------------------------------------------------------------- */
    /* 1 & 2. Read Expanded File, Analyze Travelers & Calculate Path*/
    /* ------------------------------------------------------------- */
    Traveler travelers[MAX_TRAVELERS];
    int num_travelers = 3; // Adjusted for multi-agent simulation

    for (int i = 0; i < num_travelers; i++) {
        travelers[i].src_node = src;
        travelers[i].dest_node = dst;

        int t_dist[MAX_NODES];
        int t_prev[MAX_NODES];

        // Calculate shortest path for each traveler
        dijkstra(g, travelers[i].src_node, t_dist, t_prev);

        travelers[i].path = (int*)malloc(MAX_NODES * sizeof(int));
        int pathLen = 0;

        if (extractPath(t_prev, travelers[i].src_node, travelers[i].dest_node, travelers[i].path, &pathLen)) {
            reversePath(travelers[i].path, pathLen);
            travelers[i].path_size = pathLen;
        } else {
            travelers[i].path_size = 0;
        }

        // Initialize animation variables
        travelers[i].current_path_index = 0;
        travelers[i].current_jump_step = 0;
        travelers[i].time_counter = 0;
        travelers[i].is_waiting_at_node = 0;
        travelers[i].is_active = (travelers[i].path_size > 0) ? 1 : 0;
    }

    /* ------------------------------------------------------------- */
    /* 3. Fork Processes for Each Traveler                          */
    /* ------------------------------------------------------------- */
    spawn_traveler_processes(travelers, num_travelers);
    printf("[SYSTEM] Paths analyzed and processes forked for %d travelers.\n", num_travelers);

    /* ------------------------------------------------------------- */
    /* 4. Visual Graphics Loop (Raylib)                            */
    /* ------------------------------------------------------------- */
    float x[MAX_NODES];
    float y[MAX_NODES];

    for (int i = 0; i < g->n; i++) {
        float angle = 2.0f * PI * i / g->n;
        x[i] = SCREEN_W / 2.0f + cosf(angle) * 230.0f;
        y[i] = SCREEN_H / 2.0f + sinf(angle) * 230.0f;
    }

    InitWindow(SCREEN_W, SCREEN_H, "Milestone 4 - Multi-Processing Simulation");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // Update physics/positions for active travelers
        for (int i = 0; i < num_travelers; i++) {
            if (travelers[i].is_active) {
                update_traveler_animation(&travelers[i], g);
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText("Multi-Agent Directed Weighted Graph", 20, 20, 24, BLACK);

        // Draw edges (Lines)
        for (int u = 0; u < g->n; u++) {
            for (Edge *e = g->adj[u]; e != NULL; e = e->next) {
                int v = e->dest;
                DrawLineEx((Vector2){x[u], y[u]}, (Vector2){x[v], y[v]}, 4, BLACK);
            }
        }

        // Draw nodes (Circles)
        for (int i = 0; i < g->n; i++) {
            DrawCircle((int)x[i], (int)y[i], NODE_R, SKYBLUE);
            DrawCircleLines((int)x[i], (int)y[i], NODE_R, DARKBLUE);

            char label[8];
            snprintf(label, sizeof(label), "%d", i);
            DrawText(label, (int)x[i] - 5, (int)y[i] - 8, 18, BLACK);
        }

        // Draw custom simulation frameworks
        for (int i = 0; i < num_travelers; i++) {
            if (travelers[i].is_active) {
                // Approximate rendering using updated coordinates
                int curr_node = travelers[i].path[travelers[i].current_path_index];
                drawTraveler(i, x[curr_node], y[curr_node], travelers[i].dest_node, 0, 0, stationNames);
            }
        }

        EndDrawing();
    }

    /* ------------------------------------------------------------- */
    /* 5. Cleanup and Reaping Children                              */
    /* ------------------------------------------------------------- */
    wait_for_all_children(travelers, num_travelers);

    for (int i = 0; i < num_travelers; i++) {
        free(travelers[i].path);
    }

    CloseWindow();
    freeGraph(g);
    return 0;
}