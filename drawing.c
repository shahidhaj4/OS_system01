#include "drawing.h"
#include "raylib.h"
#include <stdio.h>


const Color TRAVELER_COLORS[MAX_TRAVELERS] = {
    {220, 50,  50,  255},   /* 0 - red       */
    {50,  150, 220, 255},   /* 1 - blue      */
    {50,  200, 80,  255},   /* 2 - green     */
    {255, 165, 0,   255},   /* 3 - orange    */
    {180, 50,  220, 255},   /* 4 - purple    */
    {0,   200, 200, 255},   /* 5 - cyan      */
    {255, 220, 0,   255},   /* 6 - yellow    */
    {255, 100, 180, 255},   /* 7 - pink      */
    {100, 180, 100, 255},   /* 8 - light green */
    {180, 120, 60,  255},   /* 9 - brown     */
};

#define ARRIVAL_BG      (Color){0,   0,   0,   200}
#define ARRIVAL_BORDER  (Color){255, 215, 0,   255}
#define ARRIVAL_TITLE   (Color){255, 215, 0,   255}
#define ARRIVAL_TEXT    WHITE

void drawEntity(float px, float py, Color color, int travelerIndex) {

    Color glow = {color.r, color.g, color.b, 80};
    DrawCircle((int)px, (int)py, NODE_R + 8, glow);

    /* Main body */
    DrawCircle((int)px, (int)py, NODE_R, color);

        Color border = {
        (unsigned char)(color.r / 2),
        (unsigned char)(color.g / 2),
        (unsigned char)(color.b / 2),
        255
    };
    DrawCircleLines((int)px, (int)py, NODE_R, border);


    char label[4];
    snprintf(label, sizeof(label), "%d", travelerIndex);
    int textW = MeasureText(label, 20);
    DrawText(label,
             (int)(px - textW / 2.0f),
             (int)(py - 10),
             20, WHITE);
}

void drawArrivalMessage(int travelerIndex, int dst, int totalWeight,
                        const char *stationNames[]) {

    int panelY = 250 + travelerIndex * 80;
    Color panelBorder = TRAVELER_COLORS[travelerIndex % MAX_TRAVELERS];

    DrawRectangle(150, panelY, 600, 70, ARRIVAL_BG);
    DrawRectangleLines(150, panelY, 600, 70, panelBorder);

      char line1[128];
    if (stationNames && stationNames[dst])
        snprintf(line1, sizeof(line1), "Traveler %d arrived at: %s (%d)",
                 travelerIndex, stationNames[dst], dst);
    else
        snprintf(line1, sizeof(line1), "Traveler %d arrived at node %d",
                 travelerIndex, dst);

    int l1W = MeasureText(line1, 20);
    DrawText(line1, 450 - l1W / 2, panelY + 10, 20, ARRIVAL_TITLE);

        char line2[64];
    snprintf(line2, sizeof(line2), "Total journey weight: %d", totalWeight);
    int l2W = MeasureText(line2, 18);
    DrawText(line2, 450 - l2W / 2, panelY + 38, 18, ARRIVAL_TEXT);
}

void drawTraveler(int travelerIndex, float px, float py,
                  int dst, int arrived, int totalWeight,
                  const char *stationNames[]) {

    Color color = TRAVELER_COLORS[travelerIndex % MAX_TRAVELERS];

        drawEntity(px, py, color, travelerIndex);

       if (arrived) {
        drawArrivalMessage(travelerIndex, dst, totalWeight, stationNames);
    }
}
