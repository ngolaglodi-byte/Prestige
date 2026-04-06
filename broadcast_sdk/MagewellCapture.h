#pragma once

// ============================================================
// Prestige AI — Magewell Capture
// Full implementation using Magewell Pro Capture SDK.
// Auto-detects USB/PCIe Magewell capture devices.
//
// SDK: https://www.magewell.com/sdk
// ============================================================

#include <QObject>
#include <QStringList>
#include <QImage>
#include <QSize>
#include <QThread>

#include "FrameMetadata.h"

#ifdef PRESTIGE_HAVE_MAGEWELL
#include "LibMWCapture/MWCapture.h"
#endif

namespace prestige {

class MagewellCapture : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectionChanged)
    Q_PROPERTY(QStringList availableDevices READ availableDevices NOTIFY devicesChanged)

public:
    explicit MagewellCapture(QObject* parent = nullptr);
    ~MagewellCapture() override;

    Q_INVOKABLE QStringList discoverDevices();
    QStringList availableDevices() const { return m_devices; }

    Q_INVOKABLE static bool isSdkAvailable();

    Q_INVOKABLE bool open(const QString& deviceName, const QSize& resolution = {1920, 1080}, int fps = 25);
    Q_INVOKABLE void close();
    bool isConnected() const { return m_connected; }
    CaptureStats stats() const { return m_stats; }

signals:
    void connectionChanged(bool connected);
    void devicesChanged();
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

#ifdef PRESTIGE_HAVE_MAGEWELL
    HCHANNEL     m_channel = nullptr;
#endif
};

} // namespace prestige
