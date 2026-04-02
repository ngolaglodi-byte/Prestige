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
    // Timecode (SMPTE 12M — LTC/VITC)
    int     tcHours     = -1;  // -1 = no timecode
    int     tcMinutes   = 0;
    int     tcSeconds   = 0;
    int     tcFrames    = 0;
    bool    tcDropFrame = false;  // Drop-frame timecode (29.97fps)
    QString timecodeString() const {
        if (tcHours < 0) return QString();
        return QStringLiteral("%1:%2:%3%4%5")
            .arg(tcHours, 2, 10, QLatin1Char('0'))
            .arg(tcMinutes, 2, 10, QLatin1Char('0'))
            .arg(tcSeconds, 2, 10, QLatin1Char('0'))
            .arg(tcDropFrame ? QLatin1String(";") : QLatin1String(":"))
            .arg(tcFrames, 2, 10, QLatin1Char('0'));
    }
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
