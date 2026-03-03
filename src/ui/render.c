#include "render.h"
#include <pspdebug.h>
#include <stdio.h>
#include <string.h>

static void draw_scanline_overlay(void) {
    int row;
    for (row = 1; row < 34; row += 2) {
        pspDebugScreenSetXY(0, row);
        pspDebugScreenPrintf("                                                            ");
    }
}

void ui_init(void) {
    pspDebugScreenInit();
}

static void render_boot(const AppState* app) {
    uint64_t elapsed = app->tick;
    int bar = (int)(elapsed % 44);
    int i;

    pspDebugScreenSetXY(0, 0);
    pspDebugScreenPrintf("PSP AI SYSTEM v1.0\n\n");
    pspDebugScreenPrintf("Initializing memory...\n");
    pspDebugScreenPrintf("Loading modules...\n");
    pspDebugScreenPrintf("Connecting AI core...\n\n");
    pspDebugScreenPrintf("[");
    for (i = 0; i < 42; ++i) {
        pspDebugScreenPrintf(i < bar ? "#" : " ");
    }
    pspDebugScreenPrintf("]\n");
    pspDebugScreenPrintf("Status: %s\n", app->status_line);
}

static void render_chat(const AppState* app) {
    int max_lines = 23;
    int start = app->history_count - max_lines;
    int i;

    if (start < 0) start = 0;

    pspDebugScreenSetXY(0, 0);
    pspDebugScreenPrintf("PSP AI TERMINAL      %s      %s\n",
        app->connected ? "NET:ONLINE" : "NET:OFFLINE",
        app->mic_recording ? "MIC:REC" : "MIC:IDLE");
    pspDebugScreenPrintf("============================================================\n");

    for (i = start; i < app->history_count; ++i) {
        const ChatMessage* m = &app->history[i];
        char preview[MAX_MESSAGE_LEN];
        int n = m->visible_chars;
        int len = (int)strlen(m->text);
        if (n > len) n = len;
        memcpy(preview, m->text, n);
        preview[n] = '\0';
        pspDebugScreenPrintf("%s> %s\n", m->role, preview);
    }

    pspDebugScreenPrintf("------------------------------------------------------------\n");
    pspDebugScreenPrintf("> %s%s\n", app->input, app->cursor_blink ? "_" : " ");
    pspDebugScreenPrintf("[%s]\n", app->status_line);
}

static void render_status(const AppState* app) {
    pspDebugScreenSetXY(0, 0);
    pspDebugScreenPrintf("SYSTEM DIAGNOSTICS\n\n");
    pspDebugScreenPrintf("Server: %s\n", app->server_url);
    pspDebugScreenPrintf("Network: %s\n", app->connected ? "Connected" : "Disconnected");
    pspDebugScreenPrintf("Mic: %s\n", app->mic_recording ? "Recording" : "Idle");
    pspDebugScreenPrintf("Messages: %d\n", app->history_count);
    pspDebugScreenPrintf("\nPress SELECT to return.\n");
}

void ui_render(const AppState* app) {
    pspDebugScreenSetBackColor(0x00000000);
    pspDebugScreenSetTextColor(0x00FFFFFF);
    pspDebugScreenClear();

    if (app->screen == APP_SCREEN_BOOT) {
        render_boot(app);
    } else if (app->screen == APP_SCREEN_STATUS) {
        render_status(app);
    } else {
        render_chat(app);
    }

    draw_scanline_overlay();
}
