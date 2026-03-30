# Prestige AI

**Professional Broadcast Overlay System with AI Face Recognition**

© 2024-2026 Prestige Technologie Company
Developed by Glody Dimputu Ngola

---

## Overview

Prestige AI is an enterprise-grade broadcast overlay system that sits between a production switcher (ATEM/TriCaster/Ross/vMix) and the final broadcast output. It receives the program feed, uses AI to detect and recognize on-screen talent, composites intelligent overlays (nameplates, lower thirds, tickers), and re-outputs the enriched signal.

## Key Features

- **AI Face Recognition** — Real-time talent detection and identification using InsightFace
- **20 Broadcast Styles** — Professional overlay designs (BFM, CNN, BBC, France 2, etc.)
- **8 Animation Types** — Slide, fade, scale, wipe, iris, glitch and more
- **Multi-Output** — RTMP, SRT, NDI, SDI/HDMI simultaneous output
- **4K Support** — Full 4K input/output with hardware encoding (VideoToolbox/NVENC)
- **Real-time Subtitles** — Whisper AI speech-to-text, 100% offline
- **RSS Ticker** — Auto-fetching headlines from RSS/Atom feeds
- **Weather Overlay** — OpenWeatherMap API integration + manual fallback
- **Social Chat** — YouTube/Twitch chat overlay
- **QR Code Overlay** — Dynamic QR code display
- **Timer/Countdown** — Configurable countdown overlay
- **Scoreboard** — Sports score management
- **Teleprompter** — Built-in script display with mirror mode
- **Macro System** — Automated action sequences
- **Web Remote** — Browser-based remote control (HTTP :8080)
- **5 Languages** — French, English, Spanish, Arabic (RTL), Chinese
- **3-Layer System** — Channel logo + Show title + Talent nameplate with auto-cycling
- **Ad Protection** — Automatic overlay hiding during commercial breaks
- **License System** — Online activation with offline fallback
- **Animated Branding** — Entry + loop animations for channel logo and name

## Architecture

```
┌─────────────────┐     ZMQ :5557     ┌──────────────────┐
│  Vision Engine   │ ──────────────► │   AI Engine       │
│  (C++/Qt 6)     │                   │   (Python)        │
│                 │ ◄────────────── │                    │
│  VideoCapture   │     ZMQ :5555     │  InsightFace      │
│  Compositor     │                   │  Whisper           │
│  OutputRouter   │                   │  Face Tracker      │
└────────┬────────┘                   └──────────────────┘
         │
         │ ZMQ :5558 (preview)
         │ ZMQ :5559 (config)
         ▼
┌─────────────────┐
│  Control Room   │
│  (C++/Qt QML)   │
│                 │
│  20 QML pages   │
│  17 controllers │
│  License system │
└─────────────────┘
```

## Requirements

- **macOS** 13+ (Apple Silicon or Intel) or **Windows** 10/11
- Qt 6.5+ with Qt Quick, Multimedia, Network
- FFmpeg 6+ (for encoding)
- ZeroMQ 4+ (inter-process communication)
- Python 3.10+ with venv (AI Engine)

## Hardware Support (Runtime Detection)

- Blackmagic DeckLink (SDI/HDMI capture)
- AJA Corvid (SDI capture)
- Magewell Pro Capture
- NDI (network video)
- USB Webcams (fallback)

## Build

```bash
# macOS
bash build_mac.sh

# Or manually
cmake -B build -G Ninja
cmake --build build --parallel
```

## Package

```bash
bash scripts/package_mac.sh
```

## License

Proprietary software. All rights reserved.

© 2024-2026 Prestige Technologie Company
Developed by Glody Dimputu Ngola

For licensing inquiries, contact Prestige Technologie Company.
