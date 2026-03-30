#pragma once

// ============================================================
// Prestige AI — Preview Sender
// Sends composited frames to Control Room for live display.
// Runs on ZMQ port 5558.
// ============================================================

#include <QObject>
#include <QImage>
#include <QTimer>

namespace prestige {

class PreviewSender : public QObject {
    Q_OBJECT

public:
    explicit PreviewSender(QObject* parent = nullptr);
    ~PreviewSender() override;

    void start(const QString& address = QStringLiteral("tcp://127.0.0.1:5558"));
    void stop();

    void sendFrame(const QImage& compositedFrame);

private:
    void* m_zmqContext = nullptr;
    void* m_zmqSocket  = nullptr;
    bool  m_connected   = false;
    int   m_framesSent  = 0;
    int   m_skipCounter = 0;
};

} // namespace prestige
