"""Prestige AI — Frame Receiver.

Receives JPEG frames from C++ Vision Engine via ZMQ.
The C++ is the master of the video pipeline.
Python only analyses — it does NOT capture video.

Protocol (port 5557):
    Message = [8 bytes frameId][8 bytes timestampMs][JPEG data]
"""

from __future__ import annotations

import logging
import struct
import time

import cv2
import numpy as np
import zmq

logger = logging.getLogger(__name__)


class FrameReceiver:
    """Receives video frames from C++ Vision Engine via ZMQ SUB."""

    def __init__(self, address: str = "tcp://127.0.0.1:5557") -> None:
        self._address = address
        self._context: zmq.Context | None = None
        self._socket: zmq.Socket | None = None
        self._connected = False
        self._frames_received = 0

    def start(self) -> None:
        self._context = zmq.Context()
        self._socket = self._context.socket(zmq.SUB)
        self._socket.subscribe(b"")
        self._socket.setsockopt(zmq.RCVHWM, 1)
        self._socket.setsockopt(zmq.CONFLATE, 1)  # Only keep latest frame
        self._socket.setsockopt(zmq.RCVTIMEO, 500)
        self._socket.connect(self._address)
        self._connected = True
        logger.info("Frame receiver connected to %s", self._address)

    def stop(self) -> None:
        if self._socket:
            self._socket.close()
            self._socket = None
        if self._context:
            self._context.term()
            self._context = None
        self._connected = False

    def receive(self) -> tuple[np.ndarray | None, int, int]:
        """Receive a frame from C++.

        Returns:
            (frame, frame_id, capture_timestamp_ms) or (None, 0, 0) on timeout
        """
        if not self._socket:
            return None, 0, 0

        try:
            msg = self._socket.recv(flags=0)
        except zmq.Again:
            return None, 0, 0
        except zmq.ZMQError:
            return None, 0, 0

        if len(msg) < 16:
            return None, 0, 0

        # Parse header
        frame_id = struct.unpack("<q", msg[:8])[0]
        capture_ts = struct.unpack("<q", msg[8:16])[0]
        jpeg_data = msg[16:]

        # Decode JPEG
        arr = np.frombuffer(jpeg_data, dtype=np.uint8)
        frame = cv2.imdecode(arr, cv2.IMREAD_COLOR)

        if frame is not None:
            self._frames_received += 1

        return frame, frame_id, capture_ts

    @property
    def connected(self) -> bool:
        return self._connected

    @property
    def frames_received(self) -> int:
        return self._frames_received
