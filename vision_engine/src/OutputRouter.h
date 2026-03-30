#pragma once

// ============================================================
// Prestige AI — Output Router
// Sends composited frames to multiple destinations simultaneously:
//   - RTMP (YouTube, Twitch, Facebook)
//   - SRT (professional contribution links)
//   - File (local recording)
//   - NDI (network video, when SDK available)
// Uses FFmpeg muxer for RTMP/SRT/File outputs.
// ============================================================

#include <QObject>
#include <QImage>
#include <QList>
#include <map>
#include <memory>

#include "FrameMetadata.h"

namespace prestige {

class VideoEncoder;

class IOutputStream : public QObject {
    Q_OBJECT
public:
    using QObject::QObject;
    virtual ~IOutputStream() = default;
    virtual bool open(const OutputConfig& config, int width, int height, int fps) = 0;
    virtual void close() = 0;
    virtual bool writePacket(const QByteArray& data, qint64 pts, qint64 dts, bool isKeyframe) = 0;
    virtual bool isOpen() const = 0;
    virtual OutputStats stats() const = 0;
};

class OutputRouter : public QObject {
    Q_OBJECT
    Q_PROPERTY(int activeCount READ activeCount NOTIFY outputChanged)

public:
    explicit OutputRouter(QObject* parent = nullptr);
    ~OutputRouter() override;

    // Add/remove outputs — resolution is auto-detected from first frame
    Q_INVOKABLE bool addOutput(int type, const QString& url, int bitrateMbps = 8, int fps = 25);
    Q_INVOKABLE void removeOutput(int type);
    Q_INVOKABLE void removeAllOutputs();

    // Send a composited frame to all active outputs
    // First call auto-initializes encoders at the frame's resolution (4K/1080p/etc)
    void sendFrame(const QImage& compositedFrame);

    int activeCount() const;
    OutputStats stats(OutputType type) const;

signals:
    void outputChanged();
    void outputError(int type, const QString& error);

private:
    struct OutputEntry {
        OutputConfig                   config;
        std::unique_ptr<VideoEncoder>  encoder;
        std::unique_ptr<IOutputStream> stream;
        OutputStats                    stats;
        int                            bitrateMbps = 8;
        int                            fps         = 25;
        bool                           active      = false;
        bool                           initialized = false; // lazy init on first frame
    };

    bool initializeEntry(OutputEntry& entry, int width, int height);

    std::map<int, std::unique_ptr<OutputEntry>> m_outputs;
};

// ── Concrete outputs ───────────────────────────────────────

class FfmpegOutputStream : public IOutputStream {
    Q_OBJECT
public:
    explicit FfmpegOutputStream(QObject* parent = nullptr);
    ~FfmpegOutputStream() override;

    bool open(const OutputConfig& config, int width, int height, int fps) override;
    void close() override;
    bool writePacket(const QByteArray& data, qint64 pts, qint64 dts, bool isKeyframe) override;
    bool isOpen() const override;
    OutputStats stats() const override;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

class FileOutputStream : public IOutputStream {
    Q_OBJECT
public:
    explicit FileOutputStream(QObject* parent = nullptr);
    ~FileOutputStream() override;

    bool open(const OutputConfig& config, int width, int height, int fps) override;
    void close() override;
    bool writePacket(const QByteArray& data, qint64 pts, qint64 dts, bool isKeyframe) override;
    bool isOpen() const override;
    OutputStats stats() const override;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace prestige
