#!/bin/bash
# ============================================================
# Prestige AI — Beta Install Checker
# Verifies system requirements for beta testers
# ============================================================

echo "=== Prestige AI Beta — System Check ==="
echo ""

ERRORS=0

# Check macOS version
OS_VER=$(sw_vers -productVersion | cut -d. -f1)
OS_FULL=$(sw_vers -productVersion)
if [ "$OS_VER" -ge 13 ] 2>/dev/null; then
    echo "[OK] macOS $OS_FULL"
else
    echo "[!!] macOS 13+ required (you have $OS_FULL)"
    ERRORS=$((ERRORS + 1))
fi

# Check architecture
ARCH=$(uname -m)
echo "[OK] Architecture: $ARCH"

# Check Python
if command -v python3 &>/dev/null; then
    PY_VER=$(python3 --version 2>&1)
    echo "[OK] $PY_VER"
else
    echo "[!!] Python 3 not found"
    echo "     Install with: brew install python@3.12"
    ERRORS=$((ERRORS + 1))
fi

# Check available disk space (need ~1GB)
AVAIL_KB=$(df -k "$HOME" | tail -1 | awk '{print $4}')
AVAIL_GB=$((AVAIL_KB / 1024 / 1024))
if [ "$AVAIL_GB" -ge 1 ]; then
    echo "[OK] Disk space: ${AVAIL_GB}GB available"
else
    echo "[!!] Low disk space: ${AVAIL_GB}GB (need 1GB+)"
    ERRORS=$((ERRORS + 1))
fi

# Check camera
if system_profiler SPCameraDataType 2>/dev/null | grep -q "Camera"; then
    echo "[OK] Camera detected"
else
    echo "[--] No camera detected (external camera required)"
fi

echo ""
if [ "$ERRORS" -eq 0 ]; then
    echo "=== System ready for Prestige AI ==="
    echo "Drag 'Prestige AI' to Applications, then right-click > Open"
else
    echo "=== $ERRORS issue(s) found — fix before launching ==="
fi
