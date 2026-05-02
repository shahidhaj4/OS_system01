#include "drawing.h"
#include "raylib.h"
#include <stdio.h>


#define TRAIN_COLOR      (Color){220, 50, 50, 255}
#define TRAIN_BORDER     (Color){120, 10, 10, 255}
#define TRAIN_LABEL      WHITE
#define ARRIVAL_BG       (Color){0, 0, 0, 200}
#define ARRIVAL_BORDER   (Color){255, 215, 0, 255}
#define ARRIVAL_TITLE    (Color){255, 215, 0, 255}
#define ARRIVAL_TEXT     WHITE

void drawEntity(float px, float py) {
        DrawCircle((int)px, (int)py, NODE_R + 8,
               (Color){220, 50, 50, 80});

        DrawCircle((int)px, (int)py, NODE_R, TRAIN_COLOR);
    DrawCircleLines((int)px, (int)py, NODE_R, TRAIN_BORDER);

        int textW = MeasureText("T", 20);
    DrawText("T",
             (int)(px - textW / 2.0f),
             (int)(py - 10),
             20, TRAIN_LABEL);
}

void drawArrivalMessage(int dst, int totalWeight, const char *stationNames[]) {
       DrawRectangle(200, 270, 500, 160, ARRIVAL_BG);
    DrawRectangleLines(200, 270, 500, 160, ARRIVAL_BORDER);


    const char *title = "Arrived at Destination!";
    int titleW = MeasureText(title, 28);
    DrawText(title, 450 - titleW / 2, 290, 28, ARRIVAL_TITLE);

        char station[128];
    if (stationNames && stationNames[dst])
        snprintf(station, sizeof(station), "Station: %s (%d)", stationNames[dst], dst);
    else
        snprintf(station, sizeof(station), "Node: %d", dst);

    int stW = MeasureText(station, 22);
    DrawText(station, 450 - stW / 2, 332, 22, ARRIVAL_TEXT);

       char weightStr[64];
    snprintf(weightStr, sizeof(weightStr), "Total journey weight: %d", totalWeight);
    int wW = MeasureText(weightStr, 20);
    DrawText(weightStr, 450 - wW / 2, 366, 20, ARRIVAL_TEXT);

       const char *hint = "Press ESC to exit";
    int hW = MeasureText(hint, 16);
    DrawText(hint, 450 - hW / 2, 398, 16, GRAY);
}

void drawAnimationFrame(float px, float py, int dst, int arrived,
                        int totalWeight, const char *stationNames[]) {
    if (arrived) {
               drawEntity(px, py);
                drawArrivalMessage(dst, totalWeight, stationNames);
    } else {
                drawEntity(px, py);
    }
}
