"""Prestige AI — AI Engine
Copyright (c) 2024-2026 Prestige Technologie Company
All rights reserved.

ARCHITECTURE:
    C++ Vision Engine captures video and sends frames to us (port 5557)
    We analyse frames -> detect faces -> track -> publish results (port 5555)
    C++ receives our results and composites overlays on the SAME frames
    -> Zero desync because we analyse the exact frames C++ outputs.

Modes:
    python main.py --dev       # Webcam mode (Python captures -- for dev without C++)
    python main.py --prod      # Receive frames from C++ Vision Engine (production)
    python main.py             # Auto-detect: try C++ first, fallback to webcam
"""

from __future__ import annotations

import argparse
import logging
import queue
import signal
import sys
import threading
import time

import cv2
import numpy as np

from detector import FaceDetector
from tracker import FaceTracker
from zmq_publisher import ZmqPublisher
from subtitle_engine import SubtitleEngine

# ── Logging ─────────────────────────────────────────────────
logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(name)s: %(message)s",
    datefmt="%H:%M:%S",
)
logger = logging.getLogger("prestige.ai")

# ── Graceful shutdown ───────────────────────────────────────
_shutdown = threading.Event()


def _handle_signal(sig: int, _frame: object) -> None:
    logger.info("Received signal %d — shutting down…", sig)
    _shutdown.set()


signal.signal(signal.SIGINT, _handle_signal)
signal.signal(signal.SIGTERM, _handle_signal)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Prestige AI Engine")
    parser.add_argument("--dev", action="store_true", help="Webcam mode (Python captures)")
    parser.add_argument("--prod", action="store_true", help="Receive frames from C++ Vision Engine")
    parser.add_argument("--camera", type=int, default=0, help="Webcam index (dev mode)")
    parser.add_argument("--zmq-pub", default="tcp://127.0.0.1:5555", help="ZMQ PUB address (results)")
    parser.add_argument("--zmq-frames", default="tcp://127.0.0.1:5557", help="ZMQ SUB address (frames from C++)")
    parser.add_argument("--talents-db", default="talents.json", help="Path to talents database")
    parser.add_argument("--threshold", type=float, default=0.4, help="Recognition threshold")
    parser.add_argument("--smoothing", type=float, default=0.3, help="Bbox smoothing factor")
    parser.add_argument("--whisper-model", default="base", choices=["tiny", "base", "small", "medium"],
                        help="Whisper model size for subtitles")
    parser.add_argument("--subtitle-lang", default="auto", help="Subtitle language (auto, fr, en, es, de, ar, zh)")
    parser.add_argument("--no-subtitles", action="store_true", help="Disable subtitle engine")
    parser.add_argument("--audio-device", type=int, default=None, help="Audio input device index")
    return parser.parse_args()


# ── Frame Source: C++ Vision Engine (production) ────────────

def prod_capture_thread(
    frame_queue: queue.Queue,
    det_queue: queue.Queue,
    zmq_frames_address: str,
) -> None:
    """Receives frames from C++ Vision Engine via ZMQ."""
    from frame_receiver import FrameReceiver

    receiver = FrameReceiver(zmq_frames_address)
    receiver.start()
    logger.info("[Capture] Receiving frames from C++ on %s", zmq_frames_address)

    fps_timer = time.monotonic()
    fps_count = 0

    while not _shutdown.is_set():
        frame, frame_id, capture_ts = receiver.receive()
        if frame is None:
            continue

        now = time.monotonic()
        fps_count += 1

        # Send to both tracking and detection queues
        try:
            while not frame_queue.empty():
                frame_queue.get_nowait()
            frame_queue.put_nowait((frame, now, frame_id, capture_ts))
        except queue.Full:
            pass

        try:
            while not det_queue.empty():
                det_queue.get_nowait()
            det_queue.put_nowait((frame.copy(), now, frame_id, capture_ts))
        except queue.Full:
            pass

        elapsed = time.monotonic() - fps_timer
        if elapsed >= 3.0:
            logger.info("[Capture] Receiving %.1f fps from C++", fps_count / elapsed)
            fps_timer = time.monotonic()
            fps_count = 0

    receiver.stop()
    logger.info("[Capture] Stopped")


# ── Frame Source: Webcam (dev mode) ─────────────────────────

def dev_capture_thread(
    frame_queue: queue.Queue,
    det_queue: queue.Queue,
    camera_index: int,
) -> None:
    """Captures from webcam directly (dev mode, no C++)."""
    logger.info("[Capture] Dev mode — opening webcam %d", camera_index)
    cap = cv2.VideoCapture(camera_index)
    if not cap.isOpened():
        logger.error("Failed to open webcam %d", camera_index)
        _shutdown.set()
        return

    cap.set(cv2.CAP_PROP_FRAME_WIDTH, 1280)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 720)
    cap.set(cv2.CAP_PROP_FPS, 30)
    cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)

    frame_count = 0

    while not _shutdown.is_set():
        ret, frame = cap.read()
        if not ret:
            time.sleep(0.005)
            continue

        frame_count += 1
        now = time.monotonic()
        capture_ts = int(now * 1000)

        try:
            while not frame_queue.empty():
                frame_queue.get_nowait()
            frame_queue.put_nowait((frame, now, frame_count, capture_ts))
        except queue.Full:
            pass

        try:
            while not det_queue.empty():
                det_queue.get_nowait()
            det_queue.put_nowait((frame.copy(), now, frame_count, capture_ts))
        except queue.Full:
            pass

    cap.release()
    logger.info("[Capture] Webcam stopped (%d frames)", frame_count)


# ── Detection Thread ────────────────────────────────────────

def detection_thread(
    detector: FaceDetector,
    tracker: FaceTracker,
    det_queue: queue.Queue,
    det_lock: threading.Lock,
) -> None:
    """Runs AI detection at max speed (~5-10fps), updates tracker."""
    logger.info("[Detection] Thread started")
    fps_timer = time.monotonic()
    fps_count = 0

    while not _shutdown.is_set():
        try:
            frame, timestamp, frame_id, capture_ts = det_queue.get(timeout=0.1)
        except queue.Empty:
            continue

        t0 = time.monotonic()
        detections = detector.detect(frame)
        dt = (time.monotonic() - t0) * 1000

        with det_lock:
            tracker.update_detections(frame, detections)

        fps_count += 1
        elapsed = time.monotonic() - fps_timer
        if elapsed >= 3.0:
            n_faces = len(detections)
            n_known = sum(1 for d in detections if d.id not in ("unknown", "face_0"))
            logger.info(
                "[Detection] %.1f fps | %dms | %d faces (%d known)",
                fps_count / elapsed, dt, n_faces, n_known,
            )
            fps_timer = time.monotonic()
            fps_count = 0

    logger.info("[Detection] Thread stopped")


# ── Tracking + Publish Thread ───────────────────────────────

def tracking_thread(
    tracker: FaceTracker,
    publisher: ZmqPublisher,
    frame_queue: queue.Queue,
    det_lock: threading.Lock,
) -> None:
    """Runs at capture rate — smooth tracking + ZMQ publish."""
    logger.info("[Tracking] Thread started")
    fps_timer = time.monotonic()
    fps_count = 0

    while not _shutdown.is_set():
        try:
            frame, timestamp, frame_id, capture_ts = frame_queue.get(timeout=0.05)
        except queue.Empty:
            continue

        with det_lock:
            tracked = tracker.update_tracking()

        visible = [t for t in tracked if t.show_overlay]
        if len(visible) == 0:
            scene_type = "empty"
        elif len(visible) == 1:
            scene_type = "single"
        else:
            scene_type = "interview"

        publisher.publish(
            tracked,
            scene_type=scene_type,
            capture_timestamp_ms=capture_ts,
            detection_latency_ms=0,
        )

        fps_count += 1
        elapsed = time.monotonic() - fps_timer
        if elapsed >= 5.0:
            logger.info("[Tracking] %.1f fps | %d tracks", fps_count / elapsed, len(tracked))
            fps_timer = time.monotonic()
            fps_count = 0

    logger.info("[Tracking] Thread stopped")


# ── Main ────────────────────────────────────────────────────

def main() -> None:
    args = parse_args()

    # Auto-detect mode — wait for Vision Engine, NEVER open webcam automatically
    if not args.dev and not args.prod:
        import zmq
        logger.info("Waiting for Vision Engine on %s...", args.zmq_frames)
        while not _shutdown.is_set():
            try:
                ctx = zmq.Context()
                sock = ctx.socket(zmq.SUB)
                sock.subscribe(b"")
                sock.setsockopt(zmq.RCVTIMEO, 2000)
                sock.setsockopt(zmq.CONFLATE, 1)
                sock.connect(args.zmq_frames)
                try:
                    sock.recv()
                    args.prod = True
                    logger.info("Vision Engine detected — production mode")
                    break
                except zmq.Again:
                    pass  # Keep waiting
                finally:
                    sock.close()
                    ctx.term()
            except Exception:
                pass
            time.sleep(1)

        if _shutdown.is_set():
            return

    # Initialize
    detector = FaceDetector(args.talents_db, threshold=args.threshold)
    detector.load_model()

    tracker = FaceTracker(smoothing=args.smoothing)

    publisher = ZmqPublisher(args.zmq_pub)
    publisher.start()

    frame_queue = queue.Queue(maxsize=2)
    det_queue = queue.Queue(maxsize=1)
    subtitle_queue = queue.Queue(maxsize=10)
    det_lock = threading.Lock()

    # Choose capture source
    if args.prod:
        capture_fn = lambda: prod_capture_thread(frame_queue, det_queue, args.zmq_frames)
        mode_name = "PRODUCTION (frames from C++)"
    else:
        capture_fn = lambda: dev_capture_thread(frame_queue, det_queue, args.camera)
        mode_name = "DEV (webcam)"

    logger.info("Mode: %s", mode_name)

    threads = [
        threading.Thread(target=capture_fn, name="capture", daemon=True),
        threading.Thread(target=detection_thread, args=(detector, tracker, det_queue, det_lock), name="detection", daemon=True),
        threading.Thread(target=tracking_thread, args=(tracker, publisher, frame_queue, det_lock), name="tracking", daemon=True),
    ]

    # ── Subtitle Engine (Whisper) ─────────────────────────────
    subtitle_engine = None
    if not args.no_subtitles:
        subtitle_engine = SubtitleEngine(
            subtitle_queue=subtitle_queue,
            model_size=args.whisper_model,
            language=args.subtitle_lang,
            device=args.audio_device,
            shutdown_event=_shutdown,
        )
        if subtitle_engine.load_model():
            # Audio capture + transcription thread
            threads.append(threading.Thread(
                target=subtitle_engine.start_capture,
                name="subtitle-capture",
                daemon=True,
            ))
            # Subtitle publish thread (reads from queue, sends via ZMQ)
            def subtitle_publish_loop():
                while not _shutdown.is_set():
                    try:
                        msg = subtitle_queue.get(timeout=0.2)
                        publisher.publish_subtitle(msg)
                    except queue.Empty:
                        continue
            threads.append(threading.Thread(
                target=subtitle_publish_loop,
                name="subtitle-publish",
                daemon=True,
            ))
            logger.info("Whisper subtitles: model=%s lang=%s", args.whisper_model, args.subtitle_lang)
        else:
            logger.info("Whisper model load failed — subtitles disabled")

    # ── Talent Manager Server (ZMQ REP :5556) ───────────────
    from talent_manager import TalentDatabase, TalentManagerServer

    talent_db = TalentDatabase(args.talents_db, "talent_photos")
    talent_server = TalentManagerServer(talent_db, "tcp://127.0.0.1:5556")
    threads.append(threading.Thread(
        target=talent_server.run,
        name="talent-manager",
        daemon=True,
    ))
    logger.info("Talent Manager: started on :5556")

    for t in threads:
        t.start()

    logger.info("Pipeline: Capture → Detection → Tracking → ZMQ(:5555)")
    logger.info("Pipeline: Talent Manager → ZMQ(:5556)")
    if subtitle_engine:
        logger.info("Pipeline: Audio → Whisper → Subtitles → ZMQ(:5555)")

    try:
        while not _shutdown.is_set():
            _shutdown.wait(timeout=1.0)
    except KeyboardInterrupt:
        pass

    logger.info("Shutting down...")
    _shutdown.set()
    for t in threads:
        t.join(timeout=3.0)
    publisher.stop()
    logger.info("AI Engine stopped.")


if __name__ == "__main__":
    main()
