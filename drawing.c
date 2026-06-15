#include "drawing.h"
#include "raylib.h"
#include <stdio.h>
#include <string.h>
#include <math.h>


const Color TRAVELER_COLORS[MAX_TRAVELERS] = {
    {220, 50,  50,  255},   /* 0 – red         */
    {50,  150, 220, 255},   /* 1 – blue        */
    {50,  200, 80,  255},   /* 2 – green       */
    {255, 165, 0,   255},   /* 3 – orange      */
    {180, 50,  220, 255},   /* 4 – purple      */
    {0,   200, 200, 255},   /* 5 – cyan        */
    {255, 220, 0,   255},   /* 6 – yellow      */
    {255, 100, 180, 255},   /* 7 – pink        */
    {100, 180, 100, 255},   /* 8 – light green */
    {180, 120, 60,  255},   /* 9 – brown       */
};


#define ARRIVAL_BG      (Color){0,   0,   0,   200}
#define ARRIVAL_TITLE   (Color){255, 215, 0,   255}
#define ARRIVAL_TEXT    WHITE
#define LOG_BG          (Color){0,   0,   0,   180}
#define LOG_BORDER      (Color){80,  80,  80,  255}
#define LOG_TEXT        (Color){180, 255, 180, 255}
#define LOG_TITLE_COLOR (Color){255, 215, 0,   255}
#define NODE_COLOR      ORANGE
#define NODE_BORDER     (Color){180, 90,  0,   255}
#define EDGE_COLOR      BLACK
#define WEIGHT_COLOR    BLUE
#define LABEL_BG        (Color){255, 255, 255, 220}
#define LABEL_BORDER    GRAY


#define WAITING_COLOR   (Color){255, 230, 0,   255}  /* bright yellow        */
#define WAITING_BORDER  (Color){180, 130, 0,   255}  /* dark gold            */
#define INSIDE_FILL     WHITE                         /* white = owns mutex   */
#define INSIDE_GLOW     (Color){255, 255, 180, 120}  /* warm glow            */
#define BADGE_BG        (Color){0,   0,   0,   200}


static char logLines[MAX_LOG_LINES][MAX_LOG_LINE_LEN];
static int  logCount = 0;

void addLogLine(const char *line) {
    if (logCount < MAX_LOG_LINES) {
        strncpy(logLines[logCount], line, MAX_LOG_LINE_LEN - 1);
        logLines[logCount][MAX_LOG_LINE_LEN - 1] = '\0';
        logCount++;
    } else {
        for (int i = 0; i < MAX_LOG_LINES - 1; i++)
            strncpy(logLines[i], logLines[i + 1], MAX_LOG_LINE_LEN);
        strncpy(logLines[MAX_LOG_LINES - 1], line, MAX_LOG_LINE_LEN - 1);
        logLines[MAX_LOG_LINES - 1][MAX_LOG_LINE_LEN - 1] = '\0';
    }
}

void drawLogPanel(void) {
    if (logCount == 0) return;

    int panelX = 10, panelY = 320;
    int panelW = 470, lineH = 18;
    int panelH = 28 + logCount * lineH;

    DrawRectangle(panelX, panelY, panelW, panelH, LOG_BG);
    DrawRectangleLines(panelX, panelY, panelW, panelH, LOG_BORDER);
    DrawText("IPC Log (pipe messages):",
             panelX + 8, panelY + 6, 14, LOG_TITLE_COLOR);

    for (int i = 0; i < logCount; i++)
        DrawText(logLines[i],
                 panelX + 8, panelY + 24 + i * lineH,
                 14, LOG_TEXT);
}


void drawEntity(float px, float py, Color color, int travelerIndex) {

    Color glow = {color.r, color.g, color.b, 80};
    DrawCircle((int)px, (int)py, NODE_R + 8, glow);
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
    DrawText(label, (int)(px - textW / 2.0f), (int)(py - 10), 20, WHITE);
}


void drawWaitingIndicator(float px, float py, int slotIndex) {
    (void)slotIndex;
    /* "WAIT" badge above the circle */
    const char *badge = "WAIT";
    int bw = MeasureText(badge, 13);
    int bx = (int)(px - bw / 2.0f);
    int by = (int)(py - NODE_R - 20);

    DrawRectangle(bx - 4, by - 2, bw + 8, 17, BADGE_BG);
    DrawRectangleLines(bx - 4, by - 2, bw + 8, 17, WAITING_BORDER);
    DrawText(badge, bx, by, 13, WAITING_COLOR);
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
}


void drawTraveler(int travelerIndex, float px, float py,
                  int dst, int arrived, int totalWeight,
                  const char *stationNames[],
                  TravelerState state,
                  int slotIndex) {

    Color baseColor = TRAVELER_COLORS[travelerIndex % MAX_TRAVELERS];


    float drawX = px;
    float drawY = py;

    if (state == WAITING && slotIndex > 0) {
        float spread = 30.0f;
        float sign   = (slotIndex % 2 == 1) ? 1.0f : -1.0f;
        float mag    = (float)((slotIndex + 1) / 2);
        drawX += sign * mag * spread;
        drawY -= 15.0f;
    }


    switch (state) {

        case WAITING: {

            DrawCircle((int)drawX, (int)drawY, NODE_R + 10,
                       (Color){255, 230, 0, 60});

            DrawCircle((int)drawX, (int)drawY, NODE_R, WAITING_COLOR);
            DrawCircleLines((int)drawX, (int)drawY, NODE_R, WAITING_BORDER);


            char label[4];
            snprintf(label, sizeof(label), "%d", travelerIndex);
            int lw = MeasureText(label, 20);
            DrawText(label, (int)(drawX - lw / 2.0f), (int)(drawY - 10),
                     20, BLACK);


            drawWaitingIndicator(drawX, drawY, slotIndex);
            break;
        }

        case INSIDE_NODE: {

            DrawCircle((int)drawX, (int)drawY, NODE_R + 10, INSIDE_GLOW);

            DrawCircle((int)drawX, (int)drawY, NODE_R, INSIDE_FILL);
            DrawCircleLines((int)drawX, (int)drawY, NODE_R + 2, baseColor);


            char label[4];
            snprintf(label, sizeof(label), "%d", travelerIndex);
            int lw = MeasureText(label, 20);
            DrawText(label, (int)(drawX - lw / 2.0f), (int)(drawY - 10),
                     20, baseColor);


            const char *badge = "IN";
            int bw = MeasureText(badge, 13);
            int bx = (int)(drawX - bw / 2.0f);
            int by = (int)(drawY - NODE_R - 20);
            DrawRectangle(bx - 4, by - 2, bw + 8, 17, BADGE_BG);
            DrawRectangleLines(bx - 4, by - 2, bw + 8, 17, baseColor);
            DrawText(badge, bx, by, 13, baseColor);
            break;
        }

        case FINISHED: {

            Color grey = {150, 150, 150, 255};
            DrawCircle((int)drawX, (int)drawY, NODE_R, grey);
            DrawCircleLines((int)drawX, (int)drawY, NODE_R,
                            (Color){80, 80, 80, 255});

            char label[4];
            snprintf(label, sizeof(label), "%d", travelerIndex);
            int lw = MeasureText(label, 20);
            DrawText(label, (int)(drawX - lw / 2.0f), (int)(drawY - 10),
                     20, WHITE);
            break;
        }

        case MOVING:
        default:

            drawEntity(drawX, drawY, baseColor, travelerIndex);
            break;
    }


    if (arrived) {
        drawArrivalMessage(travelerIndex, dst, totalWeight, stationNames);
    }
}


void drawGraph(Graph *g, float *x, float *y,
               float radius, const char *stationNames[]) {

    for (int u = 0; u < g->n; u++) {
        for (Edge *e = g->adj[u]; e != NULL; e = e->next) {
            int v = e->dest;

            DrawLineEx((Vector2){x[u], y[u]},
                       (Vector2){x[v], y[v]}, 3, EDGE_COLOR);

            float dx = x[v] - x[u];
            float dy = y[v] - y[u];
            float len = sqrtf(dx * dx + dy * dy);
            if (len > 0) {
                float ux = dx / len, uy = dy / len;
                Vector2 tip   = {x[v] - ux * radius, y[v] - uy * radius};
                Vector2 left  = {tip.x - ux*12 - uy*6, tip.y - uy*12 + ux*6};
                Vector2 right = {tip.x - ux*12 + uy*6, tip.y - uy*12 - ux*6};
                DrawTriangle(tip, left, right, DARKGRAY);
            }

            int midX = (int)((x[u] + x[v]) / 2);
            int midY = (int)((y[u] + y[v]) / 2);
            char weight[16];
            snprintf(weight, sizeof(weight), "%d", e->weight);
            DrawText(weight, midX, midY, 18, WEIGHT_COLOR);
        }
    }

    float centerX = 900 / 2.0f;
    float centerY = 700 / 2.0f;

    for (int i = 0; i < g->n; i++) {
        DrawCircle((int)x[i], (int)y[i], radius, NODE_COLOR);
        DrawCircleLines((int)x[i], (int)y[i], radius, NODE_BORDER);

        char num[8];
        snprintf(num, sizeof(num), "%d", i);
        int numW = MeasureText(num, 18);
        DrawText(num, (int)(x[i] - numW / 2.0f), (int)(y[i] - 9), 18, BLACK);

        char label[64];
        if (stationNames && stationNames[i])
            snprintf(label, sizeof(label), "%s", stationNames[i]);
        else
            snprintf(label, sizeof(label), "Node %d", i);

        int textW = MeasureText(label, 16);
        int textX, textY;

        if (x[i] > centerX + 20) {
            textX = (int)(x[i] + radius + 5);
            textY = (int)(y[i] - 8);
        } else if (x[i] < centerX - 20) {
            textX = (int)(x[i] - textW - radius - 5);
            textY = (int)(y[i] - 8);
        } else if (y[i] < centerY) {
            textX = (int)(x[i] - textW / 2);
            textY = (int)(y[i] - radius - 20);
        } else {
            textX = (int)(x[i] - textW / 2);
            textY = (int)(y[i] + radius + 8);
        }

        int padding = 3;
        DrawRectangle(textX - padding, textY - padding,
                      textW + padding * 2, 16 + padding * 2, LABEL_BG);
        DrawRectangleLines(textX - padding, textY - padding,
                           textW + padding * 2, 16 + padding * 2, LABEL_BORDER);

        DrawText(label, textX + 1, textY + 1, 16, DARKGRAY);
        DrawText(label, textX,     textY,     16, BLACK);
    }
}