#pragma once

// ============================================================
// Prestige AI — Talent Data Structures
// Thread-safe talent state used by the Vision Engine
// ============================================================

#include <QString>
#include <QList>
#include <QMutex>
#include <QMutexLocker>
#include <QRectF>

namespace prestige {

struct TalentOverlay {
    QString id;
    QString name;
    QString role;
    double  confidence  = 0.0;
    QRectF  bbox;               // Normalized or pixel coordinates
    bool    showOverlay = true;
    QString overlayStyle = QStringLiteral("default");
};

// Thread-safe container for the latest detection results
class TalentStore {
public:
    void update(const QList<TalentOverlay>& newTalents) {
        QMutexLocker lock(&m_mutex);
        m_talents = newTalents;
    }

    QList<TalentOverlay> snapshot() const {
        QMutexLocker lock(&m_mutex);
        return m_talents;
    }

    void clear() {
        QMutexLocker lock(&m_mutex);
        m_talents.clear();
    }

private:
    mutable QMutex m_mutex;
    QList<TalentOverlay> m_talents;
};

} // namespace prestige
