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
    int     fps          = 25;
    int     bitrateMbps  = 8;
    QString codec        = QStringLiteral("h264");  // h264, hevc
    QString profile      = QStringLiteral("high");
    int     gopSize      = 50;
    bool    lowLatency   = true;
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

    QString codecName() const;

signals:
    void errorOccurred(const QString& error);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace prestige
