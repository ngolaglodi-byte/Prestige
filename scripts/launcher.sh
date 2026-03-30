#!/bin/bash
# ============================================================
# Prestige AI — Main Launcher
# Starts Python AI Engine + Vision Engine + Control Room
# ============================================================

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
RESOURCES="$SCRIPT_DIR/../Resources"
AI_ENGINE="$RESOURCES/ai_engine"
LOG_DIR="$HOME/Library/Logs/PrestigeAI"
CONFIG_DIR="$HOME/.prestige-ai"

mkdir -p "$LOG_DIR"
mkdir -p "$CONFIG_DIR"

TIMESTAMP=$(date "+%Y-%m-%d %H:%M:%S")
echo "[$TIMESTAMP] Prestige AI v1.0-beta starting" | tee "$LOG_DIR/launch.log"

# Cleanup on exit
cleanup() {
    echo "[Prestige AI] Shutting down..." | tee -a "$LOG_DIR/launch.log"
    [ -n "$AI_PID" ]     && kill "$AI_PID" 2>/dev/null
    [ -n "$TALENT_PID" ] && kill "$TALENT_PID" 2>/dev/null
    [ -n "$VISION_PID" ] && kill "$VISION_PID" 2>/dev/null
    wait 2>/dev/null
    echo "[Prestige AI] Stopped." | tee -a "$LOG_DIR/launch.log"
}
trap cleanup EXIT

# ── 1. Find Python ──────────────────────────────────────────
PYTHON=""
for p in python3.12 python3.11 python3.10 python3; do
    if command -v "$p" &>/dev/null; then
        PYTHON="$p"
        break
    fi
done

if [ -z "$PYTHON" ]; then
    osascript -e 'display alert "Python 3 requis" message "Installe Python avec: brew install python@3.12" as critical'
    exit 1
fi
echo "[Setup] Using $($PYTHON --version)" | tee -a "$LOG_DIR/launch.log"

# ── 2. Python venv ────────────────────────────────────────
# The venv is PRE-BUNDLED in the .app (100% offline, no internet needed).
# If not bundled (dev mode), create it with pip install.
if [ -d "$AI_ENGINE/venv/bin/python3" ] || [ -d "$AI_ENGINE/venv/bin/python" ]; then
    echo "[AI] Pre-bundled Python venv found — no internet needed" | tee -a "$LOG_DIR/launch.log"
else
    echo "[AI] No bundled venv — installing dependencies (internet required)..." | tee -a "$LOG_DIR/launch.log"
    osascript -e 'display notification "Installation des dépendances Python (~2 min)..." with title "Prestige AI" subtitle "Premier lancement"' 2>/dev/null

    cd "$AI_ENGINE"
    "$PYTHON" -m venv venv
    source venv/bin/activate
    pip install --quiet --upgrade pip
    pip install --quiet -r requirements.txt 2>>"$LOG_DIR/pip_install.log"
    pip install --quiet insightface 2>>"$LOG_DIR/pip_install.log" || \
        echo "[AI] InsightFace unavailable — OpenCV fallback active" | tee -a "$LOG_DIR/launch.log"

    echo "[AI] Python setup complete" | tee -a "$LOG_DIR/launch.log"
    osascript -e 'display notification "Installation terminée !" with title "Prestige AI"' 2>/dev/null
fi

# ── 3. Start Python AI Engine (background) ──────────────────
echo "[AI] Starting AI Engine..." | tee -a "$LOG_DIR/launch.log"
cd "$AI_ENGINE"
source venv/bin/activate
"$PYTHON" main.py --dev > "$LOG_DIR/ai_engine.log" 2>&1 &
AI_PID=$!
echo "[AI] PID: $AI_PID" | tee -a "$LOG_DIR/launch.log"

# ── 3b. Start Talent Manager Server (background) ────────────
echo "[Talent] Starting Talent Manager..." | tee -a "$LOG_DIR/launch.log"
"$PYTHON" talent_manager.py > "$LOG_DIR/talent_manager.log" 2>&1 &
TALENT_PID=$!
echo "[Talent] PID: $TALENT_PID" | tee -a "$LOG_DIR/launch.log"

# Wait for ZeroMQ to be ready
sleep 2

# ── 4. Start Vision Engine (background) ─────────────────────
echo "[Vision] Starting Vision Engine..." | tee -a "$LOG_DIR/launch.log"
"$SCRIPT_DIR/prestige_vision" > "$LOG_DIR/vision.log" 2>&1 &
VISION_PID=$!
echo "[Vision] PID: $VISION_PID" | tee -a "$LOG_DIR/launch.log"

sleep 1

# ── 5. Start Control Room (foreground — main window) ────────
echo "[Control] Starting Control Room..." | tee -a "$LOG_DIR/launch.log"
"$SCRIPT_DIR/prestige_control"

# When Control Room exits, cleanup runs via trap
