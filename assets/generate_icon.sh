#!/bin/bash
set -e
# ============================================================
# Prestige AI — Icon Generator
# Generates .icns from Python-rendered PNGs
# ============================================================

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ICONSET="$SCRIPT_DIR/AppIcon.iconset"

mkdir -p "$ICONSET"

echo "=== Generating Prestige AI icon ==="

# Find a working Python with Pillow
PYTHON=""
if [ -f "$SCRIPT_DIR/../ai_engine/venv/bin/python3" ]; then
    PYTHON="$SCRIPT_DIR/../ai_engine/venv/bin/python3"
elif command -v python3.12 &>/dev/null; then
    PYTHON=python3.12
elif command -v python3 &>/dev/null; then
    PYTHON=python3
fi

if [ -z "$PYTHON" ]; then
    echo "ERROR: Python3 not found"
    exit 1
fi

# Generate all required icon sizes using Pillow
"$PYTHON" - "$ICONSET" << 'PYEOF'
import sys, math
from pathlib import Path

try:
    from PIL import Image, ImageDraw, ImageFont
except ImportError:
    import subprocess
    subprocess.check_call([sys.executable, "-m", "pip", "install", "--quiet", "Pillow"])
    from PIL import Image, ImageDraw, ImageFont

iconset = Path(sys.argv[1])

# macOS required sizes: name → pixel size
SIZES = {
    "icon_16x16":        16,
    "icon_16x16@2x":     32,
    "icon_32x32":        32,
    "icon_32x32@2x":     64,
    "icon_128x128":      128,
    "icon_128x128@2x":   256,
    "icon_256x256":      256,
    "icon_256x256@2x":   512,
    "icon_512x512":      512,
    "icon_512x512@2x":   1024,
}

def render_icon(size: int) -> Image.Image:
    """Render a professional broadcast-style icon."""
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    pad = max(1, size // 64)
    radius = size // 5

    # Background — dark broadcast blue
    draw.rounded_rectangle(
        [pad, pad, size - pad - 1, size - pad - 1],
        radius=radius,
        fill=(10, 10, 26, 255),
    )

    # Border — subtle purple accent
    draw.rounded_rectangle(
        [pad, pad, size - pad - 1, size - pad - 1],
        radius=radius,
        outline=(91, 79, 219, 180),
        width=max(1, size // 80),
    )

    # "P" letter — main branding
    p_size = int(size * 0.52)
    try:
        font_p = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", p_size)
    except (OSError, IOError):
        font_p = ImageFont.load_default()

    bbox = draw.textbbox((0, 0), "P", font=font_p)
    tw = bbox[2] - bbox[0]
    th = bbox[3] - bbox[1]
    px = (size - tw) // 2 - bbox[0]
    py = (size - th) // 2 - bbox[1] + int(size * 0.02)
    draw.text((px, py), "P", fill=(91, 79, 219, 255), font=font_p)

    # Green "AI" badge — top-right
    badge_r = max(4, int(size * 0.12))
    badge_cx = int(size * 0.76)
    badge_cy = int(size * 0.24)
    draw.ellipse(
        [badge_cx - badge_r, badge_cy - badge_r,
         badge_cx + badge_r, badge_cy + badge_r],
        fill=(29, 185, 84, 255),
    )

    if size >= 64:
        ai_size = max(8, int(badge_r * 0.85))
        try:
            font_ai = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", ai_size)
        except (OSError, IOError):
            font_ai = ImageFont.load_default()
        bbox_ai = draw.textbbox((0, 0), "AI", font=font_ai)
        atw = bbox_ai[2] - bbox_ai[0]
        ath = bbox_ai[3] - bbox_ai[1]
        draw.text(
            (badge_cx - atw // 2 - bbox_ai[0],
             badge_cy - ath // 2 - bbox_ai[1]),
            "AI", fill=(255, 255, 255, 255), font=font_ai,
        )

    # Red ON-AIR dot — bottom-right
    if size >= 32:
        dot_r = max(2, int(size * 0.04))
        dot_cx = int(size * 0.82)
        dot_cy = int(size * 0.82)
        draw.ellipse(
            [dot_cx - dot_r, dot_cy - dot_r,
             dot_cx + dot_r, dot_cy + dot_r],
            fill=(227, 6, 19, 255),
        )

    return img


for name, px in SIZES.items():
    icon = render_icon(px)
    out = iconset / f"{name}.png"
    icon.save(str(out), "PNG")
    print(f"  {name}.png ({px}x{px})")

print("All icon sizes generated.")
PYEOF

# Generate .icns using macOS iconutil
iconutil -c icns "$ICONSET" -o "$SCRIPT_DIR/icon.icns"

echo "Icon created: $SCRIPT_DIR/icon.icns"
