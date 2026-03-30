#pragma once

// ============================================================
// Prestige AI — Hardware Scanner
// Auto-detects all installed broadcast hardware and SDKs.
// Runs at startup, populates the source selection UI.
// ============================================================

#include <QObject>
#include <QStringList>
#include <QVariantList>

namespace prestige {

struct DetectedDevice {
    QString type;       // "decklink", "aja", "magewell", "ndi", "webcam"
    QString name;       // "DeckLink 4K Extreme", "AJA Corvid 44", etc.
    QString driver;     // "DeckLink SDK 12.5", "AJA NTV2", etc.
    bool    available;  // true if SDK + driver + hardware all present
};

class HardwareScanner : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList devices READ devices NOTIFY scanComplete)
    Q_PROPERTY(bool scanning READ isScanning NOTIFY scanningChanged)

public:
    explicit HardwareScanner(QObject* parent = nullptr);

    // Scan all hardware — call once at startup
    Q_INVOKABLE void scan();

    QVariantList devices() const { return m_devices; }
    bool isScanning() const { return m_scanning; }

    // Quick checks
    Q_INVOKABLE bool hasDeckLink() const;
    Q_INVOKABLE bool hasAja() const;
    Q_INVOKABLE bool hasMagewell() const;
    Q_INVOKABLE bool hasNdi() const;

    // SDK status
    Q_INVOKABLE QVariantList sdkStatus() const;

signals:
    void scanComplete();
    void scanningChanged();

private:
    void scanDeckLink();
    void scanAja();
    void scanMagewell();
    void scanNdi();
    void scanWebcams();

    QVariantList m_devices;
    bool m_scanning = false;
    bool m_hasDeckLink = false;
    bool m_hasAja = false;
    bool m_hasMagewell = false;
    bool m_hasNdi = false;
};

} // namespace prestige
