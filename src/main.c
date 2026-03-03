#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <psprtc.h>
#include <psputility.h>
#include <psputility_netmodules.h>

#include <string.h>
#include <stdio.h>

#include "core/app.h"
#include "core/commands.h"
#include "ui/render.h"
#include "ui/animations.h"
#include "network/http_client.h"
#include "audio/mic.h"

PSP_MODULE_INFO("PSP_AI_TERMINAL", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

static int exit_callback(int arg1, int arg2, void* common) {
    (void)arg1;
    (void)arg2;
    (void)common;
    sceKernelExitGame();
    return 0;
}

static int callback_thread(SceSize args, void* argp) {
    int cbid;
    (void)args;
    (void)argp;
    cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    sceKernelRegisterExitCallback(cbid);
    sceKernelSleepThreadCB();
    return 0;
}

static int setup_callbacks(void) {
    int thid = sceKernelCreateThread("update_thread", callback_thread, 0x11, 0xFA0, 0, 0);
    if (thid >= 0) {
        sceKernelStartThread(thid, 0, 0);
    }
    return thid;
}

static void load_net_modules(void) {
    sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
    sceUtilityLoadNetModule(PSP_NET_MODULE_INET);
    sceUtilityLoadNetModule(PSP_NET_MODULE_PARSEURI);
    sceUtilityLoadNetModule(PSP_NET_MODULE_PARSEHTTP);
    sceUtilityLoadNetModule(PSP_NET_MODULE_HTTP);
    sceUtilityLoadNetModule(PSP_NET_MODULE_SSL);
}

static void append_char(AppState* app, char c) {
    if (app->input_len < INPUT_LEN - 1) {
        app->input[app->input_len++] = c;
        app->input[app->input_len] = '\0';
    }
}

static char map_button_to_char(SceCtrlData* pad) {
    if (pad->Buttons & PSP_CTRL_TRIANGLE) return 'a';
    if (pad->Buttons & PSP_CTRL_CIRCLE) return 'e';
    if (pad->Buttons & PSP_CTRL_CROSS) return 'i';
    if (pad->Buttons & PSP_CTRL_SQUARE) return 'o';
    if (pad->Buttons & PSP_CTRL_UP) return 'u';
    if (pad->Buttons & PSP_CTRL_DOWN) return ' ';
    return 0;
}

int main(void) {
    AppState app;
    SceCtrlData pad;
    SceCtrlData prev;
    uint64_t now_us;

    memset(&prev, 0, sizeof(prev));

    setup_callbacks();
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

    app_init(&app);
    ui_init();
    ui_render(&app);
    sceDisplayWaitVblankStart();

    now_us = sceKernelGetSystemTimeWide();
    app.boot_start_us = now_us;

    load_net_modules();
    if (net_init_wifi() == 0) {
        app.connected = 1;
        snprintf(app.status_line, sizeof(app.status_line), "NETWORK ONLINE");
    } else {
        app.connected = 0;
        snprintf(app.status_line, sizeof(app.status_line), "NETWORK OFFLINE");
    }

    mic_init();
    app_add_message(&app, "system", "Type /help for commands. Hold R to use voice input.");

    while (app.running) {
        char cmd_out[MAX_MESSAGE_LEN];
        char reply[MAX_MESSAGE_LEN];

        sceCtrlReadBufferPositive(&pad, 1);
        now_us = sceKernelGetSystemTimeWide();
        app_update(&app, &pad);
        anim_update(&app, now_us);

        if ((pad.Buttons & PSP_CTRL_SELECT) && !(prev.Buttons & PSP_CTRL_SELECT)) {
            app.screen = (app.screen == APP_SCREEN_STATUS) ? APP_SCREEN_CHAT : APP_SCREEN_STATUS;
        }

        if (app.screen == APP_SCREEN_CHAT) {
            char c = map_button_to_char(&pad);
            if (c && !(prev.Buttons & pad.Buttons)) {
                append_char(&app, c);
            }

            if ((pad.Buttons & PSP_CTRL_LEFT) && !(prev.Buttons & PSP_CTRL_LEFT) && app.input_len > 0) {
                app.input[--app.input_len] = '\0';
            }

            if ((pad.Buttons & PSP_CTRL_START) && !(prev.Buttons & PSP_CTRL_START) && app.input_len > 0) {
                app_add_message(&app, "you", app.input);

                if (app.input[0] == '/' && handle_command(&app, app.input, cmd_out, sizeof(cmd_out))) {
                    app_add_message(&app, "system", cmd_out);
                } else if (app.connected) {
                    if (net_post_chat(app.server_url, app.input, reply, sizeof(reply)) == 0) {
                        app_add_message(&app, "ai", reply);
                    } else {
                        app_add_message(&app, "error", "Failed to reach local AI server.");
                    }
                } else {
                    app_add_message(&app, "error", "No network connection.");
                }

                app.input[0] = '\0';
                app.input_len = 0;
            }

            if ((pad.Buttons & PSP_CTRL_RTRIGGER) && !(prev.Buttons & PSP_CTRL_RTRIGGER)) {
                app.mic_recording = 1;
                snprintf(app.status_line, sizeof(app.status_line), "VOICE RECORDING...");
            }

            if (!(pad.Buttons & PSP_CTRL_RTRIGGER) && (prev.Buttons & PSP_CTRL_RTRIGGER)) {
                char voice_text[MAX_MESSAGE_LEN];
                app.mic_recording = 0;
                snprintf(app.status_line, sizeof(app.status_line), "PROCESSING VOICE...");

                if (mic_record_wav("ms0:/PSP/GAME/PSP_AI_TERMINAL/voice.wav", 2500) > 0 && app.connected) {
                    if (net_post_voice_file(app.server_url, "ms0:/PSP/GAME/PSP_AI_TERMINAL/voice.wav", voice_text, sizeof(voice_text)) == 0) {
                        if (voice_text[0]) {
                            app_add_message(&app, "you", voice_text);
                            if (net_post_chat(app.server_url, voice_text, reply, sizeof(reply)) == 0) {
                                app_add_message(&app, "ai", reply);
                            }
                        }
                    }
                }

                snprintf(app.status_line, sizeof(app.status_line), "READY");
            }
        }

        ui_render(&app);
        prev = pad;
        sceDisplayWaitVblankStart();
    }

    mic_shutdown();
    net_shutdown();
    app_shutdown();
    sceKernelExitGame();
    return 0;
}
