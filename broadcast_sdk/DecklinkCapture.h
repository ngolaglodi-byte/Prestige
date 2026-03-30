#pragma once

// ============================================================
// Prestige AI — Blackmagic DeckLink Capture
// Full implementation using DeckLink SDK.
// Auto-detects installed DeckLink cards and drivers.
//
// Install SDK: download from blackmagicdesign.com/developer
// macOS: installs to /Library/Frameworks/DeckLinkAPI.framework
// Windows: installs COM interfaces
// ============================================================

#include <QObject>
#include <QStringList>
#include <QImage>
#include <QSize>
#include <QMutex>
#include <QThread>

#include "FrameMetadata.h"

#ifdef PRESTIGE_HAVE_DECKLINK
// DeckLink SDK headers (installed by Blackmagic Desktop Video)
#ifdef Q_OS_MAC
#include <DeckLinkAPI.h>
#elif defined(Q_OS_WIN)
#include <comutil.h>
#include "DeckLinkAPI_i.h"
#endif
#endif

namespace prestige {

class DecklinkCapture : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectionChanged)
    Q_PROPERTY(QStringList availableDevices READ availableDevices NOTIFY devicesChanged)

public:
    explicit DecklinkCapture(QObject* parent = nullptr);
    ~DecklinkCapture() override;

    // Auto-detect all DeckLink cards in the system
    Q_INVOKABLE QStringList discoverDevices();
    QStringList availableDevices() const { return m_devices; }

    // Check if DeckLink SDK/driver is installed
    Q_INVOKABLE static bool isSdkAvailable();
    Q_INVOKABLE static QString sdkVersion();

    // Connection
    Q_INVOKABLE bool open(const QString& deviceName, const QSize& resolution = {1920, 1080}, int fps = 25);
    Q_INVOKABLE void close();
    bool isConnected() const { return m_connected; }
    CaptureStats stats() const { return m_stats; }

signals:
    void connectionChanged(bool connected);
    void devicesChanged();
    void connectionLost();
    void frameReady(const QImage& frame, qint64 timestamp);
    void errorOccurred(const QString& error);

private:
    void captureLoop();

    QStringList  m_devices;
    bool         m_connected  = false;
    bool         m_capturing  = false;
    QSize        m_resolution = {1920, 1080};
    int          m_fps        = 25;
    CaptureStats m_stats;
    QThread      m_captureThread;

#ifdef PRESTIGE_HAVE_DECKLINK
#ifdef Q_OS_MAC
    IDeckLink*          m_deckLink      = nullptr;
    IDeckLinkInput*     m_deckLinkInput = nullptr;
#endif
#endif
};

} // namespace prestige
