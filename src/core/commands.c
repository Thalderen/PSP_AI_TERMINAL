#include "commands.h"
#include <string.h>
#include <stdio.h>

int handle_command(AppState* app, const char* input, char* out_text, int out_size) {
    if (strncmp(input, "/help", 5) == 0) {
        snprintf(out_text, out_size, "Commands: /help /clear /status /voice /mode");
        return 1;
    }

    if (strncmp(input, "/clear", 6) == 0) {
        app->history_count = 0;
        snprintf(out_text, out_size, "Chat cleared.");
        return 1;
    }

    if (strncmp(input, "/status", 7) == 0) {
        snprintf(out_text, out_size, "WiFi:%s  Mic:%s  Server:%s",
            app->connected ? "ON" : "OFF",
            app->mic_recording ? "REC" : "IDLE",
            app->server_url);
        return 1;
    }

    if (strncmp(input, "/voice", 6) == 0) {
        snprintf(out_text, out_size, "Hold R to record voice and release to send.");
        return 1;
    }

    if (strncmp(input, "/mode", 5) == 0) {
        snprintf(out_text, out_size, "Personality modes live on server: classic, terse, creative.");
        return 1;
    }

    return 0;
}
