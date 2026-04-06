// ============================================================
// Prestige AI — Design Templates (Professional Implementation)
// Multi-layer animated designs for each overlay type
// ============================================================

#include "DesignTemplates.h"
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QRadialGradient>
#include <cmath>

namespace prestige {

DesignRegistry& DesignRegistry::instance() {
    static DesignRegistry reg;
    return reg;
}

void DesignRegistry::addDesign(DesignTemplate&& d) {
    m_designs.push_back(std::move(d));
}

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

// ── Helper: easing ──
static double easeOut(double t, double power = 3.0) {
    return 1.0 - std::pow(1.0 - std::clamp(t, 0.0, 1.0), power);
}

// ═══════════════════════════════════════════════════════════════
// RENDER — dispatch to the correct design
// ═══════════════════════════════════════════════════════════════

void DesignRegistry::render(QPainter& painter, const QString& category, const QString& designId,
                             const QRectF& rect, double timeSec, double scale,
                             const QColor& accentColor, const QString& text,
                             const QString& subText, const QFont& font,
                             const QColor& textColor)
{
    const DesignTemplate* design = getDesign(category, designId);
    if (!design) return;

    for (const auto& layer : design->layers) {
        if (!layer.draw) continue;
        double elapsed = timeSec - layer.startSec;
        double entryProg = (layer.durSec > 0) ? easeOut(elapsed / layer.durSec, layer.easePower) : 1.0;
        double loopPhase = layer.looping ? std::sin(elapsed * layer.loopSpeed * 2.0 * M_PI) : 0.0;

        painter.save();
        layer.draw(painter, rect, entryProg, loopPhase, scale, accentColor);
        painter.restore();
    }

    // Draw text on top using user's configured text color
    if (!text.isEmpty()) {
        painter.save();
        QFont f = font.pointSize() > 0 ? font : QFont("Helvetica Neue", static_cast<int>(14 * scale), QFont::Bold);
        painter.setFont(f);
        QFontMetrics fm(f);

        // Text shadow for broadcast legibility
        QColor shadowColor(0, 0, 0, 100);

        if (subText.isEmpty()) {
            painter.setPen(shadowColor);
            painter.drawText(rect.adjusted(1*scale, 1*scale, 1*scale, 1*scale), Qt::AlignCenter, text);
            painter.setPen(textColor);
            painter.drawText(rect, Qt::AlignCenter, text);
        } else {
            int totalH = fm.height() * 2 + 2;
            double ty = rect.y() + (rect.height() - totalH) / 2.0;
            // Main text shadow + color
            painter.setPen(shadowColor);
            painter.drawText(QRectF(rect.x()+scale, ty+scale, rect.width(), fm.height()), Qt::AlignHCenter, text);
            painter.setPen(textColor);
            painter.drawText(QRectF(rect.x(), ty, rect.width(), fm.height()), Qt::AlignHCenter, text);
            // Subtext (slightly dimmer version of textColor)
            QFont sf(f); sf.setWeight(QFont::Normal); sf.setPointSize(f.pointSize() - 2);
            painter.setFont(sf);
            QColor subColor(textColor.red(), textColor.green(), textColor.blue(), 200);
            painter.setPen(subColor);
            painter.drawText(QRectF(rect.x(), ty + fm.height() + 2, rect.width(), fm.height()), Qt::AlignHCenter, subText);
        }
        painter.restore();
    }
}

// ═══════════════════════════════════════════════════════════════
// REGISTER ALL DESIGNS
// ═══════════════════════════════════════════════════════════════

void DesignRegistry::registerAll() {
    registerChannelDesigns();
    registerTitleDesigns();
    registerTickerDesigns();
    registerScoreboardDesigns();
    registerClockDesigns();
    registerWeatherDesigns();
    registerNameplateDesigns();
}

// ── CHANNEL NAME DESIGNS ─────────────────────────────────────

void DesignRegistry::registerChannelDesigns() {
    // Glass dark background
    auto glassLayer = [](const QColor& bg, double opacity, double radius) {
        return AnimLayer{0.0, 0.3, 3.0, false, 0, [bg, opacity, radius](QPainter& p, const QRectF& r, double prog, double, double, const QColor&) {
            p.setPen(Qt::NoPen);
            QColor c = bg; c.setAlphaF(opacity * prog);
            p.setBrush(c);
            p.drawRoundedRect(r, radius, radius);
        }};
    };
    auto accentLine = [](bool bottom, double thickness) {
        return AnimLayer{0.1, 0.4, 3.0, false, 0, [bottom, thickness](QPainter& p, const QRectF& r, double prog, double, double s, const QColor& ac) {
            p.setPen(Qt::NoPen); p.setBrush(ac);
            double y = bottom ? r.bottom() - thickness * s : r.y();
            p.drawRect(QRectF(r.x(), y, r.width() * prog, thickness * s));
        }};
    };

    // ══════════════════════════════════════════════════════════════
    // AFTER EFFECTS-LEVEL CHANNEL NAME DESIGNS
    // Each design is a complete motion graphics composition.
    // The user picks one, types their name — broadcast-ready.
    // All colors adapt to the user's accent color.
    // ══════════════════════════════════════════════════════════════

    // 1. ACCENT BAR — CNN-style: solid accent left bar + dark glass panel
    addDesign({"ch_rectangle_red", "Accent Bar", "channel", {
        // Dark glass panel slides in from left
        AnimLayer{0.0, 0.4, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor&) {
            Q_UNUSED(s)
            QRectF clipped(r.x(), r.y(), r.width() * prog, r.height());
            QLinearGradient g(r.topLeft(), r.topRight());
            g.setColorAt(0, QColor(16, 16, 22, 230)); g.setColorAt(1, QColor(10, 10, 16, 220));
            p.setPen(Qt::NoPen); p.setBrush(g); p.drawRoundedRect(clipped, 3, 3);
        }},
        // Thick accent bar left (wipes down)
        AnimLayer{0.05, 0.35, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor& ac) {
            p.setPen(Qt::NoPen); p.setBrush(ac);
            p.drawRect(QRectF(r.x(), r.y(), 4 * s, r.height() * prog));
        }},
        // Top highlight
        AnimLayer{0.2, 0.3, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor&) {
            p.setPen(QPen(QColor(255,255,255, static_cast<int>(20*prog)), 0.5));
            p.drawLine(QPointF(r.x()+4, r.y()), QPointF(r.x()+r.width()*prog-4, r.y()));
        }}
    }});

    // 2. SPLIT LINES — Two horizontal lines expand, text between them
    addDesign({"ch_rectangle_blue", "Split Lines", "channel", {
        // Subtle dark backdrop
        glassLayer(QColor(8, 8, 14), 0.60, 0),
        // Top line draws from center outward
        AnimLayer{0.0, 0.5, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor& ac) {
            Q_UNUSED(s)
            double cx = r.x() + r.width()/2.0;
            double hw = (r.width()/2.0) * prog; // half-width expanding
            p.setPen(QPen(ac, 1.5)); p.drawLine(QPointF(cx-hw, r.y()), QPointF(cx+hw, r.y()));
        }},
        // Bottom line draws from center outward (delayed)
        AnimLayer{0.08, 0.5, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor& ac) {
            Q_UNUSED(s)
            double cx = r.x() + r.width()/2.0;
            double hw = (r.width()/2.0) * prog;
            p.setPen(QPen(ac, 1.5)); p.drawLine(QPointF(cx-hw, r.bottom()), QPointF(cx+hw, r.bottom()));
        }}
    }});

    // 3. PILL GLASS — Rounded pill with inner radial glow + shimmer loop
    addDesign({"ch_pill_dark", "Pill Glass", "channel", {
        // Pill background
        AnimLayer{0.0, 0.35, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor&) {
            double pillR = r.height() / 2.0;
            QColor bg(12, 12, 20); bg.setAlphaF(0.90 * prog);
            p.setPen(Qt::NoPen); p.setBrush(bg); p.drawRoundedRect(r, pillR, pillR);
        }},
        // Inner radial glow at left (accent colored)
        AnimLayer{0.1, 0.4, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor& ac) {
            double pillR = r.height() / 2.0;
            QRadialGradient rg(r.x() + pillR, r.center().y(), r.height());
            QColor c = ac; c.setAlpha(static_cast<int>(40 * prog));
            rg.setColorAt(0, c); rg.setColorAt(1, QColor(0,0,0,0));
            p.setPen(Qt::NoPen); p.setBrush(rg);
            QPainterPath clip; clip.addRoundedRect(r, pillR, pillR); p.setClipPath(clip);
            p.drawRect(r); p.setClipping(false);
        }},
        // Shimmer sweep that loops
        AnimLayer{0.5, 0.3, 3.0, true, 0.15, [](QPainter& p, const QRectF& r, double, double loop, double s, const QColor&) {
            Q_UNUSED(s)
            double pos = (loop + 1.0) / 2.0; // normalize -1..1 to 0..1
            double sw = 40; double sx = r.x() + (r.width()+sw*2)*pos - sw;
            QLinearGradient sg(sx, r.y(), sx+sw, r.y());
            sg.setColorAt(0, QColor(255,255,255,0)); sg.setColorAt(0.5, QColor(255,255,255,22));
            sg.setColorAt(1, QColor(255,255,255,0));
            double pillR = r.height() / 2.0;
            QPainterPath clip; clip.addRoundedRect(r, pillR, pillR); p.setClipPath(clip);
            p.setPen(Qt::NoPen); p.setBrush(sg); p.drawRect(r); p.setClipping(false);
        }}
    }});

    // 4. BRANDED FILL — Accent color pill, clean and bold
    addDesign({"ch_pill_accent", "Branded Fill", "channel", {
        AnimLayer{0.0, 0.35, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor& ac) {
            double pillR = r.height() / 2.0;
            QLinearGradient g(r.topLeft(), r.bottomRight());
            QColor c1 = ac; c1.setAlphaF(0.90 * prog);
            QColor c2 = ac.lighter(120); c2.setAlphaF(0.85 * prog);
            g.setColorAt(0, c1); g.setColorAt(1, c2);
            p.setPen(Qt::NoPen); p.setBrush(g); p.drawRoundedRect(r, pillR, pillR);
        }},
        // Light highlight top
        AnimLayer{0.15, 0.3, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor&) {
            double pillR = r.height() / 2.0;
            QPainterPath clip; clip.addRoundedRect(r, pillR, pillR); p.setClipPath(clip);
            QLinearGradient hl(r.topLeft(), QPointF(r.left(), r.top()+r.height()*0.4));
            hl.setColorAt(0, QColor(255,255,255, static_cast<int>(30*prog))); hl.setColorAt(1, QColor(255,255,255,0));
            p.setPen(Qt::NoPen); p.setBrush(hl); p.drawRect(r); p.setClipping(false);
        }}
    }});

    // 5. CORNER BRACKETS — Text with animated [ ] corner decorations
    addDesign({"ch_square_solid", "Corner Brackets", "channel", {
        // Very subtle dark backdrop
        glassLayer(QColor(0, 0, 0), 0.35, 2),
        // Top-left + bottom-right corner brackets animate in
        AnimLayer{0.0, 0.5, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor& ac) {
            double len = 10 * s * prog; // bracket arm length
            p.setPen(QPen(ac, 1.5 * s));
            // Top-left corner
            p.drawLine(QPointF(r.x(), r.y()), QPointF(r.x()+len, r.y()));
            p.drawLine(QPointF(r.x(), r.y()), QPointF(r.x(), r.y()+len));
            // Bottom-right corner
            p.drawLine(QPointF(r.right(), r.bottom()), QPointF(r.right()-len, r.bottom()));
            p.drawLine(QPointF(r.right(), r.bottom()), QPointF(r.right(), r.bottom()-len));
        }},
        // Top-right + bottom-left (slightly delayed)
        AnimLayer{0.08, 0.5, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor& ac) {
            double len = 10 * s * prog;
            p.setPen(QPen(ac, 1.5 * s));
            // Top-right
            p.drawLine(QPointF(r.right(), r.y()), QPointF(r.right()-len, r.y()));
            p.drawLine(QPointF(r.right(), r.y()), QPointF(r.right(), r.y()+len));
            // Bottom-left
            p.drawLine(QPointF(r.x(), r.bottom()), QPointF(r.x()+len, r.bottom()));
            p.drawLine(QPointF(r.x(), r.bottom()), QPointF(r.x(), r.bottom()-len));
        }}
    }});

    // 6. FROSTED PANEL — Heavy frosted glass, border glow, Al Jazeera vibe
    addDesign({"ch_glass_frost", "Frosted Panel", "channel", {
        // Frosted white glass
        AnimLayer{0.0, 0.35, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor&) {
            QColor bg(240, 240, 250); bg.setAlphaF(0.14 * prog);
            p.setPen(Qt::NoPen); p.setBrush(bg); p.drawRoundedRect(r, 8, 8);
        }},
        // Fine white border
        AnimLayer{0.1, 0.3, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor&) {
            p.setPen(QPen(QColor(255,255,255, static_cast<int>(25 * prog)), 1));
            p.setBrush(Qt::NoBrush); p.drawRoundedRect(r.adjusted(0.5,0.5,-0.5,-0.5), 8, 8);
        }},
        // Bottom accent line slides in from left
        AnimLayer{0.15, 0.4, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor& ac) {
            QColor c = ac; c.setAlpha(static_cast<int>(160 * prog));
            p.setPen(Qt::NoPen); p.setBrush(c);
            p.drawRect(QRectF(r.x(), r.bottom()-2*s, r.width()*prog, 2*s));
        }}
    }});

    // 7. NEON SIGN — Dark bg, pulsing accent border + inner glow
    addDesign({"ch_neon_border", "Neon Sign", "channel", {
        glassLayer(QColor(4, 4, 10), 0.85, 6),
        // Pulsing neon border
        AnimLayer{0.0, 0.4, 3.0, true, 0.3, [](QPainter& p, const QRectF& r, double prog, double loop, double, const QColor& ac) {
            double glow = 0.5 + std::abs(loop) * 0.4;
            int a = static_cast<int>(200 * prog * glow);
            p.setPen(QPen(QColor(ac.red(), ac.green(), ac.blue(), a), 2));
            p.setBrush(Qt::NoBrush); p.drawRoundedRect(r.adjusted(1,1,-1,-1), 5, 5);
        }},
        // Inner edge glow (fills inward from edges)
        AnimLayer{0.1, 0.4, 3.0, true, 0.3, [](QPainter& p, const QRectF& r, double prog, double loop, double s, const QColor& ac) {
            Q_UNUSED(s)
            double glow = 0.3 + std::abs(loop) * 0.25;
            QColor c = ac; c.setAlpha(static_cast<int>(25 * prog * glow));
            QPainterPath clip; clip.addRoundedRect(r, 6, 6); p.setClipPath(clip);
            // Left glow
            QLinearGradient lg(r.topLeft(), QPointF(r.left()+r.width()*0.3, r.top()));
            lg.setColorAt(0, c); lg.setColorAt(1, QColor(0,0,0,0));
            p.setPen(Qt::NoPen); p.setBrush(lg); p.drawRect(r);
            // Right glow
            QLinearGradient rg(r.topRight(), QPointF(r.right()-r.width()*0.3, r.top()));
            rg.setColorAt(0, c); rg.setColorAt(1, QColor(0,0,0,0));
            p.setBrush(rg); p.drawRect(r);
            p.setClipping(false);
        }}
    }});

    // 8. GOLD CINEMA — Film festival: ultra-dark + animated gold bars top & bottom
    addDesign({"ch_gold_luxury", "Gold Cinema", "channel", {
        glassLayer(QColor(8, 6, 3), 0.94, 2),
        // Gold bottom bar (wipes right)
        AnimLayer{0.05, 0.45, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor&) {
            QLinearGradient g(r.bottomLeft(), r.bottomRight());
            g.setColorAt(0, QColor(170,140,60, static_cast<int>(200*prog)));
            g.setColorAt(0.5, QColor(220,185,100, static_cast<int>(255*prog)));
            g.setColorAt(1, QColor(170,140,60, static_cast<int>(200*prog)));
            p.setPen(Qt::NoPen); p.setBrush(g);
            p.drawRect(QRectF(r.x(), r.bottom()-3*s, r.width()*prog, 3*s));
        }},
        // Gold top line (thinner, wipes right delayed)
        AnimLayer{0.12, 0.4, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor&) {
            QLinearGradient g(r.topLeft(), r.topRight());
            g.setColorAt(0, QColor(170,140,60, static_cast<int>(150*prog)));
            g.setColorAt(0.5, QColor(210,180,90, static_cast<int>(180*prog)));
            g.setColorAt(1, QColor(170,140,60, static_cast<int>(150*prog)));
            p.setPen(Qt::NoPen); p.setBrush(g);
            p.drawRect(QRectF(r.x(), r.y(), r.width()*prog, 1.5));
        }},
        // Slow shimmer sweep loop
        AnimLayer{1.0, 0.2, 3.0, true, 0.12, [](QPainter& p, const QRectF& r, double, double loop, double, const QColor&) {
            double pos = (loop+1.0)/2.0;
            double sw = 50; double sx = r.x()+(r.width()+sw*2)*pos-sw;
            QLinearGradient sg(sx, r.y(), sx+sw, r.y());
            sg.setColorAt(0, QColor(255,220,140,0)); sg.setColorAt(0.5, QColor(255,220,140,18));
            sg.setColorAt(1, QColor(255,220,140,0));
            QPainterPath clip; clip.addRoundedRect(r, 2, 2); p.setClipPath(clip);
            p.setPen(Qt::NoPen); p.setBrush(sg); p.drawRect(r); p.setClipping(false);
        }}
    }});

    // 9. UNDERLINE REVEAL — Clean text, animated underline draws across
    addDesign({"ch_white_clean", "Underline Reveal", "channel", {
        // Very light frosted background
        AnimLayer{0.0, 0.3, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor&) {
            QColor bg(252, 252, 255); bg.setAlphaF(0.88 * prog);
            p.setPen(Qt::NoPen); p.setBrush(bg); p.drawRoundedRect(r, 4, 4);
        }},
        // Animated underline — draws from left with accent color
        AnimLayer{0.15, 0.45, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor& ac) {
            double lineY = r.bottom() - 4 * s;
            p.setPen(Qt::NoPen); p.setBrush(ac);
            p.drawRect(QRectF(r.x() + 4*s, lineY, (r.width() - 8*s) * prog, 2.5*s));
        }}
    }});

    // 10. DUAL BARS — Two parallel accent bars (top+bottom) with glass between
    addDesign({"ch_green_eco", "Dual Bars", "channel", {
        // Dark glass center
        glassLayer(QColor(10, 10, 16), 0.82, 0),
        // Top accent bar (wipes from left)
        AnimLayer{0.0, 0.4, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor& ac) {
            p.setPen(Qt::NoPen); p.setBrush(ac);
            p.drawRect(QRectF(r.x(), r.y(), r.width()*prog, 2.5*s));
        }},
        // Bottom accent bar (wipes from right — opposite direction!)
        AnimLayer{0.05, 0.4, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor& ac) {
            p.setPen(Qt::NoPen); p.setBrush(ac);
            double w = r.width() * prog;
            p.drawRect(QRectF(r.right()-w, r.bottom()-2.5*s, w, 2.5*s));
        }}
    }});

    // 11. GRADIENT WAVE — Horizontal accent gradient that shifts slowly
    addDesign({"ch_orange_warm", "Gradient Wave", "channel", {
        // Dark base
        glassLayer(QColor(10, 10, 14), 0.75, 5),
        // Animated gradient that shifts hue position over time
        AnimLayer{0.0, 0.4, 3.0, true, 0.08, [](QPainter& p, const QRectF& r, double prog, double loop, double, const QColor& ac) {
            double shift = (loop + 1.0) / 2.0; // 0..1
            QLinearGradient g(r.topLeft(), r.topRight());
            QColor c1 = ac; c1.setAlpha(static_cast<int>(180 * prog));
            QColor c2 = ac.darker(150); c2.setAlpha(static_cast<int>(140 * prog));
            g.setColorAt(shift * 0.3, QColor(0,0,0,0));
            g.setColorAt(qBound(0.0, shift * 0.5 + 0.1, 1.0), c1);
            g.setColorAt(qBound(0.0, shift * 0.5 + 0.4, 1.0), c2);
            g.setColorAt(1.0, QColor(0,0,0,0));
            QPainterPath clip; clip.addRoundedRect(r, 5, 5); p.setClipPath(clip);
            p.setPen(Qt::NoPen); p.setBrush(g); p.drawRect(r); p.setClipping(false);
        }}
    }});

    // 12. ANGLED FLAG — Parallelogram shape with fold shadow
    addDesign({"ch_purple_royal", "Angled Flag", "channel", {
        // Accent-colored parallelogram
        AnimLayer{0.0, 0.4, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor& ac) {
            double skew = 10 * s;
            double w = r.width() * prog;
            QPainterPath path;
            path.moveTo(r.x() + skew, r.y());
            path.lineTo(r.x() + w, r.y());
            path.lineTo(r.x() + w - skew, r.bottom());
            path.lineTo(r.x(), r.bottom());
            path.closeSubpath();
            QColor c = ac; c.setAlphaF(0.88 * prog);
            p.setPen(Qt::NoPen); p.setBrush(c); p.drawPath(path);
        }},
        // Highlight gradient on top half
        AnimLayer{0.15, 0.3, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor&) {
            double skew = 10 * s;
            QPainterPath clip;
            clip.moveTo(r.x()+skew, r.y()); clip.lineTo(r.right(), r.y());
            clip.lineTo(r.right()-skew, r.bottom()); clip.lineTo(r.x(), r.bottom()); clip.closeSubpath();
            p.setClipPath(clip);
            QLinearGradient hl(r.topLeft(), QPointF(r.left(), r.top()+r.height()*0.5));
            hl.setColorAt(0, QColor(255,255,255, static_cast<int>(35*prog))); hl.setColorAt(1, QColor(255,255,255,0));
            p.setPen(Qt::NoPen); p.setBrush(hl); p.drawRect(r); p.setClipping(false);
        }}
    }});

    // 13. MINIMAL DOT — Just a colored dot + text, ultra-clean
    addDesign({"ch_transparent", "Minimal Dot", "channel", {
        // Barely visible dark backdrop
        glassLayer(QColor(0, 0, 0), 0.18, 4),
        // Accent dot that scales in (left of text area)
        AnimLayer{0.0, 0.4, 4.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor& ac) {
            double dotR = 3.5 * s * prog;
            double cx = r.x() + 6 * s;
            double cy = r.center().y();
            p.setPen(Qt::NoPen); p.setBrush(ac);
            p.drawEllipse(QPointF(cx, cy), dotR, dotR);
        }}
    }});

    // 14. MIDNIGHT GLASS — Premium diagonal gradient + accent edge glow
    addDesign({"ch_gradient_dark", "Midnight Glass", "channel", {
        AnimLayer{0.0, 0.4, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor& ac) {
            QLinearGradient g(r.topLeft(), r.bottomRight());
            g.setColorAt(0.0, QColor(12, 12, 24, static_cast<int>(235*prog)));
            g.setColorAt(0.5, QColor(18, 14, 32, static_cast<int>(230*prog)));
            g.setColorAt(1.0, QColor(ac.red()/5, ac.green()/5, ac.blue()/5, static_cast<int>(225*prog)));
            p.setPen(Qt::NoPen); p.setBrush(g); p.drawRoundedRect(r, 5, 5);
        }},
        // Accent border that fades in
        AnimLayer{0.2, 0.35, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor& ac) {
            QColor c = ac; c.setAlpha(static_cast<int>(50 * prog));
            p.setPen(QPen(c, 1)); p.setBrush(Qt::NoBrush); p.drawRoundedRect(r.adjusted(0.5,0.5,-0.5,-0.5), 5, 5);
        }},
        // Top edge highlight
        AnimLayer{0.25, 0.3, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor&) {
            p.setPen(QPen(QColor(255,255,255,static_cast<int>(18*prog)), 0.5));
            p.drawLine(QPointF(r.x()+5, r.y()), QPointF(r.x()+r.width()*prog-5, r.y()));
        }}
    }});

    // 15. REVEAL WIPE — Accent block wipes right revealing dark glass underneath
    addDesign({"ch_cinema", "Reveal Wipe", "channel", {
        // Dark glass base (instant)
        glassLayer(QColor(10, 10, 16), 0.90, 3),
        // Accent-colored wipe block that reveals from left then retreats
        AnimLayer{0.0, 0.6, 2.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor& ac) {
            // prog 0→0.5: accent block wipes RIGHT covering the rect
            // prog 0.5→1.0: accent block continues RIGHT revealing dark glass
            double phase = prog * 2.0;
            double leftEdge, rightEdge;
            if (phase <= 1.0) {
                leftEdge = r.x();
                rightEdge = r.x() + r.width() * phase;
            } else {
                leftEdge = r.x() + r.width() * (phase - 1.0);
                rightEdge = r.x() + r.width();
            }
            QColor c = ac; c.setAlphaF(0.90);
            p.setPen(Qt::NoPen); p.setBrush(c);
            p.drawRect(QRectF(leftEdge, r.y(), rightEdge - leftEdge, r.height()));
        }},
        // Thin accent line at bottom (persists after wipe)
        AnimLayer{0.6, 0.3, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor& ac) {
            p.setPen(Qt::NoPen); p.setBrush(ac);
            p.drawRect(QRectF(r.x(), r.bottom()-2*s, r.width()*prog, 2*s));
        }}
    }});
}

// ── TITLE DESIGNS ────────────────────────────────────────────

void DesignRegistry::registerTitleDesigns() {
    auto solidBg = [](const QColor& bg, double radius) {
        return AnimLayer{0.0, 0.3, 3.0, false, 0, [bg, radius](QPainter& p, const QRectF& r, double prog, double, double, const QColor&) {
            QColor c = bg; c.setAlphaF(0.85 * prog);
            p.setPen(Qt::NoPen); p.setBrush(c); p.drawRoundedRect(r, radius, radius);
        }};
    };
    auto accentBar = [](bool top, double thickness) {
        return AnimLayer{0.05, 0.4, 3.0, false, 0, [top, thickness](QPainter& p, const QRectF& r, double prog, double, double s, const QColor& ac) {
            p.setPen(Qt::NoPen); p.setBrush(ac);
            double y = top ? r.y() : r.bottom() - thickness * s;
            p.drawRect(QRectF(r.x(), y, r.width() * prog, thickness * s));
        }};
    };

    // ══════════════════════════════════════════════════════════
    // AE-LEVEL TITLE DESIGNS — accent color drives all visuals
    // ══════════════════════════════════════════════════════════

    // Helper: accent-tinted dark glass
    auto accentGlass = [](double opacity, double radius) {
        return AnimLayer{0.0, 0.35, 3.0, false, 0, [opacity, radius](QPainter& p, const QRectF& r, double prog, double, double, const QColor& ac) {
            QColor bg(ac.red()/8, ac.green()/8, ac.blue()/8); bg.setAlphaF(opacity * prog);
            p.setPen(Qt::NoPen); p.setBrush(bg); p.drawRoundedRect(r, radius, radius);
            // Glass highlight
            QLinearGradient hl(r.topLeft(), QPointF(r.left(), r.top() + r.height() * 0.4));
            hl.setColorAt(0, QColor(255,255,255, static_cast<int>(20 * prog))); hl.setColorAt(1, QColor(255,255,255,0));
            p.setBrush(hl); p.drawRoundedRect(r, radius, radius);
        }};
    };

    // 1. Accent Bar — dark glass + left accent bar wipe
    addDesign({"tt_broadcast", "Accent Bar", "title", {
        accentGlass(0.90, 4),
        AnimLayer{0.05, 0.4, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor& ac) {
            p.setPen(Qt::NoPen); p.setBrush(ac);
            p.drawRect(QRectF(r.x(), r.y(), 4 * s, r.height() * prog));
        }}
    }});

    // 2. Split Lines — lines expand from center
    addDesign({"tt_glass_dark", "Split Lines", "title", {
        solidBg(QColor(8, 8, 14), 0),
        AnimLayer{0.0, 0.5, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor& ac) {
            Q_UNUSED(s) double cx = r.x() + r.width()/2.0; double hw = r.width()/2.0 * prog;
            p.setPen(QPen(ac, 1.5)); p.drawLine(QPointF(cx-hw, r.y()), QPointF(cx+hw, r.y()));
        }},
        AnimLayer{0.08, 0.5, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor& ac) {
            Q_UNUSED(s) double cx = r.x() + r.width()/2.0; double hw = r.width()/2.0 * prog;
            p.setPen(QPen(ac, 1.5)); p.drawLine(QPointF(cx-hw, r.bottom()), QPointF(cx+hw, r.bottom()));
        }}
    }});

    // 3. Accent Fill — accent color pill
    addDesign({"tt_solid_accent", "Accent Fill", "title", {
        AnimLayer{0.0, 0.35, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor& ac) {
            QLinearGradient g(r.topLeft(), r.bottomRight());
            QColor c1 = ac; c1.setAlphaF(0.90 * prog); QColor c2 = ac.lighter(120); c2.setAlphaF(0.85 * prog);
            g.setColorAt(0, c1); g.setColorAt(1, c2);
            p.setPen(Qt::NoPen); p.setBrush(g); p.drawRoundedRect(r, r.height()/2.0, r.height()/2.0);
        }}
    }});

    // 4. Underline Reveal — subtle bg + animated underline
    addDesign({"tt_minimal_line", "Underline Reveal", "title", {
        solidBg(QColor(0, 0, 0), 4),
        AnimLayer{0.15, 0.45, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor& ac) {
            p.setPen(Qt::NoPen); p.setBrush(ac);
            p.drawRect(QRectF(r.x() + 4*s, r.bottom() - 3*s, (r.width() - 8*s) * prog, 2.5*s));
        }}
    }});

    // 5. Corner Brackets — animated [ ] corners
    addDesign({"tt_elegant_frame", "Corner Brackets", "title", {
        solidBg(QColor(0, 0, 0), 2),
        AnimLayer{0.0, 0.5, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor& ac) {
            double len = 12 * s * prog;
            p.setPen(QPen(ac, 1.5 * s));
            p.drawLine(QPointF(r.x(), r.y()), QPointF(r.x()+len, r.y()));
            p.drawLine(QPointF(r.x(), r.y()), QPointF(r.x(), r.y()+len));
            p.drawLine(QPointF(r.right(), r.bottom()), QPointF(r.right()-len, r.bottom()));
            p.drawLine(QPointF(r.right(), r.bottom()), QPointF(r.right(), r.bottom()-len));
        }},
        AnimLayer{0.08, 0.5, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor& ac) {
            double len = 12 * s * prog;
            p.setPen(QPen(ac, 1.5 * s));
            p.drawLine(QPointF(r.right(), r.y()), QPointF(r.right()-len, r.y()));
            p.drawLine(QPointF(r.right(), r.y()), QPointF(r.right(), r.y()+len));
            p.drawLine(QPointF(r.x(), r.bottom()), QPointF(r.x()+len, r.bottom()));
            p.drawLine(QPointF(r.x(), r.bottom()), QPointF(r.x(), r.bottom()-len));
        }}
    }});

    // 6. White Clean — light bg + accent underline
    addDesign({"tt_modern_clean", "White Clean", "title", {
        solidBg(QColor(252, 252, 255), 6),
        accentBar(false, 2.5)
    }});

    // 7. Gradient Wave — accent gradient shifts
    addDesign({"tt_gradient_blue", "Gradient Wave", "title", {
        solidBg(QColor(10, 10, 14), 5),
        AnimLayer{0.0, 0.4, 3.0, true, 0.08, [](QPainter& p, const QRectF& r, double prog, double loop, double, const QColor& ac) {
            double shift = (loop + 1.0) / 2.0;
            QLinearGradient g(r.topLeft(), r.topRight());
            QColor c1 = ac; c1.setAlpha(static_cast<int>(180 * prog));
            QColor c2 = ac.darker(150); c2.setAlpha(static_cast<int>(140 * prog));
            g.setColorAt(shift * 0.3, QColor(0,0,0,0));
            g.setColorAt(qBound(0.0, shift * 0.5 + 0.1, 1.0), c1);
            g.setColorAt(qBound(0.0, shift * 0.5 + 0.4, 1.0), c2);
            g.setColorAt(1.0, QColor(0,0,0,0));
            QPainterPath clip; clip.addRoundedRect(r, 5, 5); p.setClipPath(clip);
            p.setPen(Qt::NoPen); p.setBrush(g); p.drawRect(r); p.setClipping(false);
        }}
    }});

    // 8. Neon Sign — pulsing border + inner glow
    addDesign({"tt_neon_outline", "Neon Sign", "title", {
        solidBg(QColor(4, 4, 10), 6),
        AnimLayer{0.0, 0.4, 3.0, true, 0.3, [](QPainter& p, const QRectF& r, double prog, double loop, double, const QColor& ac) {
            double glow = 0.5 + std::abs(loop) * 0.4;
            p.setPen(QPen(QColor(ac.red(), ac.green(), ac.blue(), static_cast<int>(200 * prog * glow)), 2));
            p.setBrush(Qt::NoBrush); p.drawRoundedRect(r.adjusted(1,1,-1,-1), 5, 5);
        }}
    }});

    // 9. Dual Bars — top + bottom accent bars
    addDesign({"tt_sport_bold", "Dual Bars", "title", {
        solidBg(QColor(10, 10, 16), 0),
        AnimLayer{0.0, 0.4, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor& ac) {
            p.setPen(Qt::NoPen); p.setBrush(ac);
            p.drawRect(QRectF(r.x(), r.y(), r.width() * prog, 3*s));
        }},
        AnimLayer{0.05, 0.4, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor& ac) {
            p.setPen(Qt::NoPen); p.setBrush(ac);
            double w = r.width() * prog;
            p.drawRect(QRectF(r.right()-w, r.bottom()-3*s, w, 3*s));
        }}
    }});

    // 10. Gold Cinema — double gold bars + shimmer
    addDesign({"tt_luxury_gold", "Gold Cinema", "title", {
        solidBg(QColor(8, 6, 3), 2),
        AnimLayer{0.05, 0.45, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor&) {
            QLinearGradient g(r.bottomLeft(), r.bottomRight());
            g.setColorAt(0, QColor(170,140,60, static_cast<int>(200*prog)));
            g.setColorAt(0.5, QColor(220,185,100, static_cast<int>(255*prog)));
            g.setColorAt(1, QColor(170,140,60, static_cast<int>(200*prog)));
            p.setPen(Qt::NoPen); p.setBrush(g);
            p.drawRect(QRectF(r.x(), r.bottom()-3*s, r.width()*prog, 3*s));
        }},
        AnimLayer{0.12, 0.4, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor&) {
            QLinearGradient g(r.topLeft(), r.topRight());
            g.setColorAt(0, QColor(170,140,60, static_cast<int>(150*prog)));
            g.setColorAt(0.5, QColor(210,180,90, static_cast<int>(180*prog)));
            g.setColorAt(1, QColor(170,140,60, static_cast<int>(150*prog)));
            p.setPen(Qt::NoPen); p.setBrush(g); p.drawRect(QRectF(r.x(), r.y(), r.width()*prog, 1.5));
        }}
    }});

    // 11. Reveal Wipe — accent block passes and reveals glass
    addDesign({"tt_breaking_red", "Reveal Wipe", "title", {
        solidBg(QColor(10, 10, 16), 3),
        AnimLayer{0.0, 0.6, 2.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor& ac) {
            double phase = prog * 2.0;
            double leftE = phase <= 1.0 ? r.x() : r.x() + r.width() * (phase - 1.0);
            double rightE = phase <= 1.0 ? r.x() + r.width() * phase : r.x() + r.width();
            QColor c = ac; c.setAlphaF(0.90);
            p.setPen(Qt::NoPen); p.setBrush(c);
            p.drawRect(QRectF(leftE, r.y(), rightE - leftE, r.height()));
        }},
        AnimLayer{0.6, 0.3, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor& ac) {
            p.setPen(Qt::NoPen); p.setBrush(ac);
            p.drawRect(QRectF(r.x(), r.bottom()-2*s, r.width()*prog, 2*s));
        }}
    }});

    // 12. Frosted Panel — glass + border + accent bottom
    addDesign({"tt_weather_sky", "Frosted Panel", "title", {
        AnimLayer{0.0, 0.35, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor&) {
            QColor bg(240, 240, 250); bg.setAlphaF(0.14 * prog);
            p.setPen(Qt::NoPen); p.setBrush(bg); p.drawRoundedRect(r, 8, 8);
        }},
        AnimLayer{0.1, 0.3, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor&) {
            p.setPen(QPen(QColor(255,255,255, static_cast<int>(25 * prog)), 1));
            p.setBrush(Qt::NoBrush); p.drawRoundedRect(r.adjusted(0.5,0.5,-0.5,-0.5), 8, 8);
        }},
        accentBar(false, 2)
    }});

    // 13. Minimal Dot — dot accent + almost no background
    addDesign({"tt_election_navy", "Minimal Dot", "title", {
        solidBg(QColor(0, 0, 0), 4),
        AnimLayer{0.0, 0.4, 4.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor& ac) {
            double dotR = 3.5 * s * prog;
            p.setPen(Qt::NoPen); p.setBrush(ac);
            p.drawEllipse(QPointF(r.x() + 6*s, r.center().y()), dotR, dotR);
        }}
    }});

    // 14. Angled Flag — parallelogram accent
    addDesign({"tt_cinema_dark", "Angled Flag", "title", {
        AnimLayer{0.0, 0.4, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor& ac) {
            double skew = 10 * s; double w = r.width() * prog;
            QPainterPath path;
            path.moveTo(r.x() + skew, r.y()); path.lineTo(r.x() + w, r.y());
            path.lineTo(r.x() + w - skew, r.bottom()); path.lineTo(r.x(), r.bottom()); path.closeSubpath();
            QColor c = ac; c.setAlphaF(0.88 * prog);
            p.setPen(Qt::NoPen); p.setBrush(c); p.drawPath(path);
        }}
    }});

    // 15. Midnight Glass — dark diagonal gradient + accent border
    addDesign({"tt_documentary", "Midnight Glass", "title", {
        accentGlass(0.90, 5),
        AnimLayer{0.2, 0.35, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor& ac) {
            QColor c = ac; c.setAlpha(static_cast<int>(50 * prog));
            p.setPen(QPen(c, 1)); p.setBrush(Qt::NoBrush); p.drawRoundedRect(r.adjusted(0.5,0.5,-0.5,-0.5), 5, 5);
        }}
    }});

    // 16-20: Keep variety
    addDesign({"tt_music_vibrant", "Accent Gradient", "title", {
        AnimLayer{0.0, 0.35, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor& ac) {
            QLinearGradient g(r.topLeft(), r.bottomRight());
            QColor c1 = ac; c1.setAlpha(static_cast<int>(220 * prog));
            QColor c2 = ac.darker(200); c2.setAlpha(static_cast<int>(200 * prog));
            g.setColorAt(0, c1); g.setColorAt(1, c2);
            p.setPen(Qt::NoPen); p.setBrush(g); p.drawRoundedRect(r, 6, 6);
        }}
    }});
    addDesign({"tt_tech_cyber", "Left Stripe", "title", {accentGlass(0.88, 3), accentBar(true, 2),
        AnimLayer{0.05, 0.35, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor& ac) {
            p.setPen(Qt::NoPen); p.setBrush(ac); p.drawRect(QRectF(r.x(), r.y(), 4*s, r.height()*prog));
        }}
    }});
    addDesign({"tt_arabic", "Pill Glass", "title", {
        AnimLayer{0.0, 0.35, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor& ac) {
            double pillR = r.height() / 2.0;
            QColor bg(ac.red()/8, ac.green()/8, ac.blue()/8); bg.setAlphaF(0.90 * prog);
            p.setPen(Qt::NoPen); p.setBrush(bg); p.drawRoundedRect(r, pillR, pillR);
        }},
        AnimLayer{0.1, 0.4, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor& ac) {
            double pillR = r.height() / 2.0;
            QRadialGradient rg(r.x() + pillR, r.center().y(), r.height());
            QColor c = ac; c.setAlpha(static_cast<int>(40 * prog));
            rg.setColorAt(0, c); rg.setColorAt(1, QColor(0,0,0,0));
            QPainterPath clip; clip.addRoundedRect(r, pillR, pillR); p.setClipPath(clip);
            p.setPen(Qt::NoPen); p.setBrush(rg); p.drawRect(r); p.setClipping(false);
        }}
    }});
    addDesign({"tt_pill_rounded", "Ghost", "title", {
        AnimLayer{0.0, 0.3, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor&) {
            p.setPen(Qt::NoPen); p.setBrush(QColor(0,0,0, static_cast<int>(55 * prog)));
            p.drawRoundedRect(r, 6, 6);
        }},
        AnimLayer{0.0, 0.3, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor&) {
            p.setPen(QPen(QColor(255,255,255, static_cast<int>(10 * prog)), 0.5));
            p.setBrush(Qt::NoBrush); p.drawRoundedRect(r, 6, 6);
        }}
    }});
    addDesign({"tt_frameless", "Frameless", "title", {
        AnimLayer{0.0, 0.3, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double s, const QColor&) {
            p.setPen(Qt::NoPen); p.setBrush(QColor(0, 0, 0, static_cast<int>(100 * prog)));
            p.drawRoundedRect(r.adjusted(2*s, 2*s, 2*s, 2*s), 4, 4);
        }}
    }});
}

// ── TICKER DESIGNS ───────────────────────────────────────────

void DesignRegistry::registerTickerDesigns() {
    auto tickerBg = [](const QColor& bg) {
        return AnimLayer{0.0, 0.2, 2.0, false, 0, [bg](QPainter& p, const QRectF& r, double prog, double, double, const QColor&) {
            QColor c = bg; c.setAlphaF(0.9 * prog);
            p.setPen(Qt::NoPen); p.setBrush(c); p.drawRect(r);
        }};
    };
    auto tickerLine = [](bool top) {
        return AnimLayer{0.0, 0.3, 2.0, false, 0, [top](QPainter& p, const QRectF& r, double prog, double, double s, const QColor& ac) {
            p.setPen(Qt::NoPen); p.setBrush(ac);
            double y = top ? r.y() : r.bottom() - 2 * s;
            p.drawRect(QRectF(r.x(), y, r.width() * prog, 2 * s));
        }};
    };

    addDesign({"tk_news_red", "News Red", "ticker", {tickerBg(QColor(180, 0, 0)), tickerLine(true)}});
    addDesign({"tk_news_blue", "News Blue", "ticker", {tickerBg(QColor(0, 40, 120)), tickerLine(true)}});
    addDesign({"tk_dark_glass", "Dark Glass", "ticker", {tickerBg(QColor(10, 10, 20))}});
    addDesign({"tk_light", "Light", "ticker", {tickerBg(QColor(240, 240, 245))}});
    addDesign({"tk_sport", "Sport", "ticker", {tickerBg(QColor(0, 80, 0)), tickerLine(true)}});
    addDesign({"tk_breaking", "Breaking", "ticker", {tickerBg(QColor(200, 0, 0)), tickerLine(true)}});
    addDesign({"tk_gradient", "Gradient", "ticker", {
        AnimLayer{0.0, 0.2, 2.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor& ac) {
            QLinearGradient g(r.topLeft(), r.topRight());
            g.setColorAt(0, QColor(ac.red(), ac.green(), ac.blue(), static_cast<int>(220 * prog)));
            g.setColorAt(1, QColor(ac.darker(200).red(), ac.darker(200).green(), ac.darker(200).blue(), static_cast<int>(220 * prog)));
            p.setPen(Qt::NoPen); p.setBrush(g); p.drawRect(r);
        }}
    }});
}

// ── SCOREBOARD DESIGNS ───────────────────────────────────────

void DesignRegistry::registerScoreboardDesigns() {
    auto sbBg = [](const QColor& bg, double radius) {
        return AnimLayer{0.0, 0.3, 3.0, false, 0, [bg, radius](QPainter& p, const QRectF& r, double prog, double, double, const QColor&) {
            QColor c = bg; c.setAlphaF(0.85 * prog);
            p.setPen(Qt::NoPen); p.setBrush(c); p.drawRoundedRect(r, radius, radius);
        }};
    };

    addDesign({"sb_glass_dark", "Glass Dark", "scoreboard", {sbBg(QColor(10, 10, 20), 6)}});
    addDesign({"sb_solid_black", "Solid Black", "scoreboard", {sbBg(QColor(0, 0, 0), 4)}});
    addDesign({"sb_sport_red", "Sport Red", "scoreboard", {sbBg(QColor(120, 0, 0), 4)}});
    addDesign({"sb_minimal", "Minimal", "scoreboard", {sbBg(QColor(30, 30, 35), 8)}});
    addDesign({"sb_gradient", "Gradient", "scoreboard", {
        AnimLayer{0.0, 0.3, 3.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor&) {
            QLinearGradient g(r.topLeft(), r.bottomLeft());
            g.setColorAt(0, QColor(20, 20, 40, static_cast<int>(220 * prog)));
            g.setColorAt(1, QColor(40, 20, 60, static_cast<int>(220 * prog)));
            p.setPen(Qt::NoPen); p.setBrush(g); p.drawRoundedRect(r, 6, 6);
        }}
    }});
}

// ── CLOCK DESIGNS ────────────────────────────────────────────

void DesignRegistry::registerClockDesigns() {
    addDesign({"ck_shadow_only", "Shadow Only", "clock", {}});
    addDesign({"ck_pill_dark", "Pill Dark", "clock", {
        AnimLayer{0.0, 0.2, 2.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor&) {
            p.setPen(Qt::NoPen); p.setBrush(QColor(0, 0, 0, static_cast<int>(150 * prog)));
            p.drawRoundedRect(r.adjusted(-6, -3, 6, 3), r.height() / 2, r.height() / 2);
        }}
    }});
    addDesign({"ck_pill_glow", "Pill Glow", "clock", {
        AnimLayer{0.0, 0.3, 2.0, true, 0.3, [](QPainter& p, const QRectF& r, double prog, double loop, double, const QColor& ac) {
            double glow = 0.5 + loop * 0.2;
            p.setPen(QPen(QColor(ac.red(), ac.green(), ac.blue(), static_cast<int>(120 * prog * glow)), 1));
            p.setBrush(QColor(0, 0, 0, static_cast<int>(130 * prog)));
            p.drawRoundedRect(r.adjusted(-6, -3, 6, 3), r.height() / 2, r.height() / 2);
        }}
    }});
    addDesign({"ck_neon", "Neon", "clock", {
        AnimLayer{0.0, 0.3, 2.0, true, 0.5, [](QPainter& p, const QRectF& r, double prog, double loop, double, const QColor& ac) {
            double glow = 0.6 + loop * 0.3;
            p.setPen(QPen(QColor(ac.red(), ac.green(), ac.blue(), static_cast<int>(200 * prog * glow)), 2));
            p.setBrush(QColor(0, 0, 0, static_cast<int>(180 * prog)));
            p.drawRoundedRect(r.adjusted(-8, -4, 8, 4), r.height() / 2, r.height() / 2);
        }}
    }});
    addDesign({"ck_glass", "Glass", "clock", {
        AnimLayer{0.0, 0.2, 2.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor&) {
            p.setPen(QPen(QColor(255, 255, 255, 30), 1));
            p.setBrush(QColor(255, 255, 255, static_cast<int>(25 * prog)));
            p.drawRoundedRect(r.adjusted(-6, -3, 6, 3), 4, 4);
        }}
    }});
    addDesign({"ck_minimal", "Minimal", "clock", {}});
    addDesign({"ck_bold_box", "Bold Box", "clock", {
        AnimLayer{0.0, 0.2, 2.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor& ac) {
            p.setPen(Qt::NoPen); p.setBrush(QColor(ac.red(), ac.green(), ac.blue(), static_cast<int>(200 * prog)));
            p.drawRect(r.adjusted(-8, -4, 8, 4));
        }}
    }});
    addDesign({"ck_accent_border", "Accent Border", "clock", {
        AnimLayer{0.0, 0.3, 2.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor& ac) {
            p.setPen(QPen(ac, 2)); p.setBrush(QColor(0, 0, 0, static_cast<int>(100 * prog)));
            p.drawRoundedRect(r.adjusted(-6, -3, 6, 3), 3, 3);
        }}
    }});
}

// ── WEATHER DESIGNS ──────────────────────────────────────────

void DesignRegistry::registerWeatherDesigns() {
    addDesign({"wt_shadow_only", "Shadow Only", "weather", {}});
    addDesign({"wt_glass_dark", "Glass Dark", "weather", {
        AnimLayer{0.0, 0.2, 2.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor&) {
            p.setPen(Qt::NoPen); p.setBrush(QColor(0, 0, 0, static_cast<int>(140 * prog)));
            p.drawRoundedRect(r, 6, 6);
        }}
    }});
    addDesign({"wt_pill", "Pill", "weather", {
        AnimLayer{0.0, 0.2, 2.0, false, 0, [](QPainter& p, const QRectF& r, double prog, double, double, const QColor&) {
            p.setPen(Qt::NoPen); p.setBrush(QColor(0, 0, 0, static_cast<int>(150 * prog)));
            p.drawRoundedRect(r, r.height() / 2, r.height() / 2);
        }}
    }});
}

// ── NAMEPLATE DESIGNS ────────────────────────────────────────

void DesignRegistry::registerNameplateDesigns() {
    auto npBg = [](const QColor& bg, double radius) {
        return AnimLayer{0.0, 0.3, 3.0, false, 0, [bg, radius](QPainter& p, const QRectF& r, double prog, double, double, const QColor&) {
            QColor c = bg; c.setAlphaF(0.85 * prog);
            p.setPen(Qt::NoPen); p.setBrush(c); p.drawRoundedRect(r, radius, radius);
        }};
    };

    addDesign({"np_broadcast_news", "Broadcast News", "nameplate", {npBg(QColor(26, 26, 46), 4)}});
    addDesign({"np_elegant_minimal", "Elegant Minimal", "nameplate", {npBg(QColor(240, 240, 245), 6)}});
    addDesign({"np_neon_tech", "Neon Tech", "nameplate", {npBg(QColor(5, 15, 25), 4)}});
    addDesign({"np_sport_bold", "Sport Bold", "nameplate", {npBg(QColor(180, 0, 0), 2)}});
    addDesign({"np_luxury_gold", "Luxury Gold", "nameplate", {npBg(QColor(40, 30, 10), 4)}});
    addDesign({"np_classic_dark", "Classic Dark", "nameplate", {npBg(QColor(20, 20, 25), 4)}});
}

} // namespace prestige
