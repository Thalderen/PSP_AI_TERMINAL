# Developer Guide

## 1. Toolchain Installation (PSP)

1. Install `pspdev` toolchain.
2. Export environment variables:
   - `PSPDEV`
   - `PSPSDK`
   - update `PATH` with `$PSPDEV/bin`
3. Verify:

```bash
psp-gcc --version
psp-config --pspsdk-path
```

## 2. Build Homebrew App

```bash
cd psp_ai_terminal
make clean
make
```

Generated output: `EBOOT.PBP`.

## 3. Install to PSP

1. Connect PSP storage.
2. Create `PSP/GAME/PSP_AI_TERMINAL/`.
3. Copy `EBOOT.PBP`.
4. Start app from XMB.

## 4. Configure Local AI Server

```bash
cd psp_ai_terminal/server
python -m venv .venv
.venv\Scripts\activate      # Windows
pip install -r requirements.txt
python main.py
```

Server listens on `0.0.0.0:8000`.

## 5. LLM Integration

1. Download a small GGUF model (2B-3B recommended for CPU).
2. Place model at `server/models/tinyllama.gguf` or set:

```bash
set PSP_AI_LLM=C:\models\your_model.gguf
```

## 6. Speech-to-Text Integration

Whisper model is auto-loaded from `PSP_AI_STT` env (`small` by default):

```bash
set PSP_AI_STT=base
```

## 7. Network Link from PSP to PC

1. Ensure PSP and PC are on the same WiFi network.
2. Set PSP server URL in `src/core/app.c`.
3. Open Windows firewall for port `8000`.
4. Validate from PC:

```bash
curl http://127.0.0.1:8000/health
```

## 8. API Contract

### `POST /chat`

Request:

```json
{"message":"Hello","mode":"classic"}
```

Response:

```json
{"reply":"Hi.","latency_ms":42}
```

### `POST /voice`

Request: raw WAV bytes as HTTP body (`Content-Type: audio/wav`).

Response:

```json
{"text":"transcribed words","latency_ms":153}
```

## 9. Debug Checklist

- `NET:OFFLINE`: WLAN profile or AP issues.
- Server errors: check console traceback.
- Empty AI responses: model prompt format mismatch.
- Choppy UI: reduce chat lines or animation rate.
- STT slow: switch `PSP_AI_STT=tiny`.

## 10. Future Enhancements

- Multipart upload and streaming STT for longer speech.
- UTF-8 safe rendering and JSON parser upgrade.
- PSP on-device settings persistence file.
- Theme variants and SFX packs.

