#include "http_client.h"

#include <psphttp.h>
#include <pspnet.h>
#include <pspnet_apctl.h>
#include <pspnet_inet.h>
#include <pspkernel.h>
#include <pspiofilemgr.h>

#include <stdio.h>
#include <string.h>

#define HTTP_HEAP_SIZE (200 * 1024)
#define MAX_HTTP_RESPONSE 512
#define MAX_VOICE_UPLOAD (64 * 1024)

int net_init_wifi(void) {
    int err = 0;
    int state = 0;
    int tries = 0;

    err = sceNetInit(128 * 1024, 42, 16 * 1024, 42, 16 * 1024);
    if (err < 0) return err;

    err = sceNetInetInit();
    if (err < 0) return err;

    err = sceNetApctlInit(0x1800, 48);
    if (err < 0) return err;

    err = sceNetApctlConnect(1);
    if (err < 0) return err;

    while (tries < 120) {
        sceNetApctlGetState(&state);
        if (state == 4) break;
        sceKernelDelayThread(100000);
        tries++;
    }

    if (state != 4) return -1;

    err = sceHttpInit(HTTP_HEAP_SIZE);
    if (err < 0) return err;

    return 0;
}

void net_shutdown(void) {
    sceHttpEnd();
    sceNetApctlDisconnect();
    sceNetApctlTerm();
    sceNetInetTerm();
    sceNetTerm();
}

static int http_post_with_type(const char* url, const void* body, int body_len, const char* content_type, char* out, int out_size) {
    int tmpl = -1;
    int conn = -1;
    int req = -1;
    int status = 0;
    int read_total = 0;

    char* mutable_url = (char*)url;
    char* mutable_content_type = (char*)content_type;
    void* mutable_body = (void*)body;

    tmpl = sceHttpCreateTemplate("PSP_AI/1.0", 1, 1);
    if (tmpl < 0) goto fail;

    conn = sceHttpCreateConnectionWithURL(tmpl, mutable_url, 0);
    if (conn < 0) goto fail;

    req = sceHttpCreateRequestWithURL(conn, PSP_HTTP_METHOD_POST, mutable_url, body_len);
    if (req < 0) goto fail;

    sceHttpAddExtraHeader(req, "Content-Type", mutable_content_type, 0);

    if (sceHttpSendRequest(req, mutable_body, body_len) < 0) goto fail;
    if (sceHttpGetStatusCode(req, &status) < 0) goto fail;
    if (status < 200 || status >= 300) goto fail;

    read_total = sceHttpReadData(req, out, out_size - 1);
    if (read_total < 0) goto fail;

    out[read_total] = '\0';

    sceHttpDeleteRequest(req);
    sceHttpDeleteConnection(conn);
    sceHttpDeleteTemplate(tmpl);
    return 0;

fail:
    if (req >= 0) sceHttpDeleteRequest(req);
    if (conn >= 0) sceHttpDeleteConnection(conn);
    if (tmpl >= 0) sceHttpDeleteTemplate(tmpl);
    return -1;
}

static void json_extract_key(const char* json, const char* key_name, char* out_value, int out_size) {
    char key[32];
    const char* p;
    int i = 0;

    snprintf(key, sizeof(key), "\"%s\":\"", key_name);
    p = strstr(json, key);
    if (!p) {
        out_value[0] = '\0';
        return;
    }

    p += strlen(key);
    while (*p && *p != '\"' && i < out_size - 1) {
        if (*p == '\\' && *(p + 1)) p++;
        out_value[i++] = *p++;
    }
    out_value[i] = '\0';
}

int net_post_chat(const char* server_url, const char* prompt, char* out_reply, int out_size) {
    char url[SERVER_URL_LEN + 16];
    char body[INPUT_LEN + 96];
    char response[MAX_HTTP_RESPONSE];
    int retry;

    snprintf(url, sizeof(url), "%s/chat", server_url);
    snprintf(body, sizeof(body), "{\"message\":\"%s\",\"mode\":\"classic\"}", prompt);

    for (retry = 0; retry < 3; ++retry) {
        if (http_post_with_type(url, body, (int)strlen(body), "application/json", response, sizeof(response)) == 0) {
            json_extract_key(response, "reply", out_reply, out_size);
            if (out_reply[0] == '\0') {
                snprintf(out_reply, out_size, "Empty reply.");
            }
            return 0;
        }
        sceKernelDelayThread(120000);
    }

    snprintf(out_reply, out_size, "Connection failed after retries.");
    return -1;
}

int net_post_voice_file(const char* server_url, const char* wav_path, char* out_text, int out_size) {
    char url[SERVER_URL_LEN + 16];
    char response[MAX_HTTP_RESPONSE];
    unsigned char upload[MAX_VOICE_UPLOAD];
    SceUID fd;
    int size;

    fd = sceIoOpen(wav_path, PSP_O_RDONLY, 0);
    if (fd < 0) {
        snprintf(out_text, out_size, "Voice file missing.");
        return -1;
    }

    size = sceIoRead(fd, upload, sizeof(upload));
    sceIoClose(fd);

    if (size <= 0) {
        snprintf(out_text, out_size, "Voice file read failed.");
        return -1;
    }

    snprintf(url, sizeof(url), "%s/voice", server_url);
    if (http_post_with_type(url, upload, size, "audio/wav", response, sizeof(response)) < 0) {
        snprintf(out_text, out_size, "Voice API failed.");
        return -1;
    }

    json_extract_key(response, "text", out_text, out_size);
    if (out_text[0] == '\0') {
        snprintf(out_text, out_size, "No speech detected.");
    }
    return 0;
}
