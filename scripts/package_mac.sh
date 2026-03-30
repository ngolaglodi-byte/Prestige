#!/bin/bash
set -e
# ============================================================
# Prestige AI — macOS .dmg Packager
# Copyright (c) 2024-2026 Prestige Technologie Company
# All rights reserved.
#
# Creates a distributable .app bundle + .dmg installer
# ============================================================

APP_NAME="Prestige AI"
APP_VERSION="1.0-beta"
APP_BUNDLE="dist/Prestige AI.app"
DMG_NAME="PrestigeAI-${APP_VERSION}-macOS.dmg"
ARCH=$(uname -m)

echo "================================================"
echo "  Prestige AI — macOS Package Builder"
echo "  Version: $APP_VERSION | Arch: $ARCH"
echo "================================================"
echo ""

# ── 1. Build ────────────────────────────────────────────────
echo "=== Step 1/8: Building from source ==="
./build_mac.sh

echo ""
echo "=== Step 2/8: Creating .app bundle ==="

# ── 2. Clean and create .app structure ──────────────────────
rm -rf "dist"
mkdir -p "$APP_BUNDLE/Contents/MacOS"
mkdir -p "$APP_BUNDLE/Contents/Resources"
mkdir -p "$APP_BUNDLE/Contents/Frameworks"
mkdir -p "$APP_BUNDLE/Contents/PlugIns"

# ── 3. Copy binaries ────────────────────────────────────────
echo "=== Step 3/8: Copying binaries ==="
cp build/control_room/prestige_control "$APP_BUNDLE/Contents/MacOS/prestige_control"
cp build/vision_engine/prestige_vision "$APP_BUNDLE/Contents/MacOS/prestige_vision"

# ── 4. Copy launcher ────────────────────────────────────────
echo "=== Step 4/8: Installing launcher ==="
cp scripts/launcher.sh "$APP_BUNDLE/Contents/MacOS/PrestigeAI"
chmod +x "$APP_BUNDLE/Contents/MacOS/PrestigeAI"

# ── 5. Copy Info.plist ──────────────────────────────────────
cp scripts/Info.plist "$APP_BUNDLE/Contents/Info.plist"

# ── 6. Copy AI Engine WITH venv + models (100% offline) ────
echo "=== Step 5/8: Bundling AI Engine (offline-ready) ==="
mkdir -p "$APP_BUNDLE/Contents/Resources/ai_engine"
for item in main.py detector.py zmq_publisher.py frame_receiver.py talent_manager.py tracker.py subtitle_engine.py social_chat.py talents.json requirements.txt; do
    if [ -e "ai_engine/$item" ]; then
        cp "ai_engine/$item" "$APP_BUNDLE/Contents/Resources/ai_engine/"
    fi
done
# Copy ONNX models (325MB — no download needed)
if [ -d "ai_engine/models" ]; then
    echo "  Bundling ONNX models (325MB)..."
    cp -r ai_engine/models/ "$APP_BUNDLE/Contents/Resources/ai_engine/models/" 2>/dev/null || true
fi
# Copy pre-built venv (692MB — no pip install needed, no internet)
if [ -d "ai_engine/venv" ]; then
    echo "  Bundling Python venv (692MB — 100% offline)..."
    cp -r ai_engine/venv/ "$APP_BUNDLE/Contents/Resources/ai_engine/venv/"
fi

# ── 7. Copy icon ────────────────────────────────────────────
echo "=== Step 6/8: Setting up icon ==="
if [ -f "resources/prestige.icns" ]; then
    cp "resources/prestige.icns" "$APP_BUNDLE/Contents/Resources/AppIcon.icns"
    echo "  Icon bundled (prestige.icns)."
else
    echo "  WARNING: No icon found — app will use default icon."
fi

# ── 8. Run macdeployqt to bundle Qt frameworks ─────────────
echo "=== Step 7/8: Bundling Qt frameworks (macdeployqt) ==="

# Find macdeployqt
MACDEPLOYQT=""
for path in \
    /opt/homebrew/bin/macdeployqt \
    /usr/local/bin/macdeployqt \
    /opt/homebrew/opt/qt/bin/macdeployqt \
    /opt/homebrew/opt/qt@6/bin/macdeployqt \
    /usr/local/opt/qt@6/bin/macdeployqt; do
    if [ -x "$path" ]; then
        MACDEPLOYQT="$path"
        break
    fi
done

if [ -z "$MACDEPLOYQT" ]; then
    echo "ERROR: macdeployqt not found!"
    echo "Install with: brew install qt@6"
    exit 1
fi
echo "  Using: $MACDEPLOYQT"

# macdeployqt needs to run on one of the actual binaries (not the shell launcher)
# We temporarily rename to make macdeployqt work, then restore.
# macdeployqt picks up the CFBundleExecutable — we point it at prestige_control.
TEMP_PLIST="$APP_BUNDLE/Contents/Info.plist"
# Temporarily set executable to prestige_control for macdeployqt
sed -i '' 's|<string>PrestigeAI</string>|<string>prestige_control</string>|' "$TEMP_PLIST"

"$MACDEPLOYQT" "$APP_BUNDLE" \
    -executable="$APP_BUNDLE/Contents/MacOS/prestige_vision" \
    -qmldir=control_room/qml \
    -qmldir=vision_engine/qml \
    -always-overwrite \
    2>&1 | grep -E "^(WARNING|ERROR)" | grep -v "Cannot resolve rpath" | grep -v "Could not parse otool" | grep -v "codesign verification" || true

# Restore the real launcher as executable
sed -i '' 's|<string>prestige_control</string>|<string>PrestigeAI</string>|' "$TEMP_PLIST"

echo "  Qt frameworks bundled."

# ── 9. Fix rpaths for prestige_vision ───────────────────────
# macdeployqt handles both binaries via -executable flag.
# Do a final check for any remaining non-bundled dylibs.
echo "  Verifying dylib paths..."

VISION_BIN="$APP_BUNDLE/Contents/MacOS/prestige_vision"
FRAMEWORKS_DIR="$APP_BUNDLE/Contents/Frameworks"

# Get all non-system dylibs used by prestige_vision
otool -L "$VISION_BIN" 2>/dev/null | grep -v "^$VISION_BIN" | \
    grep -v "/usr/lib/" | grep -v "/System/" | \
    awk '{print $1}' | while read -r dylib; do

    dylib_name=$(basename "$dylib")

    # Check if this framework was already bundled by macdeployqt
    if [ -f "$FRAMEWORKS_DIR/$dylib_name" ]; then
        install_name_tool -change "$dylib" \
            "@executable_path/../Frameworks/$dylib_name" \
            "$VISION_BIN" 2>/dev/null || true
    elif [[ "$dylib" == *".framework/"* ]]; then
        # It's a framework reference — extract framework name
        fw_name=$(echo "$dylib" | sed 's|.*/\([^/]*\.framework\)/.*|\1|')
        if [ -d "$FRAMEWORKS_DIR/$fw_name" ]; then
            fw_relative=$(echo "$dylib" | sed "s|.*/$fw_name/|$fw_name/|")
            install_name_tool -change "$dylib" \
                "@executable_path/../Frameworks/$fw_relative" \
                "$VISION_BIN" 2>/dev/null || true
        fi
    fi
done
echo "  Dylib paths fixed."

# ── 10. Ad-hoc codesign ────────────────────────────────────
echo "  Ad-hoc code signing..."
codesign --force --deep --sign - "$APP_BUNDLE" 2>/dev/null
echo "  Code signing complete."

# ── 11. Create .dmg ─────────────────────────────────────────
echo ""
echo "=== Step 8/8: Creating .dmg installer ==="
DMG_STAGING="dist/dmg_staging"
rm -rf "$DMG_STAGING"
mkdir -p "$DMG_STAGING"

cp -R "$APP_BUNDLE" "$DMG_STAGING/"
ln -sf /Applications "$DMG_STAGING/Applications"

# Create a background instructions file
cat > "$DMG_STAGING/.background_info.txt" << 'EOF'
Drag "Prestige AI" to "Applications" to install.
EOF

hdiutil create \
    -volname "Prestige AI Beta" \
    -srcfolder "$DMG_STAGING" \
    -ov \
    -format UDZO \
    -imagekey zlib-level=9 \
    "dist/$DMG_NAME"

rm -rf "$DMG_STAGING"

# ── Done ────────────────────────────────────────────────────
DMG_SIZE=$(du -sh "dist/$DMG_NAME" | cut -f1)
APP_SIZE=$(du -sh "$APP_BUNDLE" | cut -f1)

echo ""
echo "================================================"
echo "  BUILD + PACKAGE COMPLETE"
echo "================================================"
echo ""
echo "  App Bundle : $APP_BUNDLE ($APP_SIZE)"
echo "  DMG        : dist/$DMG_NAME ($DMG_SIZE)"
echo "  Arch       : $ARCH"
echo ""
echo "  To test the app directly:"
echo "    open \"$APP_BUNDLE\""
echo ""
echo "  To test the DMG:"
echo "    open \"dist/$DMG_NAME\""
echo "================================================"
