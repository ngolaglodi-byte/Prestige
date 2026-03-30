"""Prestige AI — Real-time Subtitle Engine (Whisper).

Captures audio from the system microphone, runs faster-whisper locally
for real-time speech-to-text, and publishes subtitles via ZMQ.

100% offline — no internet required. Models are bundled in models/whisper/.
"""

from __future__ import annotations

import json
import logging
import queue
import threading
import time
from pathlib import Path

import numpy as np

logger = logging.getLogger(__name__)

# Audio settings
SAMPLE_RATE = 16000   # Whisper expects 16kHz
CHANNELS = 1          # Mono
CHUNK_DURATION = 3.0  # Seconds per transcription chunk
OVERLAP = 0.5         # Overlap between chunks for continuity
SILENCE_THRESHOLD = 0.01  # RMS below this = silence


class SubtitleEngine:
    """Real-time subtitle engine using faster-whisper (CTranslate2)."""

    def __init__(
        self,
        subtitle_queue: queue.Queue,
        model_size: str = "base",
        language: str = "auto",
        device: str | None = None,
        shutdown_event: threading.Event | None = None,
    ):
        self._subtitle_queue = subtitle_queue
        self._model_size = model_size
        self._language = language if language != "auto" else None
        self._device = device
        self._shutdown = shutdown_event or threading.Event()
        self._model = None
        self._audio_buffer = np.zeros(0, dtype=np.float32)
        self._lock = threading.Lock()
        self._last_text = ""

    def load_model(self) -> bool:
        """Load the Whisper model (bundled or download)."""
        try:
            from faster_whisper import WhisperModel

            # Check for bundled model first
            bundled = Path(__file__).parent / "models" / "whisper" / self._model_size
            if bundled.exists():
                model_path = str(bundled)
                logger.info("[Whisper] Loading bundled model: %s", bundled)
            else:
                model_path = self._model_size
                logger.info("[Whisper] Loading model: %s (will download if needed)", model_path)

            self._model = WhisperModel(
                model_path,
                device="cpu",
                compute_type="int8",
                cpu_threads=4,
            )
            logger.info("[Whisper] Model '%s' loaded successfully", self._model_size)
            return True
        except ImportError:
            logger.warning("[Whisper] faster-whisper not installed — subtitles disabled")
            return False
        except Exception as e:
            logger.error("[Whisper] Failed to load model: %s", e)
            return False

    def start_capture(self) -> None:
        """Start audio capture thread using sounddevice."""
        try:
            import sounddevice as sd
        except ImportError:
            logger.warning("[Whisper] sounddevice not installed — subtitles disabled")
            return

        device_info = sd.query_devices(kind="input")
        logger.info("[Whisper] Audio input: %s", device_info.get("name", "default"))

        def audio_callback(indata: np.ndarray, frames: int, time_info: dict, status) -> None:
            if status:
                logger.debug("[Whisper] Audio status: %s", status)
            with self._lock:
                self._audio_buffer = np.append(self._audio_buffer, indata[:, 0].copy())

        try:
            stream = sd.InputStream(
                samplerate=SAMPLE_RATE,
                channels=CHANNELS,
                dtype="float32",
                blocksize=int(SAMPLE_RATE * 0.1),  # 100ms blocks
                callback=audio_callback,
                device=self._device if self._device else None,
            )
            stream.start()
            logger.info("[Whisper] Audio capture started (16kHz mono)")

            self._transcription_loop()

            stream.stop()
            stream.close()
            logger.info("[Whisper] Audio capture stopped")
        except Exception as e:
            logger.error("[Whisper] Audio capture error: %s", e)

    def _transcription_loop(self) -> None:
        """Main loop: accumulate audio, transcribe, publish."""
        if not self._model:
            logger.error("[Whisper] Model not loaded — cannot transcribe")
            return

        min_samples = int(SAMPLE_RATE * CHUNK_DURATION)
        overlap_samples = int(SAMPLE_RATE * OVERLAP)

        while not self._shutdown.is_set():
            time.sleep(0.2)  # Check every 200ms

            with self._lock:
                if len(self._audio_buffer) < min_samples:
                    continue
                chunk = self._audio_buffer[:min_samples].copy()
                # Keep overlap for continuity
                self._audio_buffer = self._audio_buffer[min_samples - overlap_samples:]

            # Skip silence
            rms = np.sqrt(np.mean(chunk ** 2))
            if rms < SILENCE_THRESHOLD:
                continue

            # Transcribe
            t0 = time.monotonic()
            try:
                segments, info = self._model.transcribe(
                    chunk,
                    language=self._language,
                    beam_size=3,
                    vad_filter=True,
                    vad_parameters={"min_silence_duration_ms": 300},
                    without_timestamps=True,
                )

                text_parts = []
                for segment in segments:
                    text_parts.append(segment.text.strip())

                text = " ".join(text_parts).strip()
                dt = (time.monotonic() - t0) * 1000

                if text and text != self._last_text:
                    self._last_text = text
                    detected_lang = info.language if info else (self._language or "fr")
                    confidence = info.language_probability if info else 0.9

                    msg = {
                        "type": "subtitle",
                        "text": text,
                        "language": detected_lang,
                        "timestamp_ms": int(time.time() * 1000),
                        "is_partial": False,
                        "confidence": round(confidence, 2),
                    }

                    try:
                        self._subtitle_queue.put_nowait(msg)
                    except queue.Full:
                        pass

                    logger.debug("[Whisper] %.0fms | lang=%s | \"%s\"", dt, detected_lang, text[:60])

            except Exception as e:
                logger.error("[Whisper] Transcription error: %s", e)

    def set_language(self, lang: str) -> None:
        """Change transcription language at runtime."""
        self._language = lang if lang != "auto" else None
        logger.info("[Whisper] Language set to: %s", lang or "auto")

    def set_model_size(self, size: str) -> None:
        """Change model size (requires reload)."""
        if size != self._model_size:
            self._model_size = size
            self.load_model()
