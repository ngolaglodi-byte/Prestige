#pragma once

// ============================================================
// Prestige AI — After Effects Easing & Expression Engine
// Complete AE Graph Editor curves + Expression Controls
// + 27 Blend Modes (GPU-ready compositing)
// ============================================================

#include <cmath>
#include <functional>
#include <QString>
#include <QStringList>
#include <QColor>
#include <QImage>

namespace prestige {
namespace ae {

// ══════════════════════════════════════════════════════════════
// 1. BÉZIER EASING (AE Graph Editor)
//    Custom cubic-bezier(x1, y1, x2, y2) — identical to AE
// ══════════════════════════════════════════════════════════════

// Solve cubic-bezier curve at time t (0→1)
// Control points: (x1,y1) and (x2,y2) — same as CSS/AE
double cubicBezier(double t, double x1, double y1, double x2, double y2);

// ── AE Preset Curves ────────────────────────────────────────
double easeInSine(double t);
double easeOutSine(double t);
double easeInOutSine(double t);
double easeInQuad(double t);
double easeOutQuad(double t);
double easeInOutQuad(double t);
double easeInCubic(double t);
double easeOutCubic(double t);
double easeInOutCubic(double t);
double easeInQuart(double t);
double easeOutQuart(double t);
double easeInOutQuart(double t);
double easeInQuint(double t);
double easeOutQuint(double t);
double easeInOutQuint(double t);
double easeInExpo(double t);
double easeOutExpo(double t);
double easeInOutExpo(double t);
double easeInCirc(double t);
double easeOutCirc(double t);
double easeInOutCirc(double t);
double easeInBack(double t);
double easeOutBack(double t);
double easeInOutBack(double t);
double easeInElastic(double t);
double easeOutElastic(double t);
double easeInOutElastic(double t);
double easeInBounce(double t);
double easeOutBounce(double t);
double easeInOutBounce(double t);

// ── Advanced Motion Curves ──────────────────────────────────
double spring(double t, double stiffness = 100.0, double damping = 10.0);
double overshoot(double t, double amount = 1.70158);
double anticipation(double t, double amount = 1.70158);
double snap(double t, double snapPoint = 0.5);
double rubberBand(double t, double elasticity = 0.3);
double smootherStep(double t); // Ken Perlin's improved smoothstep

// Easing by name (for UI/config)
using EaseFunc = std::function<double(double)>;
EaseFunc easingByName(const QString& name);
QStringList allEasingNames();

// ══════════════════════════════════════════════════════════════
// 2. AE EXPRESSION CONTROLS
//    Runtime expressions evaluated per-frame
// ══════════════════════════════════════════════════════════════

// wiggle(freq, amp) — random oscillation (AE's most popular expression)
double wiggle(double time, double freq, double amp, int seed = 0);

// loopOut("cycle"|"pingpong"|"offset"|"continue", numKeyframes)
double loopOut(double time, double duration, const QString& type = "cycle");

// posterizeTime(fps) — reduce animation framerate
double posterizeTime(double time, double fps);

// valueAtTime(time) — temporal offset (for echo/trail effects)
double valueAtTime(double currentValue, double time, double offset, double decay = 0.8);

// random(min, max) — seeded random in range
double random(double min, double max, int seed);

// noise(t) — Perlin-like smooth noise (1D)
double noise1D(double t);

// noise2D(x, y) — 2D smooth noise
double noise2D(double x, double y);

// linear(t, tMin, tMax, vMin, vMax) — AE linear interpolation expression
double linear(double t, double tMin, double tMax, double vMin, double vMax);

// ease(t, tMin, tMax, vMin, vMax) — AE ease expression (smooth interpolation)
double ease(double t, double tMin, double tMax, double vMin, double vMax);

// clamp(value, min, max)
double clamp(double value, double min, double max);

// ══════════════════════════════════════════════════════════════
// 3. AE BLEND MODES (27 modes)
//    Per-pixel compositing operations (GPU + CPU fallback)
// ══════════════════════════════════════════════════════════════

enum class BlendMode {
    Normal,
    Dissolve,
    // ── Darkening ──
    Darken,
    Multiply,
    ColorBurn,
    LinearBurn,
    DarkerColor,
    // ── Lightening ──
    Lighten,
    Screen,
    ColorDodge,
    LinearDodge,  // Add
    LighterColor,
    // ── Contrast ──
    Overlay,
    SoftLight,
    HardLight,
    VividLight,
    LinearLight,
    PinLight,
    HardMix,
    // ── Inversion ──
    Difference,
    Exclusion,
    Subtract,
    Divide,
    // ── Component ──
    Hue,
    Saturation,
    Color,
    Luminosity,
};

// CPU blend: composite overlay onto base with given blend mode
QImage blendImages(const QImage& base, const QImage& overlay,
                   BlendMode mode, double opacity = 1.0);

// Per-pixel blend (for CPU fallback)
QColor blendPixel(const QColor& base, const QColor& overlay, BlendMode mode);

// Blend mode by name (for UI)
BlendMode blendModeByName(const QString& name);
QString blendModeName(BlendMode mode);
QStringList allBlendModeNames();

// ══════════════════════════════════════════════════════════════
// 4. GLSL SNIPPETS for GPU blend modes
//    Returns the GLSL function body for a given blend mode
// ══════════════════════════════════════════════════════════════

// Full fragment shader for blend mode compositing
const char* blendModeFragmentShader();

} // namespace ae
} // namespace prestige
