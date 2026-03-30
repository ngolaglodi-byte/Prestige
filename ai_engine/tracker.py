"""Prestige AI — Face Tracker.

Smooth 30fps face tracking between 5fps AI detections.
Uses linear prediction + EMA smoothing (no heavy OpenCV tracker).

Architecture:
    Detection thread (5fps) → updates track targets with fresh bbox
    Tracking thread (30fps) → predicts position + smooths → ZMQ publish

Why NOT OpenCV TrackerMIL/KCF:
    - 12-25ms per frame = kills the 30fps budget
    - Not needed: broadcast subjects are seated, move slowly
    - Linear prediction + smoothing is <0.1ms and produces better results
"""

from __future__ import annotations

import logging
import time
from dataclasses import dataclass, field

import numpy as np

from zmq_publisher import BBox, TalentMatch

logger = logging.getLogger(__name__)


@dataclass
class TrackedFace:
    """A face being tracked across frames."""
    id: str = ""
    name: str = ""
    role: str = ""
    confidence: float = 0.0
    overlay_style: str = "default"
    show_overlay: bool = True

    # Bounding box: [x, y, w, h] as float for smooth interpolation
    bbox: np.ndarray = field(default_factory=lambda: np.zeros(4, dtype=np.float64))
    target_bbox: np.ndarray = field(default_factory=lambda: np.zeros(4, dtype=np.float64))
    velocity: np.ndarray = field(default_factory=lambda: np.zeros(4, dtype=np.float64))

    # Timing
    last_detection_time: float = 0.0
    first_seen: float = 0.0
    frames_since_detection: int = 0


class FaceTracker:
    """Manages multiple tracked faces with linear prediction + EMA smoothing.

    Usage:
        tracker = FaceTracker()

        # Called by detection thread (~5fps):
        tracker.update_detections(detections)

        # Called by tracking thread (30fps):
        tracked = tracker.update_tracking()
    """

    def __init__(
        self,
        smoothing: float = 0.3,
        max_lost_seconds: float = 1.5,
        iou_threshold: float = 0.25,
        **kwargs,  # Accept but ignore tracker_type etc.
    ) -> None:
        self._smoothing = smoothing
        self._max_lost_seconds = max_lost_seconds
        self._iou_threshold = iou_threshold
        self._tracks: list[TrackedFace] = []
        self._next_id = 0

    @property
    def active_tracks(self) -> list[TrackedFace]:
        now = time.monotonic()
        return [t for t in self._tracks if (now - t.last_detection_time) < self._max_lost_seconds]

    def update_detections(self, frame: np.ndarray, detections: list[TalentMatch]) -> None:
        """Called when AI detection produces new results (~5fps)."""
        now = time.monotonic()
        det_bboxes = np.array(
            [[d.bbox.x, d.bbox.y, d.bbox.w, d.bbox.h] for d in detections],
            dtype=np.float64,
        ) if detections else np.empty((0, 4))

        matched_tracks: set[int] = set()
        matched_dets: set[int] = set()

        # Match by IoU
        if self._tracks and len(detections) > 0:
            track_bboxes = np.array([t.target_bbox for t in self._tracks])
            iou_mat = self._iou_matrix(track_bboxes, det_bboxes)

            while True:
                if iou_mat.size == 0:
                    break
                idx = np.unravel_index(np.argmax(iou_mat), iou_mat.shape)
                if iou_mat[idx] < self._iou_threshold:
                    break

                ti, di = idx
                matched_tracks.add(int(ti))
                matched_dets.add(int(di))

                track = self._tracks[ti]
                det = detections[di]
                old_bbox = track.target_bbox.copy()

                track.name = det.name
                track.role = det.role
                track.confidence = det.confidence
                track.show_overlay = det.show_overlay
                track.overlay_style = det.overlay_style
                track.target_bbox = det_bboxes[di].copy()
                track.velocity = (track.target_bbox - old_bbox) * 0.5
                track.last_detection_time = now
                track.frames_since_detection = 0

                iou_mat[ti, :] = -1
                iou_mat[:, di] = -1

        # New tracks
        for di, det in enumerate(detections):
            if di in matched_dets:
                continue
            bb = det_bboxes[di]
            self._tracks.append(TrackedFace(
                id=det.id if det.id not in ("unknown", "") else f"track_{self._next_id}",
                name=det.name, role=det.role,
                confidence=det.confidence,
                show_overlay=det.show_overlay,
                overlay_style=det.overlay_style,
                bbox=bb.copy(), target_bbox=bb.copy(),
                last_detection_time=now, first_seen=now,
            ))
            self._next_id += 1

        # Increment lost counter for unmatched
        for ti in range(len(self._tracks)):
            if ti not in matched_tracks:
                self._tracks[ti].frames_since_detection += 1

        # Prune lost tracks
        self._tracks = [
            t for t in self._tracks
            if (now - t.last_detection_time) < self._max_lost_seconds
        ]

    def update_tracking(self, frame: np.ndarray = None) -> list[TalentMatch]:
        """Called every frame (30fps). Linear prediction + EMA smoothing.

        This is extremely fast (<0.05ms) because it's just math on 4 floats.
        No image processing at all.
        """
        now = time.monotonic()
        alpha = self._smoothing
        results: list[TalentMatch] = []

        for track in self._tracks:
            if (now - track.last_detection_time) >= self._max_lost_seconds:
                continue

            track.frames_since_detection += 1

            # Predict: target + velocity (linear extrapolation)
            predicted = track.target_bbox + track.velocity

            # Smooth: EMA between current position and predicted target
            track.bbox = alpha * predicted + (1.0 - alpha) * track.bbox

            # Decay velocity over time (prevents drift)
            track.velocity *= 0.85

            bx, by, bw, bh = track.bbox
            results.append(TalentMatch(
                id=track.id,
                name=track.name,
                role=track.role,
                confidence=track.confidence,
                bbox=BBox(x=int(bx), y=int(by), w=int(bw), h=int(bh)),
                show_overlay=track.show_overlay,
                overlay_style=track.overlay_style,
            ))

        return results

    def clear(self) -> None:
        self._tracks.clear()

    @staticmethod
    def _iou_matrix(a: np.ndarray, b: np.ndarray) -> np.ndarray:
        """Vectorized IoU: a is (N,4), b is (M,4), both [x,y,w,h]."""
        ax1 = a[:, 0][:, None]
        ay1 = a[:, 1][:, None]
        ax2 = (a[:, 0] + a[:, 2])[:, None]
        ay2 = (a[:, 1] + a[:, 3])[:, None]

        bx1 = b[:, 0][None, :]
        by1 = b[:, 1][None, :]
        bx2 = (b[:, 0] + b[:, 2])[None, :]
        by2 = (b[:, 1] + b[:, 3])[None, :]

        ix1 = np.maximum(ax1, bx1)
        iy1 = np.maximum(ay1, by1)
        ix2 = np.minimum(ax2, bx2)
        iy2 = np.minimum(ay2, by2)

        inter = np.maximum(0, ix2 - ix1) * np.maximum(0, iy2 - iy1)
        area_a = a[:, 2][:, None] * a[:, 3][:, None]
        area_b = b[:, 2][None, :] * b[:, 3][None, :]

        union = area_a + area_b - inter
        return np.where(union > 0, inter / union, 0)
