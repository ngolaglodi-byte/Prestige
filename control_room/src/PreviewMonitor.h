#pragma once

// ============================================================
// Prestige AI — Preview Monitor
// Receives composited frames from Vision Engine via ZMQ :5558
// and provides them to QML as a live updating image.
// ============================================================

#include <QObject>
#include <QImage>
#include <QThread>
#include <QMutex>
#include <QTimer>

namespace prestige {

class PreviewMonitor : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool active READ isActive NOTIFY activeChanged)
    Q_PROPERTY(int sourceWidth READ sourceWidth NOTIFY resolutionChanged)
    Q_PROPERTY(int sourceHeight READ sourceHeight NOTIFY resolutionChanged)
    Q_PROPERTY(double fps READ fps NOTIFY fpsChanged)
    Q_PROPERTY(QString imageUrl READ imageUrl NOTIFY frameUpdated)

public:
    explicit PreviewMonitor(QObject* parent = nullptr);
    ~PreviewMonitor() override;

    bool isActive() const { return m_active; }
    int sourceWidth() const { return m_width; }
    int sourceHeight() const { return m_height; }
    double fps() const { return m_fps; }
    QString imageUrl() const { return m_imageUrl; }

    // Get latest frame (for QQuickImageProvider)
    QImage latestFrame() const;

    Q_INVOKABLE void start(const QString& address = QStringLiteral("tcp://127.0.0.1:5558"));
    Q_INVOKABLE void stop();

signals:
    void activeChanged();
    void resolutionChanged();
    void fpsChanged();
    void frameUpdated();

private:
    void receiveLoop(const QString& address);

    QThread m_thread;
    bool    m_active  = false;
    bool    m_running = false;
    int     m_width   = 0;
    int     m_height  = 0;
    double  m_fps     = 0.0;
    QString m_imageUrl;

    mutable QMutex m_frameMutex;
    QImage  m_latestFrame;
    int     m_frameCount = 0;
};

} // namespace prestige
