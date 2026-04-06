// ============================================================
// Prestige AI — AJA NTV2 Capture Implementation
// ============================================================

#include "AjaCapture.h"
#include <QDebug>
#include <QTimer>
#include <QDateTime>
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

    // Enable audio input (48kHz 16-bit stereo — SMPTE 299M)
    m_card->SetAudioSystemInputSource(NTV2_AUDIOSYSTEM_1, NTV2_AUDIO_EMBEDDED, NTV2_EMBEDDED_AUDIO_INPUT_VIDEO_1);

    m_connected = true;
    emit connectionChanged(true);

    // Start capture thread
    m_captureTimer = new QTimer(this);
    m_captureTimer->setInterval(1000 / qMax(1, fps));
    connect(m_captureTimer, &QTimer::timeout, this, [this]() {
        if (!m_card || !m_connected) return;

        // Read frame from DMA
        ULWord* pBuffer = nullptr;
        ULWord bufferSize = 0;
        m_card->DMAReadFrame(0, pBuffer, bufferSize);

        if (pBuffer && bufferSize > 0) {
            // Convert NTV2 frame to QImage (UYVY → RGB32)
            QImage frame(m_resolution, QImage::Format_RGB32);
            // Simple UYVY to RGB conversion
            const uint8_t* src = reinterpret_cast<const uint8_t*>(pBuffer);
            for (int y = 0; y < m_resolution.height(); ++y) {
                QRgb* dst = reinterpret_cast<QRgb*>(frame.scanLine(y));
                for (int x = 0; x < m_resolution.width(); x += 2) {
                    int idx = (y * m_resolution.width() + x) * 2;
                    int u  = src[idx] - 128;
                    int y0 = src[idx+1];
                    int v  = src[idx+2] - 128;
                    int y1 = src[idx+3];
                    dst[x]   = qRgb(qBound(0, y0 + 1.402*v, 255), qBound(0, y0 - 0.344*u - 0.714*v, 255), qBound(0, y0 + 1.772*u, 255));
                    dst[x+1] = qRgb(qBound(0, y1 + 1.402*v, 255), qBound(0, y1 - 0.344*u - 0.714*v, 255), qBound(0, y1 + 1.772*u, 255));
                }
            }
            m_frameId++;
            emit frameReady(frame, QDateTime::currentMSecsSinceEpoch());
        }
    });
    m_captureTimer->start();

    qInfo() << "[AJA] Opened:" << deviceName << resolution << fps << "fps — capture started";
    return true;
#else
    Q_UNUSED(deviceName)
    emit errorOccurred("AJA SDK not compiled — rebuild with -DPRESTIGE_ENABLE_AJA=ON");
    return false;
#endif
}

void AjaCapture::close()
{
    if (m_captureTimer) {
        m_captureTimer->stop();
        delete m_captureTimer;
        m_captureTimer = nullptr;
    }
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
