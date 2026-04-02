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

// ══════════════════════════════════════════════════════════════
// AE EXTENDED TEXT ANIMATIONS (25)
// Per-character 3D, range selectors, advanced effects
// ══════════════════════════════════════════════════════════════
void matrixRain(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color);
void cascadeReveal(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color);
void elasticDrop(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color);
void spiralIn(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color);
void flipBoard(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color);
void glowReveal(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color);
void smokeIn(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color);
void scatterAssemble(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color);
void slotMachine(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color);
void stampPress(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color);
void wipePerLetter(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color);
void swingDrop(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color);
void neonFlicker(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color);
void gravityCrush(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color);
void rubberStretch(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color);
void fadeInRandom(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color);
void curvedPath(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color);
void zoomBurst(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color);
void jitterShake(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color);
void shadowExpand(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color);
void rangeWipe(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color);
void textShatter(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color);
void reflectReveal(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color);
void perspectiveTilt(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color);
void liquidFill(QPainter& p, const QString& text, const QRectF& r, double prog, const QFont& font, const QColor& color);

// ══════════════════════════════════════════════════════════════
// AE EXTENDED TRANSITIONS (20)
// Card Wipe, Venetian Blinds, Radial Wipe, Iris, etc.
// ══════════════════════════════════════════════════════════════
void cardWipe(QPainter& p, const QSize& size, double prog, int rows, int cols);
void venetianBlinds(QPainter& p, const QSize& size, double prog, int blades, bool horizontal);
void radialWipe(QPainter& p, const QSize& size, double prog, bool clockwise);
void irisWipe(QPainter& p, const QSize& size, double prog, const QString& shape);
void blockDissolve(QPainter& p, const QSize& size, double prog, int blockSize);
void checkerWipe(QPainter& p, const QSize& size, double prog, int divisions);
void spiralWipe(QPainter& p, const QSize& size, double prog, bool clockwise);
void barnDoor(QPainter& p, const QSize& size, double prog, bool horizontal);
void matrixWipe(QPainter& p, const QSize& size, double prog, int columns);
void pinwheel(QPainter& p, const QSize& size, double prog, int blades);
void zigzagWipe(QPainter& p, const QSize& size, double prog, int teeth);
void diamondWipe(QPainter& p, const QSize& size, double prog);
void heartWipe(QPainter& p, const QSize& size, double prog);
void starWipe(QPainter& p, const QSize& size, double prog, int points);
void clockWipe(QPainter& p, const QSize& size, double prog, bool clockwise);
void slideReveal(QPainter& p, const QSize& size, double prog, const QString& direction);
void splitWipe(QPainter& p, const QSize& size, double prog, bool horizontal);
void doorWipe(QPainter& p, const QSize& size, double prog);
void rippleDissolve(QPainter& p, const QSize& size, double prog);
void particleDissolve(QPainter& p, const QSize& size, double prog, int count);

// ══════════════════════════════════════════════════════════════
// AE SHAPE LAYER OPERATIONS (10)
// Trim Paths, Repeater, Offset Path, etc.
// ══════════════════════════════════════════════════════════════
void trimPath(QPainter& p, const std::vector<QPointF>& points, double startTrim, double endTrim, const QColor& color, double width);
void repeaterShape(QPainter& p, const QRectF& shape, int copies, double offsetX, double offsetY, double rotation, double scaleStep, const QColor& color);
void offsetPath(QPainter& p, const std::vector<QPointF>& points, double offset, const QColor& color, double width);
void zigZagPath(QPainter& p, const std::vector<QPointF>& points, double size, int ridges, const QColor& color, double width);
void puckerBloat(QPainter& p, const QRectF& r, double amount, const QColor& color, double width);
void roundCorners(QPainter& p, const QRectF& r, double radius, const QColor& color, double width);
void wigglePath(QPainter& p, const std::vector<QPointF>& points, double size, double detail, double phase, const QColor& color, double width);
void dashedPath(QPainter& p, const std::vector<QPointF>& points, double dashLen, double gapLen, double offset, const QColor& color, double width);
void taperStroke(QPainter& p, const std::vector<QPointF>& points, double startWidth, double endWidth, const QColor& color);
void morphPaths(QPainter& p, const std::vector<QPointF>& from, const std::vector<QPointF>& to, double prog, const QColor& color, double width);

} // namespace fx

// ── Effect ID list for UI ─────────────────────────────────
QStringList allEffectIds();
QStringList allEffectNames();
QString effectCategoryForId(const QString& id);

} // namespace prestige
