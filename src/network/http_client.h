#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include "../core/app.h"

int net_init_wifi(void);
void net_shutdown(void);
int net_post_chat(const char* server_url, const char* prompt, char* out_reply, int out_size);
int net_post_voice_file(const char* server_url, const char* wav_path, char* out_text, int out_size);

#endif
