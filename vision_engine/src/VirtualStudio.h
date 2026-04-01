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

    // Customization — colors
    void setPrimaryColor(const QColor& c) { if (m_primaryColor != c) { m_primaryColor = c; m_bgDirty = true; } }
    void setSecondaryColor(const QColor& c) { if (m_secondaryColor != c) { m_secondaryColor = c; m_bgDirty = true; } }
    void setAccentColor(const QColor& c) { if (m_accentColor != c) { m_accentColor = c; m_bgDirty = true; } }
    void setFloorColor(const QColor& c) { if (m_floorColor != c) { m_floorColor = c; m_bgDirty = true; } }
    void setLightColor(const QColor& c) { m_lightColor = c; }
    void setLightIntensity(double v) { m_lightIntensity = v; }
    void setAnimationsEnabled(bool v) { m_animEnabled = v; }

    // Custom background image (overrides procedural studio)
    void setCustomBackgroundPath(const QString& path);
    bool hasCustomBackground() const { return !m_customBg.isNull(); }

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

    // Custom colors (when default = invalid, use template colors)
    QColor  m_primaryColor;      // Main background gradient
    QColor  m_secondaryColor;    // Secondary gradient / accent areas
    QColor  m_accentColor;       // Glow lines, particles, LED
    QColor  m_floorColor;        // Floor area
    QColor  m_lightColor = QColor(255, 255, 255);
    double  m_lightIntensity = 1.0;
    bool    m_animEnabled = true;

    // Custom background image (imported by user)
    QImage  m_customBg;
    QString m_customBgPath;

    int     m_frameCounter    = 0;
    bool    m_bgDirty         = true;
    QImage  m_cachedBg;
};

} // namespace prestige
