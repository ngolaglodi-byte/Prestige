// ============================================================
// Prestige AI — Overlay Controller Implementation
// ============================================================

#include "OverlayController.h"
#include <QDebug>

namespace prestige {

OverlayController::OverlayController(QObject* parent)
    : QObject(parent)
{
}

void OverlayController::setGlobalVisible(bool visible)
{
    if (m_globalVisible == visible) return;
    m_globalVisible = visible;
    emit globalVisibleChanged(visible);
    qInfo() << "[OverlayController] Global visible:" << visible;
}

void OverlayController::setNameplatesEnabled(bool enabled)
{
    if (m_nameplates == enabled) return;
    m_nameplates = enabled;
    emit nameplatesEnabledChanged(enabled);
    qInfo() << "[OverlayController] Nameplates:" << enabled;
}

void OverlayController::setLowerThirdsEnabled(bool enabled)
{
    if (m_lowerThirds == enabled) return;
    m_lowerThirds = enabled;
    emit lowerThirdsEnabledChanged(enabled);
    qInfo() << "[OverlayController] Lower thirds:" << enabled;
}

void OverlayController::setTickerEnabled(bool enabled)
{
    if (m_ticker == enabled) return;
    m_ticker = enabled;
    emit tickerEnabledChanged(enabled);
    qInfo() << "[OverlayController] Ticker:" << enabled;
}

void OverlayController::setOverlayStyle(const QString& type, const QString& style)
{
    emit styleChanged(type, style);
    qInfo() << "[OverlayController] Style for" << type << "→" << style;
}

} // namespace prestige
