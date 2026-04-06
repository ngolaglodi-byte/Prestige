#pragma once

// ============================================================
// Prestige AI — AJA NTV2 Capture
// Full implementation using AJA NTV2 open-source SDK.
// Auto-detects installed AJA cards and drivers.
//
// SDK: https://github.com/aja-video/libajantv2 (open source)
// macOS: brew install or build from source
// ============================================================

#include <QObject>
#include <QStringList>
#include <QImage>
#include <QSize>
#include <QThread>

#include "FrameMetadata.h"

#ifdef PRESTIGE_HAVE_AJA
#include "ntv2card.h"
#include "ntv2enums.h"
#include "ntv2devicescanner.h"
#endif

namespace prestige {

class AjaCapture : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectionChanged)
    Q_PROPERTY(QStringList availableDevices READ availableDevices NOTIFY devicesChanged)

public:
    explicit AjaCapture(QObject* parent = nullptr);
    ~AjaCapture() override;

    Q_INVOKABLE QStringList discoverDevices();
    QStringList availableDevices() const { return m_devices; }

    Q_INVOKABLE static bool isSdkAvailable();
    Q_INVOKABLE static QString sdkVersion();

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
    QStringList  m_devices;
    bool         m_connected  = false;
    QSize        m_resolution = {1920, 1080};
    int          m_fps        = 25;
    qint64       m_frameId    = 0;
    QTimer*      m_captureTimer = nullptr;
    CaptureStats m_stats;

#ifdef PRESTIGE_HAVE_AJA
    CNTV2Card*   m_card = nullptr;
#endif
};

} // namespace prestige
