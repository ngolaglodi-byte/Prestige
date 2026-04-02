#pragma once

// ============================================================
// Prestige AI — Broadcast Effects Engine
// 68 professional animation effects (After Effects quality)
// Applied to: nameplates, lower thirds, titles, logos, channel names
// ============================================================

#include <QPainter>
#include <QRectF>
#include <QSize>
#include <QColor>
#include <QString>
#include <QStringList>
#include <cmath>
#include <vector>

namespace prestige {

// ── Effect Categories ──────────────────────────────────────
// Each effect takes: painter, target rect, progress (0→1), frame size, accent color, text

namespace fx {

// ── TEXT ANIMATIONS (10) ───────────────────────────────────
void typewriter(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color);
void bounceIn(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color);
void waveText(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color);
void trackingExpand(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color);
void fadeUpPerLetter(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color);
void scaleUpPerLetter(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color);
void rotateInPerLetter(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color);
void blurIn(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color);
void slidePerLetter(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color);
void kineticPop(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color);

// ── LOWER THIRD ANIMATIONS (8) ────────────────────────────
void lineDraw(QPainter& p, const QRectF& r, double prog, const QColor& accent, double scale);
void barSlide(QPainter& p, const QRectF& r, double prog, const QColor& accent, double scale);
void shapeMorph(QPainter& p, const QRectF& r, double prog, const QColor& accent, double scale);
void splitReveal(QPainter& p, const QRectF& r, double prog, const QColor& accent, double scale);
void bracketExpand(QPainter& p, const QRectF& r, double prog, const QColor& accent, double scale);
void underlineGrow(QPainter& p, const QRectF& r, double prog, const QColor& accent, double scale);
void boxWipe(QPainter& p, const QRectF& r, double prog, const QColor& accent, double scale);
void cornerBuild(QPainter& p, const QRectF& r, double prog, const QColor& accent, double scale);

// ── LOGO REVEALS (8) ──────────────────────────────────────
void fadeGlow(QPainter& p, const QRectF& r, double prog, const QColor& accent);
void lightStreak(QPainter& p, const QRectF& r, double prog, const QColor& accent);
void particleForm(QPainter& p, const QRectF& r, double prog, const QColor& accent, int frameCount);
void scaleBounce(QPainter& p, const QRectF& r, double prog);
void shatterIn(QPainter& p, const QRectF& r, double prog);
void blurZoom(QPainter& p, const QRectF& r, double prog);
void rotate3D(QPainter& p, const QRectF& r, double prog);
void pulseReveal(QPainter& p, const QRectF& r, double prog, const QColor& accent);

// ── TRANSITIONS (8) ───────────────────────────────────────
void wipeLinear(QPainter& p, const QSize& size, double prog, bool leftToRight = true);
void pushSlide(QPainter& p, const QSize& size, double prog, bool horizontal = true);
void zoomThrough(QPainter& p, const QSize& size, double prog);
void glitchTransition(QPainter& p, const QSize& size, double prog, int frameCount);
void lightLeak(QPainter& p, const QSize& size, double prog, const QColor& accent);
void inkBleed(QPainter& p, const QSize& size, double prog);
void spinTransition(QPainter& p, const QSize& size, double prog);
void crossDissolve(QPainter& p, const QSize& size, double prog);

// ── GLOW & LIGHT (6) ──────────────────────────────────────
void neonGlow(QPainter& p, const QRectF& r, const QColor& color, double intensity);
void bloom(QPainter& p, const QRectF& r, const QColor& color, double intensity);
void lightRays(QPainter& p, const QRectF& r, double angle, const QColor& color, double intensity);
void lensFlare(QPainter& p, const QPointF& center, double radius, const QColor& color);
void shimmer(QPainter& p, const QRectF& r, double phase, const QColor& color);
void edgeGlow(QPainter& p, const QRectF& r, const QColor& color, double width);

// ── DISTORTION (5) ────────────────────────────────────────
void glitchRGB(QPainter& p, const QRectF& r, double intensity, int frameCount);
void chromaticAberration(QPainter& p, const QString& text, const QRectF& r, const QFont& font, const QColor& color, double offset);
void waveDistort(QPainter& p, const QRectF& r, double phase, double amplitude);
void pixelSort(QPainter& p, const QRectF& r, double prog);
void vhsEffect(QPainter& p, const QRectF& r, double intensity, int frameCount);

// ── PARTICLE EFFECTS (7) ──────────────────────────────────
void sparkles(QPainter& p, const QRectF& r, int count, double phase, const QColor& color);
void bokeh(QPainter& p, const QRectF& r, int count, double phase, const QColor& color);
void dustParticles(QPainter& p, const QRectF& r, int count, double phase);
void fireEmbers(QPainter& p, const QRectF& r, int count, double phase);
void confetti(QPainter& p, const QRectF& r, int count, double phase);
void snow(QPainter& p, const QRectF& r, int count, double phase);
void risingParticles(QPainter& p, const QRectF& r, int count, double phase, const QColor& color);

// ── SHAPE/LINE ANIMATIONS (6) ─────────────────────────────
void lineDrawOn(QPainter& p, const QPointF& from, const QPointF& to, double prog, const QColor& color, double width);
void circleExpand(QPainter& p, const QPointF& center, double maxRadius, double prog, const QColor& color);
void rectangleBuild(QPainter& p, const QRectF& r, double prog, const QColor& color, double lineWidth);
void pathTrace(QPainter& p, const std::vector<QPointF>& points, double prog, const QColor& color, double width);
void gridReveal(QPainter& p, const QRectF& r, double prog, const QColor& color, int cols, int rows);
void hexagonPattern(QPainter& p, const QRectF& r, double prog, const QColor& color, double hexSize);

// ── BLUR/FOCUS (5) ────────────────────────────────────────
void gaussianBlurIn(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color);
void radialBlur(QPainter& p, const QRectF& r, double prog, const QColor& color);
void directionalBlur(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color, double angle);
void tiltShift(QPainter& p, const QRectF& r, double focusY, double range);
void defocus(QPainter& p, const QRectF& r, double prog, const QColor& color);

// ── COLOR/STYLE (5) ───────────────────────────────────────
void colorSweep(QPainter& p, const QRectF& r, double prog, const QColor& from, const QColor& to);
void gradientShift(QPainter& p, const QRectF& r, double phase, const QColor& c1, const QColor& c2);
void duotone(QPainter& p, const QRectF& r, const QColor& shadow, const QColor& highlight);
void shadowDropAnimate(QPainter& p, const QRectF& r, double prog, const QColor& shadow);
void outlineStroke(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color, double width);

} // namespace fx

// ── Effect ID list for UI ─────────────────────────────────
QStringList allEffectIds();
QStringList allEffectNames();
QString effectCategoryForId(const QString& id);

} // namespace prestige
