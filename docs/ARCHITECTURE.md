# System Architecture

## High-Level Diagram

```text
+------------------------------ PSP Homebrew ------------------------------+
|                                                                          |
|  +------------------+   +----------------+   +----------------------+    |
|  | Input Layer      |   | UI Layer       |   | Service Layer        |    |
|  | - Buttons        |-->| - Boot Screen  |-->| - Chat Dispatcher    |    |
|  | - Analog         |   | - Terminal View|   | - Command Parser     |    |
|  | - Mic Trigger(R) |   | - Diagnostics  |   | - Retry Controller   |    |
|  +------------------+   +----------------+   +----------+-----------+    |
|                                                               |           |
+---------------------------------------------------------------|-----------+
                                                                |
                                                                | HTTP JSON
                                                                v
+------------------------------ Local PC Server ------------------------------+
|                                                                             |
|  +----------------------+    +------------------------+   +---------------+ |
|  | FastAPI Endpoints    |--->| AI Engine              |   | STT Engine    | |
|  | - /chat              |    | - llama.cpp (GGUF)     |   | - Whisper     | |
|  | - /voice             |    | - personality modes     |   | - transcribe  | |
|  +----------------------+    +------------------------+   +---------------+ |
|                                                                             |
+-----------------------------------------------------------------------------+
```

## Runtime Sequence: Text Chat

1. User enters text on PSP.
2. PSP sends `POST /chat` with JSON payload.
3. Server runs LLM inference.
4. Server returns `reply`.
5. PSP animates reply with typing/fade effect.

## Runtime Sequence: Voice

1. User holds `R` (push-to-talk).
2. PSP records WAV.
3. PSP sends `POST /voice` containing captured path.
4. Server transcribes audio using Whisper.
5. PSP sends transcribed text to `/chat`.
6. Response appears in terminal history.

## Memory Strategy (PSP)

- Fixed chat slots: 64 messages.
- Fixed max message size: 256 chars.
- No dynamic allocations in render/update path.
- Lightweight text renderer to reduce draw cost.
