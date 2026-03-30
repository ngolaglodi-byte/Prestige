#pragma once

// ============================================================
// Prestige AI — Frame Sender
// Sends captured video frames to Python AI Engine via ZMQ
// C++ is the master of the video pipeline.
// ============================================================

#include <QObject>
#include <QImage>
#include <QSize>

namespace prestige {

class FrameSender : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)
    Q_PROPERTY(int framesSent READ framesSent NOTIFY statsUpdated)

public:
    explicit FrameSender(QObject* parent = nullptr);
    ~FrameSender() override;

    void start(const QString& address = QStringLiteral("tcp://127.0.0.1:5557"));
    void stop();

    bool isConnected() const { return m_connected; }
    int  framesSent() const { return m_framesSent; }

    // Send a frame to Python for analysis
    // Returns immediately (non-blocking). Drops frame if Python is busy.
    void sendFrame(const QImage& frame, qint64 frameId, qint64 captureTimestampMs);

signals:
    void connectedChanged(bool connected);
    void statsUpdated();

private:
    void* m_zmqContext = nullptr;
    void* m_zmqSocket  = nullptr;
    bool  m_connected   = false;
    int   m_framesSent  = 0;
    int   m_framesDropped = 0;
    int   m_jpegQuality = 70;   // Balance speed vs quality for AI analysis
    QSize m_sendResolution = {640, 480}; // Downscale for AI (not full res)
};

} // namespace prestige
