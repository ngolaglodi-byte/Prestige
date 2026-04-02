// ============================================================
// Prestige AI — AE Extended Broadcast Effects
// 55 new effects: 25 text animators, 20 transitions, 10 shape ops
// ============================================================

#include "BroadcastEffects.h"
#include "AEEasing.h"
#include <QFontMetrics>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QPainterPath>
#include <QPen>
#include <QtMath>
#include <algorithm>

namespace prestige { namespace fx {

// ── Helpers ─────────────────────────────────────────────────
static double clamp01(double v) { return std::max(0.0, std::min(1.0, v)); }
static double pseudoRand(int seed) { return ((seed * 1103515245 + 12345) & 0x7fffffff) / static_cast<double>(0x7fffffff); }

// ══════════════════════════════════════════════════════════════
// AE EXTENDED TEXT ANIMATIONS (25)
// ══════════════════════════════════════════════════════════════

void matrixRain(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    QFontMetrics fm(font); p.setFont(font);
    int x = static_cast<int>(r.x());
    for (int i = 0; i < text.length(); ++i) {
        double delay = pseudoRand(i * 31) * 0.5;
        double t = clamp01((prog - delay) / (1.0 - delay));
        double fallY = (1.0 - t) * -r.height() * 2.0;
        p.save();
        QColor c = color; c.setAlphaF(clamp01(t * 2));
        p.setPen(c);
        p.drawText(x, static_cast<int>(r.y() + r.height() * 0.75 + fallY), QString(text[i]));
        // Trail characters (matrix style)
        if (t < 0.8) {
            for (int j = 1; j <= 3; ++j) {
                QColor trail = color; trail.setAlphaF(clamp01(t) * (0.3 / j));
                p.setPen(trail);
                QChar randomChar(0x30A0 + static_cast<int>(pseudoRand(i * 17 + j * 7) * 96));
                p.drawText(x, static_cast<int>(r.y() + r.height() * 0.75 + fallY - j * fm.height() * 0.8), QString(randomChar));
            }
        }
        p.restore();
        x += fm.horizontalAdvance(text[i]);
    }
}

void cascadeReveal(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    QFontMetrics fm(font); p.setFont(font);
    int x = static_cast<int>(r.x());
    for (int i = 0; i < text.length(); ++i) {
        double delay = i * 0.04;
        double t = clamp01((prog - delay) / 0.3);
        double ease = ae::easeOutExpo(t);
        double scaleY = ease;
        p.save();
        p.setOpacity(clamp01(t * 5));
        p.translate(x + fm.horizontalAdvance(text[i]) / 2.0, r.y() + r.height());
        p.scale(1.0, scaleY);
        p.setPen(color);
        p.drawText(QPointF(-fm.horizontalAdvance(text[i]) / 2.0, -r.height() * 0.25), QString(text[i]));
        p.restore();
        x += fm.horizontalAdvance(text[i]);
    }
}

void elasticDrop(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    QFontMetrics fm(font); p.setFont(font); p.setPen(color);
    int x = static_cast<int>(r.x());
    for (int i = 0; i < text.length(); ++i) {
        double delay = i * 0.05;
        double t = clamp01((prog - delay) / (1.0 - text.length() * 0.025));
        double drop = ae::easeOutElastic(t);
        double yOff = (1.0 - drop) * -r.height() * 1.5;
        p.save();
        p.setOpacity(clamp01(t * 4));
        p.drawText(x, static_cast<int>(r.y() + r.height() * 0.75 + yOff), QString(text[i]));
        p.restore();
        x += fm.horizontalAdvance(text[i]);
    }
}

void spiralIn(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    QFontMetrics fm(font); p.setFont(font); p.setPen(color);
    int x = static_cast<int>(r.x());
    double cx = r.x() + r.width() / 2.0, cy = r.y() + r.height() / 2.0;
    for (int i = 0; i < text.length(); ++i) {
        double delay = i * 0.04;
        double t = clamp01((prog - delay) / 0.4);
        double ease = ae::easeOutCubic(t);
        double angle = (1.0 - ease) * M_PI * 4;
        double dist = (1.0 - ease) * r.width() * 0.5;
        double lx = x + std::cos(angle) * dist;
        double ly = r.y() + r.height() * 0.75 + std::sin(angle) * dist;
        double finalX = x;
        double finalY = r.y() + r.height() * 0.75;
        double px = finalX + (lx - finalX) * (1.0 - ease);
        double py = finalY + (ly - finalY) * (1.0 - ease);
        p.save();
        p.setOpacity(clamp01(t * 3));
        p.drawText(static_cast<int>(px), static_cast<int>(py), QString(text[i]));
        p.restore();
        x += fm.horizontalAdvance(text[i]);
    }
}

void flipBoard(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    QFontMetrics fm(font); p.setFont(font);
    int x = static_cast<int>(r.x());
    for (int i = 0; i < text.length(); ++i) {
        double delay = i * 0.06;
        double t = clamp01((prog - delay) / 0.25);
        double flip = ae::easeOutBack(t);
        p.save();
        p.setOpacity(clamp01(t * 5));
        int charX = x + fm.horizontalAdvance(text[i]) / 2;
        int charY = static_cast<int>(r.y() + r.height() * 0.5);
        p.translate(charX, charY);
        p.scale(1.0, flip); // Vertical flip effect
        p.setPen(color);
        // Draw background card
        QRectF card(-fm.horizontalAdvance(text[i]) * 0.6, -fm.height() * 0.6,
                    fm.horizontalAdvance(text[i]) * 1.2, fm.height() * 1.2);
        p.fillRect(card, QColor(0, 0, 0, 120));
        p.drawText(QPointF(-fm.horizontalAdvance(text[i]) / 2.0, fm.height() * 0.3), QString(text[i]));
        p.restore();
        x += fm.horizontalAdvance(text[i]);
    }
}

void glowReveal(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    QFontMetrics fm(font); p.setFont(font);
    int x = static_cast<int>(r.x());
    for (int i = 0; i < text.length(); ++i) {
        double delay = i * 0.04;
        double t = clamp01((prog - delay) / 0.35);
        // Glow phase first, then character appears
        if (t > 0.01) {
            double glowT = clamp01(t * 2);
            double charT = clamp01((t - 0.3) / 0.7);
            // Draw glow
            p.save();
            QColor glow = color; glow.setAlphaF(glowT * (1.0 - charT) * 0.5);
            p.setPen(glow);
            for (int g = 3; g >= 1; --g) {
                QFont glowFont = font; glowFont.setPixelSize(font.pixelSize() + g * 2);
                p.setFont(glowFont);
                p.drawText(x - g, static_cast<int>(r.y() + r.height() * 0.75), QString(text[i]));
            }
            // Draw char
            p.setFont(font);
            p.setOpacity(ae::easeOutCubic(charT));
            p.setPen(color);
            p.drawText(x, static_cast<int>(r.y() + r.height() * 0.75), QString(text[i]));
            p.restore();
        }
        x += fm.horizontalAdvance(text[i]);
    }
}

void smokeIn(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    QFontMetrics fm(font); p.setFont(font);
    int x = static_cast<int>(r.x());
    for (int i = 0; i < text.length(); ++i) {
        double delay = i * 0.05;
        double t = clamp01((prog - delay) / 0.4);
        double ease = ae::easeOutCubic(t);
        double blur = (1.0 - ease) * 8;
        double yOff = (1.0 - ease) * r.height() * 0.3;
        p.save();
        p.setOpacity(ease);
        p.setPen(color);
        // Multiple offset copies for blur effect
        int passes = static_cast<int>(blur);
        for (int b = -passes; b <= passes; ++b) {
            double ba = 1.0 / (1.0 + std::abs(b));
            p.setOpacity(ease * ba * 0.3);
            p.drawText(x + b, static_cast<int>(r.y() + r.height() * 0.75 + yOff), QString(text[i]));
        }
        p.setOpacity(ease);
        p.drawText(x, static_cast<int>(r.y() + r.height() * 0.75 + yOff), QString(text[i]));
        p.restore();
        x += fm.horizontalAdvance(text[i]);
    }
}

void scatterAssemble(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    QFontMetrics fm(font); p.setFont(font); p.setPen(color);
    int x = static_cast<int>(r.x());
    for (int i = 0; i < text.length(); ++i) {
        double delay = i * 0.03;
        double t = clamp01((prog - delay) / 0.5);
        double ease = ae::easeOutCubic(t);
        double scatterX = (pseudoRand(i * 41) - 0.5) * r.width() * (1.0 - ease);
        double scatterY = (pseudoRand(i * 73) - 0.5) * r.height() * 2 * (1.0 - ease);
        double rotation = (pseudoRand(i * 59) - 0.5) * 360 * (1.0 - ease);
        p.save();
        p.setOpacity(clamp01(t * 3));
        p.translate(x + fm.horizontalAdvance(text[i]) / 2.0 + scatterX,
                    r.y() + r.height() * 0.75 + scatterY);
        p.rotate(rotation);
        p.drawText(QPointF(-fm.horizontalAdvance(text[i]) / 2.0, 0), QString(text[i]));
        p.restore();
        x += fm.horizontalAdvance(text[i]);
    }
}

void slotMachine(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    QFontMetrics fm(font); p.setFont(font); p.setPen(color);
    int x = static_cast<int>(r.x());
    for (int i = 0; i < text.length(); ++i) {
        double delay = i * 0.08;
        double t = clamp01((prog - delay) / 0.3);
        double ease = ae::easeOutBounce(t);
        // Spin through characters
        int spins = static_cast<int>((1.0 - t) * 8);
        QChar displayChar = (t < 0.95 && spins > 0) ?
            QChar('A' + (static_cast<int>(pseudoRand(i * 31 + spins) * 26)) % 26) : text[i];
        double yOff = (1.0 - ease) * fm.height() * 3;
        p.save();
        p.setOpacity(clamp01(t * 5));
        p.drawText(x, static_cast<int>(r.y() + r.height() * 0.75 + std::fmod(yOff, fm.height())), QString(displayChar));
        p.restore();
        x += fm.horizontalAdvance(text[i]);
    }
}

void stampPress(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    QFontMetrics fm(font); p.setFont(font); p.setPen(color);
    int x = static_cast<int>(r.x());
    for (int i = 0; i < text.length(); ++i) {
        double delay = i * 0.06;
        double t = clamp01((prog - delay) / 0.2);
        double scale = t < 0.5 ? 3.0 - t * 4.0 : 1.0; // Starts big, stamps down
        double alpha = t < 0.5 ? t * 2 : 1.0;
        p.save();
        p.setOpacity(alpha);
        int cx = x + fm.horizontalAdvance(text[i]) / 2;
        int cy = static_cast<int>(r.y() + r.height() * 0.5);
        p.translate(cx, cy);
        p.scale(scale, scale);
        p.drawText(QPointF(-fm.horizontalAdvance(text[i]) / 2.0, fm.height() * 0.3), QString(text[i]));
        p.restore();
        x += fm.horizontalAdvance(text[i]);
    }
}

void wipePerLetter(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    QFontMetrics fm(font); p.setFont(font); p.setPen(color);
    int x = static_cast<int>(r.x());
    for (int i = 0; i < text.length(); ++i) {
        double delay = i * 0.04;
        double t = clamp01((prog - delay) / 0.25);
        int charW = fm.horizontalAdvance(text[i]);
        p.save();
        p.setClipRect(x, static_cast<int>(r.y()), static_cast<int>(charW * t), static_cast<int>(r.height()));
        p.drawText(x, static_cast<int>(r.y() + r.height() * 0.75), QString(text[i]));
        p.restore();
        x += charW;
    }
}

void swingDrop(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    QFontMetrics fm(font); p.setFont(font); p.setPen(color);
    int x = static_cast<int>(r.x());
    for (int i = 0; i < text.length(); ++i) {
        double delay = i * 0.05;
        double t = clamp01((prog - delay) / 0.5);
        double swing = ae::easeOutElastic(t);
        double angle = (1.0 - swing) * 45.0 * (i % 2 == 0 ? 1 : -1);
        p.save();
        p.setOpacity(clamp01(t * 3));
        p.translate(x + fm.horizontalAdvance(text[i]) / 2.0, r.y());
        p.rotate(angle);
        p.drawText(QPointF(-fm.horizontalAdvance(text[i]) / 2.0, r.height() * 0.75), QString(text[i]));
        p.restore();
        x += fm.horizontalAdvance(text[i]);
    }
}

void neonFlicker(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    QFontMetrics fm(font); p.setFont(font);
    int x = static_cast<int>(r.x());
    for (int i = 0; i < text.length(); ++i) {
        double delay = i * 0.06;
        double t = clamp01((prog - delay) / 0.4);
        // Flicker: random on/off during early phase
        double flicker = (t < 0.6) ? (pseudoRand(static_cast<int>(t * 20) + i * 7) > 0.4 ? 1.0 : 0.2) : 1.0;
        p.save();
        p.setOpacity(t * flicker);
        // Neon glow layers
        for (int g = 2; g >= 0; --g) {
            QColor glow = color; glow.setAlphaF(0.15 * flicker);
            p.setPen(glow);
            p.drawText(x - g, static_cast<int>(r.y() + r.height() * 0.75 - g), QString(text[i]));
            p.drawText(x + g, static_cast<int>(r.y() + r.height() * 0.75 + g), QString(text[i]));
        }
        p.setPen(color);
        p.drawText(x, static_cast<int>(r.y() + r.height() * 0.75), QString(text[i]));
        p.restore();
        x += fm.horizontalAdvance(text[i]);
    }
}

void gravityCrush(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    QFontMetrics fm(font); p.setFont(font); p.setPen(color);
    int x = static_cast<int>(r.x());
    for (int i = 0; i < text.length(); ++i) {
        double delay = i * 0.04;
        double t = clamp01((prog - delay) / 0.35);
        double bounce = ae::easeOutBounce(t);
        double yOff = (1.0 - bounce) * -r.height() * 3;
        double squash = t > 0.85 ? 1.0 + (1.0 - t) * 0.3 : 1.0; // Slight squash on land
        p.save();
        p.setOpacity(clamp01(t * 4));
        p.translate(x + fm.horizontalAdvance(text[i]) / 2.0, r.y() + r.height() * 0.75 + yOff);
        p.scale(1.0 / squash, squash);
        p.drawText(QPointF(-fm.horizontalAdvance(text[i]) / 2.0, 0), QString(text[i]));
        p.restore();
        x += fm.horizontalAdvance(text[i]);
    }
}

void rubberStretch(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    QFontMetrics fm(font); p.setFont(font); p.setPen(color);
    int x = static_cast<int>(r.x());
    for (int i = 0; i < text.length(); ++i) {
        double delay = i * 0.05;
        double t = clamp01((prog - delay) / 0.4);
        double elastic = ae::easeOutElastic(t);
        double scaleX = 0.3 + elastic * 0.7;
        double scaleY = 1.0 + (1.0 - elastic) * 0.5;
        p.save();
        p.setOpacity(clamp01(t * 3));
        p.translate(x + fm.horizontalAdvance(text[i]) / 2.0, r.y() + r.height() * 0.5);
        p.scale(scaleX, scaleY);
        p.drawText(QPointF(-fm.horizontalAdvance(text[i]) / 2.0, fm.height() * 0.3), QString(text[i]));
        p.restore();
        x += fm.horizontalAdvance(text[i]);
    }
}

void fadeInRandom(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    QFontMetrics fm(font); p.setFont(font); p.setPen(color);
    int x = static_cast<int>(r.x());
    for (int i = 0; i < text.length(); ++i) {
        double randomDelay = pseudoRand(i * 47) * 0.6;
        double t = clamp01((prog - randomDelay) / 0.3);
        p.save();
        p.setOpacity(ae::easeOutCubic(t));
        p.drawText(x, static_cast<int>(r.y() + r.height() * 0.75), QString(text[i]));
        p.restore();
        x += fm.horizontalAdvance(text[i]);
    }
}

void curvedPath(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    QFontMetrics fm(font); p.setFont(font); p.setPen(color);
    int x = static_cast<int>(r.x());
    for (int i = 0; i < text.length(); ++i) {
        double t = clamp01(prog);
        double pos = static_cast<double>(i) / std::max(static_cast<qsizetype>(1), text.length() - 1);
        double curveY = std::sin(pos * M_PI) * r.height() * 0.3 * (1.0 - t);
        p.save();
        p.setOpacity(clamp01(prog * 2 - i * 0.05));
        p.drawText(x, static_cast<int>(r.y() + r.height() * 0.75 + curveY), QString(text[i]));
        p.restore();
        x += fm.horizontalAdvance(text[i]);
    }
}

void zoomBurst(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    p.setFont(font); p.setPen(color);
    double scale = prog < 0.3 ? ae::easeOutBack(prog / 0.3) * 2.0 : 1.0 + (1.0 - ae::easeOutCubic((prog - 0.3) / 0.7));
    p.save();
    p.setOpacity(clamp01(prog * 4));
    p.translate(r.center());
    p.scale(scale, scale);
    p.drawText(QRectF(-r.width() / 2, -r.height() / 2, r.width(), r.height()),
               Qt::AlignVCenter | Qt::AlignLeft, text);
    p.restore();
}

void jitterShake(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    QFontMetrics fm(font); p.setFont(font); p.setPen(color);
    int x = static_cast<int>(r.x());
    for (int i = 0; i < text.length(); ++i) {
        double t = clamp01(prog);
        double jitterAmt = (1.0 - t) * 5.0;
        double jx = ae::wiggle(prog * 10, 8, jitterAmt, i * 3);
        double jy = ae::wiggle(prog * 10, 6, jitterAmt, i * 7 + 100);
        p.save();
        p.setOpacity(clamp01(prog * 3 - i * 0.03));
        p.drawText(x + static_cast<int>(jx), static_cast<int>(r.y() + r.height() * 0.75 + jy), QString(text[i]));
        p.restore();
        x += fm.horizontalAdvance(text[i]);
    }
}

void shadowExpand(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    p.setFont(font);
    double t = ae::easeOutCubic(prog);
    // Expanding shadow
    for (int s = 5; s >= 1; --s) {
        double offset = s * (1.0 - t) * 3;
        QColor shadow = QColor(0, 0, 0, static_cast<int>(30 * t));
        p.setPen(shadow);
        p.drawText(QRectF(r.x() + offset, r.y() + offset, r.width(), r.height()),
                   Qt::AlignVCenter | Qt::AlignLeft, text);
    }
    p.setPen(color);
    p.setOpacity(t);
    p.drawText(r, Qt::AlignVCenter | Qt::AlignLeft, text);
    p.setOpacity(1.0);
}

void rangeWipe(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    QFontMetrics fm(font); p.setFont(font); p.setPen(color);
    // AE Range Selector: start/end wipe across text
    double rangeStart = std::max(0.0, prog * 1.5 - 0.5);
    double rangeEnd = std::min(1.0, prog * 1.5);
    int x = static_cast<int>(r.x());
    for (int i = 0; i < text.length(); ++i) {
        double pos = static_cast<double>(i) / std::max(static_cast<qsizetype>(1), text.length() - 1);
        double alpha = (pos >= rangeStart && pos <= rangeEnd) ? 1.0 : 0.0;
        double feather = 0.1;
        if (pos < rangeStart) alpha = clamp01(1.0 - (rangeStart - pos) / feather);
        if (pos > rangeEnd) alpha = clamp01(1.0 - (pos - rangeEnd) / feather);
        p.save();
        p.setOpacity(alpha);
        p.drawText(x, static_cast<int>(r.y() + r.height() * 0.75), QString(text[i]));
        p.restore();
        x += fm.horizontalAdvance(text[i]);
    }
}

void textShatter(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    // AE Shatter: Voronoi fragmentation with per-piece physics
    QFontMetrics fm(font); p.setFont(font);
    if (prog < 0.1) {
        // Intact text
        p.setPen(color);
        p.drawText(r, Qt::AlignVCenter | Qt::AlignLeft, text);
        return;
    }
    // Explosion phase: each character becomes a fragment
    int x = static_cast<int>(r.x());
    double explodeProg = clamp01((prog - 0.1) / 0.9);
    for (int i = 0; i < text.length(); ++i) {
        // Per-fragment physics
        double seed1 = pseudoRand(i * 53 + 7);
        double seed2 = pseudoRand(i * 37 + 13);
        double seed3 = pseudoRand(i * 71 + 29);
        // Radial explosion direction from center of text
        double cx = r.x() + r.width() / 2.0;
        double fragX = x - cx;
        double dirX = fragX * 2.0 + (seed1 - 0.5) * r.width();
        double dirY = (seed2 - 0.5) * r.height() * 3.0;
        // Gravity pulls down
        double gravity = explodeProg * explodeProg * r.height() * 2.0;
        double px = x + dirX * explodeProg;
        double py = r.y() + r.height() * 0.75 + dirY * explodeProg + gravity;
        // Rotation per fragment
        double rotation = (seed3 - 0.5) * 720 * explodeProg;
        // Fade out
        double alpha = 1.0 - explodeProg;
        // Scale shrinks
        double scale = 1.0 - explodeProg * 0.5;
        p.save();
        p.setOpacity(clamp01(alpha));
        p.translate(px + fm.horizontalAdvance(text[i]) / 2.0, py);
        p.rotate(rotation);
        p.scale(scale, scale);
        p.setPen(color);
        p.drawText(QPointF(-fm.horizontalAdvance(text[i]) / 2.0, 0), QString(text[i]));
        p.restore();
        x += fm.horizontalAdvance(text[i]);
    }
}

void reflectReveal(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    p.setFont(font);
    double t = ae::easeOutCubic(prog);
    // Main text
    p.setPen(color);
    p.setOpacity(t);
    p.drawText(r, Qt::AlignVCenter | Qt::AlignLeft, text);
    // Reflection
    p.save();
    p.translate(0, r.height() * 0.8);
    p.scale(1, -0.5);
    QColor reflColor = color; reflColor.setAlphaF(0.3 * t);
    p.setPen(reflColor);
    p.drawText(r, Qt::AlignVCenter | Qt::AlignLeft, text);
    p.restore();
    p.setOpacity(1.0);
}

void perspectiveTilt(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    p.setFont(font); p.setPen(color);
    double t = ae::easeOutCubic(prog);
    p.save();
    p.setOpacity(t);
    p.translate(r.center());
    // Simulate perspective via non-uniform scale
    double tilt = (1.0 - t) * 0.5;
    p.scale(1.0 - tilt, 1.0);
    p.translate(-r.center());
    p.drawText(r, Qt::AlignVCenter | Qt::AlignLeft, text);
    p.restore();
}

void liquidFill(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color) {
    QFontMetrics fm(font); p.setFont(font);
    double fillLevel = ae::easeOutCubic(prog);
    double clipY = r.y() + r.height() * (1.0 - fillLevel);
    // Draw dimmed full text
    QColor dim = color; dim.setAlphaF(0.15);
    p.setPen(dim);
    p.drawText(r, Qt::AlignVCenter | Qt::AlignLeft, text);
    // Draw filled text (clipped to fill level)
    p.save();
    p.setClipRect(r.x(), clipY, r.width(), r.height() * fillLevel + 1);
    p.setPen(color);
    p.drawText(r, Qt::AlignVCenter | Qt::AlignLeft, text);
    p.restore();
}

// ══════════════════════════════════════════════════════════════
// AE EXTENDED TRANSITIONS (20)
// ══════════════════════════════════════════════════════════════

void cardWipe(QPainter& p, const QSize& size, double prog, int rows, int cols) {
    int cellW = size.width() / std::max(cols, 1);
    int cellH = size.height() / std::max(rows, 1);
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            double delay = (r + c) * 0.05;
            double t = clamp01((prog - delay) / 0.3);
            if (t > 0.01) {
                p.save();
                p.setOpacity(1.0 - ae::easeOutCubic(t));
                p.fillRect(c * cellW, r * cellH, cellW, cellH, Qt::black);
                p.restore();
            }
        }
    }
}

void venetianBlinds(QPainter& p, const QSize& size, double prog, int blades, bool horizontal) {
    double bladeSize = horizontal ? size.height() / static_cast<double>(blades) : size.width() / static_cast<double>(blades);
    double reveal = ae::easeOutCubic(prog);
    for (int i = 0; i < blades; ++i) {
        double pos = i * bladeSize;
        double clipSize = bladeSize * reveal;
        p.save();
        if (horizontal)
            p.setClipRect(0, static_cast<int>(pos), size.width(), static_cast<int>(clipSize));
        else
            p.setClipRect(static_cast<int>(pos), 0, static_cast<int>(clipSize), size.height());
        p.fillRect(0, 0, size.width(), size.height(), QColor(0, 0, 0, static_cast<int>(255 * (1.0 - prog))));
        p.restore();
    }
}

void radialWipe(QPainter& p, const QSize& size, double prog, bool clockwise) {
    double angle = prog * 360.0;
    QPainterPath path;
    QPointF center(size.width() / 2.0, size.height() / 2.0);
    double radius = std::sqrt(size.width() * size.width() + size.height() * size.height());
    path.moveTo(center);
    double startAngle = clockwise ? 90 : 90 - angle;
    path.arcTo(center.x() - radius, center.y() - radius, radius * 2, radius * 2,
               startAngle, clockwise ? -angle : angle);
    path.closeSubpath();
    p.setClipPath(path);
}

void irisWipe(QPainter& p, const QSize& size, double prog, const QString& shape) {
    double t = ae::easeOutCubic(prog);
    QPointF center(size.width() / 2.0, size.height() / 2.0);
    double maxR = std::sqrt(size.width() * size.width() + size.height() * size.height()) / 2.0;
    double r = t * maxR;
    QPainterPath path;
    if (shape == "diamond") {
        path.moveTo(center.x(), center.y() - r);
        path.lineTo(center.x() + r, center.y());
        path.lineTo(center.x(), center.y() + r);
        path.lineTo(center.x() - r, center.y());
        path.closeSubpath();
    } else if (shape == "star") {
        for (int i = 0; i < 10; ++i) {
            double angle = i * M_PI / 5 - M_PI / 2;
            double rr = (i % 2 == 0) ? r : r * 0.5;
            QPointF pt(center.x() + std::cos(angle) * rr, center.y() + std::sin(angle) * rr);
            if (i == 0) path.moveTo(pt); else path.lineTo(pt);
        }
        path.closeSubpath();
    } else { // circle (default)
        path.addEllipse(center, r, r);
    }
    p.setClipPath(path);
}

void blockDissolve(QPainter& p, const QSize& size, double prog, int blockSize) {
    int bw = std::max(2, blockSize);
    int cols = size.width() / bw + 1;
    int rows = size.height() / bw + 1;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            double rnd = pseudoRand(r * 997 + c * 31);
            if (rnd < prog) continue; // Revealed
            p.fillRect(c * bw, r * bw, bw, bw, Qt::black);
        }
    }
}

void checkerWipe(QPainter& p, const QSize& size, double prog, int divisions) {
    int cellW = size.width() / std::max(divisions, 1);
    int cellH = size.height() / std::max(divisions, 1);
    for (int r = 0; r < divisions; ++r) {
        for (int c = 0; c < divisions; ++c) {
            bool even = (r + c) % 2 == 0;
            double delay = even ? 0.0 : 0.3;
            double t = clamp01((prog - delay) / (1.0 - delay));
            if (t < 1.0)
                p.fillRect(c * cellW, r * cellH, cellW, cellH, QColor(0, 0, 0, static_cast<int>(255 * (1.0 - t))));
        }
    }
}

void spiralWipe(QPainter& p, const QSize& size, double prog, bool clockwise) {
    // AE Spiral Wipe: expanding spiral arm reveals the frame
    QPointF center(size.width() / 2.0, size.height() / 2.0);
    double maxR = std::sqrt(size.width() * size.width() + size.height() * size.height()) / 2.0;
    double revealRadius = prog * maxR;
    double totalRotation = prog * 4.0 * M_PI; // 2 full spiral turns

    for (int y = 0; y < size.height(); y += 3) {
        for (int x = 0; x < size.width(); x += 3) {
            double dx = x - center.x(), dy = y - center.y();
            double dist = std::sqrt(dx * dx + dy * dy);
            double angle = std::atan2(dy, dx);
            if (!clockwise) angle = -angle;
            // Spiral: reveal threshold depends on distance + angle
            double spiralPhase = std::fmod(angle + M_PI + dist * 0.02, 2.0 * M_PI) / (2.0 * M_PI);
            double threshold = spiralPhase * maxR;
            if (dist > revealRadius || threshold > revealRadius)
                p.fillRect(x, y, 3, 3, QColor(0, 0, 0, static_cast<int>(255 * (1.0 - prog))));
        }
    }
}

void barnDoor(QPainter& p, const QSize& size, double prog, bool horizontal) {
    double t = ae::easeOutCubic(prog);
    if (horizontal) {
        int half = static_cast<int>(size.width() / 2.0 * (1.0 - t));
        p.fillRect(0, 0, half, size.height(), Qt::black);
        p.fillRect(size.width() - half, 0, half, size.height(), Qt::black);
    } else {
        int half = static_cast<int>(size.height() / 2.0 * (1.0 - t));
        p.fillRect(0, 0, size.width(), half, Qt::black);
        p.fillRect(0, size.height() - half, size.width(), half, Qt::black);
    }
}

void matrixWipe(QPainter& p, const QSize& size, double prog, int columns) {
    int colW = size.width() / std::max(columns, 1);
    for (int c = 0; c < columns; ++c) {
        double delay = pseudoRand(c * 41) * 0.5;
        double t = clamp01((prog - delay) / 0.5);
        int revealH = static_cast<int>(t * size.height());
        if (revealH < size.height())
            p.fillRect(c * colW, revealH, colW, size.height() - revealH, Qt::black);
    }
}

void pinwheel(QPainter& p, const QSize& size, double prog, int blades) {
    QPointF center(size.width() / 2.0, size.height() / 2.0);
    double maxR = std::sqrt(size.width() * size.width() + size.height() * size.height());
    double sweep = prog * 360.0 / blades;
    for (int i = 0; i < blades; ++i) {
        double startAngle = i * 360.0 / blades;
        QPainterPath path;
        path.moveTo(center);
        path.arcTo(center.x() - maxR, center.y() - maxR, maxR * 2, maxR * 2,
                   90 - startAngle - sweep, sweep);
        path.closeSubpath();
        p.save();
        p.setClipPath(path);
        p.fillRect(0, 0, size.width(), size.height(), QColor(0, 0, 0, 0));
        p.restore();
    }
}

void zigzagWipe(QPainter& p, const QSize& size, double prog, int teeth) {
    double wipeX = prog * size.width();
    double toothH = size.height() / static_cast<double>(std::max(teeth, 1));
    QPainterPath path;
    path.moveTo(wipeX, 0);
    for (int i = 0; i < teeth; ++i) {
        double y1 = i * toothH, y2 = (i + 0.5) * toothH, y3 = (i + 1) * toothH;
        path.lineTo(wipeX + toothH * 0.5, y2);
        path.lineTo(wipeX, y3);
    }
    path.lineTo(0, size.height());
    path.lineTo(0, 0);
    path.closeSubpath();
    p.save();
    p.setClipPath(path);
    p.fillRect(0, 0, size.width(), size.height(), Qt::black);
    p.restore();
}

void diamondWipe(QPainter& p, const QSize& size, double prog) {
    irisWipe(p, size, prog, "diamond");
}

void heartWipe(QPainter& p, const QSize& size, double prog) {
    // AE Heart Wipe: cardioid heart-shaped clip expanding from center
    double t = ae::easeOutCubic(prog);
    QPointF center(size.width() / 2.0, size.height() / 2.0);
    double maxR = std::sqrt(size.width() * size.width() + size.height() * size.height()) / 2.0;
    double scale = t * maxR * 0.7;

    QPainterPath path;
    // Heart curve (parametric cardioid)
    int steps = 100;
    for (int i = 0; i <= steps; ++i) {
        double angle = 2.0 * M_PI * i / steps;
        // Heart parametric equations
        double hx = 16.0 * std::pow(std::sin(angle), 3);
        double hy = -(13.0 * std::cos(angle) - 5.0 * std::cos(2 * angle) - 2.0 * std::cos(3 * angle) - std::cos(4 * angle));
        // Normalize to [-1, 1] range then scale
        double px = center.x() + hx * scale / 16.0;
        double py = center.y() + hy * scale / 16.0;
        if (i == 0) path.moveTo(px, py);
        else path.lineTo(px, py);
    }
    path.closeSubpath();
    p.setClipPath(path);
}

void starWipe(QPainter& p, const QSize& size, double prog, int points) {
    irisWipe(p, size, prog, "star");
}

void clockWipe(QPainter& p, const QSize& size, double prog, bool clockwise) {
    // AE Clock Wipe: single hand sweeps from 12 o'clock, crisp edge
    // Distinct from radialWipe: always starts at top, has a thinner reveal wedge
    QPointF center(size.width() / 2.0, size.height() / 2.0);
    double maxR = std::sqrt(size.width() * size.width() + size.height() * size.height());
    double sweepAngle = prog * 360.0;

    // Draw opaque mask everywhere EXCEPT the revealed sector
    QPainterPath fullRect;
    fullRect.addRect(0, 0, size.width(), size.height());

    QPainterPath sector;
    sector.moveTo(center);
    // Start at 12 o'clock (90 degrees in Qt)
    double startAngle = clockwise ? 90.0 : 90.0 - sweepAngle;
    double span = clockwise ? -sweepAngle : sweepAngle;
    sector.arcTo(center.x() - maxR, center.y() - maxR, maxR * 2, maxR * 2, startAngle, span);
    sector.closeSubpath();

    QPainterPath mask = fullRect.subtracted(sector);
    p.fillPath(mask, QColor(0, 0, 0, static_cast<int>(255 * (1.0 - prog * 0.3))));
}

void slideReveal(QPainter& p, const QSize& size, double prog, const QString& direction) {
    double t = ae::easeOutCubic(prog);
    int offX = 0, offY = 0;
    if (direction == "left") offX = static_cast<int>(size.width() * (1.0 - t));
    else if (direction == "right") offX = static_cast<int>(-size.width() * (1.0 - t));
    else if (direction == "up") offY = static_cast<int>(size.height() * (1.0 - t));
    else offY = static_cast<int>(-size.height() * (1.0 - t));
    p.translate(offX, offY);
}

void splitWipe(QPainter& p, const QSize& size, double prog, bool horizontal) {
    // AE Split Wipe: asymmetric split from an offset center point
    // Different from barnDoor: split point is 40/60 instead of 50/50
    double t = ae::easeOutCubic(prog);
    double splitRatio = 0.4; // Offset split (not centered like barnDoor)
    if (horizontal) {
        int splitX = static_cast<int>(size.width() * splitRatio);
        int leftReveal = static_cast<int>(splitX * t);
        int rightReveal = static_cast<int>((size.width() - splitX) * t);
        p.fillRect(0, 0, splitX - leftReveal, size.height(), Qt::black);
        p.fillRect(splitX + rightReveal, 0, size.width() - splitX - rightReveal, size.height(), Qt::black);
    } else {
        int splitY = static_cast<int>(size.height() * splitRatio);
        int topReveal = static_cast<int>(splitY * t);
        int bottomReveal = static_cast<int>((size.height() - splitY) * t);
        p.fillRect(0, 0, size.width(), splitY - topReveal, Qt::black);
        p.fillRect(0, splitY + bottomReveal, size.width(), size.height() - splitY - bottomReveal, Qt::black);
    }
}

void doorWipe(QPainter& p, const QSize& size, double prog) {
    // AE Door Wipe: 3D perspective doors swinging open on hinges
    double t = ae::easeOutCubic(prog);
    int halfW = size.width() / 2;

    // Left door: perspective skew as it opens (hinge on left edge)
    p.save();
    double leftAngle = t * 75.0; // Swing angle in degrees
    double leftScale = std::cos(leftAngle * M_PI / 180.0); // Perspective foreshortening
    QTransform leftT;
    leftT.translate(0, 0);
    leftT.scale(std::max(0.01, leftScale), 1.0);
    // Darken as it swings away (3D shadow)
    int leftAlpha = static_cast<int>(255 * (1.0 - t * 0.8));
    p.setTransform(leftT);
    p.fillRect(0, 0, halfW, size.height(), QColor(0, 0, 0, leftAlpha));
    p.restore();

    // Right door: perspective skew (hinge on right edge)
    p.save();
    double rightScale = std::max(0.01, std::cos(leftAngle * M_PI / 180.0));
    QTransform rightT;
    rightT.translate(size.width(), 0);
    rightT.scale(-rightScale, 1.0);
    int rightAlpha = static_cast<int>(255 * (1.0 - t * 0.8));
    p.setTransform(rightT);
    p.fillRect(0, 0, halfW, size.height(), QColor(0, 0, 0, rightAlpha));
    p.restore();
}

void rippleDissolve(QPainter& p, const QSize& size, double prog) {
    QPointF center(size.width() / 2.0, size.height() / 2.0);
    double maxR = std::sqrt(size.width() * size.width() + size.height() * size.height()) / 2.0;
    double rippleR = prog * maxR;
    // Multiple concentric rings
    for (int y = 0; y < size.height(); y += 4) {
        for (int x = 0; x < size.width(); x += 4) {
            double dist = std::sqrt((x - center.x()) * (x - center.x()) + (y - center.y()) * (y - center.y()));
            double wave = std::sin(dist * 0.05 - prog * 20) * 0.5 + 0.5;
            if (dist > rippleR - wave * 30)
                p.fillRect(x, y, 4, 4, QColor(0, 0, 0, static_cast<int>(200 * (1.0 - prog))));
        }
    }
}

void particleDissolve(QPainter& p, const QSize& size, double prog, int count) {
    for (int i = 0; i < count; ++i) {
        double rx = pseudoRand(i * 31);
        double ry = pseudoRand(i * 73);
        double rDelay = pseudoRand(i * 17) * 0.5;
        double t = clamp01((prog - rDelay) / 0.5);
        if (t < 0.01) continue;
        double x = rx * size.width();
        double y = ry * size.height() - t * size.height() * 0.5;
        double alpha = 1.0 - t;
        double sz = 3 + pseudoRand(i * 41) * 5;
        p.save();
        p.setOpacity(alpha);
        p.setBrush(QColor(255, 255, 255, 200));
        p.setPen(Qt::NoPen);
        p.drawEllipse(QPointF(x, y), sz, sz);
        p.restore();
    }
}

// ══════════════════════════════════════════════════════════════
// AE SHAPE LAYER OPERATIONS (10)
// ══════════════════════════════════════════════════════════════

void trimPath(QPainter& p, const std::vector<QPointF>& points, double startTrim, double endTrim, const QColor& color, double width) {
    if (points.size() < 2) return;
    p.setPen(QPen(color, width, Qt::SolidLine, Qt::RoundCap));
    int total = static_cast<int>(points.size());
    int start = static_cast<int>(startTrim * (total - 1));
    int end = static_cast<int>(endTrim * (total - 1));
    QPainterPath path;
    for (int i = start; i <= end && i < total; ++i) {
        if (i == start) path.moveTo(points[i]);
        else path.lineTo(points[i]);
    }
    p.drawPath(path);
}

void repeaterShape(QPainter& p, const QRectF& shape, int copies, double offsetX, double offsetY, double rotation, double scaleStep, const QColor& color) {
    p.setPen(QPen(color, 2));
    p.setBrush(Qt::NoBrush);
    for (int i = 0; i < copies; ++i) {
        p.save();
        p.translate(i * offsetX, i * offsetY);
        double s = std::pow(scaleStep, i);
        p.scale(s, s);
        p.rotate(rotation * i);
        p.drawRect(shape);
        p.restore();
    }
}

void offsetPath(QPainter& p, const std::vector<QPointF>& points, double offset, const QColor& color, double width) {
    if (points.size() < 2) return;
    // Draw original + offset copy
    QPainterPath path;
    for (size_t i = 0; i < points.size(); ++i) {
        if (i == 0) path.moveTo(points[i]);
        else path.lineTo(points[i]);
    }
    p.setPen(QPen(color, width));
    p.drawPath(path);
    // Offset copy
    p.save();
    p.translate(offset, offset);
    QColor dim = color; dim.setAlphaF(0.5);
    p.setPen(QPen(dim, width));
    p.drawPath(path);
    p.restore();
}

void zigZagPath(QPainter& p, const std::vector<QPointF>& points, double size, int ridges, const QColor& color, double width) {
    if (points.size() < 2) return;
    p.setPen(QPen(color, width, Qt::SolidLine, Qt::RoundCap));
    QPainterPath path;
    path.moveTo(points[0]);
    for (size_t i = 1; i < points.size(); ++i) {
        QPointF dir = points[i] - points[i - 1];
        double len = std::sqrt(dir.x() * dir.x() + dir.y() * dir.y());
        if (len < 1) continue;
        QPointF norm(-dir.y() / len, dir.x() / len);
        for (int r = 0; r < ridges; ++r) {
            double t = (r + 0.5) / ridges;
            double sign = (r % 2 == 0) ? 1.0 : -1.0;
            QPointF mid = points[i - 1] + dir * t + norm * size * sign;
            path.lineTo(mid);
        }
        path.lineTo(points[i]);
    }
    p.drawPath(path);
}

void puckerBloat(QPainter& p, const QRectF& r, double amount, const QColor& color, double width) {
    p.setPen(QPen(color, width));
    p.setBrush(Qt::NoBrush);
    QPainterPath path;
    QPointF center = r.center();
    int segments = 60;
    for (int i = 0; i <= segments; ++i) {
        double angle = 2.0 * M_PI * i / segments;
        double rx = r.width() / 2.0, ry = r.height() / 2.0;
        double dist = 1.0 + amount * std::sin(angle * 4); // Pucker/bloat
        double x = center.x() + std::cos(angle) * rx * dist;
        double y = center.y() + std::sin(angle) * ry * dist;
        if (i == 0) path.moveTo(x, y); else path.lineTo(x, y);
    }
    p.drawPath(path);
}

void roundCorners(QPainter& p, const QRectF& r, double radius, const QColor& color, double width) {
    p.setPen(QPen(color, width));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(r, radius, radius);
}

void wigglePath(QPainter& p, const std::vector<QPointF>& points, double size, double detail, double phase, const QColor& color, double width) {
    if (points.size() < 2) return;
    p.setPen(QPen(color, width, Qt::SolidLine, Qt::RoundCap));
    QPainterPath path;
    for (size_t i = 0; i < points.size(); ++i) {
        double wx = ae::wiggle(phase + i * detail, 5.0, size, static_cast<int>(i) * 3);
        double wy = ae::wiggle(phase + i * detail + 100, 4.0, size, static_cast<int>(i) * 7);
        QPointF pt(points[i].x() + wx, points[i].y() + wy);
        if (i == 0) path.moveTo(pt); else path.lineTo(pt);
    }
    p.drawPath(path);
}

void dashedPath(QPainter& p, const std::vector<QPointF>& points, double dashLen, double gapLen, double offset, const QColor& color, double width) {
    if (points.size() < 2) return;
    QPen pen(color, width);
    QVector<qreal> pattern;
    pattern << dashLen << gapLen;
    pen.setDashPattern(pattern);
    pen.setDashOffset(offset);
    p.setPen(pen);
    QPainterPath path;
    for (size_t i = 0; i < points.size(); ++i) {
        if (i == 0) path.moveTo(points[i]); else path.lineTo(points[i]);
    }
    p.drawPath(path);
}

void taperStroke(QPainter& p, const std::vector<QPointF>& points, double startWidth, double endWidth, const QColor& color) {
    if (points.size() < 2) return;
    for (size_t i = 1; i < points.size(); ++i) {
        double t = static_cast<double>(i) / (points.size() - 1);
        double w = startWidth + t * (endWidth - startWidth);
        p.setPen(QPen(color, w, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(points[i - 1], points[i]);
    }
}

void morphPaths(QPainter& p, const std::vector<QPointF>& from, const std::vector<QPointF>& to, double prog, const QColor& color, double width) {
    size_t count = std::min(from.size(), to.size());
    if (count < 2) return;
    p.setPen(QPen(color, width, Qt::SolidLine, Qt::RoundCap));
    double t = ae::easeInOutCubic(prog);
    QPainterPath path;
    for (size_t i = 0; i < count; ++i) {
        QPointF pt(from[i].x() + t * (to[i].x() - from[i].x()),
                   from[i].y() + t * (to[i].y() - from[i].y()));
        if (i == 0) path.moveTo(pt); else path.lineTo(pt);
    }
    p.drawPath(path);
}

}} // namespace prestige::fx
