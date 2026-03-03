#include "mic.h"

#include <pspaudio.h>
#include <pspiofilemgr.h>
#include <pspkernel.h>

#include <stdint.h>
#include <string.h>

#define SAMPLE_RATE 11025
#define CHANNELS 1
#define BITS_PER_SAMPLE 16
#define FRAME_SAMPLES 256

static int g_audio_ready = 0;

static void write_wav_header(SceUID fd, int data_bytes) {
    uint8_t hdr[44];
    uint32_t chunk_size = 36 + data_bytes;
    uint32_t byte_rate = SAMPLE_RATE * CHANNELS * (BITS_PER_SAMPLE / 8);
    uint16_t block_align = CHANNELS * (BITS_PER_SAMPLE / 8);

    memset(hdr, 0, sizeof(hdr));
    memcpy(hdr + 0, "RIFF", 4);
    memcpy(hdr + 8, "WAVEfmt ", 8);

    hdr[4] = (uint8_t)(chunk_size & 0xFF);
    hdr[5] = (uint8_t)((chunk_size >> 8) & 0xFF);
    hdr[6] = (uint8_t)((chunk_size >> 16) & 0xFF);
    hdr[7] = (uint8_t)((chunk_size >> 24) & 0xFF);

    hdr[16] = 16;
    hdr[20] = 1;
    hdr[22] = CHANNELS;
    hdr[24] = (uint8_t)(SAMPLE_RATE & 0xFF);
    hdr[25] = (uint8_t)((SAMPLE_RATE >> 8) & 0xFF);
    hdr[26] = (uint8_t)((SAMPLE_RATE >> 16) & 0xFF);
    hdr[27] = (uint8_t)((SAMPLE_RATE >> 24) & 0xFF);

    hdr[28] = (uint8_t)(byte_rate & 0xFF);
    hdr[29] = (uint8_t)((byte_rate >> 8) & 0xFF);
    hdr[30] = (uint8_t)((byte_rate >> 16) & 0xFF);
    hdr[31] = (uint8_t)((byte_rate >> 24) & 0xFF);

    hdr[32] = (uint8_t)(block_align & 0xFF);
    hdr[33] = (uint8_t)((block_align >> 8) & 0xFF);
    hdr[34] = BITS_PER_SAMPLE;

    memcpy(hdr + 36, "data", 4);
    hdr[40] = (uint8_t)(data_bytes & 0xFF);
    hdr[41] = (uint8_t)((data_bytes >> 8) & 0xFF);
    hdr[42] = (uint8_t)((data_bytes >> 16) & 0xFF);
    hdr[43] = (uint8_t)((data_bytes >> 24) & 0xFF);

    sceIoWrite(fd, hdr, sizeof(hdr));
}

int mic_init(void) {
    g_audio_ready = 1;
    return 0;
}

void mic_shutdown(void) {
    g_audio_ready = 0;
}

int mic_record_wav(const char* path, int max_ms) {
    SceUID fd;
    int16_t samples[FRAME_SAMPLES];
    int loops;
    int i;
    int data_bytes = 0;

    if (!g_audio_ready) {
        return -1;
    }

    fd = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    if (fd < 0) {
        return -1;
    }

    write_wav_header(fd, 0);

    loops = (SAMPLE_RATE * max_ms / 1000) / FRAME_SAMPLES;
    for (i = 0; i < loops; ++i) {
    int ret = sceAudioInputBlocking(FRAME_SAMPLES, SAMPLE_RATE, samples);
        if (ret < 0) {
            break;
        }
        sceIoWrite(fd, samples, FRAME_SAMPLES * (int)sizeof(int16_t));
        data_bytes += FRAME_SAMPLES * (int)sizeof(int16_t);
    }

    sceIoLseek(fd, 0, PSP_SEEK_SET);
    write_wav_header(fd, data_bytes);
    sceIoClose(fd);

    return data_bytes;
}

