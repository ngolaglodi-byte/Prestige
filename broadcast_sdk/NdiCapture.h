#pragma once

// ============================================================
// Prestige AI — NDI Capture (SDK 6)
// Full NDI source discovery, connection, and frame capture
// ============================================================

#include <QObject>
#include <QThread>
#include <QStringList>
#include <QSize>
#include <QImage>

#include "FrameMetadata.h"

#ifdef PRESTIGE_HAVE_NDI
#include <Processing.NDI.Lib.h>
#endif

namespace prestige {

class NdiCapture : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectionChanged)
    Q_PROPERTY(QStringList availableSources READ availableSources NOTIFY sourcesChanged)

public:
    explicit NdiCapture(QObject* parent = nullptr);
    ~NdiCapture() override;

    // Discovery
    Q_INVOKABLE QStringList discoverSources(int timeoutMs = 3000);
    QStringList availableSources() const { return m_availableSources; }
    Q_INVOKABLE void refreshSources();

    // Connection
    Q_INVOKABLE bool connectSource(const QString& sourceName);
    Q_INVOKABLE bool disconnect();
    bool isConnected() const { return m_connected; }

    // Info
    QSize resolution() const { return m_resolution; }
    int fps() const { return m_fps; }
    CaptureStats stats() const { return m_stats; }

    // Frame capture control
    Q_INVOKABLE void startCapture();
    Q_INVOKABLE void stopCapture();

signals:
    void connectionChanged(bool connected);
    void connectionLost();
    void sourcesChanged();
    void errorOccurred(const QString& error);
    void frameReady(const QImage& frame);
    void statsUpdated(double fps, double latencyMs);

private:
    void captureLoop();

    bool         m_connected    = false;
    bool         m_capturing    = false;
    QSize        m_resolution   = {1920, 1080};
    int          m_fps          = 25;
    CaptureStats m_stats;
    QStringList  m_availableSources;
    QThread      m_captureThread;

#ifdef PRESTIGE_HAVE_NDI
    NDIlib_find_instance_t m_findInstance = nullptr;
    NDIlib_recv_instance_t m_recvInstance = nullptr;
#endif
};

} // namespace prestige
