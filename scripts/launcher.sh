#!/bin/bash
# ============================================================
# Prestige AI — Main Launcher (macOS)
# Copyright (c) 2024-2026 Prestige Technologie Company
#
# Architecture: 2 processes (Python AI eliminated)
#   1. Vision Engine (video + AI + compositor + output — single C++ binary)
#   2. Control Room (user interface — main window)
#
# AI runs natively in the Vision Engine via ONNX Runtime + whisper.cpp.
# No Python. No ZMQ between AI and Vision. No venv.
#
# When the Control Room closes, everything shuts down.
# ============================================================

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
LOG_DIR="$HOME/Library/Logs/PrestigeAI"

mkdir -p "$LOG_DIR"
mkdir -p "$HOME/.prestige-ai"

echo "[$(date '+%H:%M:%S')] Prestige AI starting..." | tee "$LOG_DIR/launch.log"

# Cleanup on exit — kill background processes
cleanup() {
    echo "[Prestige AI] Shutting down..." | tee -a "$LOG_DIR/launch.log"
    [ -n "$VISION_PID" ] && kill "$VISION_PID" 2>/dev/null
    wait 2>/dev/null
    echo "[Prestige AI] Stopped." | tee -a "$LOG_DIR/launch.log"
}
trap cleanup EXIT

# ── 1. Start Vision Engine (background) ──────────────────────
# Includes: video capture, AI detection (ONNX), tracking, compositor,
#           whisper subtitles, social chat, output routing
echo "[Vision+AI] Starting..." | tee -a "$LOG_DIR/launch.log"

VISION_BIN="$SCRIPT_DIR/prestige_vision"
if [ ! -f "$VISION_BIN" ]; then
    # Dev layout: look in build directory
    VISION_BIN="$SCRIPT_DIR/../build/vision_engine/prestige_vision"
fi

"$VISION_BIN" > "$LOG_DIR/vision.log" 2>&1 &
VISION_PID=$!
echo "[Vision+AI] PID=$VISION_PID (detection + tracking + subtitles + output)" | tee -a "$LOG_DIR/launch.log"

sleep 1

# ── 2. Start Control Room (foreground — main window) ─────────
echo "[Control Room] Starting..." | tee -a "$LOG_DIR/launch.log"

CONTROL_BIN="$SCRIPT_DIR/prestige_control"
if [ ! -f "$CONTROL_BIN" ]; then
    CONTROL_BIN="$SCRIPT_DIR/../build/control_room/prestige_control"
fi

"$CONTROL_BIN"

# When Control Room closes → cleanup kills Vision Engine
