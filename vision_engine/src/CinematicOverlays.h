#pragma once

// ============================================================
// Prestige AI — Cinematic Overlay Animation Engine
// Professional multi-layer sequenced animations for broadcast
//
// Each overlay element animates independently:
//   Layer 1: Accent bar slides in (0.0s - 0.3s)
//   Layer 2: Background expands (0.15s - 0.45s)
//   Layer 3: Light sweep (0.25s - 0.55s)
//   Layer 4: Name text reveals (0.35s - 0.7s)
//   Layer 5: Role fades in (0.5s - 0.85s)
//   Layer 6: Decorative particles (0.7s - continuous)
//
// Quality target: After Effects / Cinema 4D motion graphics
// Copyright (c) 2024-2026 Prestige Technologie Company
// ============================================================

#include <QPainter>
#include <QRectF>
#include <QColor>
#include <QFont>
#include <QString>
#include <QStringList>
#include <vector>
#include <functional>
#include <cmath>

namespace prestige {
namespace cinema {

// ── Animation Layer ──────────────────────────────────────────
struct AnimLayer {
    double delaySec    = 0.0;    // When this layer starts (seconds from overlay trigger)
    double durationSec = 0.3;    // How long the entry animation takes
    double easePower   = 3.0;    // Easing curve exponent (1=linear, 2=quad, 3=cubic)
    bool   hasLoop     = false;  // Continuous animation after entry
    double loopSpeed   = 1.0;    // Loop oscillation speed

    // Draw function: (painter, targetRect, entryProgress 0→1, loopPhase, globalScale, colors)
    using DrawFn = std::function<void(QPainter&, const QRectF&, double, double, double,
                                       const QColor& accent, const QColor& bg,
                                       const QString& text, const QString& subText,
                                       const QFont& font)>;
    DrawFn draw;
};

// ── Animation Preset ─────────────────────────────────────────
struct AnimPreset {
    QString id;
    QString name;
    QString category;  // "nameplate", "title", "logo", "ticker", "scoreboard", etc.
    std::vector<AnimLayer> layers;
    double totalDuration() const {
        double max = 0;
        for (const auto& l : layers)
            max = std::max(max, l.delaySec + l.durationSec);
        return max;
    }
};

// ── Cinematic Renderer ──────────────────────────────────────
class CinematicRenderer {
public:
    static CinematicRenderer& instance();

    // Render an overlay animation at a given time
    void render(QPainter& painter, const QString& category, const QString& presetId,
                const QRectF& rect, double timeSec, double scale,
                const QColor& accent, const QColor& bg,
                const QString& text = "", const QString& subText = "",
                const QFont& font = QFont());

    // Get available presets for a category
    QStringList presetIds(const QString& category) const;
    QStringList presetNames(const QString& category) const;
    const AnimPreset* preset(const QString& category, const QString& id) const;

private:
    CinematicRenderer();
    void registerAll();

    // Registration per overlay type
    void registerNameplatePresets();
    void registerTitlePresets();
    void registerLogoPresets();
    void registerTickerPresets();
    void registerScoreboardPresets();
    void registerWeatherPresets();
    void registerClockPresets();
    void registerSubtitlePresets();
    void registerSocialPresets();

    std::vector<AnimPreset> m_presets;
};

// ── Easing helpers (cinema-grade) ────────────────────────────
double easeOutExpo(double t);
double easeOutBack(double t);
double easeOutElastic(double t);
double easeOutBounce(double t);
double easeInOutCubic(double t);
double easeOutQuart(double t);
double easeOutCirc(double t);
double spring(double t, double tension = 0.3);

} // namespace cinema
} // namespace prestige
