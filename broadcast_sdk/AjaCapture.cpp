// ============================================================
// Prestige AI — AJA NTV2 Capture Implementation
// ============================================================

#include "AjaCapture.h"
#include <QDebug>
#include <QFile>

namespace prestige {

AjaCapture::AjaCapture(QObject* parent)
    : QObject(parent)
{
    if (isSdkAvailable()) {
        qInfo() << "[AJA] SDK detected:" << sdkVersion();
        discoverDevices();
    } else {
        qInfo() << "[AJA] SDK not installed — install from github.com/aja-video/libajantv2";
    }
}

AjaCapture::~AjaCapture()
{
    close();
}

bool AjaCapture::isSdkAvailable()
{
#ifdef PRESTIGE_HAVE_AJA
    return true;
#else
    // Runtime check for AJA driver
    #ifdef Q_OS_MAC
    return QFile::exists("/Library/Extensions/AJA_NTV2.kext") ||
           QFile::exists("/Library/Extensions/AJAVirtualDevice.kext");
    #elif defined(Q_OS_WIN)
    return QFile::exists("C:/Program Files/AJA/NTV2");
    #endif
    return false;
#endif
}

QString AjaCapture::sdkVersion()
{
#ifdef PRESTIGE_HAVE_AJA
    return "AJA NTV2 SDK";
#endif
    return "not installed";
}

QStringList AjaCapture::discoverDevices()
{
    m_devices.clear();

#ifdef PRESTIGE_HAVE_AJA
    CNTV2DeviceScanner scanner;

    for (uint32_t i = 0; i < scanner.GetNumDevices(); ++i) {
        CNTV2Card card;
        if (CNTV2DeviceScanner::GetDeviceAtIndex(i, card)) {
            std::string name;
            card.GetDisplayName(name);
            m_devices.append(QString::fromStdString(name));
            qInfo() << "[AJA] Found device:" << QString::fromStdString(name);
        }
    }
#endif

    if (m_devices.isEmpty()) {
        qInfo() << "[AJA] No devices found";
    }

    emit devicesChanged();
    return m_devices;
}

bool AjaCapture::open(const QString& deviceName, const QSize& resolution, int fps)
{
    m_resolution = resolution;
    m_fps = fps;

#ifdef PRESTIGE_HAVE_AJA
    m_card = new CNTV2Card;

    // Find device by name
    CNTV2DeviceScanner scanner;
    bool found = false;
    for (uint32_t i = 0; i < scanner.GetNumDevices(); ++i) {
        if (CNTV2DeviceScanner::GetDeviceAtIndex(i, *m_card)) {
            std::string name;
            m_card->GetDisplayName(name);
            if (QString::fromStdString(name) == deviceName) {
                found = true;
                break;
            }
        }
    }

    if (!found) {
        emit errorOccurred("AJA device not found: " + deviceName);
        delete m_card;
        m_card = nullptr;
        return false;
    }

    // Configure input
    NTV2VideoFormat videoFormat = NTV2_FORMAT_1080p_2500;
    if (resolution == QSize(3840, 2160)) {
        videoFormat = (fps >= 50) ? NTV2_FORMAT_4x2048x1080p_5000 : NTV2_FORMAT_4x2048x1080p_2500;
    } else if (resolution == QSize(1920, 1080)) {
        videoFormat = (fps >= 50) ? NTV2_FORMAT_1080p_5000 : NTV2_FORMAT_1080p_2500;
    } else if (resolution == QSize(1280, 720)) {
        videoFormat = NTV2_FORMAT_720p_5000;
    }

    m_card->SetVideoFormat(videoFormat);
    m_card->SetMode(NTV2_CHANNEL1, NTV2_MODE_INPUT);

    m_connected = true;
    emit connectionChanged(true);
    qInfo() << "[AJA] Opened:" << deviceName << resolution << fps << "fps";
    return true;
#else
    Q_UNUSED(deviceName)
    emit errorOccurred("AJA SDK not compiled — rebuild with -DPRESTIGE_ENABLE_AJA=ON");
    return false;
#endif
}

void AjaCapture::close()
{
#ifdef PRESTIGE_HAVE_AJA
    if (m_card) {
        delete m_card;
        m_card = nullptr;
    }
#endif
    m_connected = false;
    emit connectionChanged(false);
}

} // namespace prestige
