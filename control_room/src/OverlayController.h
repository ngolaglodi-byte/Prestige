#pragma once

// ============================================================
// Prestige AI — Overlay Controller
// Manages overlay activation/deactivation from the Control Room
// ============================================================

#include <QObject>
#include <QStringList>
#include <QVariantMap>
#include <QList>

namespace prestige {

struct OverlayState {
    QString id;
    QString type;       // "nameplate", "lowerthird", "ticker"
    bool    enabled = true;
    QString style   = "default";
};

class OverlayController : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool globalVisible READ globalVisible WRITE setGlobalVisible NOTIFY globalVisibleChanged)
    Q_PROPERTY(bool nameplatesEnabled READ nameplatesEnabled WRITE setNameplatesEnabled NOTIFY nameplatesEnabledChanged)
    Q_PROPERTY(bool lowerThirdsEnabled READ lowerThirdsEnabled WRITE setLowerThirdsEnabled NOTIFY lowerThirdsEnabledChanged)
    Q_PROPERTY(bool tickerEnabled READ tickerEnabled WRITE setTickerEnabled NOTIFY tickerEnabledChanged)

public:
    explicit OverlayController(QObject* parent = nullptr);

    bool globalVisible() const { return m_globalVisible; }
    void setGlobalVisible(bool visible);

    bool nameplatesEnabled() const { return m_nameplates; }
    void setNameplatesEnabled(bool enabled);

    bool lowerThirdsEnabled() const { return m_lowerThirds; }
    void setLowerThirdsEnabled(bool enabled);

    bool tickerEnabled() const { return m_ticker; }
    void setTickerEnabled(bool enabled);

    Q_INVOKABLE void setOverlayStyle(const QString& type, const QString& style);

signals:
    void globalVisibleChanged(bool visible);
    void nameplatesEnabledChanged(bool enabled);
    void lowerThirdsEnabledChanged(bool enabled);
    void tickerEnabledChanged(bool enabled);
    void styleChanged(const QString& type, const QString& style);

private:
    bool m_globalVisible = true;
    bool m_nameplates    = true;
    bool m_lowerThirds   = true;
    bool m_ticker        = false;
};

} // namespace prestige
