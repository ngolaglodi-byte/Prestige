// ============================================================
// Prestige AI — Design Templates (After Effects quality)
// Multi-layer animated designs for every overlay
// ============================================================

#include "DesignTemplates.h"
#include <QLinearGradient>
#include <QRadialGradient>
#include <QPainterPath>
#include <QPen>
#include <QFontMetrics>
#include <cmath>
#include <algorithm>

namespace prestige {

static double ease(double t, double power) { return 1.0 - std::pow(1.0 - std::clamp(t, 0.0, 1.0), power); }
static double easeBack(double t) { double s = 1.70158; double t1 = std::clamp(t, 0.0, 1.0); return t1 * t1 * ((s + 1) * t1 - s); }

DesignRegistry& DesignRegistry::instance() {
    static DesignRegistry reg;
    return reg;
}

void DesignRegistry::addDesign(DesignTemplate&& d) { m_designs.push_back(std::move(d)); }

std::vector<DesignTemplate> DesignRegistry::getDesigns(const QString& category) const {
    std::vector<DesignTemplate> result;
    for (const auto& d : m_designs)
        if (d.category == category) result.push_back(d);
    return result;
}

const DesignTemplate* DesignRegistry::getDesign(const QString& category, const QString& designId) const {
    for (const auto& d : m_designs)
        if (d.category == category && d.id == designId) return &d;
    return nullptr;
}

QStringList DesignRegistry::getDesignIds(const QString& category) const {
    QStringList ids;
    for (const auto& d : m_designs)
        if (d.category == category) ids.append(d.id);
    return ids;
}

QStringList DesignRegistry::getDesignNames(const QString& category) const {
    QStringList names;
    for (const auto& d : m_designs)
        if (d.category == category) names.append(d.name);
    return names;
}

void DesignRegistry::render(QPainter& painter, const QString& category, const QString& designId,
                             const QRectF& rect, double timeSec, double scale,
                             const QColor& accentColor, const QString& text,
                             const QString& subText, const QFont& font)
{
    auto* design = getDesign(category, designId);
    if (!design) return;

    for (const auto& layer : design->layers) {
        double elapsed = timeSec - layer.startSec;
        if (elapsed < 0) continue; // Layer hasn't started yet

        double entryProg = std::clamp(elapsed / layer.durSec, 0.0, 1.0);
        entryProg = ease(entryProg, layer.easePower);

        double loopPhase = 0;
        if (layer.looping && entryProg >= 1.0) {
            double loopTime = timeSec - layer.startSec - layer.durSec;
            loopPhase = std::sin(loopTime * layer.loopSpeed * 2.0);
        }

        if (layer.draw) {
            painter.save();
            layer.draw(painter, rect, entryProg, loopPhase, scale, accentColor);
            painter.restore();
        }
    }

    // Draw text on top (always last layer)
    if (!text.isEmpty()) {
        QFont f = font.family().isEmpty() ? QFont("Helvetica Neue", static_cast<int>(16 * scale), QFont::Bold) : font;
        painter.setFont(f);
        painter.setPen(Qt::white);
        QRectF textRect = rect.adjusted(rect.width() * 0.05, 0, -rect.width() * 0.05, subText.isEmpty() ? 0 : -rect.height() * 0.3);
        painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, text);
        if (!subText.isEmpty()) {
            QFont sf(f.family(), static_cast<int>(f.pointSize() * 0.75));
            painter.setFont(sf);
            painter.setPen(QColor(255, 255, 255, 180));
            QRectF subRect(rect.x() + rect.width() * 0.05, rect.y() + rect.height() * 0.55, rect.width() * 0.9, rect.height() * 0.4);
            painter.drawText(subRect, Qt::AlignVCenter | Qt::AlignLeft, subText);
        }
    }
}

void DesignRegistry::registerAll() {
    registerNameplateDesigns();
    registerTitleDesigns();
    registerChannelDesigns();
    registerTickerDesigns();
    registerScoreboardDesigns();
    registerClockDesigns();
    registerWeatherDesigns();
}

// ══════════════════════════════════════════════════════════════
// NAMEPLATE DESIGNS (30)
// ══════════════════════════════════════════════════════════════

void DesignRegistry::registerNameplateDesigns() {
    // ── 1. Broadcast News (BFM style) ─────────────────────
    {
        DesignTemplate d; d.id = "np_broadcast_news"; d.name = "Broadcast News"; d.category = "nameplate";
        // Layer 1: Dark background wipe from left
        d.layers.push_back({0.0, 0.4, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor&) {
            double w = r.width() * prog;
            QLinearGradient g(r.x(), 0, r.x() + w, 0);
            g.setColorAt(0, QColor(10, 10, 15, 220));
            g.setColorAt(0.95, QColor(10, 10, 15, 220));
            g.setColorAt(1, QColor(10, 10, 15, 0));
            p.setPen(Qt::NoPen); p.setBrush(g);
            p.drawRoundedRect(QRectF(r.x(), r.y(), w, r.height()), 4, 4);
        }});
        // Layer 2: Accent bar slides in
        d.layers.push_back({0.1, 0.3, 4.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor& ac) {
            double h = r.height() * prog;
            p.setPen(Qt::NoPen); p.setBrush(ac);
            p.drawRoundedRect(QRectF(r.x(), r.y() + (r.height() - h) / 2, 3 * s, h), 2, 2);
        }});
        // Layer 3: Bottom accent line draws
        d.layers.push_back({0.25, 0.5, 2.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor& ac) {
            double w = r.width() * prog;
            p.setPen(QPen(ac, 2 * s)); p.drawLine(QPointF(r.x(), r.bottom()), QPointF(r.x() + w, r.bottom()));
        }});
        // Layer 4: Glow pulse on accent bar (loop)
        d.layers.push_back({0.6, 0.1, 1.0, true, 1.5, [](QPainter& p, const QRectF& r, double, double loop, double s, const QColor& ac) {
            int alpha = static_cast<int>(20 + 15 * loop);
            p.setPen(Qt::NoPen); p.setBrush(QColor(ac.red(), ac.green(), ac.blue(), alpha));
            p.drawRoundedRect(QRectF(r.x() - 2, r.y() - 2, 7 * s, r.height() + 4), 3, 3);
        }});
        addDesign(std::move(d));
    }

    // ── 2. Elegant Minimal ────────────────────────────────
    {
        DesignTemplate d; d.id = "np_elegant_minimal"; d.name = "Elegant Minimal"; d.category = "nameplate";
        // Layer 1: Thin line draws from center
        d.layers.push_back({0.0, 0.5, 2.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor& ac) {
            double cx = r.center().x();
            double halfW = r.width() * 0.4 * prog;
            p.setPen(QPen(ac, 1.5 * s, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(QPointF(cx - halfW, r.bottom()), QPointF(cx + halfW, r.bottom()));
        }});
        // Layer 2: Background fades in
        d.layers.push_back({0.2, 0.4, 2.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor&) {
            p.setPen(Qt::NoPen); p.setBrush(QColor(0, 0, 0, static_cast<int>(prog * 140)));
            p.drawRoundedRect(r, 6, 6);
        }});
        // Layer 3: Top line shimmer (loop)
        d.layers.push_back({0.7, 0.1, 1.0, true, 2.0, [](QPainter& p, const QRectF& r, double, double loop, double s, const QColor& ac) {
            double x = r.x() + r.width() * (0.5 + 0.4 * loop);
            QLinearGradient g(x - 20, 0, x + 20, 0);
            g.setColorAt(0, QColor(ac.red(), ac.green(), ac.blue(), 0));
            g.setColorAt(0.5, QColor(ac.red(), ac.green(), ac.blue(), 30));
            g.setColorAt(1, QColor(ac.red(), ac.green(), ac.blue(), 0));
            p.setPen(Qt::NoPen); p.setBrush(g);
            p.drawRect(QRectF(r.x(), r.y(), r.width(), 1.5 * s));
        }});
        addDesign(std::move(d));
    }

    // ── 3. Neon Tech ──────────────────────────────────────
    {
        DesignTemplate d; d.id = "np_neon_tech"; d.name = "Neon Tech"; d.category = "nameplate";
        d.layers.push_back({0.0, 0.3, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor&) {
            p.setPen(Qt::NoPen); p.setBrush(QColor(5, 10, 20, static_cast<int>(prog * 200)));
            p.drawRect(r);
        }});
        // Neon border
        d.layers.push_back({0.15, 0.4, 2.0, true, 2.0, [](QPainter& p, const QRectF& r, double prog, double loop, double s, const QColor& ac) {
            for (int i = 3; i >= 0; --i) {
                double spread = i * 3 * s;
                int alpha = static_cast<int>(prog * (50 - i * 10) * (0.7 + 0.3 * loop));
                p.setPen(QPen(QColor(ac.red(), ac.green(), ac.blue(), alpha), (i == 0 ? 2 : 1) * s));
                p.setBrush(Qt::NoBrush);
                p.drawRect(r.adjusted(-spread, -spread, spread, spread));
            }
        }});
        // Scan lines
        d.layers.push_back({0.3, 0.2, 1.0, true, 3.0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor& ac) {
            p.setPen(QPen(QColor(ac.red(), ac.green(), ac.blue(), static_cast<int>(prog * 15)), 1));
            for (double y = r.y(); y < r.bottom(); y += 3) p.drawLine(QPointF(r.x(), y), QPointF(r.right(), y));
        }});
        addDesign(std::move(d));
    }

    // ── 4. Sport Bold ─────────────────────────────────────
    {
        DesignTemplate d; d.id = "np_sport_bold"; d.name = "Sport Bold"; d.category = "nameplate";
        // Diagonal accent bar
        d.layers.push_back({0.0, 0.3, 4.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor& ac) {
            double w = r.height() * 0.6;
            QPolygonF poly;
            poly << QPointF(r.x(), r.y()) << QPointF(r.x() + w * prog, r.y())
                 << QPointF(r.x() + w * prog - r.height() * 0.3, r.bottom()) << QPointF(r.x(), r.bottom());
            p.setPen(Qt::NoPen); p.setBrush(ac); p.drawPolygon(poly);
        }});
        // Dark background
        d.layers.push_back({0.05, 0.35, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor&) {
            double offset = r.height() * 0.5;
            double w = (r.width() - offset) * prog;
            p.setPen(Qt::NoPen); p.setBrush(QColor(5, 5, 10, 230));
            p.drawRect(QRectF(r.x() + offset, r.y(), w, r.height()));
        }});
        // Bold underline
        d.layers.push_back({0.3, 0.3, 2.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor& ac) {
            p.setPen(QPen(ac, 3 * s, Qt::SolidLine, Qt::SquareCap));
            p.drawLine(QPointF(r.x(), r.bottom()), QPointF(r.x() + r.width() * prog, r.bottom()));
        }});
        addDesign(std::move(d));
    }

    // ── 5. Luxury Gold ────────────────────────────────────
    {
        DesignTemplate d; d.id = "np_luxury_gold"; d.name = "Luxury Gold"; d.category = "nameplate";
        d.layers.push_back({0.0, 0.5, 2.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor&) {
            p.setPen(Qt::NoPen); p.setBrush(QColor(10, 8, 6, static_cast<int>(prog * 210)));
            p.drawRoundedRect(r, 2, 2);
        }});
        // Gold top + bottom lines
        d.layers.push_back({0.2, 0.6, 2.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor&) {
            double w = r.width() * prog;
            QColor gold(212, 175, 55);
            p.setPen(QPen(gold, 1 * s)); p.drawLine(QPointF(r.x(), r.y()), QPointF(r.x() + w, r.y()));
            p.drawLine(QPointF(r.right() - w, r.bottom()), QPointF(r.right(), r.bottom()));
        }});
        // Shimmer (loop)
        d.layers.push_back({0.8, 0.1, 1.0, true, 1.0, [](QPainter& p, const QRectF& r, double, double loop, double, const QColor&) {
            double x = r.x() + r.width() * (0.5 + 0.5 * loop);
            QLinearGradient g(x - 30, 0, x + 30, 0);
            QColor gold(212, 175, 55);
            g.setColorAt(0, QColor(gold.red(), gold.green(), gold.blue(), 0));
            g.setColorAt(0.5, QColor(gold.red(), gold.green(), gold.blue(), 40));
            g.setColorAt(1, QColor(gold.red(), gold.green(), gold.blue(), 0));
            p.setPen(Qt::NoPen); p.setBrush(g); p.drawRect(r);
        }});
        addDesign(std::move(d));
    }

    // ── 6-10: More nameplate designs ──────────────────────
    auto addSimpleNP = [this](const QString& id, const QString& name, QColor bgCol, double bgOp, bool hasBar, bool hasLine) {
        DesignTemplate d; d.id = id; d.name = name; d.category = "nameplate";
        d.layers.push_back({0.0, 0.35, 3.0, false, 0, [bgCol, bgOp](QPainter& p, const QRectF& r, double prog, double, double, const QColor&) {
            double w = r.width() * prog;
            p.setPen(Qt::NoPen); p.setBrush(QColor(bgCol.red(), bgCol.green(), bgCol.blue(), static_cast<int>(bgOp * 255)));
            p.drawRoundedRect(QRectF(r.x(), r.y(), w, r.height()), 4, 4);
        }});
        if (hasBar) {
            d.layers.push_back({0.1, 0.25, 4.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor& ac) {
                p.setPen(Qt::NoPen); p.setBrush(ac);
                p.drawRoundedRect(QRectF(r.x(), r.y(), 3 * s, r.height() * prog), 1.5, 1.5);
            }});
        }
        if (hasLine) {
            d.layers.push_back({0.3, 0.4, 2.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor& ac) {
                p.setPen(QPen(ac, 2 * s)); p.drawLine(QPointF(r.x(), r.bottom()), QPointF(r.x() + r.width() * prog, r.bottom()));
            }});
        }
        addDesign(std::move(d));
    };

    addSimpleNP("np_classic_dark", "Classic Dark", QColor(15, 15, 20), 0.85, true, true);
    addSimpleNP("np_glass_frost", "Glass Frost", QColor(30, 30, 40), 0.6, false, true);
    addSimpleNP("np_solid_red", "Solid Red", QColor(180, 20, 20), 0.9, false, false);
    addSimpleNP("np_ocean_blue", "Ocean Blue", QColor(10, 40, 80), 0.85, true, false);
    addSimpleNP("np_forest_green", "Forest Green", QColor(10, 60, 30), 0.85, true, true);
    addSimpleNP("np_purple_royal", "Purple Royal", QColor(40, 10, 60), 0.85, true, true);
    addSimpleNP("np_white_clean", "White Clean", QColor(240, 240, 245), 0.9, true, false);
    addSimpleNP("np_gradient_warm", "Gradient Warm", QColor(80, 30, 10), 0.85, false, true);
    addSimpleNP("np_gradient_cool", "Gradient Cool", QColor(10, 30, 80), 0.85, false, true);
    addSimpleNP("np_transparent_line", "Transparent Line", QColor(0, 0, 0), 0.3, false, true);
    addSimpleNP("np_pill_shape", "Pill Shape", QColor(20, 20, 25), 0.85, false, false);
    addSimpleNP("np_accent_fill", "Accent Fill", QColor(200, 0, 0), 0.9, false, true);
    addSimpleNP("np_dark_gradient", "Dark Gradient", QColor(5, 5, 10), 0.9, true, true);
    addSimpleNP("np_bright_modern", "Bright Modern", QColor(245, 245, 250), 0.95, true, false);
    addSimpleNP("np_cinema_gold", "Cinema Gold", QColor(15, 12, 8), 0.9, false, true);
    addSimpleNP("np_breaking_urgent", "Breaking Urgent", QColor(200, 0, 0), 0.95, false, false);
    addSimpleNP("np_election_split", "Election Split", QColor(10, 15, 40), 0.85, true, true);
    addSimpleNP("np_weather_sky", "Weather Sky", QColor(20, 60, 120), 0.8, false, true);
    addSimpleNP("np_music_vibrant", "Music Vibrant", QColor(60, 10, 60), 0.85, true, false);
    addSimpleNP("np_documentary", "Documentary", QColor(30, 28, 25), 0.8, false, true);
    addSimpleNP("np_interview_warm", "Interview Warm", QColor(45, 35, 25), 0.75, true, false);
    addSimpleNP("np_arabic_rtl", "Arabic RTL", QColor(40, 15, 10), 0.85, true, true);
    addSimpleNP("np_chinese_modern", "Chinese Modern", QColor(180, 20, 20), 0.85, true, false);
    addSimpleNP("np_football_match", "Football Match", QColor(10, 50, 20), 0.9, true, true);
    addSimpleNP("np_olympics_rings", "Olympics", QColor(240, 240, 245), 0.95, false, true);
}

// ══════════════════════════════════════════════════════════════
// TITLE DESIGNS (20)
// ══════════════════════════════════════════════════════════════

void DesignRegistry::registerTitleDesigns() {
    auto addTitle = [this](const QString& id, const QString& name, QColor bg, double op, bool topLine, bool bottomLine) {
        DesignTemplate d; d.id = id; d.name = name; d.category = "title";
        d.layers.push_back({0.0, 0.4, 3.0, false, 0, [bg, op](QPainter& p, const QRectF& r, double prog, double, double, const QColor&) {
            p.setPen(Qt::NoPen); p.setBrush(QColor(bg.red(), bg.green(), bg.blue(), static_cast<int>(op * 255 * prog)));
            p.drawRoundedRect(r, 4, 4);
        }});
        if (topLine) {
            d.layers.push_back({0.2, 0.5, 2.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor& ac) {
                p.setPen(QPen(ac, 2 * s)); p.drawLine(QPointF(r.x(), r.y()), QPointF(r.x() + r.width() * prog, r.y()));
            }});
        }
        if (bottomLine) {
            d.layers.push_back({0.3, 0.4, 2.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor& ac) {
                p.setPen(QPen(ac, 1.5 * s)); p.drawLine(QPointF(r.x(), r.bottom()), QPointF(r.x() + r.width() * prog, r.bottom()));
            }});
        }
        addDesign(std::move(d));
    };

    addTitle("tt_broadcast", "Broadcast Standard", QColor(26, 26, 46), 0.9, false, true);
    addTitle("tt_glass_dark", "Glass Dark", QColor(10, 10, 15), 0.7, true, true);
    addTitle("tt_solid_accent", "Solid Accent", QColor(200, 0, 0), 0.9, false, false);
    addTitle("tt_minimal_line", "Minimal Line", QColor(0, 0, 0), 0.3, false, true);
    addTitle("tt_elegant_frame", "Elegant Frame", QColor(15, 12, 10), 0.85, true, true);
    addTitle("tt_modern_clean", "Modern Clean", QColor(245, 245, 250), 0.95, false, true);
    addTitle("tt_gradient_blue", "Gradient Blue", QColor(10, 30, 80), 0.85, true, false);
    addTitle("tt_neon_outline", "Neon Outline", QColor(5, 10, 20), 0.8, true, true);
    addTitle("tt_sport_bold", "Sport Bold", QColor(5, 5, 10), 0.9, false, true);
    addTitle("tt_luxury_gold", "Luxury Gold", QColor(15, 12, 8), 0.85, true, true);
    addTitle("tt_breaking_red", "Breaking Red", QColor(180, 0, 0), 0.95, false, false);
    addTitle("tt_weather_sky", "Weather Sky", QColor(20, 60, 120), 0.8, false, true);
    addTitle("tt_election_navy", "Election Navy", QColor(10, 15, 50), 0.9, true, false);
    addTitle("tt_cinema_dark", "Cinema Dark", QColor(0, 0, 0), 0.9, true, true);
    addTitle("tt_documentary", "Documentary", QColor(35, 30, 25), 0.75, false, true);
    addTitle("tt_music_vibrant", "Music Vibrant", QColor(50, 10, 50), 0.85, true, false);
    addTitle("tt_tech_cyber", "Tech Cyber", QColor(5, 10, 20), 0.85, true, true);
    addTitle("tt_arabic", "Arabic", QColor(80, 10, 10), 0.85, true, false);
    addTitle("tt_pill_rounded", "Pill Rounded", QColor(20, 20, 25), 0.85, false, false);
    addTitle("tt_frameless", "Frameless", QColor(0, 0, 0), 0.0, false, true);
}

// ══════════════════════════════════════════════════════════════
// CHANNEL NAME DESIGNS (15)
// ══════════════════════════════════════════════════════════════

void DesignRegistry::registerChannelDesigns() {
    auto addChannel = [this](const QString& id, const QString& name, QColor bg, bool rounded) {
        DesignTemplate d; d.id = id; d.name = name; d.category = "channel";
        d.layers.push_back({0.0, 0.3, 3.0, false, 0, [bg, rounded](QPainter& p, const QRectF& r, double prog, double, double, const QColor&) {
            p.setPen(Qt::NoPen); p.setBrush(QColor(bg.red(), bg.green(), bg.blue(), static_cast<int>(220 * prog)));
            if (rounded) p.drawRoundedRect(r, r.height() / 2, r.height() / 2);
            else p.drawRoundedRect(r, 3, 3);
        }});
        d.layers.push_back({0.15, 0.4, 2.0, true, 1.5, [](QPainter& p, const QRectF& r, double prog, double loop, double s, const QColor& ac) {
            int alpha = static_cast<int>(prog * (15 + 10 * loop));
            p.setPen(Qt::NoPen); p.setBrush(QColor(ac.red(), ac.green(), ac.blue(), alpha));
            p.drawRoundedRect(r.adjusted(-2, -2, 2, 2), 4, 4);
        }});
        addDesign(std::move(d));
    };

    addChannel("ch_rectangle_red", "Rectangle Red", QColor(200, 0, 0), false);
    addChannel("ch_rectangle_blue", "Rectangle Blue", QColor(0, 50, 160), false);
    addChannel("ch_pill_dark", "Pill Dark", QColor(20, 20, 25), true);
    addChannel("ch_pill_accent", "Pill Accent", QColor(91, 79, 219), true);
    addChannel("ch_square_solid", "Square Solid", QColor(200, 0, 0), false);
    addChannel("ch_glass_frost", "Glass Frost", QColor(40, 40, 50), false);
    addChannel("ch_neon_border", "Neon Border", QColor(5, 10, 20), false);
    addChannel("ch_gold_luxury", "Gold Luxury", QColor(15, 12, 8), false);
    addChannel("ch_white_clean", "White Clean", QColor(245, 245, 250), false);
    addChannel("ch_green_eco", "Green Eco", QColor(10, 60, 30), false);
    addChannel("ch_orange_warm", "Orange Warm", QColor(200, 80, 0), false);
    addChannel("ch_purple_royal", "Purple Royal", QColor(60, 10, 80), true);
    addChannel("ch_transparent", "Transparent", QColor(0, 0, 0), false);
    addChannel("ch_gradient_dark", "Gradient Dark", QColor(10, 10, 15), false);
    addChannel("ch_cinema", "Cinema", QColor(0, 0, 0), false);
}

// ══════════════════════════════════════════════════════════════
// TICKER DESIGNS (10)
// ══════════════════════════════════════════════════════════════

void DesignRegistry::registerTickerDesigns() {
    auto addTicker = [this](const QString& id, const QString& name, QColor bg) {
        DesignTemplate d; d.id = id; d.name = name; d.category = "ticker";
        d.layers.push_back({0.0, 0.3, 3.0, false, 0, [bg](QPainter& p, const QRectF& r, double prog, double, double, const QColor&) {
            p.setPen(Qt::NoPen); p.setBrush(QColor(bg.red(), bg.green(), bg.blue(), static_cast<int>(230 * prog)));
            p.drawRect(r);
        }});
        d.layers.push_back({0.1, 0.4, 2.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor& ac) {
            p.setPen(QPen(ac, 2 * s)); p.drawLine(QPointF(r.x(), r.y()), QPointF(r.x() + r.width() * prog, r.y()));
        }});
        addDesign(std::move(d));
    };

    addTicker("tk_news_red", "News Red", QColor(200, 0, 0));
    addTicker("tk_news_blue", "News Blue", QColor(0, 50, 160));
    addTicker("tk_dark_minimal", "Dark Minimal", QColor(10, 10, 15));
    addTicker("tk_sport_green", "Sport Green", QColor(0, 100, 30));
    addTicker("tk_breaking_red", "Breaking Red", QColor(220, 0, 0));
    addTicker("tk_tech_dark", "Tech Dark", QColor(5, 10, 20));
    addTicker("tk_white_clean", "White Clean", QColor(240, 240, 245));
    addTicker("tk_gold_luxury", "Gold Luxury", QColor(30, 25, 15));
    addTicker("tk_purple_pop", "Purple Pop", QColor(91, 79, 219));
    addTicker("tk_orange_warm", "Orange Warm", QColor(200, 80, 0));
}

// ══════════════════════════════════════════════════════════════
// SCOREBOARD DESIGNS (10)
// ══════════════════════════════════════════════════════════════

void DesignRegistry::registerScoreboardDesigns() {
    auto addScore = [this](const QString& id, const QString& name, QColor bg, double op) {
        DesignTemplate d; d.id = id; d.name = name; d.category = "scoreboard";
        d.layers.push_back({0.0, 0.3, 3.0, false, 0, [bg, op](QPainter& p, const QRectF& r, double prog, double, double, const QColor&) {
            p.setPen(Qt::NoPen); p.setBrush(QColor(bg.red(), bg.green(), bg.blue(), static_cast<int>(op * 255 * prog)));
            p.drawRoundedRect(r, 6, 6);
        }});
        d.layers.push_back({0.15, 0.3, 2.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor&) {
            p.setPen(QPen(QColor(255, 255, 255, static_cast<int>(20 * prog)), 1));
            p.setBrush(Qt::NoBrush); p.drawRoundedRect(r, 6, 6);
        }});
        addDesign(std::move(d));
    };

    addScore("sb_glass_dark", "Glass Dark", QColor(0, 0, 0), 0.35);
    addScore("sb_solid_dark", "Solid Dark", QColor(10, 10, 15), 0.9);
    addScore("sb_sport_green", "Sport Green", QColor(0, 60, 20), 0.85);
    addScore("sb_fifa_style", "FIFA Style", QColor(0, 0, 0), 0.4);
    addScore("sb_uefa_modern", "UEFA Modern", QColor(10, 10, 30), 0.5);
    addScore("sb_neon_border", "Neon Border", QColor(5, 10, 20), 0.7);
    addScore("sb_white_clean", "White Clean", QColor(240, 240, 245), 0.9);
    addScore("sb_red_accent", "Red Accent", QColor(80, 0, 0), 0.85);
    addScore("sb_blue_accent", "Blue Accent", QColor(0, 30, 80), 0.85);
    addScore("sb_minimal", "Minimal", QColor(0, 0, 0), 0.25);
}

// ══════════════════════════════════════════════════════════════
// CLOCK DESIGNS (8)
// ══════════════════════════════════════════════════════════════

void DesignRegistry::registerClockDesigns() {
    auto addClock = [this](const QString& id, const QString& name, bool hasBg, bool hasGlow) {
        DesignTemplate d; d.id = id; d.name = name; d.category = "clock";
        if (hasBg) {
            d.layers.push_back({0.0, 0.2, 2.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor&) {
                p.setPen(Qt::NoPen); p.setBrush(QColor(0, 0, 0, static_cast<int>(120 * prog)));
                p.drawRoundedRect(r, r.height() / 2, r.height() / 2);
            }});
        }
        if (hasGlow) {
            d.layers.push_back({0.3, 0.1, 1.0, true, 1.5, [](QPainter& p, const QRectF& r, double, double loop, double, const QColor& ac) {
                int alpha = static_cast<int>(10 + 8 * loop);
                p.setPen(Qt::NoPen); p.setBrush(QColor(ac.red(), ac.green(), ac.blue(), alpha));
                p.drawRoundedRect(r.adjusted(-3, -3, 3, 3), r.height() / 2 + 3, r.height() / 2 + 3);
            }});
        }
        addDesign(std::move(d));
    };

    addClock("ck_shadow_only", "Shadow Only", false, false);
    addClock("ck_pill_dark", "Pill Dark", true, false);
    addClock("ck_pill_glow", "Pill Glow", true, true);
    addClock("ck_neon", "Neon", false, true);
    addClock("ck_glass", "Glass", true, false);
    addClock("ck_minimal", "Minimal", false, false);
    addClock("ck_bold_box", "Bold Box", true, false);
    addClock("ck_accent_border", "Accent Border", true, true);
}

// ══════════════════════════════════════════════════════════════
// WEATHER DESIGNS (8)
// ══════════════════════════════════════════════════════════════

void DesignRegistry::registerWeatherDesigns() {
    auto addWeather = [this](const QString& id, const QString& name, bool hasBg) {
        DesignTemplate d; d.id = id; d.name = name; d.category = "weather";
        if (hasBg) {
            d.layers.push_back({0.0, 0.3, 2.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor&) {
                p.setPen(Qt::NoPen); p.setBrush(QColor(0, 0, 0, static_cast<int>(100 * prog)));
                p.drawRoundedRect(r, 8, 8);
            }});
        }
        // Shadow for text
        d.layers.push_back({0.0, 0.1, 1.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor&) {
            Q_UNUSED(p) Q_UNUSED(r) Q_UNUSED(prog) // Shadow handled by text rendering
        }});
        addDesign(std::move(d));
    };

    addWeather("wt_shadow_only", "Shadow Only", false);
    addWeather("wt_glass_dark", "Glass Dark", true);
    addWeather("wt_pill_frost", "Pill Frost", true);
    addWeather("wt_minimal", "Minimal", false);
    addWeather("wt_card_dark", "Card Dark", true);
    addWeather("wt_neon_glow", "Neon Glow", false);
    addWeather("wt_solid_dark", "Solid Dark", true);
    addWeather("wt_transparent", "Transparent", false);
}

} // namespace prestige
