// ============================================================
// Prestige AI — Broadcast Effects (Professional Implementation)
// QPainter-based text animations, glow, particles, transitions
// ============================================================

#include "BroadcastEffects.h"
#include <QFontMetrics>
#include <QPainterPath>
#include <QLinearGradient>
#include <QRadialGradient>
#include <algorithm>

namespace prestige {

QStringList allEffectIds() { return {"typewriter","bounce_in","wave_text","tracking_expand","fade_up_letter","scale_up_letter","rotate_in_letter","blur_in","slide_per_letter","kinetic_pop"}; }
QStringList allEffectNames() { return {"Typewriter","Bounce In","Wave Text","Tracking Expand","Fade Up Letter","Scale Up Letter","Rotate In Letter","Blur In","Slide Per Letter","Kinetic Pop"}; }
QString effectCategoryForId(const QString&) { return "text"; }

namespace fx {

// ═══════════════════════════════════════════════════════
// TEXT ANIMATIONS (10) — per-letter effects
// ═══════════════════════════════════════════════════════

void typewriter(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    p.save(); p.setFont(font); p.setPen(color);
    int visChars = qMax(1, static_cast<int>(text.length() * prog));
    QString visible = text.left(visChars);
    p.drawText(r, Qt::AlignVCenter | Qt::AlignLeft, visible);
    // Cursor blink
    if (prog < 1.0) {
        QFontMetrics fm(font);
        double cursorX = r.x() + fm.horizontalAdvance(visible) + 2;
        if (static_cast<int>(prog * 20) % 2 == 0) {
            p.setPen(QPen(color, 2));
            p.drawLine(QPointF(cursorX, r.y() + r.height() * 0.2), QPointF(cursorX, r.y() + r.height() * 0.8));
        }
    }
    p.restore();
}

void bounceIn(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    p.save(); p.setFont(font);
    QFontMetrics fm(font);
    double x = r.x() + (r.width() - fm.horizontalAdvance(text)) / 2.0;
    double baseY = r.y() + r.height() / 2.0 + fm.ascent() / 2.0;
    for (int i = 0; i < text.length(); ++i) {
        double charProg = std::clamp((prog * text.length() - i) * 1.5, 0.0, 1.0);
        double bounce = charProg < 1.0 ? -std::abs(std::sin(charProg * M_PI * 2.5)) * (1.0 - charProg) * 15.0 : 0.0;
        p.setPen(QColor(color.red(), color.green(), color.blue(), static_cast<int>(255 * std::min(1.0, charProg * 2))));
        p.drawText(QPointF(x, baseY + bounce), text.mid(i, 1));
        x += fm.horizontalAdvance(text[i]);
    }
    p.restore();
}

void waveText(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    p.save(); p.setFont(font); p.setPen(color);
    QFontMetrics fm(font);
    double x = r.x() + (r.width() - fm.horizontalAdvance(text)) / 2.0;
    double baseY = r.y() + r.height() / 2.0 + fm.ascent() / 2.0;
    for (int i = 0; i < text.length(); ++i) {
        double wave = std::sin((prog * 4.0 + i * 0.3)) * 6.0 * (1.0 - prog);
        double alpha = std::clamp(prog * text.length() - i, 0.0, 1.0);
        p.setOpacity(alpha);
        p.drawText(QPointF(x, baseY + wave), text.mid(i, 1));
        x += fm.horizontalAdvance(text[i]);
    }
    p.setOpacity(1.0);
    p.restore();
}

void trackingExpand(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    p.save(); p.setFont(font); p.setPen(color);
    QFontMetrics fm(font);
    double extraSpacing = (1.0 - prog) * 20.0;
    double totalW = fm.horizontalAdvance(text) + extraSpacing * (text.length() - 1);
    double x = r.x() + (r.width() - totalW) / 2.0;
    double baseY = r.y() + r.height() / 2.0 + fm.ascent() / 2.0;
    p.setOpacity(std::min(1.0, prog * 3.0));
    for (int i = 0; i < text.length(); ++i) {
        p.drawText(QPointF(x, baseY), text.mid(i, 1));
        x += fm.horizontalAdvance(text[i]) + extraSpacing;
    }
    p.setOpacity(1.0);
    p.restore();
}

void fadeUpPerLetter(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    p.save(); p.setFont(font);
    QFontMetrics fm(font);
    double x = r.x() + (r.width() - fm.horizontalAdvance(text)) / 2.0;
    double baseY = r.y() + r.height() / 2.0 + fm.ascent() / 2.0;
    for (int i = 0; i < text.length(); ++i) {
        double charProg = std::clamp((prog * (text.length() + 3) - i) / 3.0, 0.0, 1.0);
        double ease = 1.0 - std::pow(1.0 - charProg, 3.0);
        double offsetY = (1.0 - ease) * 20.0;
        p.setPen(QColor(color.red(), color.green(), color.blue(), static_cast<int>(255 * ease)));
        p.drawText(QPointF(x, baseY + offsetY), text.mid(i, 1));
        x += fm.horizontalAdvance(text[i]);
    }
    p.restore();
}

void scaleUpPerLetter(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    p.save();
    QFontMetrics fm(font);
    double x = r.x() + (r.width() - fm.horizontalAdvance(text)) / 2.0;
    double baseY = r.y() + r.height() / 2.0 + fm.ascent() / 2.0;
    for (int i = 0; i < text.length(); ++i) {
        double charProg = std::clamp((prog * (text.length() + 2) - i) / 2.0, 0.0, 1.0);
        double sc = 0.3 + charProg * 0.7;
        p.save();
        p.translate(x + fm.horizontalAdvance(text[i]) / 2.0, baseY - fm.ascent() / 2.0);
        p.scale(sc, sc);
        p.translate(-(fm.horizontalAdvance(text[i]) / 2.0), fm.ascent() / 2.0);
        p.setFont(font);
        p.setPen(QColor(color.red(), color.green(), color.blue(), static_cast<int>(255 * charProg)));
        p.drawText(QPointF(0, 0), text.mid(i, 1));
        p.restore();
        x += fm.horizontalAdvance(text[i]);
    }
    p.restore();
}

void rotateInPerLetter(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    p.save();
    QFontMetrics fm(font);
    double x = r.x() + (r.width() - fm.horizontalAdvance(text)) / 2.0;
    double baseY = r.y() + r.height() / 2.0 + fm.ascent() / 2.0;
    for (int i = 0; i < text.length(); ++i) {
        double charProg = std::clamp((prog * (text.length() + 2) - i) / 2.0, 0.0, 1.0);
        double angle = (1.0 - charProg) * 90.0;
        double cw = fm.horizontalAdvance(text[i]);
        p.save();
        p.translate(x + cw / 2.0, baseY - fm.ascent() / 2.0);
        p.rotate(-angle);
        p.setFont(font);
        p.setPen(QColor(color.red(), color.green(), color.blue(), static_cast<int>(255 * charProg)));
        p.drawText(QPointF(-cw / 2.0, fm.ascent() / 2.0), text.mid(i, 1));
        p.restore();
        x += cw;
    }
    p.restore();
}

void blurIn(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    // Simulate blur with multiple offset draws at low opacity, converging
    p.save(); p.setFont(font);
    double blur = (1.0 - prog) * 8.0;
    if (blur > 0.5) {
        for (int i = 0; i < 5; ++i) {
            double angle = i * M_PI * 2 / 5;
            double ox = std::cos(angle) * blur;
            double oy = std::sin(angle) * blur;
            p.setPen(QColor(color.red(), color.green(), color.blue(), static_cast<int>(30 * prog)));
            p.drawText(r.adjusted(ox, oy, ox, oy), Qt::AlignCenter, text);
        }
    }
    p.setPen(QColor(color.red(), color.green(), color.blue(), static_cast<int>(255 * prog)));
    p.drawText(r, Qt::AlignCenter, text);
    p.restore();
}

void slidePerLetter(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    p.save(); p.setFont(font);
    QFontMetrics fm(font);
    double x = r.x() + (r.width() - fm.horizontalAdvance(text)) / 2.0;
    double baseY = r.y() + r.height() / 2.0 + fm.ascent() / 2.0;
    for (int i = 0; i < text.length(); ++i) {
        double charProg = std::clamp((prog * (text.length() + 3) - i) / 3.0, 0.0, 1.0);
        double ease = 1.0 - std::pow(1.0 - charProg, 3.0);
        double slideX = (1.0 - ease) * -30.0;
        p.setPen(QColor(color.red(), color.green(), color.blue(), static_cast<int>(255 * ease)));
        p.drawText(QPointF(x + slideX, baseY), text.mid(i, 1));
        x += fm.horizontalAdvance(text[i]);
    }
    p.restore();
}

void kineticPop(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    p.save();
    QFontMetrics fm(font);
    double x = r.x() + (r.width() - fm.horizontalAdvance(text)) / 2.0;
    double baseY = r.y() + r.height() / 2.0 + fm.ascent() / 2.0;
    for (int i = 0; i < text.length(); ++i) {
        double charProg = std::clamp((prog * (text.length() + 2) - i) / 2.0, 0.0, 1.0);
        // Overshoot bounce
        double sc = charProg < 0.6 ? charProg / 0.6 * 1.2 : 1.2 - (charProg - 0.6) / 0.4 * 0.2;
        double cw = fm.horizontalAdvance(text[i]);
        p.save();
        p.translate(x + cw / 2.0, baseY - fm.ascent() / 2.0);
        p.scale(sc, sc);
        p.setFont(font);
        p.setPen(QColor(color.red(), color.green(), color.blue(), static_cast<int>(255 * std::min(1.0, charProg * 2))));
        p.drawText(QPointF(-cw / 2.0, fm.ascent() / 2.0), text.mid(i, 1));
        p.restore();
        x += cw;
    }
    p.restore();
}

// ═══════════════════════════════════════════════════════
// LOWER THIRD ANIMATIONS (8)
// ═══════════════════════════════════════════════════════

void lineDraw(QPainter& p, const QRectF& r, double prog, const QColor& accent, double scale) {
    p.save(); p.setPen(QPen(accent, 2 * scale)); p.setOpacity(prog);
    double w = r.width() * prog;
    p.drawLine(QPointF(r.x(), r.bottom()), QPointF(r.x() + w, r.bottom()));
    p.restore();
}

void barSlide(QPainter& p, const QRectF& r, double prog, const QColor& accent, double scale) {
    p.save(); p.setPen(Qt::NoPen); p.setBrush(accent); p.setOpacity(0.6 * prog);
    double barH = 3 * scale;
    p.drawRect(QRectF(r.x(), r.bottom() - barH, r.width() * prog, barH));
    p.restore();
}

void shapeMorph(QPainter& p, const QRectF& r, double prog, const QColor& accent, double scale) {
    p.save(); p.setPen(QPen(accent, 1.5 * scale)); p.setBrush(Qt::NoBrush); p.setOpacity(prog);
    double radius = r.height() / 2.0 * (1.0 - prog) + 4 * scale * prog;
    p.drawRoundedRect(r, radius, radius);
    p.restore();
}

void splitReveal(QPainter& p, const QRectF& r, double prog, const QColor& accent, double scale) {
    p.save(); p.setPen(Qt::NoPen); p.setBrush(accent); p.setOpacity(0.4 * prog);
    double halfW = r.width() / 2.0 * prog;
    double cx = r.center().x();
    p.drawRect(QRectF(cx - halfW, r.y(), halfW * 2, r.height()));
    p.restore();
}

void bracketExpand(QPainter& p, const QRectF& r, double prog, const QColor& accent, double scale) {
    p.save(); p.setPen(QPen(accent, 2 * scale)); p.setOpacity(prog);
    double h = r.height() * prog;
    double cy = r.center().y();
    // Left bracket
    p.drawLine(QPointF(r.x(), cy - h / 2), QPointF(r.x(), cy + h / 2));
    p.drawLine(QPointF(r.x(), cy - h / 2), QPointF(r.x() + 6 * scale, cy - h / 2));
    p.drawLine(QPointF(r.x(), cy + h / 2), QPointF(r.x() + 6 * scale, cy + h / 2));
    // Right bracket
    p.drawLine(QPointF(r.right(), cy - h / 2), QPointF(r.right(), cy + h / 2));
    p.drawLine(QPointF(r.right(), cy - h / 2), QPointF(r.right() - 6 * scale, cy - h / 2));
    p.drawLine(QPointF(r.right(), cy + h / 2), QPointF(r.right() - 6 * scale, cy + h / 2));
    p.restore();
}

void underlineGrow(QPainter& p, const QRectF& r, double prog, const QColor& accent, double scale) {
    p.save(); p.setPen(QPen(accent, 2 * scale)); p.setOpacity(prog);
    double w = r.width() * prog;
    p.drawLine(QPointF(r.x(), r.bottom() + 2 * scale), QPointF(r.x() + w, r.bottom() + 2 * scale));
    p.restore();
}

void boxWipe(QPainter& p, const QRectF& r, double prog, const QColor& accent, double scale) {
    (void)scale;
    p.save(); p.setPen(Qt::NoPen); p.setBrush(accent); p.setOpacity(0.3);
    p.drawRect(QRectF(r.x(), r.y(), r.width() * prog, r.height()));
    p.restore();
}

void cornerBuild(QPainter& p, const QRectF& r, double prog, const QColor& accent, double scale) {
    p.save(); p.setPen(QPen(accent, 1.5 * scale)); p.setOpacity(prog);
    double len = 10 * scale * prog;
    // Top-left
    p.drawLine(QPointF(r.x(), r.y()), QPointF(r.x() + len, r.y()));
    p.drawLine(QPointF(r.x(), r.y()), QPointF(r.x(), r.y() + len));
    // Bottom-right
    p.drawLine(QPointF(r.right(), r.bottom()), QPointF(r.right() - len, r.bottom()));
    p.drawLine(QPointF(r.right(), r.bottom()), QPointF(r.right(), r.bottom() - len));
    p.restore();
}

// ═══════════════════════════════════════════════════════
// LOGO REVEALS (8)
// ═══════════════════════════════════════════════════════

void fadeGlow(QPainter& p, const QRectF& r, double prog, const QColor& accent) {
    p.save(); p.setPen(Qt::NoPen);
    QRadialGradient glow(r.center(), r.width() * 0.6);
    glow.setColorAt(0, QColor(accent.red(), accent.green(), accent.blue(), static_cast<int>(80 * prog)));
    glow.setColorAt(1, QColor(accent.red(), accent.green(), accent.blue(), 0));
    p.setBrush(glow); p.drawRect(r.adjusted(-20, -20, 20, 20));
    p.restore();
}

void lightStreak(QPainter& p, const QRectF& r, double prog, const QColor& accent) {
    p.save(); p.setPen(Qt::NoPen);
    double sx = r.x() + r.width() * prog;
    QLinearGradient streak(sx - 30, r.y(), sx + 30, r.y());
    streak.setColorAt(0, QColor(255, 255, 255, 0));
    streak.setColorAt(0.5, QColor(accent.red(), accent.green(), accent.blue(), static_cast<int>(120 * (1.0 - prog))));
    streak.setColorAt(1, QColor(255, 255, 255, 0));
    p.setBrush(streak); p.drawRect(r);
    p.restore();
}

void particleForm(QPainter& p, const QRectF& r, double prog, const QColor& accent, int frameCount) {
    p.save(); p.setPen(Qt::NoPen);
    int count = 12;
    for (int i = 0; i < count; ++i) {
        double angle = (i * 2.0 * M_PI / count) + frameCount * 0.05;
        double dist = (1.0 - prog) * r.width() * 0.5;
        double px = r.center().x() + std::cos(angle) * dist;
        double py = r.center().y() + std::sin(angle) * dist;
        double sz = 3.0 * prog;
        p.setBrush(QColor(accent.red(), accent.green(), accent.blue(), static_cast<int>(200 * prog)));
        p.drawEllipse(QPointF(px, py), sz, sz);
    }
    p.restore();
}

void scaleBounce(QPainter& p, const QRectF& r, double prog) {
    (void)p; (void)r; (void)prog; // Scale handled by caller
}

void shatterIn(QPainter& p, const QRectF& r, double prog) {
    if (prog > 0.8) return;
    p.save(); p.setPen(Qt::NoPen);
    int pieces = 8;
    for (int i = 0; i < pieces; ++i) {
        double angle = i * M_PI * 2 / pieces;
        double dist = (1.0 - prog) * 40;
        double px = r.center().x() + std::cos(angle) * dist - 3;
        double py = r.center().y() + std::sin(angle) * dist - 3;
        p.setBrush(QColor(255, 255, 255, static_cast<int>(60 * (1.0 - prog))));
        p.drawRect(QRectF(px, py, 6, 6));
    }
    p.restore();
}

void blurZoom(QPainter& p, const QRectF& r, double prog) { (void)p; (void)r; (void)prog; }
void rotate3D(QPainter& p, const QRectF& r, double prog) { (void)p; (void)r; (void)prog; }

void pulseReveal(QPainter& p, const QRectF& r, double prog, const QColor& accent) {
    p.save(); p.setPen(Qt::NoPen);
    double radius = r.width() * 0.5 * prog;
    QRadialGradient pulse(r.center(), radius);
    pulse.setColorAt(0, QColor(accent.red(), accent.green(), accent.blue(), static_cast<int>(60 * (1.0 - prog))));
    pulse.setColorAt(1, QColor(accent.red(), accent.green(), accent.blue(), 0));
    p.setBrush(pulse); p.drawEllipse(r.center(), radius, radius);
    p.restore();
}

// ═══════════════════════════════════════════════════════
// TRANSITIONS (8)
// ═══════════════════════════════════════════════════════

void wipeLinear(QPainter& p, const QSize& size, double prog, bool leftToRight) {
    p.save(); p.setPen(Qt::NoPen); p.setBrush(QColor(0, 0, 0, 200));
    if (leftToRight)
        p.drawRect(QRectF(size.width() * prog, 0, size.width() * (1.0 - prog), size.height()));
    else
        p.drawRect(QRectF(0, 0, size.width() * (1.0 - prog), size.height()));
    p.restore();
}

void pushSlide(QPainter& p, const QSize& size, double prog, bool horizontal) { (void)p; (void)size; (void)prog; (void)horizontal; }
void zoomThrough(QPainter& p, const QSize& size, double prog) { (void)p; (void)size; (void)prog; }
void glitchTransition(QPainter& p, const QSize& size, double prog, int frameCount) { (void)p; (void)size; (void)prog; (void)frameCount; }
void lightLeak(QPainter& p, const QSize& size, double prog, const QColor& accent) { (void)p; (void)size; (void)prog; (void)accent; }
void inkBleed(QPainter& p, const QSize& size, double prog) { (void)p; (void)size; (void)prog; }
void spinTransition(QPainter& p, const QSize& size, double prog) { (void)p; (void)size; (void)prog; }
void crossDissolve(QPainter& p, const QSize& size, double prog) { (void)p; (void)size; (void)prog; }

// ═══════════════════════════════════════════════════════
// GLOW & LIGHT (6)
// ═══════════════════════════════════════════════════════

void neonGlow(QPainter& p, const QRectF& r, const QColor& color, double intensity) {
    p.save(); p.setPen(Qt::NoPen);
    for (int i = 3; i >= 1; --i) {
        double spread = i * 3.0;
        QColor c = color; c.setAlphaF(intensity * 0.15 / i);
        p.setBrush(c);
        p.drawRoundedRect(r.adjusted(-spread, -spread, spread, spread), spread + 4, spread + 4);
    }
    p.restore();
}

void bloom(QPainter& p, const QRectF& r, const QColor& color, double intensity) {
    p.save(); p.setPen(Qt::NoPen);
    QRadialGradient grad(r.center(), r.width() * 0.6);
    grad.setColorAt(0, QColor(color.red(), color.green(), color.blue(), static_cast<int>(80 * intensity)));
    grad.setColorAt(1, QColor(color.red(), color.green(), color.blue(), 0));
    p.setBrush(grad);
    p.drawRect(r.adjusted(-20, -20, 20, 20));
    p.restore();
}

void lightRays(QPainter& p, const QRectF& r, double angle, const QColor& color, double intensity) {
    p.save(); p.setPen(Qt::NoPen);
    int rays = 6;
    for (int i = 0; i < rays; ++i) {
        double a = angle + i * M_PI / rays;
        double dx = std::cos(a) * r.width() * 0.5;
        double dy = std::sin(a) * r.height() * 0.5;
        QLinearGradient ray(r.center(), QPointF(r.center().x() + dx, r.center().y() + dy));
        ray.setColorAt(0, QColor(color.red(), color.green(), color.blue(), static_cast<int>(60 * intensity)));
        ray.setColorAt(1, QColor(255, 255, 255, 0));
        p.setBrush(ray); p.drawRect(r);
    }
    p.restore();
}

void lensFlare(QPainter& p, const QPointF& center, double radius, const QColor& color) {
    p.save(); p.setPen(Qt::NoPen);
    QRadialGradient flare(center, radius);
    flare.setColorAt(0, QColor(255, 255, 255, 120));
    flare.setColorAt(0.3, QColor(color.red(), color.green(), color.blue(), 60));
    flare.setColorAt(1, QColor(0, 0, 0, 0));
    p.setBrush(flare); p.drawEllipse(center, radius, radius);
    p.restore();
}

void shimmer(QPainter& p, const QRectF& r, double phase, const QColor& color) {
    p.save(); p.setPen(Qt::NoPen);
    double sx = r.x() + r.width() * std::fmod(phase, 1.0);
    QLinearGradient grad(sx - 15, r.y(), sx + 15, r.y());
    grad.setColorAt(0, QColor(255, 255, 255, 0));
    grad.setColorAt(0.5, QColor(color.red(), color.green(), color.blue(), 50));
    grad.setColorAt(1, QColor(255, 255, 255, 0));
    p.setBrush(grad);
    p.setClipRect(r);
    p.drawRect(r);
    p.setClipping(false);
    p.restore();
}

void edgeGlow(QPainter& p, const QRectF& r, const QColor& color, double width) {
    p.save();
    p.setPen(QPen(QColor(color.red(), color.green(), color.blue(), 100), width));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(r, 4, 4);
    p.restore();
}

// ═══════════════════════════════════════════════════════
// DISTORTION (5)
// ═══════════════════════════════════════════════════════

void glitchRGB(QPainter& p, const QRectF& r, double intensity, int frameCount) {
    if (frameCount % 7 > 4) return; // Intermittent
    p.save();
    double off = intensity * 4;
    p.setOpacity(0.3);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 0, 0, 40)); p.drawRect(r.adjusted(-off, 0, -off, 0));
    p.setBrush(QColor(0, 255, 0, 40)); p.drawRect(r);
    p.setBrush(QColor(0, 0, 255, 40)); p.drawRect(r.adjusted(off, 0, off, 0));
    p.restore();
}

void chromaticAberration(QPainter& p, const QString& text, const QRectF& r, const QFont& font, const QColor& color, double offset) {
    p.save(); p.setFont(font);
    p.setPen(QColor(255, 0, 0, 80)); p.drawText(r.adjusted(-offset, 0, -offset, 0), Qt::AlignCenter, text);
    p.setPen(QColor(0, 0, 255, 80)); p.drawText(r.adjusted(offset, 0, offset, 0), Qt::AlignCenter, text);
    p.setPen(color); p.drawText(r, Qt::AlignCenter, text);
    p.restore();
}

void waveDistort(QPainter& p, const QRectF& r, double phase, double amplitude) { (void)p; (void)r; (void)phase; (void)amplitude; }
void pixelSort(QPainter& p, const QRectF& r, double prog) { (void)p; (void)r; (void)prog; }
void vhsEffect(QPainter& p, const QRectF& r, double intensity, int frameCount) { (void)p; (void)r; (void)intensity; (void)frameCount; }

// ═══════════════════════════════════════════════════════
// PARTICLES (7)
// ═══════════════════════════════════════════════════════

void sparkles(QPainter& p, const QRectF& r, int count, double phase, const QColor& color) {
    p.save(); p.setPen(Qt::NoPen);
    for (int i = 0; i < count; ++i) {
        double seed = i * 137.508;
        double x = r.x() + std::fmod(seed * 0.618 + phase * 50, r.width());
        double y = r.y() + std::fmod(seed * 0.382 + phase * 30, r.height());
        double sz = 1.5 + std::sin(phase * 3 + i) * 1.0;
        int alpha = static_cast<int>(150 + 100 * std::sin(phase * 5 + i * 0.7));
        p.setBrush(QColor(color.red(), color.green(), color.blue(), std::clamp(alpha, 0, 255)));
        p.drawEllipse(QPointF(x, y), sz, sz);
    }
    p.restore();
}

void bokeh(QPainter& p, const QRectF& r, int count, double phase, const QColor& color) {
    p.save(); p.setPen(Qt::NoPen);
    for (int i = 0; i < count; ++i) {
        double seed = i * 97.3;
        double x = r.x() + std::fmod(seed * 0.7 + phase * 20, r.width());
        double y = r.y() + std::fmod(seed * 0.5 + phase * 15, r.height());
        double sz = 4 + std::sin(i * 1.3) * 3;
        int alpha = static_cast<int>(40 + 30 * std::sin(phase * 2 + i));
        p.setBrush(QColor(color.red(), color.green(), color.blue(), std::clamp(alpha, 0, 255)));
        p.drawEllipse(QPointF(x, y), sz, sz);
    }
    p.restore();
}

void dustParticles(QPainter& p, const QRectF& r, int count, double phase) {
    sparkles(p, r, count, phase, QColor(200, 200, 180));
}

void fireEmbers(QPainter& p, const QRectF& r, int count, double phase) {
    p.save(); p.setPen(Qt::NoPen);
    for (int i = 0; i < count; ++i) {
        double seed = i * 73.7;
        double x = r.x() + std::fmod(seed * 0.8, r.width());
        double y = r.bottom() - std::fmod(seed * 0.3 + phase * 60, r.height());
        double sz = 1.5 + std::sin(i * 0.9) * 1.0;
        int g = static_cast<int>(80 + 60 * std::sin(phase * 4 + i));
        p.setBrush(QColor(255, std::clamp(g, 0, 255), 0, 180));
        p.drawEllipse(QPointF(x, y), sz, sz);
    }
    p.restore();
}

void confetti(QPainter& p, const QRectF& r, int count, double phase) {
    p.save(); p.setPen(Qt::NoPen);
    QColor colors[] = {QColor(255,0,0), QColor(0,200,0), QColor(0,100,255), QColor(255,200,0), QColor(255,0,200)};
    for (int i = 0; i < count; ++i) {
        double seed = i * 53.1;
        double x = r.x() + std::fmod(seed * 0.6, r.width());
        double y = r.y() + std::fmod(seed * 0.4 + phase * 40, r.height());
        QColor c = colors[i % 5]; c.setAlpha(180);
        p.setBrush(c);
        p.save();
        p.translate(x, y);
        p.rotate(phase * 100 + i * 30);
        p.drawRect(QRectF(-2, -1, 4, 2));
        p.restore();
    }
    p.restore();
}

void snow(QPainter& p, const QRectF& r, int count, double phase) {
    sparkles(p, r, count, phase, Qt::white);
}

void risingParticles(QPainter& p, const QRectF& r, int count, double phase, const QColor& color) {
    p.save(); p.setPen(Qt::NoPen);
    for (int i = 0; i < count; ++i) {
        double seed = i * 83.7;
        double x = r.x() + std::fmod(seed * 0.7, r.width());
        double y = r.bottom() - std::fmod(seed * 0.2 + phase * 50, r.height());
        double sz = 1.5 + std::sin(i * 1.1) * 1.0;
        int alpha = static_cast<int>(180 - (r.bottom() - y) / r.height() * 180);
        p.setBrush(QColor(color.red(), color.green(), color.blue(), std::clamp(alpha, 0, 255)));
        p.drawEllipse(QPointF(x, y), sz, sz);
    }
    p.restore();
}

// ═══════════════════════════════════════════════════════
// SHAPE OPERATIONS (6) + BLUR (5) + COLOR (5)
// ═══════════════════════════════════════════════════════

void lineDrawOn(QPainter& p, const QPointF& p1, const QPointF& p2, double prog, const QColor& color, double width) {
    QPainter& pp = p; pp.save();
    pp.setPen(QPen(color, width));
    QPointF mid(p1.x() + (p2.x() - p1.x()) * prog, p1.y() + (p2.y() - p1.y()) * prog);
    pp.drawLine(p1, mid);
    pp.restore();
}

void circleExpand(QPainter& p, const QPointF& center, double radius, double prog, const QColor& color) {
    p.save(); p.setPen(QPen(color, 2)); p.setBrush(Qt::NoBrush);
    p.drawEllipse(center, radius * prog, radius * prog);
    p.restore();
}

void rectangleBuild(QPainter& p, const QRectF& r, double prog, const QColor& color, double width) {
    p.save(); p.setPen(QPen(color, width)); p.setBrush(Qt::NoBrush);
    double w = r.width() * prog, h = r.height() * prog;
    p.drawRect(QRectF(r.center().x() - w / 2, r.center().y() - h / 2, w, h));
    p.restore();
}

void pathTrace(QPainter& p, const std::vector<QPointF>& points, double prog, const QColor& color, double width) {
    if (points.size() < 2) return;
    p.save(); p.setPen(QPen(color, width)); p.setBrush(Qt::NoBrush);
    int n = static_cast<int>(points.size() * prog);
    for (int i = 0; i < n - 1; ++i)
        p.drawLine(points[i], points[i + 1]);
    p.restore();
}

void gridReveal(QPainter& p, const QRectF& r, double prog, const QColor& color, int rows, int cols) {
    p.save(); p.setPen(Qt::NoPen); p.setBrush(color);
    double cw = r.width() / cols, ch = r.height() / rows;
    int total = rows * cols;
    int visible = static_cast<int>(total * prog);
    for (int i = 0; i < visible; ++i) {
        int row = i / cols, col = i % cols;
        p.setOpacity(0.4); p.drawRect(QRectF(r.x() + col * cw, r.y() + row * ch, cw - 1, ch - 1));
    }
    p.restore();
}

void hexagonPattern(QPainter& p, const QRectF& r, double prog, const QColor& color, double size) {
    (void)p; (void)r; (void)prog; (void)color; (void)size;
}

void gaussianBlurIn(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    blurIn(p, text, r, prog, font, color);
}
void radialBlur(QPainter& p, const QRectF& r, double phase, const QColor& color) { (void)p; (void)r; (void)phase; (void)color; }
void directionalBlur(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color, double angle) {
    (void)angle; blurIn(p, text, r, prog, font, color);
}
void tiltShift(QPainter& p, const QRectF& r, double focusY, double blurAmount) { (void)p; (void)r; (void)focusY; (void)blurAmount; }
void defocus(QPainter& p, const QRectF& r, double amount, const QColor& tint) { (void)p; (void)r; (void)amount; (void)tint; }

void colorSweep(QPainter& p, const QRectF& r, double prog, const QColor& c1, const QColor& c2) {
    p.save(); p.setPen(Qt::NoPen);
    QLinearGradient grad(r.left(), r.top(), r.right(), r.top());
    grad.setColorAt(0, QColor(c1.red(), c1.green(), c1.blue(), static_cast<int>(60 * prog)));
    grad.setColorAt(1, QColor(c2.red(), c2.green(), c2.blue(), static_cast<int>(60 * prog)));
    p.setBrush(grad); p.drawRect(r);
    p.restore();
}
void gradientShift(QPainter& p, const QRectF& r, double phase, const QColor& c1, const QColor& c2) { colorSweep(p, r, phase, c1, c2); }
void duotone(QPainter& p, const QRectF& r, const QColor& c1, const QColor& c2) { colorSweep(p, r, 1.0, c1, c2); }
void shadowDropAnimate(QPainter& p, const QRectF& r, double prog, const QColor& color) {
    p.save(); p.setPen(Qt::NoPen);
    double off = 3 * prog;
    p.setBrush(QColor(color.red(), color.green(), color.blue(), static_cast<int>(80 * prog)));
    p.drawRoundedRect(r.adjusted(off, off, off, off), 4, 4);
    p.restore();
}
void outlineStroke(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color, double width) {
    p.save(); p.setFont(font);
    QPainterPath path; path.addText(r.x(), r.y() + r.height() * 0.7, font, text);
    p.setPen(QPen(color, width * prog)); p.setBrush(Qt::NoBrush);
    p.drawPath(path);
    p.restore();
}

// ══════════════════════════════════════════════════════════════
// AE EXTENDED TEXT ANIMATIONS — Per-letter motion graphics
// ══════════════════════════════════════════════════════════════

void matrixRain(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    p.save(); p.setFont(font); QFontMetrics fm(font);
    double totalW = fm.horizontalAdvance(text);
    double startX = r.x() + (r.width() - totalW) / 2.0;
    double baseY = r.y() + r.height() * 0.72;
    for (int i = 0; i < text.length(); i++) {
        double letterProg = std::max(0.0, std::min(1.0, prog * text.length() - i));
        double dropY = (1.0 - letterProg) * -r.height() * 0.8; // falls from above
        int alpha = static_cast<int>(255 * letterProg);
        p.setPen(QColor(color.red(), color.green(), color.blue(), alpha));
        p.drawText(QPointF(startX, baseY + dropY), text.mid(i, 1));
        startX += fm.horizontalAdvance(text[i]);
    }
    p.restore();
}

void cascadeReveal(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    p.save(); p.setFont(font); QFontMetrics fm(font);
    double totalW = fm.horizontalAdvance(text);
    double startX = r.x() + (r.width() - totalW) / 2.0;
    double baseY = r.y() + r.height() * 0.72;
    for (int i = 0; i < text.length(); i++) {
        double delay = static_cast<double>(i) / qMax(1, text.length());
        double lp = std::max(0.0, std::min(1.0, (prog - delay * 0.6) / 0.4));
        double eased = 1.0 - (1.0 - lp) * (1.0 - lp) * (1.0 - lp); // easeOutCubic
        int alpha = static_cast<int>(255 * eased);
        double slideY = (1.0 - eased) * 15; // slides down from above
        p.setPen(QColor(color.red(), color.green(), color.blue(), alpha));
        p.drawText(QPointF(startX, baseY - slideY), text.mid(i, 1));
        startX += fm.horizontalAdvance(text[i]);
    }
    p.restore();
}

void elasticDrop(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    p.save(); p.setFont(font); QFontMetrics fm(font);
    double totalW = fm.horizontalAdvance(text);
    double startX = r.x() + (r.width() - totalW) / 2.0;
    double baseY = r.y() + r.height() * 0.72;
    for (int i = 0; i < text.length(); i++) {
        double delay = static_cast<double>(i) / qMax(1, text.length());
        double lp = std::max(0.0, std::min(1.0, (prog - delay * 0.5) / 0.5));
        // Elastic easing: overshoot then settle
        double eased = lp < 1.0 ? 1.0 - std::cos(lp * M_PI * 2.5) * std::exp(-lp * 4.0) : 1.0;
        double dropY = (1.0 - eased) * -30;
        int alpha = static_cast<int>(255 * std::min(1.0, lp * 2.0));
        p.setPen(QColor(color.red(), color.green(), color.blue(), alpha));
        p.drawText(QPointF(startX, baseY + dropY), text.mid(i, 1));
        startX += fm.horizontalAdvance(text[i]);
    }
    p.restore();
}

void spiralIn(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    p.save(); p.setFont(font); QFontMetrics fm(font);
    double totalW = fm.horizontalAdvance(text);
    double startX = r.x() + (r.width() - totalW) / 2.0;
    double baseY = r.y() + r.height() * 0.72;
    double cx = r.center().x(), cy = r.center().y();
    for (int i = 0; i < text.length(); i++) {
        double delay = static_cast<double>(i) / qMax(1, text.length());
        double lp = std::max(0.0, std::min(1.0, (prog - delay * 0.5) / 0.5));
        double eased = 1.0 - (1.0 - lp) * (1.0 - lp); // easeOutQuad
        double angle = (1.0 - eased) * M_PI * 2.0;
        double dist = (1.0 - eased) * 60;
        double targetX = startX;
        double offX = std::cos(angle) * dist;
        double offY = std::sin(angle) * dist;
        Q_UNUSED(cx) Q_UNUSED(cy)
        int alpha = static_cast<int>(255 * eased);
        p.setPen(QColor(color.red(), color.green(), color.blue(), alpha));
        p.drawText(QPointF(targetX + offX, baseY + offY), text.mid(i, 1));
        startX += fm.horizontalAdvance(text[i]);
    }
    p.restore();
}

void flipBoard(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    p.save(); p.setFont(font); QFontMetrics fm(font);
    double totalW = fm.horizontalAdvance(text);
    double startX = r.x() + (r.width() - totalW) / 2.0;
    double baseY = r.y() + r.height() * 0.72;
    for (int i = 0; i < text.length(); i++) {
        double delay = static_cast<double>(i) / qMax(1, text.length());
        double lp = std::max(0.0, std::min(1.0, (prog - delay * 0.6) / 0.4));
        // Simulate vertical flip: scale Y from 0 to 1
        double scaleY = lp;
        int alpha = static_cast<int>(255 * std::min(1.0, lp * 1.5));
        p.save();
        double charCx = startX + fm.horizontalAdvance(text[i]) / 2.0;
        p.translate(charCx, baseY);
        p.scale(1.0, scaleY);
        p.translate(-charCx, -baseY);
        p.setPen(QColor(color.red(), color.green(), color.blue(), alpha));
        p.drawText(QPointF(startX, baseY), text.mid(i, 1));
        p.restore();
        startX += fm.horizontalAdvance(text[i]);
    }
    p.restore();
}

void scatterAssemble(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    p.save(); p.setFont(font); QFontMetrics fm(font);
    double totalW = fm.horizontalAdvance(text);
    double startX = r.x() + (r.width() - totalW) / 2.0;
    double baseY = r.y() + r.height() * 0.72;
    for (int i = 0; i < text.length(); i++) {
        double delay = static_cast<double>(i) / qMax(1, text.length());
        double lp = std::max(0.0, std::min(1.0, (prog - delay * 0.4) / 0.6));
        double eased = 1.0 - (1.0 - lp) * (1.0 - lp) * (1.0 - lp);
        // Scatter: random-ish offset based on char index
        double seed = std::sin(i * 47.13) * 43758.5453;
        double randX = (seed - std::floor(seed)) * 2.0 - 1.0; // -1..1
        double seed2 = std::sin(i * 83.71) * 27391.1;
        double randY = (seed2 - std::floor(seed2)) * 2.0 - 1.0;
        double offX = (1.0 - eased) * randX * 80;
        double offY = (1.0 - eased) * randY * 50;
        int alpha = static_cast<int>(255 * eased);
        p.setPen(QColor(color.red(), color.green(), color.blue(), alpha));
        p.drawText(QPointF(startX + offX, baseY + offY), text.mid(i, 1));
        startX += fm.horizontalAdvance(text[i]);
    }
    p.restore();
}

void stampPress(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    p.save(); p.setFont(font); QFontMetrics fm(font);
    double totalW = fm.horizontalAdvance(text);
    double startX = r.x() + (r.width() - totalW) / 2.0;
    double baseY = r.y() + r.height() * 0.72;
    for (int i = 0; i < text.length(); i++) {
        double delay = static_cast<double>(i) / qMax(1, text.length());
        double lp = std::max(0.0, std::min(1.0, (prog - delay * 0.5) / 0.5));
        // Stamp: scale from 3x down to 1x with overshoot
        double scale = lp < 0.6 ? 3.0 - (3.0 - 0.9) * (lp / 0.6) : 0.9 + 0.1 * ((lp - 0.6) / 0.4);
        int alpha = static_cast<int>(255 * std::min(1.0, lp * 2.5));
        p.save();
        double charCx = startX + fm.horizontalAdvance(text[i]) / 2.0;
        p.translate(charCx, baseY);
        p.scale(scale, scale);
        p.translate(-charCx, -baseY);
        p.setPen(QColor(color.red(), color.green(), color.blue(), alpha));
        p.drawText(QPointF(startX, baseY), text.mid(i, 1));
        p.restore();
        startX += fm.horizontalAdvance(text[i]);
    }
    p.restore();
}

void swingDrop(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    p.save(); p.setFont(font); QFontMetrics fm(font);
    double totalW = fm.horizontalAdvance(text);
    double startX = r.x() + (r.width() - totalW) / 2.0;
    double baseY = r.y() + r.height() * 0.72;
    for (int i = 0; i < text.length(); i++) {
        double delay = static_cast<double>(i) / qMax(1, text.length());
        double lp = std::max(0.0, std::min(1.0, (prog - delay * 0.5) / 0.5));
        // Pendulum swing: rotation from -45 to 0 with damped oscillation
        double swing = (1.0 - lp) * 45.0 * std::cos(lp * M_PI * 3.0) * std::exp(-lp * 3.0);
        int alpha = static_cast<int>(255 * std::min(1.0, lp * 2.0));
        p.save();
        double charCx = startX + fm.horizontalAdvance(text[i]) / 2.0;
        p.translate(charCx, baseY - fm.ascent());
        p.rotate(swing);
        p.translate(-charCx, -(baseY - fm.ascent()));
        p.setPen(QColor(color.red(), color.green(), color.blue(), alpha));
        p.drawText(QPointF(startX, baseY), text.mid(i, 1));
        p.restore();
        startX += fm.horizontalAdvance(text[i]);
    }
    p.restore();
}

void neonFlicker(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    p.save(); p.setFont(font); QFontMetrics fm(font);
    double totalW = fm.horizontalAdvance(text);
    double startX = r.x() + (r.width() - totalW) / 2.0;
    double baseY = r.y() + r.height() * 0.72;
    for (int i = 0; i < text.length(); i++) {
        double delay = static_cast<double>(i) / qMax(1, text.length());
        double lp = std::max(0.0, std::min(1.0, (prog - delay * 0.5) / 0.5));
        // Neon flicker: random on/off then stabilize
        double flicker = 1.0;
        if (lp < 0.7) {
            double seed = std::sin(i * 31.17 + lp * 97.3) * 43758.5453;
            flicker = (seed - std::floor(seed)) > 0.4 ? 1.0 : 0.0;
        }
        int alpha = static_cast<int>(255 * lp * flicker);
        p.setPen(QColor(color.red(), color.green(), color.blue(), alpha));
        p.drawText(QPointF(startX, baseY), text.mid(i, 1));
        startX += fm.horizontalAdvance(text[i]);
    }
    p.restore();
}

void rubberStretch(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    p.save(); p.setFont(font); QFontMetrics fm(font);
    double totalW = fm.horizontalAdvance(text);
    double startX = r.x() + (r.width() - totalW) / 2.0;
    double baseY = r.y() + r.height() * 0.72;
    for (int i = 0; i < text.length(); i++) {
        double delay = static_cast<double>(i) / qMax(1, text.length());
        double lp = std::max(0.0, std::min(1.0, (prog - delay * 0.4) / 0.6));
        // Rubber: scaleX stretches then snaps back, scaleY compresses
        double scaleX = lp < 0.5 ? 0.3 + lp * 3.4 : 2.0 - (lp - 0.5) * 2.0;
        double scaleY = lp < 0.5 ? 1.5 - lp * 1.0 : 1.0;
        if (lp >= 1.0) { scaleX = 1.0; scaleY = 1.0; }
        int alpha = static_cast<int>(255 * std::min(1.0, lp * 2.0));
        p.save();
        double charCx = startX + fm.horizontalAdvance(text[i]) / 2.0;
        p.translate(charCx, baseY);
        p.scale(scaleX, scaleY);
        p.translate(-charCx, -baseY);
        p.setPen(QColor(color.red(), color.green(), color.blue(), alpha));
        p.drawText(QPointF(startX, baseY), text.mid(i, 1));
        p.restore();
        startX += fm.horizontalAdvance(text[i]);
    }
    p.restore();
}

} // namespace fx
} // namespace prestige
