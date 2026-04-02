#pragma once

// ============================================================
// Prestige AI — Video Encoder
// Real FFmpeg encoding: H.264/H.265 via VideoToolbox (macOS)
// or libx264/libx265 (software fallback)
// ============================================================

#include <QObject>
#include <QImage>
#include <QByteArray>
#include <QSize>
#include <memory>

namespace prestige {

struct EncoderConfig {
    QSize   resolution   = {1920, 1080};
    int     fpsNum       = 25;          // Numerator (supports fractional: 30000/1001 = 29.97)
    int     fpsDen       = 1;           // Denominator (1 = integer fps, 1001 = NTSC drop)
    int     bitrateMbps  = 8;
    QString codec        = QStringLiteral("h264");   // h264, hevc
    QString profile      = QStringLiteral("high");   // baseline, main, high (H.264)
    int     gopSize      = 50;
    bool    lowLatency   = true;
    // Rate control
    QString rateControl  = QStringLiteral("abr");    // abr, cbr, vbr
    int     maxBitrateMbps = 0;         // For VBR: max bitrate (0 = auto)
    int     bufSizeMbps  = 0;           // VBV buffer size (0 = auto)
    // Pixel format
    QString chromaFormat = QStringLiteral("yuv420p"); // yuv420p, yuv422p (broadcast 4:2:2)
    int     bitDepth     = 8;           // 8 or 10 (SMPTE ST 2084 / HDR)
    // Audio
    bool    audioEnabled     = true;
    int     audioSampleRate  = 48000;   // 48kHz (broadcast standard)
    int     audioChannels    = 2;       // Stereo
    int     audioBitrate     = 128000;  // 128 kbps AAC
};

struct EncodedPacket {
    QByteArray data;
    qint64     pts        = 0;
    qint64     dts        = 0;
    bool       isKeyframe = false;
};

class VideoEncoder : public QObject {
    Q_OBJECT

public:
    explicit VideoEncoder(QObject* parent = nullptr);
    ~VideoEncoder() override;

    bool initialize(const EncoderConfig& config);
    void shutdown();
    bool isInitialized() const;

    // Encode a composited frame (QImage RGB32 → H.264 packets)
    QList<EncodedPacket> encode(const QImage& frame);

    // Encode audio samples (PCM S16 interleaved → AAC packets)
    QList<EncodedPacket> encodeAudio(const QByteArray& pcmData);

    QString codecName() const;
    bool hasAudio() const;

signals:
    void errorOccurred(const QString& error);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace prestige
