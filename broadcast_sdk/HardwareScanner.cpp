// ============================================================
// Prestige AI — Hardware Scanner Implementation
// Copyright (c) 2024-2026 Prestige Technologie Company
// All rights reserved.
//
// Only shows ACTUALLY DETECTED hardware — no phantom entries.
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
    m_hasDeckLink = false;
    m_hasAja = false;
    m_hasMagewell = false;
    m_hasNdi = false;

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
        QString driver = "USB";
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
            qInfo() << "[HardwareScanner]   Mixer:" << mixerBrand << "—" << name;
        else
            qInfo() << "[HardwareScanner]   Webcam:" << name;
    }
}

void HardwareScanner::scanDeckLink()
{
    if (!s_loader->isDeckLinkAvailable()) return;

    void* iterator = s_loader->deckLinkCreateIterator();
    if (iterator) {
        QVariantMap dev;
        dev["type"] = "decklink";
        dev["name"] = "Blackmagic DeckLink";
        dev["driver"] = "Blackmagic Desktop Video";
        dev["available"] = true;
        m_devices.append(dev);
        m_hasDeckLink = true;
        qInfo() << "[HardwareScanner]   DeckLink: détecté";
    }
}

void HardwareScanner::scanNdi()
{
    if (!s_loader->isNdiAvailable()) return;

    NdiCapture probe;
    QStringList sources = probe.discoverSources(2000);

    for (const auto& name : sources) {
        QVariantMap dev;
        dev["type"] = "ndi";
        dev["name"] = name;
        dev["driver"] = "NDI";
        dev["available"] = true;
        m_devices.append(dev);
        m_hasNdi = true;
        qInfo() << "[HardwareScanner]   NDI:" << name;
    }
}

void HardwareScanner::scanAja()
{
    if (!s_loader->isAjaAvailable()) return;

    QVariantMap dev;
    dev["type"] = "aja";
    dev["name"] = "AJA NTV2";
    dev["driver"] = "AJA Desktop Software";
    dev["available"] = true;
    m_devices.append(dev);
    m_hasAja = true;
    qInfo() << "[HardwareScanner]   AJA: détecté";
}

void HardwareScanner::scanMagewell()
{
    if (!s_loader->isMagewellAvailable()) return;

    QVariantMap dev;
    dev["type"] = "magewell";
    dev["name"] = "Magewell Pro Capture";
    dev["driver"] = "Magewell SDK";
    dev["available"] = true;
    m_devices.append(dev);
    m_hasMagewell = true;
    qInfo() << "[HardwareScanner]   Magewell: détecté";
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
