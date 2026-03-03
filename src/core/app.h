#ifndef APP_H
#define APP_H

#include <pspkernel.h>
#include <pspctrl.h>
#include <stdint.h>

#define MAX_CHAT_MESSAGES 64
#define MAX_MESSAGE_LEN 256
#define INPUT_LEN 180
#define SERVER_URL_LEN 128

typedef enum {
    APP_SCREEN_BOOT = 0,
    APP_SCREEN_CHAT,
    APP_SCREEN_STATUS,
    APP_SCREEN_SETTINGS
} AppScreen;

typedef struct {
    char role[8];
    char text[MAX_MESSAGE_LEN];
    int visible_chars;
    int alpha;
} ChatMessage;

typedef struct {
    AppScreen screen;
    int running;
    int connected;
    int mic_recording;
    int boot_done;
    uint64_t tick;
    uint64_t boot_start_us;

    char input[INPUT_LEN];
    int input_len;
    int cursor_blink;

    char server_url[SERVER_URL_LEN];

    ChatMessage history[MAX_CHAT_MESSAGES];
    int history_count;
    int history_scroll;

    char status_line[128];
} AppState;

void app_init(AppState* app);
void app_update(AppState* app, SceCtrlData* pad);
void app_shutdown(void);
void app_add_message(AppState* app, const char* role, const char* text);

#endif
