#ifndef MIC_H
#define MIC_H

int mic_init(void);
void mic_shutdown(void);
int mic_record_wav(const char* path, int max_ms);

#endif
