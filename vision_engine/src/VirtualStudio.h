#pragma once

// ============================================================
// Prestige AI — Virtual Studio
// Chroma Key + Studio Backgrounds + Animated Elements
// Broadcast-standard virtual set system.
// ============================================================

#include <QImage>
#include <QPainter>
#include <QSize>
#include <QString>
#include <cmath>

namespace prestige {

class VirtualStudio {
public:
    VirtualStudio() = default;

    // Main entry point — returns processed frame (or passthrough if disabled)
    QImage process(const QImage& rawFrame);

    // Setters
    void setEnabled(bool v) { m_enabled = v; }
    void setStudioId(int id) { if (m_studioId != id) { m_studioId = id; m_bgDirty = true; } }
    void setChromaKeyEnabled(bool v) { m_chromaKeyEnabled = v; }
    void setChromaKeyColor(const QString& c) { m_chromaKeyColor = c; }
    void setChromaKeyTolerance(double v) { m_tolerance = v; }
    void setChromaKeySmooth(double v) { m_smooth = v; }

private:
    QImage applyChromaKey(const QImage& frame);
    QImage renderStudioBackground(const QSize& size);
    void renderAnimatedElements(QPainter& painter, const QSize& size);
    QImage compositeTalentOnStudio(const QImage& studio, const QImage& keyedTalent);

    bool    m_enabled         = false;
    int     m_studioId        = 0;
    bool    m_chromaKeyEnabled = false;
    QString m_chromaKeyColor  = "green";
    double  m_tolerance       = 0.35;
    double  m_smooth          = 0.05;

    int     m_frameCounter    = 0;
    bool    m_bgDirty         = true;
    QImage  m_cachedBg;
};

} // namespace prestige
