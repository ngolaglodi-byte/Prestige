// ============================================================
// Prestige AI — Hardware Scanner Implementation
// Uses RuntimeLoader for detection — no #ifdef guards needed.
// Works the same on macOS and Windows.
// ============================================================

#include "HardwareScanner.h"
#include "RuntimeLoader.h"
#include "NdiCapture.h"

#include <QCameraDevice>
#include <QMediaDevices>
#include <QVariantMap>
#include <QDebug>

namespace prestige {

static RuntimeLoader* s_loader = nullptr;

HardwareScanner::HardwareScanner(QObject* parent)
    : QObject(parent)
{
    if (!s_loader) {
        s_loader = new RuntimeLoader(this);
        s_loader->probeAll();
    }
}

void HardwareScanner::scan()
{
    m_scanning = true;
    emit scanningChanged();
    m_devices.clear();

    qInfo() << "[HardwareScanner] Scanning all broadcast hardware...";

    scanWebcams();
    scanDeckLink();
    scanNdi();
    scanAja();
    scanMagewell();

    m_scanning = false;
    emit scanningChanged();
    emit scanComplete();

    int available = 0;
    for (const auto& d : m_devices) {
        if (d.toMap()["available"].toBool()) available++;
    }
    qInfo() << "[HardwareScanner] Scan complete:" << m_devices.size() << "entries," << available << "available";
}

void HardwareScanner::scanWebcams()
{
    auto cameras = QMediaDevices::videoInputs();
    for (const auto& cam : cameras) {
        QVariantMap dev;
        QString name = cam.description();
        QString devType = "webcam";
        QString driver = "Système";
        QString mixerBrand;

        // Detect mixer brands from USB device name
        if (name.contains("Blackmagic", Qt::CaseInsensitive) || name.contains("ATEM", Qt::CaseInsensitive)) {
            devType = "mixer_usb";
            mixerBrand = "Blackmagic ATEM";
            driver = "ATEM USB (UVC)";
        } else if (name.contains("Roland", Qt::CaseInsensitive) || name.contains("VR-", Qt::CaseInsensitive) || name.contains("V-8HD", Qt::CaseInsensitive)) {
            devType = "mixer_usb";
            mixerBrand = "Roland";
            driver = "Roland USB (UVC)";
        } else if (name.contains("Panasonic", Qt::CaseInsensitive) || name.contains("AV-HS", Qt::CaseInsensitive)) {
            devType = "mixer_usb";
            mixerBrand = "Panasonic";
            driver = "Panasonic USB";
        } else if (name.contains("Elgato", Qt::CaseInsensitive) || name.contains("Cam Link", Qt::CaseInsensitive) || name.contains("HD60", Qt::CaseInsensitive)) {
            devType = "capture_card";
            mixerBrand = "Elgato";
            driver = "Elgato Capture";
        } else if (name.contains("Magewell", Qt::CaseInsensitive) || name.contains("USB Capture", Qt::CaseInsensitive)) {
            devType = "capture_card";
            mixerBrand = "Magewell USB";
            driver = "Magewell USB Capture";
        } else if (name.contains("AVerMedia", Qt::CaseInsensitive)) {
            devType = "capture_card";
            mixerBrand = "AVerMedia";
            driver = "AVerMedia Capture";
        }

        dev["type"] = devType;
        dev["name"] = name;
        dev["driver"] = driver;
        dev["available"] = true;
        dev["id"] = cam.id();
        dev["mixerBrand"] = mixerBrand;
        m_devices.append(dev);

        if (!mixerBrand.isEmpty())
            qInfo() << "[HardwareScanner]   Mixer détecté:" << mixerBrand << "—" << name;
        else
            qInfo() << "[HardwareScanner]   Webcam:" << name;
    }
}

void HardwareScanner::scanDeckLink()
{
    bool loaded = s_loader->isDeckLinkAvailable();

    if (loaded) {
        // SDK is loaded — enumerate cards via the runtime-loaded function
        void* iterator = s_loader->deckLinkCreateIterator();
        if (iterator) {
            // In production this would iterate IDeckLink instances
            // For now we report that the SDK is available and cards can be probed
            QVariantMap dev;
            dev["type"] = "decklink";
            dev["name"] = "Blackmagic DeckLink (SDK chargé)";
            dev["driver"] = "Blackmagic Desktop Video";
            dev["available"] = true;
            m_devices.append(dev);
            m_hasDeckLink = true;
            qInfo() << "[HardwareScanner]   DeckLink: SDK chargé, cartes détectables";
        }
    } else {
        QVariantMap dev;
        dev["type"] = "decklink";
        dev["name"] = "Blackmagic DeckLink";
        dev["driver"] = "Non installé — installez Blackmagic Desktop Video";
        dev["available"] = false;
        m_devices.append(dev);
    }
}

void HardwareScanner::scanNdi()
{
    bool loaded = s_loader->isNdiAvailable();

    if (loaded) {
        // NDI SDK loaded — scan network for sources
        NdiCapture probe;
        QStringList sources = probe.discoverSources(2000);

        if (!sources.isEmpty()) {
            for (const auto& name : sources) {
                QVariantMap dev;
                dev["type"] = "ndi";
                dev["name"] = name;
                dev["driver"] = "NDI SDK";
                dev["available"] = true;
                m_devices.append(dev);
                m_hasNdi = true;
                qInfo() << "[HardwareScanner]   NDI:" << name;
            }
        } else {
            QVariantMap dev;
            dev["type"] = "ndi";
            dev["name"] = "NDI (aucune source sur le réseau)";
            dev["driver"] = "NDI SDK chargé";
            dev["available"] = false;
            m_devices.append(dev);
        }
    } else {
        QVariantMap dev;
        dev["type"] = "ndi";
        dev["name"] = "NDI";
        dev["driver"] = "Non installé — brew install libndi (macOS) ou NDI Tools (Windows)";
        dev["available"] = false;
        m_devices.append(dev);
    }
}

void HardwareScanner::scanAja()
{
    bool driverPresent = s_loader->isAjaAvailable();

    QVariantMap dev;
    dev["type"] = "aja";
    if (driverPresent) {
        dev["name"] = "AJA NTV2 (pilote détecté)";
        dev["driver"] = "AJA Desktop Software";
        dev["available"] = true;
        m_hasAja = true;
        qInfo() << "[HardwareScanner]   AJA: pilote détecté";
    } else {
        dev["name"] = "AJA NTV2";
        dev["driver"] = "Non installé — installez AJA Desktop Software depuis aja.com";
        dev["available"] = false;
    }
    m_devices.append(dev);
}

void HardwareScanner::scanMagewell()
{
    bool loaded = s_loader->isMagewellAvailable();

    QVariantMap dev;
    dev["type"] = "magewell";
    if (loaded) {
        dev["name"] = "Magewell Pro Capture (SDK chargé)";
        dev["driver"] = "Magewell Pro Capture";
        dev["available"] = true;
        m_hasMagewell = true;
        qInfo() << "[HardwareScanner]   Magewell: SDK chargé";
    } else {
        dev["name"] = "Magewell Pro Capture";
        dev["driver"] = "Non installé — téléchargez depuis magewell.com/sdk";
        dev["available"] = false;
    }
    m_devices.append(dev);
}

bool HardwareScanner::hasDeckLink() const { return m_hasDeckLink; }
bool HardwareScanner::hasAja() const { return m_hasAja; }
bool HardwareScanner::hasMagewell() const { return m_hasMagewell; }
bool HardwareScanner::hasNdi() const { return m_hasNdi; }

QVariantList HardwareScanner::sdkStatus() const
{
    return s_loader ? s_loader->sdkStatus() : QVariantList();
}

} // namespace prestige
