#include "dijkstra.h"
#include "raylib.h"
#include <math.h>

void draw_arrow(Vector2 start, Vector2 end, float thickness, Color color, int weight) {
    DrawLineEx(start, end, thickness, color);

    float angle = atan2f(end.y - start.y, end.x - start.x);
    float arrow_size = 15.0f;

    Vector2 p1 = { end.x - arrow_size * cosf(angle - PI/6), end.y - arrow_size * sinf(angle - PI/6) };
    Vector2 p2 = { end.x - arrow_size * cosf(angle + PI/6), end.y - arrow_size * sinf(angle + PI/6) };

    DrawTriangle(end, p1, p2, color);

    Vector2 mid = { (start.x + end.x)/2, (start.y + end.y)/2 };
    DrawText(TextFormat("%d", weight), mid.x + 5, mid.y + 5, 20, DARKGRAY);
}

// Fix 4: draw a self-loop as a small circle above the node
void draw_self_loop(Vector2 node_pos, float node_radius, int weight) {
    Vector2 loop_center = { node_pos.x, node_pos.y - node_radius - 18.0f };
    DrawCircleLines((int)loop_center.x, (int)loop_center.y, 18.0f, GRAY);
    // Arrowhead at the bottom of the loop circle, pointing back into the node
    DrawCircleV((Vector2){ loop_center.x, loop_center.y + 18.0f }, 4.0f, GRAY);
    DrawText(TextFormat("%d", weight),
             (int)(loop_center.x + 20), (int)(loop_center.y - 10), 18, DARKGRAY);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    if (!load_graph(argv[1])) {
        fprintf(stderr, "Error loading graph from %s\n", argv[1]);
        return 1;
    }

    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Graph Visualization - Milestone 2");
    SetTargetFPS(60);

    Vector2 positions[MAX_NODES];
    float radius = 220.0f;
    float node_radius = 25.0f;
    Vector2 center = { screenWidth / 2.0f, screenHeight / 2.0f };
    for (int i = 0; i < num_nodes; i++) {
        float angle = (2 * PI * i) / num_nodes;
        positions[i].x = center.x + radius * cosf(angle);
        positions[i].y = center.y + radius * sinf(angle);
    }

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        for (int i = 0; i < num_nodes; i++) {
            for (Edge* e = graph[i].head; e != NULL; e = e->next) {

                // Fix 4: handle self-loops separately to avoid division by zero
                if (i == e->to) {
                    draw_self_loop(positions[i], node_radius, e->weight);
                    continue;
                }

                Vector2 dir = {
                    positions[e->to].x - positions[i].x,
                    positions[e->to].y - positions[i].y
                };
                float len = sqrtf(dir.x * dir.x + dir.y * dir.y);
                Vector2 unit_dir = { dir.x / len, dir.y / len };

                Vector2 start = {
                    positions[i].x    + unit_dir.x * node_radius,
                    positions[i].y    + unit_dir.y * node_radius
                };
                Vector2 end = {
                    positions[e->to].x - unit_dir.x * node_radius,
                    positions[e->to].y - unit_dir.y * node_radius
                };

                draw_arrow(start, end, 2.0f, GRAY, e->weight);
            }
        }

        for (int i = 0; i < num_nodes; i++) {
            DrawCircleV(positions[i], node_radius, LIGHTGRAY);
            DrawCircleLines(positions[i].x, positions[i].y, node_radius, DARKGRAY);
            DrawText(TextFormat("%d", i), positions[i].x - 5, positions[i].y - 10, 20, BLACK);
        }

        DrawText("Milestone 2: Graph Visualization", 10, 10, 20, DARKBLUE);
        EndDrawing();
    }

    CloseWindow();
    free_graph();
    return 0;
}