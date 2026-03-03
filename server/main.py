from __future__ import annotations

import os
import tempfile
import time
from pathlib import Path
from typing import Literal

from fastapi import FastAPI, HTTPException, Request
from pydantic import BaseModel, Field

app = FastAPI(title="PSP AI Terminal Server", version="1.0.0")


class ChatRequest(BaseModel):
    message: str = Field(min_length=1, max_length=1024)
    mode: Literal["classic", "terse", "creative"] = "classic"


class ChatResponse(BaseModel):
    reply: str
    latency_ms: int


class VoiceResponse(BaseModel):
    text: str
    latency_ms: int


class LocalAI:
    def __init__(self) -> None:
        self.model = None
        self.whisper = None
        self.model_path = os.getenv("PSP_AI_LLM", "models/tinyllama.gguf")
        self.whisper_path = os.getenv("PSP_AI_STT", "small")
        self._boot()

    def _boot(self) -> None:
        if Path(self.model_path).exists():
            try:
                from llama_cpp import Llama

                self.model = Llama(
                    model_path=self.model_path,
                    n_threads=max(2, os.cpu_count() or 4),
                    n_ctx=1024,
                    n_gpu_layers=0,
                    verbose=False,
                )
                print(f"[AI] Loaded LLM: {self.model_path}")
            except Exception as exc:
                print(f"[AI] LLM load failed: {exc}")

        try:
            from faster_whisper import WhisperModel

            self.whisper = WhisperModel(self.whisper_path, device="cpu", compute_type="int8")
            print(f"[STT] Loaded Whisper model: {self.whisper_path}")
        except Exception as exc:
            print(f"[STT] Whisper load failed: {exc}")

    @staticmethod
    def _system_prompt(mode: str) -> str:
        if mode == "terse":
            return "You are a concise technical assistant. Answer in <=3 lines."
        if mode == "creative":
            return "You are an imaginative retro-futuristic assistant for PSP users."
        return "You are a reliable systems assistant focused on practical answers."

    def chat(self, message: str, mode: str) -> str:
        if self.model is None:
            return (
                "Local LLM is not loaded. Place a GGUF model at models/tinyllama.gguf "
                "or set PSP_AI_LLM env var."
            )

        prompt = (
            f"<|system|>\n{self._system_prompt(mode)}\n"
            f"<|user|>\n{message}\n"
            "<|assistant|>\n"
        )

        output = self.model(
            prompt,
            max_tokens=180,
            stop=["<|user|>", "<|system|>"],
            temperature=0.7 if mode == "creative" else 0.3,
        )
        text = output["choices"][0]["text"].strip()
        return text or "No response generated."

    def transcribe(self, wav_path: str) -> str:
        if self.whisper is None:
            raise RuntimeError("Whisper model not loaded")

        path = Path(wav_path)
        if not path.exists():
            raise FileNotFoundError(f"Audio file not found: {wav_path}")

        segments, _ = self.whisper.transcribe(str(path), language="en", vad_filter=True)
        text = " ".join(segment.text.strip() for segment in segments).strip()
        return text


engine = LocalAI()


@app.get("/health")
def health() -> dict:
    return {
        "ok": True,
        "llm_loaded": engine.model is not None,
        "stt_loaded": engine.whisper is not None,
    }


@app.post("/chat", response_model=ChatResponse)
def chat(req: ChatRequest) -> ChatResponse:
    t0 = time.perf_counter()
    reply = engine.chat(req.message, req.mode)
    dt = int((time.perf_counter() - t0) * 1000)
    return ChatResponse(reply=reply, latency_ms=dt)


@app.post("/voice", response_model=VoiceResponse)
async def voice(req: Request) -> VoiceResponse:
    t0 = time.perf_counter()
    raw = await req.body()
    if not raw:
        raise HTTPException(status_code=400, detail="Empty audio body")

    suffix = ".wav"
    temp_path = None
    try:
        with tempfile.NamedTemporaryFile(delete=False, suffix=suffix) as tmp:
            tmp.write(raw)
            temp_path = tmp.name

        text = engine.transcribe(temp_path)
    except Exception as exc:
        raise HTTPException(status_code=500, detail=str(exc)) from exc
    finally:
        if temp_path:
            try:
                Path(temp_path).unlink(missing_ok=True)
            except Exception:
                pass

    dt = int((time.perf_counter() - t0) * 1000)
    return VoiceResponse(text=text, latency_ms=dt)


if __name__ == "__main__":
    import uvicorn

    uvicorn.run("main:app", host="0.0.0.0", port=8000, reload=False)
