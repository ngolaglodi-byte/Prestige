#pragma once

// ============================================================
// Prestige AI — Design Templates System
// Multi-layer animated designs for each overlay type
// Each design = 5-8 layers with precise timing (After Effects quality)
// ============================================================

#include <QPainter>
#include <QRectF>
#include <QColor>
#include <QFont>
#include <QString>
#include <QImage>
#include <vector>
#include <functional>

namespace prestige {

// ── Animation Layer ───────────────────────────────────────
// Each layer has: what to draw, when to start, how long, what easing
struct AnimLayer {
    double startSec  = 0.0;    // When this layer starts (seconds)
    double durSec    = 0.3;    // Duration of entry animation
    double easePower = 3.0;    // Easing curve power (1=linear, 2=quad, 3=cubic)
    bool   looping   = false;  // Continuous loop after entry
    double loopSpeed = 1.0;    // Loop speed multiplier

    // Draw function: (painter, rect, entryProgress 0→1, loopPhase, scale, accentColor)
    using DrawFunc = std::function<void(QPainter&, const QRectF&, double, double, double, const QColor&)>;
    DrawFunc draw;
};

// ── Design Template ───────────────────────────────────────
struct DesignTemplate {
    QString id;
    QString name;
    QString category; // "nameplate", "title", "channel", "ticker", "scoreboard", etc.
    std::vector<AnimLayer> layers;
};

// ── Design Template Registry ──────────────────────────────
class DesignRegistry {
public:
    static DesignRegistry& instance();

    void registerAll();

    // Get designs by overlay type
    std::vector<DesignTemplate> getDesigns(const QString& category) const;
    const DesignTemplate* getDesign(const QString& category, const QString& designId) const;

    // Get all design IDs for a category
    QStringList getDesignIds(const QString& category) const;
    QStringList getDesignNames(const QString& category) const;

    // Render a design at a given time
    void render(QPainter& painter, const QString& category, const QString& designId,
                const QRectF& rect, double timeSec, double scale,
                const QColor& accentColor, const QString& text = "",
                const QString& subText = "", const QFont& font = QFont(),
                const QColor& textColor = Qt::white);

private:
    DesignRegistry() { registerAll(); }
    void addDesign(DesignTemplate&& d);

    // Register designs per category
    void registerNameplateDesigns();
    void registerTitleDesigns();
    void registerChannelDesigns();
    void registerTickerDesigns();
    void registerScoreboardDesigns();
    void registerClockDesigns();
    void registerWeatherDesigns();

    std::vector<DesignTemplate> m_designs;
};

} // namespace prestige
