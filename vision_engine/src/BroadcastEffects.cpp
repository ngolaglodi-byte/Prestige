// ============================================================
// Prestige AI — Broadcast Effects Engine (68 effects)
// Professional animation effects — After Effects quality
// ============================================================

#include "BroadcastEffects.h"
#include <QFontMetrics>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QConicalGradient>
#include <QPainterPath>
#include <QPen>
#include <QtMath>
#include <algorithm>

namespace prestige { namespace fx {

// ══════════════════════════════════════════════════════════════
// Helper: easing functions (After Effects standard curves)
// ══════════════════════════════════════════════════════════════

static double easeOutBack(double t) { double s = 1.70158; return 1.0 + (t - 1.0) * (t - 1.0) * ((s + 1.0) * (t - 1.0) + s); }
static double easeOutElastic(double t) { if (t <= 0) return 0; if (t >= 1) return 1; return std::pow(2, -10*t) * std::sin((t-0.1)*5*M_PI) + 1.0; }
static double easeOutBounce(double t) { if (t < 1/2.75) return 7.5625*t*t; if (t < 2/2.75) { t-=1.5/2.75; return 7.5625*t*t+0.75; } if (t < 2.5/2.75) { t-=2.25/2.75; return 7.5625*t*t+0.9375; } t-=2.625/2.75; return 7.5625*t*t+0.984375; }
static double easeOutCubic(double t) { return 1.0 - std::pow(1.0 - t, 3); }
static double easeInOutQuad(double t) { return t < 0.5 ? 2*t*t : 1-std::pow(-2*t+2,2)/2; }
static double clamp01(double v) { return std::max(0.0, std::min(1.0, v)); }

// Deterministic pseudo-random (seedable, no state)
static double pseudoRand(int seed) { return ((seed * 1103515245 + 12345) & 0x7fffffff) / static_cast<double>(0x7fffffff); }

// ══════════════════════════════════════════════════════════════
// TEXT ANIMATIONS (10)
// ══════════════════════════════════════════════════════════════

void typewriter(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    int visibleChars = static_cast<int>(text.length() * clamp01(prog));
    QString visible = text.left(visibleChars);
    p.setFont(font); p.setPen(color);
    p.drawText(r, Qt::AlignVCenter | Qt::AlignLeft, visible);
    // Cursor blink
    if (prog < 1.0) {
        QFontMetrics fm(font);
        int cursorX = static_cast<int>(r.x()) + fm.horizontalAdvance(visible);
        if (static_cast<int>(prog * 20) % 2 == 0)
            p.fillRect(cursorX + 2, static_cast<int>(r.y() + r.height() * 0.2), 2, static_cast<int>(r.height() * 0.6), color);
    }
}

void bounceIn(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    QFontMetrics fm(font);
    p.setFont(font); p.setPen(color);
    int x = static_cast<int>(r.x());
    for (int i = 0; i < text.length(); ++i) {
        double delay = i * 0.06;
        double t = clamp01((prog - delay) / (1.0 - delay * text.length() * 0.5));
        double bounce = easeOutBounce(t);
        double offsetY = (1.0 - bounce) * -r.height() * 0.8;
        p.save();
        p.setOpacity(clamp01(t * 3));
        p.drawText(x, static_cast<int>(r.y() + r.height() * 0.75 + offsetY), QString(text[i]));
        p.restore();
        x += fm.horizontalAdvance(text[i]);
    }
}

void waveText(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    QFontMetrics fm(font);
    p.setFont(font); p.setPen(color);
    int x = static_cast<int>(r.x());
    for (int i = 0; i < text.length(); ++i) {
        double wave = std::sin(prog * M_PI * 2 + i * 0.5) * r.height() * 0.15;
        double opacity = clamp01(prog * 2 - i * 0.05);
        p.save();
        p.setOpacity(opacity);
        p.drawText(x, static_cast<int>(r.y() + r.height() * 0.75 + wave), QString(text[i]));
        p.restore();
        x += fm.horizontalAdvance(text[i]);
    }
}

void trackingExpand(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    QFontMetrics fm(font);
    p.setFont(font); p.setPen(color);
    double spacing = (1.0 - easeOutCubic(prog)) * 20.0;
    int totalW = fm.horizontalAdvance(text) + static_cast<int>(spacing * (text.length() - 1));
    int x = static_cast<int>(r.x() + (r.width() - totalW) / 2);
    p.setOpacity(clamp01(prog * 3));
    for (int i = 0; i < text.length(); ++i) {
        p.drawText(x, static_cast<int>(r.y() + r.height() * 0.75), QString(text[i]));
        x += fm.horizontalAdvance(text[i]) + static_cast<int>(spacing);
    }
    p.setOpacity(1.0);
}

void fadeUpPerLetter(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    QFontMetrics fm(font);
    p.setFont(font);
    int x = static_cast<int>(r.x());
    for (int i = 0; i < text.length(); ++i) {
        double t = clamp01((prog - i * 0.04) * 2.5);
        double offsetY = (1.0 - easeOutCubic(t)) * r.height() * 0.5;
        p.save();
        p.setOpacity(t);
        p.setPen(color);
        p.drawText(x, static_cast<int>(r.y() + r.height() * 0.75 + offsetY), QString(text[i]));
        p.restore();
        x += fm.horizontalAdvance(text[i]);
    }
}

void scaleUpPerLetter(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    QFontMetrics fm(font);
    int x = static_cast<int>(r.x());
    for (int i = 0; i < text.length(); ++i) {
        double t = clamp01((prog - i * 0.04) * 2.5);
        double s = 0.3 + 0.7 * easeOutBack(t);
        int charW = fm.horizontalAdvance(text[i]);
        p.save();
        p.setOpacity(clamp01(t * 2));
        p.translate(x + charW / 2, r.y() + r.height() / 2);
        p.scale(s, s);
        p.setFont(font); p.setPen(color);
        p.drawText(-charW / 2, fm.ascent() / 2, QString(text[i]));
        p.restore();
        x += charW;
    }
}

void rotateInPerLetter(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    QFontMetrics fm(font);
    int x = static_cast<int>(r.x());
    for (int i = 0; i < text.length(); ++i) {
        double t = clamp01((prog - i * 0.04) * 2.5);
        double angle = (1.0 - easeOutCubic(t)) * -90;
        int charW = fm.horizontalAdvance(text[i]);
        p.save();
        p.setOpacity(clamp01(t * 2));
        p.translate(x + charW / 2, r.y() + r.height() * 0.75);
        p.rotate(angle);
        p.setFont(font); p.setPen(color);
        p.drawText(-charW / 2, 0, QString(text[i]));
        p.restore();
        x += charW;
    }
}

void blurIn(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    int passes = static_cast<int>((1.0 - prog) * 6);
    p.setFont(font);
    for (int i = passes; i >= 0; --i) {
        double offset = i * 2.0;
        int alpha = (i == 0) ? 255 : static_cast<int>(30.0 / (i + 1));
        p.setPen(QColor(color.red(), color.green(), color.blue(), alpha));
        p.drawText(r.adjusted(offset, 0, 0, 0), Qt::AlignVCenter | Qt::AlignLeft, text);
        p.drawText(r.adjusted(-offset, 0, 0, 0), Qt::AlignVCenter | Qt::AlignLeft, text);
    }
    p.setOpacity(clamp01(prog * 2));
    p.setPen(color);
    p.drawText(r, Qt::AlignVCenter | Qt::AlignLeft, text);
    p.setOpacity(1.0);
}

void slidePerLetter(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    QFontMetrics fm(font);
    p.setFont(font); p.setPen(color);
    int x = static_cast<int>(r.x());
    for (int i = 0; i < text.length(); ++i) {
        double t = clamp01((prog - i * 0.03) * 2.0);
        double offsetX = (1.0 - easeOutCubic(t)) * r.width() * 0.3;
        p.save();
        p.setOpacity(clamp01(t * 2));
        p.drawText(static_cast<int>(x + offsetX), static_cast<int>(r.y() + r.height() * 0.75), QString(text[i]));
        p.restore();
        x += fm.horizontalAdvance(text[i]);
    }
}

void kineticPop(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    QFontMetrics fm(font);
    int x = static_cast<int>(r.x());
    for (int i = 0; i < text.length(); ++i) {
        double t = clamp01((prog - i * 0.05) * 3.0);
        double s = easeOutElastic(t);
        int charW = fm.horizontalAdvance(text[i]);
        p.save();
        p.setOpacity(clamp01(t * 2));
        p.translate(x + charW / 2, r.y() + r.height() / 2);
        p.scale(s, s);
        p.setFont(font); p.setPen(color);
        p.drawText(-charW / 2, fm.ascent() / 2, QString(text[i]));
        p.restore();
        x += charW;
    }
}

// ══════════════════════════════════════════════════════════════
// LOWER THIRD ANIMATIONS (8)
// ══════════════════════════════════════════════════════════════

void lineDraw(QPainter& p, const QRectF& r, double prog, const QColor& accent, double scale) {
    double w = r.width() * easeOutCubic(prog);
    p.setPen(QPen(accent, 2 * scale, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(QPointF(r.x(), r.bottom()), QPointF(r.x() + w, r.bottom()));
    if (prog > 0.3) {
        double w2 = r.width() * 0.6 * easeOutCubic((prog - 0.3) / 0.7);
        p.setPen(QPen(accent.lighter(130), 1 * scale));
        p.drawLine(QPointF(r.x(), r.bottom() - 4 * scale), QPointF(r.x() + w2, r.bottom() - 4 * scale));
    }
}

void barSlide(QPainter& p, const QRectF& r, double prog, const QColor& accent, double scale) {
    double barW = r.width() * easeOutCubic(clamp01(prog * 1.5));
    double barH = 4 * scale;
    p.setPen(Qt::NoPen);
    p.setBrush(accent);
    p.drawRoundedRect(QRectF(r.x(), r.bottom() - barH, barW, barH), barH / 2, barH / 2);
    if (prog > 0.4) {
        double bgW = r.width() * easeOutCubic((prog - 0.4) / 0.6);
        p.setBrush(QColor(accent.red(), accent.green(), accent.blue(), 30));
        p.drawRect(QRectF(r.x(), r.y(), bgW, r.height()));
    }
}

void shapeMorph(QPainter& p, const QRectF& r, double prog, const QColor& accent, double scale) {
    double t = easeOutCubic(prog);
    double radius = r.height() * 0.5 * (1.0 - t);
    double w = r.width() * t;
    p.setPen(QPen(accent, 1.5 * scale));
    p.setBrush(QColor(accent.red(), accent.green(), accent.blue(), static_cast<int>(20 * t)));
    p.drawRoundedRect(QRectF(r.x(), r.y(), w, r.height()), radius, radius);
}

void splitReveal(QPainter& p, const QRectF& r, double prog, const QColor& accent, double scale) {
    double half = r.width() / 2;
    double offset = half * (1.0 - easeOutCubic(prog));
    p.setPen(QPen(accent, 2 * scale));
    p.drawLine(QPointF(r.center().x() - offset, r.bottom()), QPointF(r.x(), r.bottom()));
    p.drawLine(QPointF(r.center().x() + offset, r.bottom()), QPointF(r.right(), r.bottom()));
}

void bracketExpand(QPainter& p, const QRectF& r, double prog, const QColor& accent, double scale) {
    double h = r.height() * easeOutCubic(prog);
    double bw = 6 * scale;
    double cy = r.center().y();
    p.setPen(QPen(accent, 2 * scale, Qt::SolidLine, Qt::SquareCap));
    // Left bracket
    p.drawLine(QPointF(r.x(), cy - h/2), QPointF(r.x(), cy + h/2));
    p.drawLine(QPointF(r.x(), cy - h/2), QPointF(r.x() + bw, cy - h/2));
    p.drawLine(QPointF(r.x(), cy + h/2), QPointF(r.x() + bw, cy + h/2));
    // Right bracket
    p.drawLine(QPointF(r.right(), cy - h/2), QPointF(r.right(), cy + h/2));
    p.drawLine(QPointF(r.right(), cy - h/2), QPointF(r.right() - bw, cy - h/2));
    p.drawLine(QPointF(r.right(), cy + h/2), QPointF(r.right() - bw, cy + h/2));
}

void underlineGrow(QPainter& p, const QRectF& r, double prog, const QColor& accent, double scale) {
    double w = r.width() * easeOutCubic(prog);
    double cx = r.center().x();
    p.setPen(QPen(accent, 3 * scale, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(QPointF(cx - w/2, r.bottom()), QPointF(cx + w/2, r.bottom()));
}

void boxWipe(QPainter& p, const QRectF& r, double prog, const QColor& accent, double scale) {
    double t = easeOutCubic(prog);
    QRectF box(r.x(), r.y(), r.width() * t, r.height());
    p.setPen(QPen(accent, 1.5 * scale));
    p.setBrush(QColor(accent.red(), accent.green(), accent.blue(), 15));
    p.drawRect(box);
}

void cornerBuild(QPainter& p, const QRectF& r, double prog, const QColor& accent, double scale) {
    double len = qMin(r.width(), r.height()) * 0.3 * easeOutCubic(prog);
    p.setPen(QPen(accent, 2 * scale, Qt::SolidLine, Qt::SquareCap));
    // Top-left
    p.drawLine(QPointF(r.x(), r.y()), QPointF(r.x() + len, r.y()));
    p.drawLine(QPointF(r.x(), r.y()), QPointF(r.x(), r.y() + len));
    // Bottom-right
    p.drawLine(QPointF(r.right(), r.bottom()), QPointF(r.right() - len, r.bottom()));
    p.drawLine(QPointF(r.right(), r.bottom()), QPointF(r.right(), r.bottom() - len));
}

// ══════════════════════════════════════════════════════════════
// LOGO REVEALS (8)
// ══════════════════════════════════════════════════════════════

void fadeGlow(QPainter& p, const QRectF& r, double prog, const QColor& accent) {
    for (int i = 3; i >= 0; --i) {
        double spread = i * 8.0;
        int alpha = static_cast<int>(clamp01(prog * 2) * (40.0 / (i + 1)));
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(accent.red(), accent.green(), accent.blue(), alpha));
        p.drawRoundedRect(r.adjusted(-spread, -spread, spread, spread), 8 + spread, 8 + spread);
    }
}

void lightStreak(QPainter& p, const QRectF& r, double prog, const QColor& accent) {
    double streakX = r.x() + r.width() * prog * 1.5 - r.width() * 0.25;
    QLinearGradient grad(streakX - 30, 0, streakX + 30, 0);
    grad.setColorAt(0, QColor(255, 255, 255, 0));
    grad.setColorAt(0.5, QColor(accent.red(), accent.green(), accent.blue(), static_cast<int>(120 * clamp01(prog * 3))));
    grad.setColorAt(1, QColor(255, 255, 255, 0));
    p.setPen(Qt::NoPen);
    p.setBrush(grad);
    p.drawRect(r.adjusted(-10, -5, 10, 5));
}

void particleForm(QPainter& p, const QRectF& r, double prog, const QColor& accent, int frameCount) {
    p.setPen(Qt::NoPen);
    int count = 30;
    for (int i = 0; i < count; ++i) {
        double px = pseudoRand(i * 37) * r.width() + r.x();
        double py = pseudoRand(i * 73) * r.height() + r.y();
        double targetX = r.center().x() + (pseudoRand(i * 11) - 0.5) * r.width() * 0.8;
        double targetY = r.center().y() + (pseudoRand(i * 23) - 0.5) * r.height() * 0.8;
        double t = clamp01(prog * 1.5);
        double cx = px + (targetX - px) * t;
        double cy = py + (targetY - py) * t;
        int alpha = static_cast<int>(clamp01(t * 2) * 180);
        double sz = 2 + pseudoRand(i * 59) * 3;
        p.setBrush(QColor(accent.red(), accent.green(), accent.blue(), alpha));
        p.drawEllipse(QPointF(cx, cy), sz, sz);
    }
}

void scaleBounce(QPainter& p, const QRectF& r, double prog) {
    Q_UNUSED(p) Q_UNUSED(r) Q_UNUSED(prog)
    // Applied via transform in the caller — sets scale factor
}

void shatterIn(QPainter& p, const QRectF& r, double prog) {
    int cols = 4, rows = 3;
    double cellW = r.width() / cols, cellH = r.height() / rows;
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            int idx = row * cols + col;
            double delay = pseudoRand(idx * 17) * 0.4;
            double t = clamp01((prog - delay) / (1.0 - delay));
            double offX = (pseudoRand(idx * 41) - 0.5) * 200 * (1.0 - t);
            double offY = (pseudoRand(idx * 67) - 0.5) * 200 * (1.0 - t);
            double rot = (pseudoRand(idx * 89) - 0.5) * 180 * (1.0 - t);
            p.save();
            p.setOpacity(clamp01(t * 2));
            QRectF cell(r.x() + col * cellW + offX, r.y() + row * cellH + offY, cellW, cellH);
            p.translate(cell.center());
            p.rotate(rot);
            p.translate(-cell.center());
            p.fillRect(cell, QColor(255, 255, 255, static_cast<int>(t * 20)));
            p.restore();
        }
    }
}

void blurZoom(QPainter& p, const QRectF& r, double prog) {
    Q_UNUSED(p) Q_UNUSED(r) Q_UNUSED(prog)
    // Applied via opacity + scale transform in caller
}

void rotate3D(QPainter& p, const QRectF& r, double prog) {
    Q_UNUSED(r)
    double angle = (1.0 - easeOutCubic(prog)) * 90;
    double scaleX = std::cos(angle * M_PI / 180.0);
    p.scale(std::max(0.01, scaleX), 1.0);
    p.setOpacity(clamp01(prog * 2));
}

void pulseReveal(QPainter& p, const QRectF& r, double prog, const QColor& accent) {
    for (int i = 2; i >= 0; --i) {
        double delay = i * 0.15;
        double t = clamp01((prog - delay) / (1.0 - delay));
        double radius = qMax(r.width(), r.height()) * 0.5 * easeOutCubic(t);
        int alpha = static_cast<int>((1.0 - t) * 60);
        p.setPen(QPen(QColor(accent.red(), accent.green(), accent.blue(), alpha), 2));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(r.center(), radius, radius * 0.6);
    }
}

// ══════════════════════════════════════════════════════════════
// TRANSITIONS (8)
// ══════════════════════════════════════════════════════════════

void wipeLinear(QPainter& p, const QSize& size, double prog, bool leftToRight) {
    double x = leftToRight ? size.width() * prog : size.width() * (1.0 - prog);
    p.setClipRect(QRectF(leftToRight ? 0 : x, 0, leftToRight ? x : size.width() - x, size.height()));
}

void pushSlide(QPainter& p, const QSize& size, double prog, bool horizontal) {
    double offset = horizontal ? size.width() * (1.0 - easeOutCubic(prog)) : size.height() * (1.0 - easeOutCubic(prog));
    p.translate(horizontal ? -offset : 0, horizontal ? 0 : -offset);
}

void zoomThrough(QPainter& p, const QSize& size, double prog) {
    double s = 1.0 + (1.0 - prog) * 2.0;
    p.translate(size.width() / 2, size.height() / 2);
    p.scale(s, s);
    p.translate(-size.width() / 2, -size.height() / 2);
    p.setOpacity(clamp01(prog * 3));
}

void glitchTransition(QPainter& p, const QSize& size, double prog, int frameCount) {
    if (prog > 0.8) return;
    int slices = 8;
    double sliceH = size.height() / slices;
    for (int i = 0; i < slices; ++i) {
        double offset = std::sin(frameCount * 0.5 + i * 2.0) * 20 * (1.0 - prog);
        p.save();
        p.setClipRect(QRectF(0, i * sliceH, size.width(), sliceH));
        p.translate(offset, 0);
        p.restore();
    }
}

void lightLeak(QPainter& p, const QSize& size, double prog, const QColor& accent) {
    QRadialGradient grad(size.width() * prog, size.height() * 0.3, size.width() * 0.4);
    grad.setColorAt(0, QColor(accent.red(), accent.green(), accent.blue(), static_cast<int>(80 * (1.0 - prog))));
    grad.setColorAt(1, QColor(accent.red(), accent.green(), accent.blue(), 0));
    p.setPen(Qt::NoPen); p.setBrush(grad);
    p.drawRect(0, 0, size.width(), size.height());
}

void inkBleed(QPainter& p, const QSize& size, double prog) {
    int circles = 12;
    p.setPen(Qt::NoPen);
    for (int i = 0; i < circles; ++i) {
        double cx = pseudoRand(i * 37) * size.width();
        double cy = pseudoRand(i * 71) * size.height();
        double maxR = size.width() * 0.3;
        double r = maxR * easeOutCubic(clamp01(prog * 2 - pseudoRand(i * 13) * 0.5));
        p.setBrush(QColor(0, 0, 0, static_cast<int>(200 * clamp01(prog * 3))));
        p.drawEllipse(QPointF(cx, cy), r, r * 0.8);
    }
}

void spinTransition(QPainter& p, const QSize& size, double prog) {
    p.translate(size.width() / 2, size.height() / 2);
    p.rotate((1.0 - easeOutCubic(prog)) * 180);
    p.scale(easeOutCubic(prog), easeOutCubic(prog));
    p.translate(-size.width() / 2, -size.height() / 2);
}

void crossDissolve(QPainter& p, const QSize& size, double prog) {
    Q_UNUSED(size)
    p.setOpacity(easeInOutQuad(prog));
}

// ══════════════════════════════════════════════════════════════
// GLOW & LIGHT (6)
// ══════════════════════════════════════════════════════════════

void neonGlow(QPainter& p, const QRectF& r, const QColor& color, double intensity) {
    for (int i = 4; i >= 0; --i) {
        double spread = i * 4.0 * intensity;
        int alpha = static_cast<int>(intensity * 50.0 / (i + 1));
        p.setPen(QPen(QColor(color.red(), color.green(), color.blue(), alpha), (i + 1) * 2));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(r.adjusted(-spread, -spread, spread, spread), 4 + spread, 4 + spread);
    }
    // Core bright line
    p.setPen(QPen(QColor(255, 255, 255, static_cast<int>(intensity * 200)), 1));
    p.drawRoundedRect(r, 4, 4);
}

void bloom(QPainter& p, const QRectF& r, const QColor& color, double intensity) {
    QRadialGradient grad(r.center(), qMax(r.width(), r.height()) * 0.6);
    grad.setColorAt(0, QColor(color.red(), color.green(), color.blue(), static_cast<int>(intensity * 80)));
    grad.setColorAt(0.5, QColor(color.red(), color.green(), color.blue(), static_cast<int>(intensity * 30)));
    grad.setColorAt(1, QColor(0, 0, 0, 0));
    p.setPen(Qt::NoPen); p.setBrush(grad);
    p.drawRect(r.adjusted(-r.width() * 0.3, -r.height() * 0.3, r.width() * 0.3, r.height() * 0.3));
}

void lightRays(QPainter& p, const QRectF& r, double angle, const QColor& color, double intensity) {
    p.save();
    p.translate(r.center());
    p.rotate(angle);
    for (int i = 0; i < 8; ++i) {
        double a = i * 45.0;
        double len = qMax(r.width(), r.height()) * 0.8;
        QLinearGradient ray(0, 0, len * std::cos(a * M_PI / 180), len * std::sin(a * M_PI / 180));
        ray.setColorAt(0, QColor(color.red(), color.green(), color.blue(), static_cast<int>(intensity * 60)));
        ray.setColorAt(1, QColor(0, 0, 0, 0));
        p.setPen(QPen(QBrush(ray), 3 * intensity));
        p.drawLine(QPointF(0, 0), QPointF(len * std::cos(a * M_PI / 180), len * std::sin(a * M_PI / 180)));
    }
    p.restore();
}

void lensFlare(QPainter& p, const QPointF& center, double radius, const QColor& color) {
    for (int i = 3; i >= 0; --i) {
        double r = radius * (i + 1) * 0.3;
        int alpha = 40 / (i + 1);
        QRadialGradient grad(center, r);
        grad.setColorAt(0, QColor(255, 255, 255, alpha));
        grad.setColorAt(0.5, QColor(color.red(), color.green(), color.blue(), alpha / 2));
        grad.setColorAt(1, QColor(0, 0, 0, 0));
        p.setPen(Qt::NoPen); p.setBrush(grad);
        p.drawEllipse(center, r, r);
    }
}

void shimmer(QPainter& p, const QRectF& r, double phase, const QColor& color) {
    double x = r.x() + r.width() * (0.5 + 0.5 * std::sin(phase));
    QLinearGradient grad(x - 40, 0, x + 40, 0);
    grad.setColorAt(0, QColor(0, 0, 0, 0));
    grad.setColorAt(0.5, QColor(color.red(), color.green(), color.blue(), 40));
    grad.setColorAt(1, QColor(0, 0, 0, 0));
    p.setPen(Qt::NoPen); p.setBrush(grad);
    p.drawRect(r);
}

void edgeGlow(QPainter& p, const QRectF& r, const QColor& color, double width) {
    for (int i = static_cast<int>(width); i >= 0; --i) {
        int alpha = static_cast<int>(60.0 / (i + 1));
        p.setPen(QPen(QColor(color.red(), color.green(), color.blue(), alpha), 1));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(r.adjusted(-i, -i, i, i), 4 + i, 4 + i);
    }
}

// ══════════════════════════════════════════════════════════════
// DISTORTION (5)
// ══════════════════════════════════════════════════════════════

void glitchRGB(QPainter& p, const QRectF& r, double intensity, int frameCount) {
    if (intensity < 0.01) return;
    double offset = intensity * 6 * std::sin(frameCount * 3.7);
    // Red channel offset
    p.save();
    p.setCompositionMode(QPainter::CompositionMode_Plus);
    p.setOpacity(0.3 * intensity);
    p.fillRect(r.adjusted(offset, 0, offset, 0), QColor(255, 0, 0, 60));
    p.fillRect(r.adjusted(-offset, 0, -offset, 0), QColor(0, 255, 255, 60));
    p.restore();
}

void chromaticAberration(QPainter& p, const QString& text, const QRectF& r, const QFont& font, const QColor& color, double offset) {
    p.setFont(font);
    p.setPen(QColor(255, 0, 0, 80));
    p.drawText(r.adjusted(-offset, 0, 0, 0), Qt::AlignVCenter, text);
    p.setPen(QColor(0, 255, 255, 80));
    p.drawText(r.adjusted(offset, 0, 0, 0), Qt::AlignVCenter, text);
    p.setPen(color);
    p.drawText(r, Qt::AlignVCenter, text);
}

void waveDistort(QPainter& p, const QRectF& r, double phase, double amplitude) {
    Q_UNUSED(p) Q_UNUSED(r) Q_UNUSED(phase) Q_UNUSED(amplitude)
    // Applied via per-line translation in caller
}

void pixelSort(QPainter& p, const QRectF& r, double prog) {
    int lines = 10;
    double lineH = r.height() / lines;
    for (int i = 0; i < lines; ++i) {
        double offset = (1.0 - prog) * pseudoRand(i * 43) * 50;
        p.save();
        p.setClipRect(QRectF(r.x(), r.y() + i * lineH, r.width(), lineH));
        p.translate(offset, 0);
        p.restore();
    }
}

void vhsEffect(QPainter& p, const QRectF& r, double intensity, int frameCount) {
    // Scan lines
    p.setPen(Qt::NoPen);
    for (int y = static_cast<int>(r.y()); y < static_cast<int>(r.bottom()); y += 4) {
        p.fillRect(QRectF(r.x(), y, r.width(), 1), QColor(0, 0, 0, static_cast<int>(intensity * 30)));
    }
    // Occasional horizontal offset
    if (frameCount % 30 < 3) {
        double offset = intensity * 10 * (pseudoRand(frameCount) - 0.5);
        p.translate(offset, 0);
    }
}

// ══════════════════════════════════════════════════════════════
// PARTICLE EFFECTS (7)
// ══════════════════════════════════════════════════════════════

void sparkles(QPainter& p, const QRectF& r, int count, double phase, const QColor& color) {
    p.setPen(Qt::NoPen);
    for (int i = 0; i < count; ++i) {
        double x = r.x() + pseudoRand(i * 37) * r.width();
        double y = r.y() + pseudoRand(i * 71) * r.height();
        double flicker = 0.5 + 0.5 * std::sin(phase * 4 + i * 1.7);
        double sz = (1 + pseudoRand(i * 13) * 3) * flicker;
        int alpha = static_cast<int>(flicker * 200);
        p.setBrush(QColor(color.red(), color.green(), color.blue(), alpha));
        // Star shape (4 thin lines)
        p.drawEllipse(QPointF(x, y), sz, sz * 0.3);
        p.drawEllipse(QPointF(x, y), sz * 0.3, sz);
    }
}

void bokeh(QPainter& p, const QRectF& r, int count, double phase, const QColor& color) {
    p.setPen(Qt::NoPen);
    for (int i = 0; i < count; ++i) {
        double x = r.x() + pseudoRand(i * 41) * r.width();
        double y = r.y() + pseudoRand(i * 67) * r.height();
        double sz = 5 + pseudoRand(i * 29) * 20;
        double drift = std::sin(phase * 0.5 + i * 0.3) * 5;
        int alpha = static_cast<int>((20 + pseudoRand(i * 83) * 40) * (0.6 + 0.4 * std::sin(phase + i)));
        QRadialGradient grad(QPointF(x + drift, y), sz);
        grad.setColorAt(0, QColor(color.red(), color.green(), color.blue(), alpha));
        grad.setColorAt(0.7, QColor(color.red(), color.green(), color.blue(), alpha / 3));
        grad.setColorAt(1, QColor(0, 0, 0, 0));
        p.setBrush(grad);
        p.drawEllipse(QPointF(x + drift, y), sz, sz);
    }
}

void dustParticles(QPainter& p, const QRectF& r, int count, double phase) {
    p.setPen(Qt::NoPen);
    for (int i = 0; i < count; ++i) {
        double x = r.x() + std::fmod(pseudoRand(i * 37) * r.width() + phase * 20 * pseudoRand(i * 11), r.width());
        double y = r.y() + std::fmod(pseudoRand(i * 71) * r.height() + phase * 10, r.height());
        double sz = 1 + pseudoRand(i * 53) * 2;
        int alpha = static_cast<int>(40 + pseudoRand(i * 89) * 60);
        p.setBrush(QColor(200, 200, 180, alpha));
        p.drawEllipse(QPointF(x, y), sz, sz);
    }
}

void fireEmbers(QPainter& p, const QRectF& r, int count, double phase) {
    p.setPen(Qt::NoPen);
    for (int i = 0; i < count; ++i) {
        double x = r.x() + pseudoRand(i * 37) * r.width();
        double baseY = r.bottom();
        double rise = std::fmod(phase * 30 * (0.5 + pseudoRand(i * 11) * 0.5), r.height());
        double y = baseY - rise;
        double drift = std::sin(phase * 2 + i) * 10;
        double sz = 1 + pseudoRand(i * 59) * 3;
        double life = 1.0 - rise / r.height();
        QColor c = (pseudoRand(i * 31) > 0.5) ? QColor(255, 140, 0, static_cast<int>(life * 200)) : QColor(255, 80, 0, static_cast<int>(life * 180));
        p.setBrush(c);
        p.drawEllipse(QPointF(x + drift, y), sz, sz * 0.7);
    }
}

void confetti(QPainter& p, const QRectF& r, int count, double phase) {
    const QColor colors[] = {QColor(255,0,0), QColor(0,200,0), QColor(0,100,255), QColor(255,200,0), QColor(255,0,200), QColor(0,255,255)};
    p.setPen(Qt::NoPen);
    for (int i = 0; i < count; ++i) {
        double x = r.x() + pseudoRand(i * 37) * r.width();
        double fall = std::fmod(phase * 40 * (0.3 + pseudoRand(i * 19) * 0.7), r.height());
        double y = r.y() + fall;
        double rot = phase * 100 + i * 45;
        double w = 3 + pseudoRand(i * 53) * 5, h = 2 + pseudoRand(i * 67) * 3;
        p.save();
        p.translate(x, y);
        p.rotate(rot);
        p.setBrush(colors[i % 6]);
        p.drawRect(QRectF(-w/2, -h/2, w, h));
        p.restore();
    }
}

void snow(QPainter& p, const QRectF& r, int count, double phase) {
    p.setPen(Qt::NoPen);
    for (int i = 0; i < count; ++i) {
        double x = r.x() + std::fmod(pseudoRand(i * 37) * r.width() + std::sin(phase + i * 0.3) * 20, r.width());
        double fall = std::fmod(phase * 15 * (0.3 + pseudoRand(i * 19) * 0.7), r.height());
        double y = r.y() + fall;
        double sz = 1.5 + pseudoRand(i * 53) * 3;
        int alpha = static_cast<int>(120 + pseudoRand(i * 89) * 100);
        p.setBrush(QColor(255, 255, 255, alpha));
        p.drawEllipse(QPointF(x, y), sz, sz);
    }
}

void risingParticles(QPainter& p, const QRectF& r, int count, double phase, const QColor& color) {
    p.setPen(Qt::NoPen);
    for (int i = 0; i < count; ++i) {
        double x = r.x() + pseudoRand(i * 37) * r.width();
        double rise = std::fmod(phase * 20 * (0.4 + pseudoRand(i * 11) * 0.6), r.height());
        double y = r.bottom() - rise;
        double drift = std::sin(phase * 1.5 + i * 0.7) * 8;
        double sz = 1.5 + pseudoRand(i * 59) * 3;
        double life = 1.0 - rise / r.height();
        p.setBrush(QColor(color.red(), color.green(), color.blue(), static_cast<int>(life * 160)));
        p.drawEllipse(QPointF(x + drift, y), sz, sz);
    }
}

// ══════════════════════════════════════════════════════════════
// SHAPE/LINE ANIMATIONS (6)
// ══════════════════════════════════════════════════════════════

void lineDrawOn(QPainter& p, const QPointF& from, const QPointF& to, double prog, const QColor& color, double width) {
    QPointF current(from.x() + (to.x() - from.x()) * easeOutCubic(prog),
                    from.y() + (to.y() - from.y()) * easeOutCubic(prog));
    p.setPen(QPen(color, width, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(from, current);
}

void circleExpand(QPainter& p, const QPointF& center, double maxRadius, double prog, const QColor& color) {
    double r = maxRadius * easeOutCubic(prog);
    int alpha = static_cast<int>((1.0 - prog * 0.5) * 255);
    p.setPen(QPen(QColor(color.red(), color.green(), color.blue(), alpha), 2));
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(center, r, r);
}

void rectangleBuild(QPainter& p, const QRectF& r, double prog, const QColor& color, double lineWidth) {
    double perim = 2 * (r.width() + r.height());
    double drawn = perim * easeOutCubic(prog);
    QPainterPath path;
    path.moveTo(r.topLeft());
    double remaining = drawn;
    // Top
    double seg = qMin(remaining, r.width()); path.lineTo(r.x() + seg, r.y()); remaining -= seg;
    // Right
    if (remaining > 0) { seg = qMin(remaining, r.height()); path.lineTo(r.right(), r.y() + seg); remaining -= seg; }
    // Bottom
    if (remaining > 0) { seg = qMin(remaining, r.width()); path.lineTo(r.right() - seg, r.bottom()); remaining -= seg; }
    // Left
    if (remaining > 0) { seg = qMin(remaining, r.height()); path.lineTo(r.x(), r.bottom() - seg); }
    p.setPen(QPen(color, lineWidth, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
    p.setBrush(Qt::NoBrush);
    p.drawPath(path);
}

void pathTrace(QPainter& p, const std::vector<QPointF>& points, double prog, const QColor& color, double width) {
    if (points.size() < 2) return;
    int visiblePoints = static_cast<int>(points.size() * easeOutCubic(prog));
    QPainterPath path;
    path.moveTo(points[0]);
    for (int i = 1; i <= visiblePoints && i < static_cast<int>(points.size()); ++i)
        path.lineTo(points[i]);
    p.setPen(QPen(color, width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.setBrush(Qt::NoBrush);
    p.drawPath(path);
}

void gridReveal(QPainter& p, const QRectF& r, double prog, const QColor& color, int cols, int rows) {
    double cellW = r.width() / cols, cellH = r.height() / rows;
    p.setPen(Qt::NoPen);
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            double delay = (row + col) * 0.05;
            double t = clamp01((prog - delay) * 3);
            int alpha = static_cast<int>(t * 40);
            p.setBrush(QColor(color.red(), color.green(), color.blue(), alpha));
            p.drawRect(QRectF(r.x() + col * cellW, r.y() + row * cellH, cellW * t, cellH * t));
        }
    }
}

void hexagonPattern(QPainter& p, const QRectF& r, double prog, const QColor& color, double hexSize) {
    p.setPen(QPen(QColor(color.red(), color.green(), color.blue(), static_cast<int>(prog * 80)), 1));
    p.setBrush(Qt::NoBrush);
    for (double y = r.y(); y < r.bottom(); y += hexSize * 1.5) {
        double offset = (static_cast<int>(y / (hexSize * 1.5)) % 2) * hexSize * 0.866;
        for (double x = r.x() + offset; x < r.right(); x += hexSize * 1.732) {
            double delay = ((x - r.x()) + (y - r.y())) / (r.width() + r.height());
            double t = clamp01((prog - delay * 0.5) * 2);
            if (t <= 0) continue;
            QPolygonF hex;
            for (int k = 0; k < 6; ++k) {
                double angle = M_PI / 3.0 * k + M_PI / 6.0;
                hex << QPointF(x + hexSize * 0.4 * t * std::cos(angle), y + hexSize * 0.4 * t * std::sin(angle));
            }
            p.drawPolygon(hex);
        }
    }
}

// ══════════════════════════════════════════════════════════════
// BLUR/FOCUS (5)
// ══════════════════════════════════════════════════════════════

void gaussianBlurIn(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    blurIn(p, text, r, prog, font, color); // Reuse blurIn with same quality
}

void radialBlur(QPainter& p, const QRectF& r, double prog, const QColor& color) {
    int rings = static_cast<int>((1.0 - prog) * 5);
    for (int i = rings; i >= 0; --i) {
        double spread = i * 3;
        int alpha = (i == 0) ? static_cast<int>(prog * 255) : static_cast<int>(20.0 / (i + 1));
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(color.red(), color.green(), color.blue(), alpha));
        p.drawRoundedRect(r.adjusted(-spread, -spread, spread, spread), 4, 4);
    }
}

void directionalBlur(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color, double angle) {
    int copies = static_cast<int>((1.0 - prog) * 8);
    double dx = std::cos(angle * M_PI / 180.0);
    double dy = std::sin(angle * M_PI / 180.0);
    p.setFont(font);
    for (int i = copies; i >= 0; --i) {
        double offset = i * 3;
        int alpha = (i == 0) ? 255 : static_cast<int>(40.0 / (i + 1));
        p.setPen(QColor(color.red(), color.green(), color.blue(), alpha));
        p.drawText(r.adjusted(dx * offset, dy * offset, 0, 0), Qt::AlignVCenter, text);
    }
}

void tiltShift(QPainter& p, const QRectF& r, double focusY, double range) {
    Q_UNUSED(p) Q_UNUSED(r) Q_UNUSED(focusY) Q_UNUSED(range)
    // Simulated via opacity gradient — applied by caller
}

void defocus(QPainter& p, const QRectF& r, double prog, const QColor& color) {
    radialBlur(p, r, prog, color); // Same visual as radialBlur
}

// ══════════════════════════════════════════════════════════════
// COLOR/STYLE (5)
// ══════════════════════════════════════════════════════════════

void colorSweep(QPainter& p, const QRectF& r, double prog, const QColor& from, const QColor& to) {
    double x = r.x() + r.width() * easeOutCubic(prog);
    QLinearGradient grad(x - 20, 0, x + 20, 0);
    grad.setColorAt(0, from);
    grad.setColorAt(1, to);
    p.setPen(Qt::NoPen); p.setBrush(grad);
    p.drawRect(r);
}

void gradientShift(QPainter& p, const QRectF& r, double phase, const QColor& c1, const QColor& c2) {
    double shift = 0.5 + 0.5 * std::sin(phase);
    QLinearGradient grad(r.topLeft(), r.bottomRight());
    grad.setColorAt(0, c1);
    grad.setColorAt(shift, c2);
    grad.setColorAt(1, c1);
    p.setPen(Qt::NoPen); p.setBrush(grad);
    p.drawRect(r);
}

void duotone(QPainter& p, const QRectF& r, const QColor& shadow, const QColor& highlight) {
    QLinearGradient grad(r.topLeft(), r.bottomRight());
    grad.setColorAt(0, shadow);
    grad.setColorAt(1, highlight);
    p.setCompositionMode(QPainter::CompositionMode_Overlay);
    p.setPen(Qt::NoPen); p.setBrush(grad);
    p.drawRect(r);
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
}

void shadowDropAnimate(QPainter& p, const QRectF& r, double prog, const QColor& shadow) {
    double offset = easeOutCubic(prog) * 8;
    for (int i = 3; i >= 0; --i) {
        double spread = i * 2;
        int alpha = static_cast<int>(prog * 40.0 / (i + 1));
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(shadow.red(), shadow.green(), shadow.blue(), alpha));
        p.drawRoundedRect(r.adjusted(-spread + offset, -spread + offset, spread + offset, spread + offset), 4, 4);
    }
}

void outlineStroke(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color, double width) {
    p.setFont(font);
    double w = width * easeOutCubic(prog);
    // Draw outline by rendering text offset in 8 directions
    QColor outlineColor(color.red(), color.green(), color.blue(), static_cast<int>(prog * 200));
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx == 0 && dy == 0) continue;
            p.setPen(outlineColor);
            p.drawText(r.adjusted(dx * w, dy * w, 0, 0), Qt::AlignVCenter, text);
        }
    }
    // Core text
    p.setPen(QColor(0, 0, 0, static_cast<int>(prog * 255)));
    p.drawText(r, Qt::AlignVCenter, text);
}

}} // namespace prestige::fx

// ══════════════════════════════════════════════════════════════
// Effect ID registry
// ══════════════════════════════════════════════════════════════

namespace prestige {

QStringList allEffectIds() {
    return {
        // Text (10)
        "typewriter", "bounce_in", "wave_text", "tracking_expand", "fade_up_letter",
        "scale_up_letter", "rotate_in_letter", "blur_in", "slide_per_letter", "kinetic_pop",
        // Lower Third (8)
        "line_draw", "bar_slide", "shape_morph", "split_reveal",
        "bracket_expand", "underline_grow", "box_wipe", "corner_build",
        // Logo Reveals (8)
        "fade_glow", "light_streak", "particle_form", "scale_bounce",
        "shatter_in", "blur_zoom", "rotate_3d", "pulse_reveal",
        // Transitions (8)
        "wipe_linear", "push_slide", "zoom_through", "glitch_transition",
        "light_leak", "ink_bleed", "spin_transition", "cross_dissolve",
        // Glow (6)
        "neon_glow", "bloom", "light_rays", "lens_flare", "shimmer", "edge_glow",
        // Distortion (5)
        "glitch_rgb", "chromatic_aberration", "wave_distort", "pixel_sort", "vhs_effect",
        // Particles (7)
        "sparkles", "bokeh", "dust", "fire_embers", "confetti", "snow", "rising_particles",
        // Shape (6)
        "line_draw_on", "circle_expand", "rectangle_build", "path_trace", "grid_reveal", "hexagon_pattern",
        // Blur (5)
        "gaussian_blur_in", "radial_blur", "directional_blur", "tilt_shift", "defocus",
        // Color (5)
        "color_sweep", "gradient_shift", "duotone", "shadow_drop_animate", "outline_stroke"
    };
}

QStringList allEffectNames() {
    return {
        "Typewriter", "Bounce In", "Wave Text", "Tracking Expand", "Fade Up Letter",
        "Scale Up Letter", "Rotate In Letter", "Blur In", "Slide Per Letter", "Kinetic Pop",
        "Line Draw", "Bar Slide", "Shape Morph", "Split Reveal",
        "Bracket Expand", "Underline Grow", "Box Wipe", "Corner Build",
        "Fade Glow", "Light Streak", "Particle Form", "Scale Bounce",
        "Shatter In", "Blur Zoom", "Rotate 3D", "Pulse Reveal",
        "Wipe Linear", "Push Slide", "Zoom Through", "Glitch Transition",
        "Light Leak", "Ink Bleed", "Spin Transition", "Cross Dissolve",
        "Neon Glow", "Bloom", "Light Rays", "Lens Flare", "Shimmer", "Edge Glow",
        "Glitch RGB", "Chromatic Aberration", "Wave Distort", "Pixel Sort", "VHS Effect",
        "Sparkles", "Bokeh", "Dust", "Fire Embers", "Confetti", "Snow", "Rising Particles",
        "Line Draw On", "Circle Expand", "Rectangle Build", "Path Trace", "Grid Reveal", "Hexagon Pattern",
        "Gaussian Blur In", "Radial Blur", "Directional Blur", "Tilt Shift", "Defocus",
        "Color Sweep", "Gradient Shift", "Duotone", "Shadow Drop Animate", "Outline Stroke"
    };
}

QString effectCategoryForId(const QString& id) {
    static const QStringList text = {"typewriter","bounce_in","wave_text","tracking_expand","fade_up_letter","scale_up_letter","rotate_in_letter","blur_in","slide_per_letter","kinetic_pop"};
    static const QStringList lt = {"line_draw","bar_slide","shape_morph","split_reveal","bracket_expand","underline_grow","box_wipe","corner_build"};
    static const QStringList logo = {"fade_glow","light_streak","particle_form","scale_bounce","shatter_in","blur_zoom","rotate_3d","pulse_reveal"};
    static const QStringList trans = {"wipe_linear","push_slide","zoom_through","glitch_transition","light_leak","ink_bleed","spin_transition","cross_dissolve"};
    static const QStringList glow = {"neon_glow","bloom","light_rays","lens_flare","shimmer","edge_glow"};
    static const QStringList dist = {"glitch_rgb","chromatic_aberration","wave_distort","pixel_sort","vhs_effect"};
    static const QStringList part = {"sparkles","bokeh","dust","fire_embers","confetti","snow","rising_particles"};
    if (text.contains(id)) return "Text";
    if (lt.contains(id)) return "Lower Third";
    if (logo.contains(id)) return "Logo";
    if (trans.contains(id)) return "Transition";
    if (glow.contains(id)) return "Glow";
    if (dist.contains(id)) return "Distortion";
    if (part.contains(id)) return "Particles";
    return "Other";
}

} // namespace prestige
