"""Prestige AI — Face Detection & Recognition Engine."""

from __future__ import annotations

import json
import logging
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

import cv2
import numpy as np

from zmq_publisher import BBox, TalentMatch

logger = logging.getLogger(__name__)


@dataclass
class TalentRecord:
    id: str
    name: str
    role: str
    embeddings: list[np.ndarray] = field(default_factory=list)
    overlay_style: str = "default"
    active: bool = True


class FaceDetector:
    """Detects and recognizes faces using InsightFace / ONNX models."""

    def __init__(self, talents_db_path: str, threshold: float = 0.6) -> None:
        self._db_path = Path(talents_db_path)
        self._threshold = threshold
        self._talents: list[TalentRecord] = []
        self._model: Any = None  # insightface.app.FaceAnalysis
        self._load_talents_db()

    def load_model(self) -> None:
        """Load the InsightFace ONNX model.

        Uses bundled models from models/buffalo_l/ first (no internet needed).
        Falls back to ~/.insightface/models/ if bundled models not found.
        """
        try:
            from insightface.app import FaceAnalysis

            # Use bundled models (shipped with the app — no download needed)
            # InsightFace expects: root/models/buffalo_l/*.onnx
            # So root = parent directory of "models/"
            bundled_path = Path(__file__).parent / "models" / "buffalo_l"
            if (bundled_path / "det_10g.onnx").exists():
                model_root = str(Path(__file__).parent)
                logger.info("Using bundled models from %s", model_root)
            else:
                model_root = None  # InsightFace will use ~/.insightface/models/
                logger.info("Bundled models not found — using default path")

            self._model = FaceAnalysis(
                name="buffalo_l",
                root=model_root,
                providers=["CoreMLExecutionProvider", "CPUExecutionProvider"],
            )
            self._model.prepare(ctx_id=0, det_size=(640, 640))
            logger.info("InsightFace model loaded successfully (no internet needed)")
        except ImportError:
            logger.warning(
                "InsightFace not available — running in stub mode (rectangles only)"
            )
            self._model = None
        except Exception as exc:
            logger.error("Failed to load InsightFace model: %s", exc)
            self._model = None

    def detect(self, frame: np.ndarray) -> list[TalentMatch]:
        """Detect faces and attempt recognition on each."""
        if self._model is None:
            return self._detect_opencv_fallback(frame)

        faces = self._model.get(frame)
        results: list[TalentMatch] = []

        for face in faces:
            bbox = face.bbox.astype(int)
            bb = BBox(
                x=int(bbox[0]),
                y=int(bbox[1]),
                w=int(bbox[2] - bbox[0]),
                h=int(bbox[3] - bbox[1]),
            )

            match = self._recognize(face.embedding)
            if match:
                match.bbox = bb
                results.append(match)
            else:
                results.append(
                    TalentMatch(
                        id="unknown",
                        name="Inconnu",
                        role="",
                        confidence=float(face.det_score),
                        bbox=bb,
                        show_overlay=False,
                    )
                )

        return results

    def _recognize(self, embedding: np.ndarray) -> TalentMatch | None:
        """Match an embedding against the talent database."""
        best_match: TalentRecord | None = None
        best_score = 0.0

        for talent in self._talents:
            if not talent.active:
                continue
            for ref_emb in talent.embeddings:
                score = float(np.dot(embedding, ref_emb) / (
                    np.linalg.norm(embedding) * np.linalg.norm(ref_emb) + 1e-6
                ))
                if score > best_score:
                    best_score = score
                    best_match = talent

        if best_match and best_score >= self._threshold:
            return TalentMatch(
                id=best_match.id,
                name=best_match.name,
                role=best_match.role,
                confidence=best_score,
                overlay_style=best_match.overlay_style,
            )
        return None

    def _detect_opencv_fallback(self, frame: np.ndarray) -> list[TalentMatch]:
        """Fallback face detection using OpenCV Haar cascades.

        Note: Haar cascade cannot recognize — only detects.
        show_overlay=False so unknown faces (ads, etc.) get no overlay.
        """
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        cascade = cv2.CascadeClassifier(
            cv2.data.haarcascades + "haarcascade_frontalface_default.xml"
        )
        faces = cascade.detectMultiScale(gray, scaleFactor=1.1, minNeighbors=5)

        results: list[TalentMatch] = []
        for i, (x, y, w, h) in enumerate(faces):
            results.append(
                TalentMatch(
                    id=f"face_{i}",
                    name="Détecté",
                    role="",
                    confidence=0.8,
                    bbox=BBox(x=int(x), y=int(y), w=int(w), h=int(h)),
                    show_overlay=False,  # No overlay on unrecognized faces
                )
            )
        return results

    def add_talent(self, name: str, role: str, images: list[np.ndarray]) -> None:
        """Add a new talent with reference images."""
        if self._model is None:
            logger.error("Cannot add talent — model not loaded")
            return

        talent_id = f"talent_{len(self._talents) + 1:03d}"
        embeddings: list[np.ndarray] = []

        for img in images:
            faces = self._model.get(img)
            if faces:
                embeddings.append(faces[0].embedding)

        if not embeddings:
            logger.warning("No faces found in reference images for %s", name)
            return

        self._talents.append(
            TalentRecord(
                id=talent_id,
                name=name,
                role=role,
                embeddings=embeddings,
            )
        )
        logger.info("Added talent: %s (%s) with %d embeddings", name, talent_id, len(embeddings))

    def save_talents_db(self) -> None:
        """Persist the talent database to JSON (embeddings as lists)."""
        data = {
            "version": "1.0",
            "talents": [
                {
                    "id": t.id,
                    "name": t.name,
                    "role": t.role,
                    "embeddings": [e.tolist() for e in t.embeddings],
                    "overlay_style": t.overlay_style,
                    "active": t.active,
                }
                for t in self._talents
            ],
        }
        self._db_path.write_text(json.dumps(data, indent=2, ensure_ascii=False), encoding="utf-8")
        logger.info("Talent database saved to %s", self._db_path)

    def _load_talents_db(self) -> None:
        """Load talent database from JSON."""
        if not self._db_path.exists():
            logger.info("No talent database found at %s — starting empty", self._db_path)
            return

        try:
            data = json.loads(self._db_path.read_text(encoding="utf-8"))
            for entry in data.get("talents", []):
                self._talents.append(
                    TalentRecord(
                        id=entry["id"],
                        name=entry["name"],
                        role=entry["role"],
                        embeddings=[np.array(e) for e in entry.get("embeddings", [])],
                        overlay_style=entry.get("overlay_style", "default"),
                        active=entry.get("active", True),
                    )
                )
            logger.info("Loaded %d talents from database", len(self._talents))
        except (json.JSONDecodeError, KeyError) as exc:
            logger.error("Failed to load talent database: %s", exc)
