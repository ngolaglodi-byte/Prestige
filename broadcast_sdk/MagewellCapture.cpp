// ============================================================
// Prestige AI — Magewell Capture Implementation
// ============================================================

#include "MagewellCapture.h"
#include <QDebug>
#include <QFile>

namespace prestige {

MagewellCapture::MagewellCapture(QObject* parent)
    : QObject(parent)
{
    if (isSdkAvailable()) {
        qInfo() << "[Magewell] SDK detected";
        discoverDevices();
    } else {
        qInfo() << "[Magewell] SDK not installed — download from magewell.com/sdk";
    }
}

MagewellCapture::~MagewellCapture()
{
    close();
}

bool MagewellCapture::isSdkAvailable()
{
#ifdef PRESTIGE_HAVE_MAGEWELL
    return true;
#else
    // Runtime check for Magewell driver
    #ifdef Q_OS_MAC
    return QFile::exists("/Library/Extensions/ProCaptureDriver.kext");
    #elif defined(Q_OS_WIN)
    return QFile::exists("C:/Program Files/Magewell");
    #endif
    return false;
#endif
}

QStringList MagewellCapture::discoverDevices()
{
    m_devices.clear();

#ifdef PRESTIGE_HAVE_MAGEWELL
    MWCaptureInitInstance();

    int deviceCount = MWGetChannelCount();
    for (int i = 0; i < deviceCount; ++i) {
        MWCAP_CHANNEL_INFO info;
        if (MWGetChannelInfoByIndex(i, &info) == MW_SUCCEEDED) {
            QString name = QString("%1 - %2").arg(info.szProductName).arg(info.szBoardSerialNo);
            m_devices.append(name);
            qInfo() << "[Magewell] Found device:" << name;
        }
    }
#endif

    if (m_devices.isEmpty()) {
        qInfo() << "[Magewell] No devices found";
    }

    emit devicesChanged();
    return m_devices;
}

bool MagewellCapture::open(const QString& deviceName, const QSize& resolution, int fps)
{
    m_resolution = resolution;
    m_fps = fps;

#ifdef PRESTIGE_HAVE_MAGEWELL
    // Find device index
    int deviceCount = MWGetChannelCount();
    int deviceIndex = -1;

    for (int i = 0; i < deviceCount; ++i) {
        MWCAP_CHANNEL_INFO info;
        if (MWGetChannelInfoByIndex(i, &info) == MW_SUCCEEDED) {
            QString name = QString("%1 - %2").arg(info.szProductName).arg(info.szBoardSerialNo);
            if (name == deviceName) {
                deviceIndex = i;
                break;
            }
        }
    }

    if (deviceIndex < 0) {
        emit errorOccurred("Magewell device not found: " + deviceName);
        return false;
    }

    WCHAR path[256];
    MWGetDevicePath(deviceIndex, path);
    m_channel = MWOpenChannelByPath(path);

    if (!m_channel) {
        emit errorOccurred("Failed to open Magewell channel");
        return false;
    }

    m_connected = true;
    emit connectionChanged(true);
    qInfo() << "[Magewell] Opened:" << deviceName << resolution << fps << "fps";
    return true;
#else
    Q_UNUSED(deviceName)
    emit errorOccurred("Magewell SDK not compiled — rebuild with -DPRESTIGE_ENABLE_MAGEWELL=ON");
    return false;
#endif
}

void MagewellCapture::close()
{
#ifdef PRESTIGE_HAVE_MAGEWELL
    if (m_channel) {
        MWCloseChannel(m_channel);
        m_channel = nullptr;
    }
    MWCaptureExitInstance();
#endif
    m_connected = false;
    emit connectionChanged(false);
}

} // namespace prestige
