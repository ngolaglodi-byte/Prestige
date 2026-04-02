#pragma once

// ============================================================
// Prestige AI — Video Capture
// C++ is the master of the video pipeline.
// Captures frames and provides them to:
//   1. FrameSender → Python AI for analysis
//   2. OverlayRenderer → QML overlay compositing
//   3. OutputRouter → final output (NDI/RTMP/SDI)
// ============================================================

#include <QObject>
#include <QImage>
#include <QSize>
#include <QCamera>
#include <QMediaCaptureSession>
#include <QVideoSink>
#include <QVideoFrame>
#include <QAudioInput>
#include <QTimer>
#include <QElapsedTimer>
#include <memory>

#include "FrameMetadata.h"
#include "DecklinkCapture.h"
#include "AjaCapture.h"
#include "MagewellCapture.h"
#include "NdiCapture.h"

namespace prestige {

class VideoCapture : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool active READ isActive NOTIFY activeChanged)
    Q_PROPERTY(double fps READ currentFps NOTIFY statsUpdated)
    Q_PROPERTY(QSize resolution READ resolution NOTIFY statsUpdated)

public:
    explicit VideoCapture(QObject* parent = nullptr);
    ~VideoCapture() override;

    Q_INVOKABLE bool openWebcam(int index = 0);
    Q_INVOKABLE bool openSource(const QString& type, const QString& source);
    Q_INVOKABLE void close();

    QString currentType() const { return m_currentType; }
    QString currentSource() const { return m_currentSource; }

    bool isActive() const { return m_active; }
    double currentFps() const { return m_currentFps; }
    QSize resolution() const { return m_resolution; }

    // Get the latest captured frame (thread-safe copy)
    QImage latestFrame() const;

    // Deinterlace: bob deinterlacing (line doubling — SMPTE RP 220 compliant)
    static QImage deinterlace(const QImage& interlacedFrame, bool topFieldFirst = true);
    qint64 latestFrameId() const { return m_frameId; }
    qint64 latestTimestampMs() const { return m_captureTimestampMs; }

signals:
    void activeChanged(bool active);
    void statsUpdated();

    // Emitted for every captured frame — connect to pipeline
    void frameCaptured(const QImage& frame, qint64 frameId, qint64 captureTimestampMs);

private slots:
    void onVideoFrameChanged(const QVideoFrame& frame);

private:
    QCamera*              m_camera     = nullptr;
    QMediaCaptureSession* m_session    = nullptr;
    QVideoSink*           m_sink       = nullptr;
    QAudioInput*          m_audioInput = nullptr;

    bool    m_active            = false;
    QImage  m_latestFrame;
    qint64  m_frameId           = 0;
    qint64  m_captureTimestampMs = 0;
    QSize   m_resolution;
    double  m_currentFps        = 0.0;

    // FPS calculation
    int           m_fpsFrameCount = 0;
    QElapsedTimer m_fpsTimer;

    // Current source tracking
    QString m_currentType;
    QString m_currentSource;

    // Hardware capture cards (created on demand, destroyed on close)
    DecklinkCapture* m_decklink = nullptr;
    AjaCapture*      m_aja      = nullptr;
    MagewellCapture* m_magewell = nullptr;
    NdiCapture*      m_ndi      = nullptr;
};

} // namespace prestige
