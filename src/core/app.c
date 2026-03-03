#include "app.h"
#include <string.h>
#include <stdio.h>

void app_init(AppState* app) {
    memset(app, 0, sizeof(*app));
    app->screen = APP_SCREEN_BOOT;
    app->running = 1;
    app->connected = 0;
    app->mic_recording = 0;
    app->boot_done = 0;
    app->history_count = 0;
    app->history_scroll = 0;
    app->input_len = 0;
    app->cursor_blink = 1;
    snprintf(app->server_url, sizeof(app->server_url), "http://192.168.1.100:8000");
    snprintf(app->status_line, sizeof(app->status_line), "BOOTING...");
}

void app_add_message(AppState* app, const char* role, const char* text) {
    if (app->history_count >= MAX_CHAT_MESSAGES) {
        memmove(&app->history[0], &app->history[1], sizeof(ChatMessage) * (MAX_CHAT_MESSAGES - 1));
        app->history_count = MAX_CHAT_MESSAGES - 1;
    }

    ChatMessage* msg = &app->history[app->history_count++];
    memset(msg, 0, sizeof(*msg));
    snprintf(msg->role, sizeof(msg->role), "%s", role);
    snprintf(msg->text, sizeof(msg->text), "%s", text);
    msg->visible_chars = 0;
    msg->alpha = 0;
}

void app_update(AppState* app, SceCtrlData* pad) {
    (void)pad;
    app->tick++;
}

void app_shutdown(void) {
}
