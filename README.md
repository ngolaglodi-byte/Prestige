# Prestige AI

**Professional Broadcast Overlay System with AI Face Recognition**

© 2024-2026 Prestige Technologie Company
Developed by Glody Dimputu Ngola

---

## Overview

Prestige AI is an enterprise-grade broadcast overlay system that sits between a production switcher (ATEM/TriCaster/Ross/vMix) and the final broadcast output. It receives the program feed, uses AI to detect and recognize on-screen talent, composites intelligent overlays (nameplates, lower thirds, tickers), and re-outputs the enriched signal.

**Always-On Passthrough Architecture** — The video signal passes through continuously 24/7. Overlays are activated/deactivated without ever interrupting the broadcast.

## Key Features

- **AI Face Recognition** — Real-time talent detection (RetinaFace/SCRFD) and identification (ArcFace) via ONNX Runtime C++ (100% offline)
- **20 Broadcast Styles** — Professional overlay designs (BFM, CNN, BBC, France 2, Al Jazeera, etc.)
- **8 Animation Types** — Slide, fade, scale, wipe, iris, glitch and more
- **Multi-Output** — RTMP, SRT, NDI, SDI/HDMI simultaneous output to 7+ destinations
- **4K Support** — Full 4K input/output with hardware encoding (VideoToolbox/NVENC)
- **Real-time Subtitles** — Whisper AI speech-to-text, 7 languages, 100% offline
- **RSS Ticker** — Auto-fetching headlines from RSS/Atom feeds with configurable appearance
- **Weather Overlay** — OpenWeatherMap API integration + manual fallback
- **Social Chat** — YouTube/Twitch live chat overlay
- **QR Code Overlay** — Dynamic QR code display with configurable position
- **Timer/Countdown** — Configurable countdown with presets (30s, 1m, 2m, 5m)
- **Scoreboard** — Sports score management with team colors and timer
- **Teleprompter** — Built-in script display with mirror mode and file import
- **Macro System** — Automated action sequences
- **Analytics** — Real-time talent screen time tracking + exportable report
- **Web Remote** — Browser-based remote control with REST API (20+ endpoints)
- **Multi-Screen** — Second monitor fullscreen output for broadcast monitoring
- **5 Languages** — French, English, Spanish, Arabic (RTL), Chinese
- **3-Layer System** — Channel logo + Show title + Talent nameplate with auto-cycling
- **Ad Protection** — Bypass mode hides all overlays during commercial breaks
- **License System** — Online activation with offline fallback
- **Animated Branding** — Entry + continuous loop animations for channel logo and name
- **Mixer Auto-Detection** — Automatically recognizes ATEM, Roland, Panasonic, Elgato, Magewell
- **WYSIWYG Preview** — Real-time preview showing exactly what viewers see

---

## System Requirements

### Minimum — Streaming (1080p, single source)

| Component | Requirement |
|-----------|-------------|
| **Processor** | Intel Core i5-8400 (3.0 GHz, 6 cores) / AMD Ryzen 5 2600 / Apple M1 |
| **Memory** | 8 GB DDR4 |
| **Graphics** | Intel UHD 630 / AMD Radeon Vega 8 (integrated) |
| **Storage** | 10 GB SSD available |
| **Operating System** | Windows 10 64-bit / macOS 13 Ventura |
| **Network** | 10 Mbps upload |
| **Display** | 1366 × 768 minimum |

### Recommended — Professional Production (1080p, multi-output)

| Component | Requirement |
|-----------|-------------|
| **Processor** | Intel Core i7-10700 (3.8 GHz, 8 cores) / AMD Ryzen 7 3700X / Apple M2 |
| **Memory** | 16 GB DDR4 |
| **Graphics** | NVIDIA GeForce GTX 1650 (4 GB VRAM) / Apple M2 integrated |
| **Storage** | 500 GB SSD NVMe |
| **Operating System** | Windows 11 / macOS 14 Sonoma |
| **Network** | 30 Mbps upload |
| **Display** | 1920 × 1080 |

### Optimal — Broadcast TV (4K, full production)

| Component | Requirement |
|-----------|-------------|
| **Processor** | Intel Core i9-12900K (3.9 GHz, 16 cores) / AMD Ryzen 9 5900X / Apple M3 Pro |
| **Memory** | 32 GB DDR5 |
| **Graphics** | NVIDIA GeForce RTX 3060 (6 GB VRAM) / Apple M3 Pro |
| **Storage** | 1 TB SSD NVMe |
| **Broadcast I/O** | Blackmagic DeckLink Mini / AJA Corvid (optional) |
| **Operating System** | Windows 11 Pro / macOS 14 Sonoma |
| **Network** | 100 Mbps upload + Gigabit LAN (for NDI) |
| **Display** | 2 × 1920 × 1080 or 1 × 3840 × 2160 |

---

## Supported Input Sources

| Source | Connection | Hardware Required |
|--------|-----------|-------------------|
| Webcam / USB Camera | USB | None |
| Blackmagic ATEM (Mini, Mini Pro, ISO, Television Studio) | USB-C | None (built-in UVC) |
| Roland Mixer (VR-1HD, V-8HD, V-1HD+) | USB | None (built-in UVC) |
| Panasonic Mixer (AV-HSW10) | USB / NDI | None |
| vMix / OBS / Wirecast | NDI (network) | NDI Tools (free) |
| TriCaster | NDI / SDI | NDI Tools or DeckLink card |
| SDI Source | SDI | Blackmagic DeckLink (~$300) |
| SDI Source | SDI | AJA Corvid (~$500) |
| HDMI Source | HDMI → USB | Elgato Cam Link ($50-130) or Magewell ($200) |
| Any NDI Source | Ethernet / Wi-Fi | NDI Tools (free) |

## Supported Output Destinations

| Destination | Protocol | Use Case |
|-------------|----------|----------|
| YouTube Live | RTMP | Web streaming |
| Twitch | RTMP | Gaming / live |
| Facebook Live | RTMPS | Social media |
| Instagram Live | RTMPS | Social media |
| TikTok LIVE | RTMP | Social media |
| X (Twitter) | RTMP | Social media |
| Custom RTMP Server | RTMP | IPTV / Web TV |
| SRT Server | SRT | Professional contribution |
| NDI Output | NDI | Local network distribution |
| SDI Output | SDI | Broadcast (with DeckLink/AJA) |
| Local Recording | File (MKV/MP4) | Archive |

---

## Architecture

```
┌─────────────────┐     ZMQ :5557     ┌──────────────────┐
│  Vision Engine   │ ──────────────►  │   AI Engine       │
│  (C++/Qt 6)     │                   │   (Python 3.12)   │
│                 │ ◄──────────────  │                    │
│  VideoCapture   │     ZMQ :5555     │  InsightFace       │
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
│  15 QML pages   │
│  18 controllers │
│  168 UI elements│
│  License system │
│  REST API       │
└─────────────────┘
```

## Hardware Support (Runtime Detection)

All broadcast SDKs are loaded dynamically at runtime. No recompilation needed — install the hardware driver and Prestige AI detects it automatically.

- Blackmagic DeckLink (SDI/HDMI capture and output)
- AJA Corvid (SDI capture)
- Magewell Pro Capture (HDMI/SDI USB capture)
- NDI (network video input/output)
- USB Webcams and Mixers (auto-detected)

## REST API

When Web Remote is enabled (port 8080), Prestige AI exposes a complete REST API:

```
GET  /api/status        — Full broadcast status
GET  /api/programs      — List programs
GET  /api/talents       — List talents
GET  /api/analytics     — Real-time analytics
GET  /api/config        — Current configuration
GET  /api/docs          — Interactive API documentation

POST /api/overlays/toggle    — Toggle overlays
POST /api/bypass/toggle      — Toggle bypass (ad protection)
POST /api/record/toggle      — Toggle recording
POST /api/program/switch     — Switch program
POST /api/style/set          — Change overlay style
POST /api/ticker/set         — Set ticker text
POST /api/countdown/start    — Start countdown timer
POST /api/countdown/stop     — Stop countdown
POST /api/scoreboard/set     — Update scoreboard
POST /api/qrcode/set         — Configure QR code
POST /api/weather/toggle     — Toggle weather overlay
POST /api/subtitle/toggle    — Toggle subtitles
POST /api/clock/toggle       — Toggle clock
POST /api/macro/:id          — Execute macro
```

Compatible with Bitfocus Companion, Elgato StreamDeck, and broadcast automation systems.

## Build from Source

```bash
# macOS
bash build_mac.sh

# Windows (Visual Studio 2022 + vcpkg)
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release

# Manual
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

### Build Dependencies

- Qt 6.5+ (Core, Gui, Widgets, Qml, Quick, QuickControls2, Multimedia, Network)
- FFmpeg 6+ (libavcodec, libavformat, libavutil, libswscale)
- ZeroMQ 4+ (libzmq)
- Python 3.10+ with venv
- CMake 3.21+ / Ninja

## Package

```bash
# macOS DMG
bash scripts/package_mac.sh

# Windows Installer (via GitHub Actions CI/CD)
# Automated with Inno Setup — produces PrestigeAI-Setup.exe
```

## License

Proprietary software. All rights reserved.

© 2024-2026 Prestige Technologie Company
Developed by Glody Dimputu Ngola

A valid license key is required to use this software.
Each license is bound to a single machine.

For licensing inquiries, contact Prestige Technologie Company.
