#include "animations.h"

void anim_update(AppState* app, uint64_t now_us) {
    int i;

    if (!app->boot_done && now_us - app->boot_start_us > 3200000ULL) {
        app->boot_done = 1;
        app->screen = APP_SCREEN_CHAT;
    }

    app->cursor_blink = ((now_us / 350000ULL) % 2) == 0;

    for (i = 0; i < app->history_count; ++i) {
        ChatMessage* m = &app->history[i];
        int len = 0;
        while (m->text[len] != '\0') {
            len++;
        }

        if (m->visible_chars < len) {
            m->visible_chars += 2;
            if (m->visible_chars > len) {
                m->visible_chars = len;
            }
        }

        if (m->alpha < 255) {
            m->alpha += 18;
            if (m->alpha > 255) {
                m->alpha = 255;
            }
        }
    }
}
