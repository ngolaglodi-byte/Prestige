#pragma once

// ============================================================
// Prestige AI — Frame Metadata
// Per-frame metadata carried through the pipeline
// ============================================================

#include <QString>
#include <QSize>
#include <cstdint>

namespace prestige {

struct FrameMetadata {
    int64_t frameId     = 0;
    int64_t timestampMs = 0;
    QSize   resolution  = {1920, 1080};
    int     fps         = 25;
    bool    interlaced  = false;
    QString colorSpace  = QStringLiteral("BT.709");
};

struct CaptureStats {
    int64_t framesCapture  = 0;
    int64_t framesDropped  = 0;
    double  currentFps     = 0.0;
    double  latencyMs      = 0.0;
};

struct CaptureConfig {
    QString sourceType;     // "ndi", "decklink", "aja", "magewell", "webcam"
    QString sourceName;     // Device or NDI source name
    QSize   resolution = {1920, 1080};
    int     fps        = 25;
};

enum class OutputType {
    SDI,
    HDMI,
    NDI,
    RTMP,
    SRT,
    HLS,
    File
};

struct OutputConfig {
    OutputType type;
    QString    url;             // For RTMP/SRT/HLS
    QString    deviceName;      // For SDI/HDMI/NDI
    int        bitrateMbps = 8;
    QString    codec = QStringLiteral("h264"); // h264, hevc, prores
};

struct OutputStats {
    int64_t framesSent    = 0;
    int64_t framesDropped = 0;
    double  bitrateMbps   = 0.0;
    double  latencyMs     = 0.0;
    bool    connected     = false;
};

} // namespace prestige
