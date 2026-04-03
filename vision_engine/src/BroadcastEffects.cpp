// ============================================================
// Prestige AI — Broadcast Effects (STUB — animations removed)
// All visual animations now handled by Lottie engine
// These stubs exist only for compilation compatibility.
// ============================================================

#include "BroadcastEffects.h"
#include <QFontMetrics>
#include <QPainterPath>

namespace prestige {

QStringList allEffectIds() { return {}; }
QStringList allEffectNames() { return {}; }
QString effectCategoryForId(const QString&) { return {}; }

namespace fx {

// All effects are no-ops — Lottie handles animations now
void typewriter(QPainter&, const QString&, const QRectF&, double, const QFont&, const QColor&) {}
void bounceIn(QPainter&, const QString&, const QRectF&, double, const QFont&, const QColor&) {}
void waveText(QPainter&, const QString&, const QRectF&, double, const QFont&, const QColor&) {}
void trackingExpand(QPainter&, const QString&, const QRectF&, double, const QFont&, const QColor&) {}
void fadeUpPerLetter(QPainter&, const QString&, const QRectF&, double, const QFont&, const QColor&) {}
void scaleUpPerLetter(QPainter&, const QString&, const QRectF&, double, const QFont&, const QColor&) {}
void rotateInPerLetter(QPainter&, const QString&, const QRectF&, double, const QFont&, const QColor&) {}
void blurIn(QPainter&, const QString&, const QRectF&, double, const QFont&, const QColor&) {}
void slidePerLetter(QPainter&, const QString&, const QRectF&, double, const QFont&, const QColor&) {}
void kineticPop(QPainter&, const QString&, const QRectF&, double, const QFont&, const QColor&) {}

void lineDraw(QPainter&, const QRectF&, double, const QColor&, double) {}
void barSlide(QPainter&, const QRectF&, double, const QColor&, double) {}
void shapeMorph(QPainter&, const QRectF&, double, const QColor&, double) {}
void splitReveal(QPainter&, const QRectF&, double, const QColor&, double) {}
void bracketExpand(QPainter&, const QRectF&, double, const QColor&, double) {}
void underlineGrow(QPainter&, const QRectF&, double, const QColor&, double) {}
void boxWipe(QPainter&, const QRectF&, double, const QColor&, double) {}
void cornerBuild(QPainter&, const QRectF&, double, const QColor&, double) {}

void fadeGlow(QPainter&, const QRectF&, double, const QColor&) {}
void lightStreak(QPainter&, const QRectF&, double, const QColor&) {}
void particleForm(QPainter&, const QRectF&, double, const QColor&, int) {}
void scaleBounce(QPainter&, const QRectF&, double) {}
void shatterIn(QPainter&, const QRectF&, double) {}
void blurZoom(QPainter&, const QRectF&, double) {}
void rotate3D(QPainter&, const QRectF&, double) {}
void pulseReveal(QPainter&, const QRectF&, double, const QColor&) {}

void wipeLinear(QPainter&, const QSize&, double, bool) {}
void pushSlide(QPainter&, const QSize&, double, bool) {}
void zoomThrough(QPainter&, const QSize&, double) {}
void glitchTransition(QPainter&, const QSize&, double, int) {}
void lightLeak(QPainter&, const QSize&, double, const QColor&) {}
void inkBleed(QPainter&, const QSize&, double) {}
void spinTransition(QPainter&, const QSize&, double) {}
void crossDissolve(QPainter&, const QSize&, double) {}

void neonGlow(QPainter&, const QRectF&, const QColor&, double) {}
void bloom(QPainter&, const QRectF&, const QColor&, double) {}
void lightRays(QPainter&, const QRectF&, double, const QColor&, double) {}
void lensFlare(QPainter&, const QPointF&, double, const QColor&) {}
void shimmer(QPainter&, const QRectF&, double, const QColor&) {}
void edgeGlow(QPainter&, const QRectF&, const QColor&, double) {}

void glitchRGB(QPainter&, const QRectF&, double, int) {}
void chromaticAberration(QPainter&, const QString&, const QRectF&, const QFont&, const QColor&, double) {}
void waveDistort(QPainter&, const QRectF&, double, double) {}
void pixelSort(QPainter&, const QRectF&, double) {}
void vhsEffect(QPainter&, const QRectF&, double, int) {}

void sparkles(QPainter&, const QRectF&, int, double, const QColor&) {}
void bokeh(QPainter&, const QRectF&, int, double, const QColor&) {}
void dustParticles(QPainter&, const QRectF&, int, double) {}
void fireEmbers(QPainter&, const QRectF&, int, double) {}
void confetti(QPainter&, const QRectF&, int, double) {}
void snow(QPainter&, const QRectF&, int, double) {}
void risingParticles(QPainter&, const QRectF&, int, double, const QColor&) {}

void lineDrawOn(QPainter&, const QPointF&, const QPointF&, double, const QColor&, double) {}
void circleExpand(QPainter&, const QPointF&, double, double, const QColor&) {}
void rectangleBuild(QPainter&, const QRectF&, double, const QColor&, double) {}
void pathTrace(QPainter&, const std::vector<QPointF>&, double, const QColor&, double) {}
void gridReveal(QPainter&, const QRectF&, double, const QColor&, int, int) {}
void hexagonPattern(QPainter&, const QRectF&, double, const QColor&, double) {}

void gaussianBlurIn(QPainter&, const QString&, const QRectF&, double, const QFont&, const QColor&) {}
void radialBlur(QPainter&, const QRectF&, double, const QColor&) {}
void directionalBlur(QPainter&, const QString&, const QRectF&, double, const QFont&, const QColor&, double) {}
void tiltShift(QPainter&, const QRectF&, double, double) {}
void defocus(QPainter&, const QRectF&, double, const QColor&) {}

void colorSweep(QPainter&, const QRectF&, double, const QColor&, const QColor&) {}
void gradientShift(QPainter&, const QRectF&, double, const QColor&, const QColor&) {}
void duotone(QPainter&, const QRectF&, const QColor&, const QColor&) {}
void shadowDropAnimate(QPainter&, const QRectF&, double, const QColor&) {}
void outlineStroke(QPainter&, const QString&, const QRectF&, double, const QFont&, const QColor&, double) {}

} // namespace fx
} // namespace prestige
