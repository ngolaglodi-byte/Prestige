"""Prestige AI — Talent Management Server.

ZeroMQ REP server for talent CRUD operations.
Runs alongside main.py on a separate port.

Protocol: JSON request/response on tcp://127.0.0.1:5556

Requests:
    {"action": "list"}
    {"action": "get", "id": "talent_001"}
    {"action": "add", "name": "...", "role": "...", "photo_path": "..."}
    {"action": "update", "id": "...", "name": "...", "role": "..."}
    {"action": "delete", "id": "..."}
    {"action": "enroll_photo", "id": "...", "photo_path": "..."}
"""

from __future__ import annotations

import json
import logging
import signal
import sys
import time
from pathlib import Path
from typing import Any

import cv2
import numpy as np
import zmq

logger = logging.getLogger(__name__)

# ── Talent database ────────────────────────────────────────

class TalentDatabase:
    """Manages talents.json with face embeddings."""

    def __init__(self, db_path: str = "talents.json", photos_dir: str = "talent_photos") -> None:
        self._path = Path(db_path)
        self._photos_dir = Path(photos_dir)
        self._photos_dir.mkdir(exist_ok=True)
        self._talents: list[dict[str, Any]] = []
        self._model: Any = None
        self._load()

    def _load(self) -> None:
        if not self._path.exists():
            self._talents = []
            return
        try:
            data = json.loads(self._path.read_text(encoding="utf-8"))
            self._talents = data.get("talents", [])
            logger.info("Loaded %d talents from %s", len(self._talents), self._path)
        except (json.JSONDecodeError, KeyError) as exc:
            logger.error("Failed to load talents: %s", exc)
            self._talents = []

    def _save(self) -> None:
        data = {"version": "1.0", "talents": self._talents}
        self._path.write_text(
            json.dumps(data, indent=2, ensure_ascii=False), encoding="utf-8"
        )
        logger.info("Saved %d talents to %s", len(self._talents), self._path)

    def _load_model(self) -> bool:
        if self._model is not None:
            return True
        try:
            from insightface.app import FaceAnalysis
            self._model = FaceAnalysis(
                name="buffalo_l",
                providers=["CUDAExecutionProvider", "CPUExecutionProvider"],
            )
            self._model.prepare(ctx_id=0, det_size=(640, 640))
            logger.info("InsightFace model loaded for enrollment")
            return True
        except Exception as exc:
            logger.warning("InsightFace not available: %s — using OpenCV fallback", exc)
            return False

    def _next_id(self) -> str:
        max_num = 0
        for t in self._talents:
            try:
                num = int(t["id"].split("_")[1])
                max_num = max(max_num, num)
            except (IndexError, ValueError):
                pass
        return f"talent_{max_num + 1:03d}"

    def list_all(self) -> list[dict[str, Any]]:
        # Return without embeddings (too large for JSON responses)
        result = []
        for t in self._talents:
            entry = {k: v for k, v in t.items() if k != "embeddings"}
            entry["has_embeddings"] = len(t.get("embeddings", [])) > 0
            entry["embedding_count"] = len(t.get("embeddings", []))
            entry["photo_path"] = str(self._photos_dir / f"{t['id']}.jpg")
            entry["photo_exists"] = (self._photos_dir / f"{t['id']}.jpg").exists()
            result.append(entry)
        return result

    def get(self, talent_id: str) -> dict[str, Any] | None:
        for t in self._talents:
            if t["id"] == talent_id:
                entry = {k: v for k, v in t.items() if k != "embeddings"}
                entry["has_embeddings"] = len(t.get("embeddings", [])) > 0
                entry["embedding_count"] = len(t.get("embeddings", []))
                return entry
        return None

    def add(self, name: str, role: str, photo_path: str = "") -> dict[str, Any]:
        talent_id = self._next_id()
        talent = {
            "id": talent_id,
            "name": name,
            "role": role,
            "embeddings": [],
            "overlay_style": "default",
            "active": True,
        }
        self._talents.append(talent)

        # Process photo if provided
        if photo_path and Path(photo_path).exists():
            self._process_photo(talent_id, photo_path)

        self._save()
        logger.info("Added talent: %s (%s) as %s", name, role, talent_id)
        return {"id": talent_id, "name": name, "role": role, "success": True}

    def update(self, talent_id: str, **kwargs: Any) -> dict[str, Any]:
        for t in self._talents:
            if t["id"] == talent_id:
                for key in ("name", "role", "overlay_style", "active"):
                    if key in kwargs:
                        t[key] = kwargs[key]
                self._save()
                return {"success": True, "id": talent_id}
        return {"success": False, "error": "Talent not found"}

    def delete(self, talent_id: str) -> dict[str, Any]:
        before = len(self._talents)
        self._talents = [t for t in self._talents if t["id"] != talent_id]
        if len(self._talents) < before:
            # Remove photo
            photo = self._photos_dir / f"{talent_id}.jpg"
            if photo.exists():
                photo.unlink()
            self._save()
            return {"success": True}
        return {"success": False, "error": "Talent not found"}

    def enroll_photo(self, talent_id: str, photo_path: str) -> dict[str, Any]:
        """Process a photo and add face embedding to talent."""
        for t in self._talents:
            if t["id"] == talent_id:
                result = self._process_photo(talent_id, photo_path)
                self._save()
                return result
        return {"success": False, "error": "Talent not found"}

    def _process_photo(self, talent_id: str, photo_path: str) -> dict[str, Any]:
        """Extract face embedding from photo and store it."""
        img = cv2.imread(photo_path)
        if img is None:
            return {"success": False, "error": f"Cannot read image: {photo_path}"}

        # Save a copy as the talent's photo
        dest = self._photos_dir / f"{talent_id}.jpg"
        cv2.imwrite(str(dest), img)

        # Try to extract embedding
        if self._load_model():
            faces = self._model.get(img)
            if not faces:
                return {"success": True, "warning": "No face detected in photo", "embedding_added": False}

            embedding = faces[0].embedding.tolist()
            for t in self._talents:
                if t["id"] == talent_id:
                    t["embeddings"].append(embedding)
                    logger.info("Added embedding for %s (total: %d)", talent_id, len(t["embeddings"]))
                    return {"success": True, "embedding_added": True, "total_embeddings": len(t["embeddings"])}

        return {"success": True, "warning": "Model not available — photo saved without embedding", "embedding_added": False}


# ── ZMQ Server ──────────────────────────────────────────────

class TalentManagerServer:
    """ZMQ REP server for talent management."""

    def __init__(self, db: TalentDatabase, address: str = "tcp://127.0.0.1:5556") -> None:
        self._db = db
        self._address = address
        self._context: zmq.Context | None = None
        self._socket: zmq.Socket | None = None
        self._running = False

    def start(self) -> None:
        self._context = zmq.Context()
        self._socket = self._context.socket(zmq.REP)
        self._socket.setsockopt(zmq.RCVTIMEO, 500)
        self._socket.setsockopt(zmq.LINGER, 0)
        self._socket.bind(self._address)
        self._running = True
        logger.info("Talent manager server started on %s", self._address)

    def stop(self) -> None:
        self._running = False
        if self._socket:
            self._socket.close()
        if self._context:
            self._context.term()
        logger.info("Talent manager server stopped")

    def run(self) -> None:
        """Main server loop."""
        self.start()
        try:
            while self._running:
                try:
                    raw = self._socket.recv()
                    request = json.loads(raw.decode("utf-8"))
                    response = self._handle(request)
                    self._socket.send(json.dumps(response, ensure_ascii=False).encode("utf-8"))
                except zmq.Again:
                    continue
                except json.JSONDecodeError as exc:
                    self._socket.send(json.dumps({"error": f"Invalid JSON: {exc}"}).encode("utf-8"))
                except Exception as exc:
                    logger.error("Server error: %s", exc)
                    try:
                        self._socket.send(json.dumps({"error": str(exc)}).encode("utf-8"))
                    except zmq.ZMQError:
                        pass
        finally:
            self.stop()

    def _handle(self, req: dict[str, Any]) -> dict[str, Any]:
        action = req.get("action", "")

        if action == "list":
            return {"talents": self._db.list_all()}

        elif action == "get":
            result = self._db.get(req.get("id", ""))
            return result if result else {"error": "Not found"}

        elif action == "add":
            return self._db.add(
                name=req.get("name", "Sans nom"),
                role=req.get("role", ""),
                photo_path=req.get("photo_path", ""),
            )

        elif action == "update":
            return self._db.update(
                talent_id=req.get("id", ""),
                **{k: v for k, v in req.items() if k not in ("action", "id")},
            )

        elif action == "delete":
            return self._db.delete(req.get("id", ""))

        elif action == "enroll_photo":
            return self._db.enroll_photo(
                talent_id=req.get("id", ""),
                photo_path=req.get("photo_path", ""),
            )

        else:
            return {"error": f"Unknown action: {action}"}


# ── Main ────────────────────────────────────────────────────

_shutdown = False

def _handle_signal(sig: int, _frame: object) -> None:
    global _shutdown
    _shutdown = True

def main() -> None:
    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)s [%(levelname)s] %(name)s: %(message)s",
        datefmt="%H:%M:%S",
    )
    signal.signal(signal.SIGINT, _handle_signal)
    signal.signal(signal.SIGTERM, _handle_signal)

    import argparse
    parser = argparse.ArgumentParser(description="Prestige AI Talent Manager")
    parser.add_argument("--db", default="talents.json", help="Path to talents.json")
    parser.add_argument("--photos", default="talent_photos", help="Photo storage directory")
    parser.add_argument("--address", default="tcp://127.0.0.1:5556", help="ZMQ REP address")
    args = parser.parse_args()

    db = TalentDatabase(args.db, args.photos)
    server = TalentManagerServer(db, args.address)

    logger.info("Starting Talent Manager Server...")
    server.run()


if __name__ == "__main__":
    main()
