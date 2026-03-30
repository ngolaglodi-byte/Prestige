"""Prestige AI — ZeroMQ Publisher for talent detection results."""

from __future__ import annotations

import json
import logging
import time
from dataclasses import dataclass, field
from typing import Any

import zmq

logger = logging.getLogger(__name__)


@dataclass
class BBox:
    x: int = 0
    y: int = 0
    w: int = 0
    h: int = 0

    def to_dict(self) -> dict[str, int]:
        return {"x": self.x, "y": self.y, "w": self.w, "h": self.h}


@dataclass
class TalentMatch:
    id: str = ""
    name: str = ""
    role: str = ""
    confidence: float = 0.0
    bbox: BBox = field(default_factory=BBox)
    show_overlay: bool = True
    overlay_style: str = "default"

    def to_dict(self) -> dict[str, Any]:
        return {
            "id": self.id,
            "name": self.name,
            "role": self.role,
            "confidence": self.confidence,
            "bbox": self.bbox.to_dict(),
            "show_overlay": self.show_overlay,
            "overlay_style": self.overlay_style,
        }


class ZmqPublisher:
    """Publishes talent detection results over ZeroMQ PUB socket."""

    def __init__(self, address: str = "tcp://127.0.0.1:5555") -> None:
        self._address = address
        self._context: zmq.Context | None = None
        self._socket: zmq.Socket | None = None
        self._frame_counter: int = 0

    def start(self) -> None:
        """Initialize and bind the PUB socket."""
        self._context = zmq.Context()
        self._socket = self._context.socket(zmq.PUB)
        self._socket.setsockopt(zmq.SNDHWM, 2)  # Low HWM for real-time
        self._socket.setsockopt(zmq.LINGER, 0)
        self._socket.bind(self._address)
        logger.info("ZMQ PUB bound on %s", self._address)

    def stop(self) -> None:
        """Close socket and terminate context."""
        if self._socket:
            self._socket.close()
            self._socket = None
        if self._context:
            self._context.term()
            self._context = None
        logger.info("ZMQ publisher stopped")

    def publish(
        self,
        talents: list[TalentMatch],
        scene_type: str = "unknown",
        overlay_visible: bool = True,
        capture_timestamp_ms: int = 0,
        detection_latency_ms: int = 0,
    ) -> None:
        """Serialize and publish a detection message with sync timestamps."""
        if not self._socket:
            logger.warning("Cannot publish — socket not initialized")
            return

        self._frame_counter += 1
        message = {
            "frame_id": self._frame_counter,
            "timestamp_ms": int(time.time() * 1000),
            "capture_timestamp_ms": capture_timestamp_ms,
            "detection_latency_ms": detection_latency_ms,
            "talents": [t.to_dict() for t in talents],
            "scene_type": scene_type,
            "face_count": len(talents),
            "overlay_global_visible": overlay_visible,
        }

        try:
            payload = json.dumps(message).encode("utf-8")
            # Send plain (backward compat) — current C++ subscribers expect this
            self._socket.send(payload, zmq.NOBLOCK)
        except zmq.Again:
            logger.debug("ZMQ send would block — dropping frame %d", self._frame_counter)
        except zmq.ZMQError as exc:
            logger.error("ZMQ publish error: %s — attempting reconnect", exc)
            self._reconnect()

    def publish_subtitle(self, subtitle_msg: dict) -> None:
        """Publish a subtitle message (from Whisper engine)."""
        if not self._socket:
            return

        try:
            payload = json.dumps(subtitle_msg).encode("utf-8")
            self._socket.send_multipart([b"subtitle", payload], zmq.NOBLOCK)
        except zmq.Again:
            pass
        except zmq.ZMQError as exc:
            logger.error("ZMQ subtitle publish error: %s", exc)

    def _reconnect(self) -> None:
        """Attempt to rebind the socket after an error."""
        try:
            if self._socket:
                self._socket.close()
            self._socket = self._context.socket(zmq.PUB)
            self._socket.setsockopt(zmq.SNDHWM, 2)
            self._socket.setsockopt(zmq.LINGER, 0)
            self._socket.bind(self._address)
            logger.info("ZMQ publisher reconnected on %s", self._address)
        except zmq.ZMQError as exc:
            logger.error("ZMQ reconnect failed: %s", exc)
