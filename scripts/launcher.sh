#!/bin/bash
# ============================================================
# Prestige AI — Main Launcher (macOS)
# Copyright (c) 2024-2026 Prestige Technologie Company
#
# Starts all 3 processes:
#   1. Python AI Engine (detection + tracking + talents + subtitles)
#   2. Vision Engine (video capture + compositor + output)
#   3. Control Room (user interface — main window)
#
# When the Control Room closes, everything shuts down.
# ============================================================

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
RESOURCES="$SCRIPT_DIR/../Resources"
AI_ENGINE="$RESOURCES/ai_engine"
LOG_DIR="$HOME/Library/Logs/PrestigeAI"

mkdir -p "$LOG_DIR"
mkdir -p "$HOME/.prestige-ai"

echo "[$(date '+%H:%M:%S')] Prestige AI starting..." | tee "$LOG_DIR/launch.log"

# Cleanup on exit — kill all background processes
cleanup() {
    echo "[Prestige AI] Shutting down..." | tee -a "$LOG_DIR/launch.log"
    [ -n "$AI_PID" ] && kill "$AI_PID" 2>/dev/null
    [ -n "$VISION_PID" ] && kill "$VISION_PID" 2>/dev/null
    wait 2>/dev/null
    echo "[Prestige AI] Stopped." | tee -a "$LOG_DIR/launch.log"
}
trap cleanup EXIT

# ── 1. Find Python ──────────────────────────────────────────
if [ -f "$AI_ENGINE/venv/bin/python3" ]; then
    PYTHON="$AI_ENGINE/venv/bin/python3"
    echo "[Setup] Using bundled Python venv (100% offline)" | tee -a "$LOG_DIR/launch.log"
else
    PYTHON=""
    for p in python3.12 python3.11 python3.10 python3; do
        if command -v "$p" &>/dev/null; then PYTHON="$p"; break; fi
    done
    if [ -z "$PYTHON" ]; then
        osascript -e 'display alert "Python 3 requis" message "Installez Python: brew install python@3.12" as critical'
        exit 1
    fi
    # Create venv if not bundled
    if [ ! -d "$AI_ENGINE/venv" ]; then
        echo "[Setup] Creating Python venv (first launch)..." | tee -a "$LOG_DIR/launch.log"
        cd "$AI_ENGINE"
        "$PYTHON" -m venv venv
        venv/bin/pip install --quiet --upgrade pip
        venv/bin/pip install --quiet insightface onnxruntime opencv-python-headless pyzmq numpy 2>>"$LOG_DIR/pip.log"
        venv/bin/pip install --quiet faster-whisper sounddevice 2>>"$LOG_DIR/pip.log"
        echo "[Setup] Python setup complete" | tee -a "$LOG_DIR/launch.log"
    fi
    PYTHON="$AI_ENGINE/venv/bin/python3"
fi

# ── 2. Start AI Engine (background) ─────────────────────────
# Includes: face detection, tracking, talent manager (:5556), subtitles
echo "[AI] Starting..." | tee -a "$LOG_DIR/launch.log"
cd "$AI_ENGINE"
"$PYTHON" main.py --dev > "$LOG_DIR/ai_engine.log" 2>&1 &
AI_PID=$!
echo "[AI] PID=$AI_PID (detection + talents + subtitles)" | tee -a "$LOG_DIR/launch.log"

sleep 2

# ── 3. Start Vision Engine (background) ─────────────────────
echo "[Vision] Starting..." | tee -a "$LOG_DIR/launch.log"
"$SCRIPT_DIR/prestige_vision" > "$LOG_DIR/vision.log" 2>&1 &
VISION_PID=$!
echo "[Vision] PID=$VISION_PID" | tee -a "$LOG_DIR/launch.log"

sleep 1

# ── 4. Start Control Room (foreground — main window) ────────
echo "[Control Room] Starting..." | tee -a "$LOG_DIR/launch.log"
"$SCRIPT_DIR/prestige_control"

# When Control Room closes → cleanup kills AI + Vision
