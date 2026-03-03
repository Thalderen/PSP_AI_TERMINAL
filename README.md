# PSP AI Terminal

A local-first AI assistant system for Sony PSP homebrew.

## Overview

`PSP AI Terminal` is a monochrome retro terminal UI running on PSP hardware, backed by a local Python AI server on your PC over WiFi.

- PSP app: C + PSP SDK
- Server: Python + FastAPI + llama.cpp + faster-whisper
- API: `POST /chat`, `POST /voice`

## Architecture

```text
+----------------------+           WiFi HTTP            +-----------------------------+
| Sony PSP Homebrew    |  <-------------------------->  | Local PC AI Server          |
|                      |                                |                             |
| - UI renderer        |                                | - FastAPI endpoints         |
| - Terminal animation |                                | - llama.cpp text generation |
| - Input + commands   |                                | - Whisper speech-to-text    |
| - Mic capture (WAV)  |                                | - Personality modes         |
+----------------------+                                +-----------------------------+
```

## Retro UI Mockup

```text
PSP AI TERMINAL                NET:ONLINE        MIC:IDLE
==========================================================
ai> Welcome to PSP AI Terminal.
you> show diagnostics
ai> CPU 333MHz. RAM budget stable. WiFi online.
----------------------------------------------------------
> /status_
[READY]
```

## Controls

- `START`: send typed message
- `SELECT`: toggle diagnostics screen
- `LEFT`: backspace
- `R`: push-to-talk voice recording
- `TRIANGLE/CIRCLE/CROSS/SQUARE/UP/DOWN`: quick character input map

## Command System

- `/help`
- `/clear`
- `/status`
- `/voice`
- `/mode`

## PSP Build

### Prerequisites

- PSP toolchain (`psp-gcc`, `psp-config`, PSPSDK env)
- PSP device or PPSSPP for testing

### Compile

```bash
cd psp_ai_terminal
make
```

`EBOOT.PBP` is generated in the project directory.

### Deploy to PSP

1. Create folder: `ms0:/PSP/GAME/PSP_AI_TERMINAL/`
2. Copy `EBOOT.PBP` there.
3. Launch from PSP Game > Memory Stick.

## Python Server Setup

### Prerequisites

- Python 3.11+
- CPU-friendly GGUF model file (example: TinyLlama)
- Optional: Whisper model files downloaded automatically by `faster-whisper`

### Install and run

```bash
cd psp_ai_terminal/server
python -m venv .venv
. .venv/bin/activate  # Windows: .venv\Scripts\activate
pip install -r requirements.txt
python main.py
```

### Environment Variables

- `PSP_AI_LLM` path to `.gguf` model (default: `models/tinyllama.gguf`)
- `PSP_AI_STT` whisper size (default: `small`)

## PSP Connection Configuration

Update `server_url` in `src/core/app.c`:

```c
snprintf(app->server_url, sizeof(app->server_url), "http://192.168.1.100:8000");
```

Use your PC's LAN IP.

## Performance Notes

- Fixed-size message buffers avoid heap churn.
- Lightweight text mode rendering minimizes GPU overhead.
- Animation updates are integer/tick-based.
- HTTP retry loop handles unstable WiFi.
- Voice recording uses mono 11.025 kHz WAV to reduce bandwidth and CPU load.

## Project Tree

```text
psp_ai_terminal/
  Makefile
  src/
    main.c
    core/
      app.c
      app.h
      commands.c
      commands.h
    ui/
      render.c
      render.h
      animations.c
      animations.h
    network/
      http_client.c
      http_client.h
    audio/
      mic.c
      mic.h
  server/
    main.py
    requirements.txt
  assets/
    fonts/
  build/
```

## Debugging Tips

- If PSP shows `NET:OFFLINE`, verify WLAN profile index in `sceNetApctlConnect(1)`.
- Use `GET /health` on server to validate model loading.
- If voice fails, verify PSP captured WAV size and server STT model load state.
- Keep prompts short to fit PSP-side JSON buffer limits.

## Disclaimer

The PSP code is production-oriented scaffold for homebrew development. Depending on your firmware/SDK version, specific APIs (especially audio input) may require small compatibility adjustments.

