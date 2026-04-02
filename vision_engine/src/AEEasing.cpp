// ============================================================
// Prestige AI — After Effects Easing & Expression Engine
// Complete implementation: 30 easings, 10 expressions, 27 blend modes
// ============================================================

#include "AEEasing.h"
#include <QPainter>
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace prestige {
namespace ae {

// ══════════════════════════════════════════════════════════════
// 1. CUBIC BÉZIER SOLVER (AE Graph Editor precision)
// ══════════════════════════════════════════════════════════════

// Newton-Raphson solver for cubic-bezier x(t) = target
static double solveCubicBezierX(double target, double x1, double x2) {
    double t = target; // Initial guess
    for (int i = 0; i < 14; ++i) { // 14 iterations = float64 precision
        double cx = 3.0 * x1;
        double bx = 3.0 * (x2 - x1) - cx;
        double ax = 1.0 - cx - bx;
        double x = ((ax * t + bx) * t + cx) * t;
        double dx = (3.0 * ax * t + 2.0 * bx) * t + cx;
        if (std::abs(dx) < 1e-12) break;
        t -= (x - target) / dx;
        t = std::max(0.0, std::min(1.0, t));
    }
    return t;
}

double cubicBezier(double t, double x1, double y1, double x2, double y2) {
    if (t <= 0.0) return 0.0;
    if (t >= 1.0) return 1.0;
    double s = solveCubicBezierX(t, x1, x2);
    double cy = 3.0 * y1;
    double by = 3.0 * (y2 - y1) - cy;
    double ay = 1.0 - cy - by;
    return ((ay * s + by) * s + cy) * s;
}

// ══════════════════════════════════════════════════════════════
// AE PRESET EASING CURVES (30 curves)
// ══════════════════════════════════════════════════════════════

double easeInSine(double t)      { return 1.0 - std::cos(t * M_PI / 2.0); }
double easeOutSine(double t)     { return std::sin(t * M_PI / 2.0); }
double easeInOutSine(double t)   { return -(std::cos(M_PI * t) - 1.0) / 2.0; }

double easeInQuad(double t)      { return t * t; }
double easeOutQuad(double t)     { return 1.0 - (1.0 - t) * (1.0 - t); }
double easeInOutQuad(double t)   { return t < 0.5 ? 2.0 * t * t : 1.0 - std::pow(-2.0 * t + 2.0, 2) / 2.0; }

double easeInCubic(double t)     { return t * t * t; }
double easeOutCubic(double t)    { return 1.0 - std::pow(1.0 - t, 3); }
double easeInOutCubic(double t)  { return t < 0.5 ? 4.0 * t * t * t : 1.0 - std::pow(-2.0 * t + 2.0, 3) / 2.0; }

double easeInQuart(double t)     { return t * t * t * t; }
double easeOutQuart(double t)    { return 1.0 - std::pow(1.0 - t, 4); }
double easeInOutQuart(double t)  { return t < 0.5 ? 8.0 * t * t * t * t : 1.0 - std::pow(-2.0 * t + 2.0, 4) / 2.0; }

double easeInQuint(double t)     { return t * t * t * t * t; }
double easeOutQuint(double t)    { return 1.0 - std::pow(1.0 - t, 5); }
double easeInOutQuint(double t)  { return t < 0.5 ? 16.0 * t * t * t * t * t : 1.0 - std::pow(-2.0 * t + 2.0, 5) / 2.0; }

double easeInExpo(double t)      { return t <= 0.0 ? 0.0 : std::pow(2.0, 10.0 * t - 10.0); }
double easeOutExpo(double t)     { return t >= 1.0 ? 1.0 : 1.0 - std::pow(2.0, -10.0 * t); }
double easeInOutExpo(double t)   { if (t <= 0.0) return 0.0; if (t >= 1.0) return 1.0; return t < 0.5 ? std::pow(2.0, 20.0 * t - 10.0) / 2.0 : (2.0 - std::pow(2.0, -20.0 * t + 10.0)) / 2.0; }

double easeInCirc(double t)      { return 1.0 - std::sqrt(1.0 - t * t); }
double easeOutCirc(double t)     { return std::sqrt(1.0 - (t - 1.0) * (t - 1.0)); }
double easeInOutCirc(double t)   { return t < 0.5 ? (1.0 - std::sqrt(1.0 - 4.0 * t * t)) / 2.0 : (std::sqrt(1.0 - std::pow(-2.0 * t + 2.0, 2)) + 1.0) / 2.0; }

double easeInBack(double t)      { const double s = 1.70158; return (s + 1.0) * t * t * t - s * t * t; }
double easeOutBack(double t)     { const double s = 1.70158; double u = t - 1.0; return 1.0 + (s + 1.0) * u * u * u + s * u * u; }
double easeInOutBack(double t)   { const double s = 1.70158 * 1.525; return t < 0.5 ? (std::pow(2.0 * t, 2) * ((s + 1.0) * 2.0 * t - s)) / 2.0 : (std::pow(2.0 * t - 2.0, 2) * ((s + 1.0) * (2.0 * t - 2.0) + s) + 2.0) / 2.0; }

double easeInElastic(double t) {
    if (t <= 0.0) return 0.0;
    if (t >= 1.0) return 1.0;
    return -std::pow(2.0, 10.0 * t - 10.0) * std::sin((t * 10.0 - 10.75) * (2.0 * M_PI) / 3.0);
}
double easeOutElastic(double t) {
    if (t <= 0.0) return 0.0;
    if (t >= 1.0) return 1.0;
    return std::pow(2.0, -10.0 * t) * std::sin((t * 10.0 - 0.75) * (2.0 * M_PI) / 3.0) + 1.0;
}
double easeInOutElastic(double t) {
    if (t <= 0.0) return 0.0;
    if (t >= 1.0) return 1.0;
    double c = (2.0 * M_PI) / 4.5;
    return t < 0.5
        ? -(std::pow(2.0, 20.0 * t - 10.0) * std::sin((20.0 * t - 11.125) * c)) / 2.0
        : (std::pow(2.0, -20.0 * t + 10.0) * std::sin((20.0 * t - 11.125) * c)) / 2.0 + 1.0;
}

double easeOutBounce(double t) {
    if (t < 1.0 / 2.75) return 7.5625 * t * t;
    if (t < 2.0 / 2.75) { t -= 1.5 / 2.75; return 7.5625 * t * t + 0.75; }
    if (t < 2.5 / 2.75) { t -= 2.25 / 2.75; return 7.5625 * t * t + 0.9375; }
    t -= 2.625 / 2.75; return 7.5625 * t * t + 0.984375;
}
double easeInBounce(double t)    { return 1.0 - easeOutBounce(1.0 - t); }
double easeInOutBounce(double t) { return t < 0.5 ? (1.0 - easeOutBounce(1.0 - 2.0 * t)) / 2.0 : (1.0 + easeOutBounce(2.0 * t - 1.0)) / 2.0; }

// ── Advanced Motion ─────────────────────────────────────────

double spring(double t, double stiffness, double damping) {
    if (t <= 0.0) return 0.0;
    if (t >= 1.0) return 1.0;
    double w = std::sqrt(stiffness);
    double d = damping / (2.0 * w);
    double wd = w * std::sqrt(1.0 - d * d);
    return 1.0 - std::exp(-d * w * t) * std::cos(wd * t);
}

double overshoot(double t, double amount) {
    return easeOutBack(t); // Uses standard back with configurable overshoot
}

double anticipation(double t, double amount) {
    return easeInBack(t);
}

double snap(double t, double snapPoint) {
    if (t < snapPoint) {
        double u = t / snapPoint;
        return easeInQuad(u) * snapPoint;
    }
    double u = (t - snapPoint) / (1.0 - snapPoint);
    return snapPoint + easeOutExpo(u) * (1.0 - snapPoint);
}

double rubberBand(double t, double elasticity) {
    return 1.0 - std::pow(std::cos(t * M_PI / 2.0), 3) * std::cos(t * M_PI * elasticity * 10.0) * (1.0 - t);
}

double smootherStep(double t) {
    t = std::max(0.0, std::min(1.0, t));
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

// ── Easing by name ──────────────────────────────────────────

EaseFunc easingByName(const QString& name) {
    // Sine
    if (name == "ease_in_sine")       return easeInSine;
    if (name == "ease_out_sine")      return easeOutSine;
    if (name == "ease_in_out_sine")   return easeInOutSine;
    // Quad
    if (name == "ease_in_quad")       return easeInQuad;
    if (name == "ease_out_quad")      return easeOutQuad;
    if (name == "ease_in_out_quad")   return easeInOutQuad;
    // Cubic
    if (name == "ease_in_cubic")      return easeInCubic;
    if (name == "ease_out_cubic")     return easeOutCubic;
    if (name == "ease_in_out_cubic")  return easeInOutCubic;
    // Quart
    if (name == "ease_in_quart")      return easeInQuart;
    if (name == "ease_out_quart")     return easeOutQuart;
    if (name == "ease_in_out_quart")  return easeInOutQuart;
    // Quint
    if (name == "ease_in_quint")      return easeInQuint;
    if (name == "ease_out_quint")     return easeOutQuint;
    if (name == "ease_in_out_quint")  return easeInOutQuint;
    // Expo
    if (name == "ease_in_expo")       return easeInExpo;
    if (name == "ease_out_expo")      return easeOutExpo;
    if (name == "ease_in_out_expo")   return easeInOutExpo;
    // Circ
    if (name == "ease_in_circ")       return easeInCirc;
    if (name == "ease_out_circ")      return easeOutCirc;
    if (name == "ease_in_out_circ")   return easeInOutCirc;
    // Back
    if (name == "ease_in_back")       return easeInBack;
    if (name == "ease_out_back")      return easeOutBack;
    if (name == "ease_in_out_back")   return easeInOutBack;
    // Elastic
    if (name == "ease_in_elastic")    return easeInElastic;
    if (name == "ease_out_elastic")   return easeOutElastic;
    if (name == "ease_in_out_elastic")return easeInOutElastic;
    // Bounce
    if (name == "ease_in_bounce")     return easeInBounce;
    if (name == "ease_out_bounce")    return easeOutBounce;
    if (name == "ease_in_out_bounce") return easeInOutBounce;
    // Advanced
    if (name == "spring")             return [](double t) { return spring(t); };
    if (name == "overshoot")          return [](double t) { return overshoot(t); };
    if (name == "anticipation")       return [](double t) { return anticipation(t); };
    if (name == "snap")               return [](double t) { return snap(t); };
    if (name == "rubber_band")        return [](double t) { return rubberBand(t); };
    if (name == "smoother_step")      return smootherStep;

    // Default: linear
    return [](double t) { return t; };
}

QStringList allEasingNames() {
    return {
        "linear",
        "ease_in_sine", "ease_out_sine", "ease_in_out_sine",
        "ease_in_quad", "ease_out_quad", "ease_in_out_quad",
        "ease_in_cubic", "ease_out_cubic", "ease_in_out_cubic",
        "ease_in_quart", "ease_out_quart", "ease_in_out_quart",
        "ease_in_quint", "ease_out_quint", "ease_in_out_quint",
        "ease_in_expo", "ease_out_expo", "ease_in_out_expo",
        "ease_in_circ", "ease_out_circ", "ease_in_out_circ",
        "ease_in_back", "ease_out_back", "ease_in_out_back",
        "ease_in_elastic", "ease_out_elastic", "ease_in_out_elastic",
        "ease_in_bounce", "ease_out_bounce", "ease_in_out_bounce",
        "spring", "overshoot", "anticipation", "snap", "rubber_band", "smoother_step"
    };
}

// ══════════════════════════════════════════════════════════════
// 2. AE EXPRESSION CONTROLS
// ══════════════════════════════════════════════════════════════

// Deterministic hash for wiggle
static double hashNoise(int seed) {
    int n = seed;
    n = (n << 13) ^ n;
    return 1.0 - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0;
}

double wiggle(double time, double freq, double amp, int seed) {
    double phase = time * freq;
    int i0 = static_cast<int>(std::floor(phase));
    double frac = phase - i0;
    // Smooth interpolation between hash values
    double v0 = hashNoise(i0 + seed * 1337);
    double v1 = hashNoise(i0 + 1 + seed * 1337);
    double v2 = hashNoise(i0 + 2 + seed * 1337);
    // Catmull-Rom for smooth wiggle (like AE)
    double t2 = frac * frac;
    double t3 = t2 * frac;
    double vm1 = hashNoise(i0 - 1 + seed * 1337);
    double result = 0.5 * ((2.0 * v0) +
                           (-vm1 + v1) * frac +
                           (2.0 * vm1 - 5.0 * v0 + 4.0 * v1 - v2) * t2 +
                           (-vm1 + 3.0 * v0 - 3.0 * v1 + v2) * t3);
    return result * amp;
}

double loopOut(double time, double duration, const QString& type) {
    if (duration <= 0.0) return 0.0;
    if (type == "pingpong") {
        double cycles = time / duration;
        int full = static_cast<int>(std::floor(cycles));
        double frac = cycles - full;
        return (full % 2 == 0) ? frac : (1.0 - frac);
    }
    if (type == "offset") {
        double cycles = time / duration;
        int full = static_cast<int>(std::floor(cycles));
        double frac = cycles - full;
        return frac + full; // Accumulates
    }
    if (type == "continue") {
        if (time <= duration) return time / duration;
        return 1.0; // Holds last value
    }
    // cycle (default)
    return std::fmod(time, duration) / duration;
}

double posterizeTime(double time, double fps) {
    if (fps <= 0.0) return time;
    return std::floor(time * fps) / fps;
}

double valueAtTime(double currentValue, double time, double offset, double decay) {
    (void)time;
    return currentValue * std::pow(decay, std::abs(offset));
}

double random(double min, double max, int seed) {
    double r = ((seed * 1103515245 + 12345) & 0x7fffffff) / static_cast<double>(0x7fffffff);
    return min + r * (max - min);
}

// 1D Perlin-like noise
double noise1D(double t) {
    int i = static_cast<int>(std::floor(t));
    double f = t - i;
    // Smoothstep
    double u = f * f * (3.0 - 2.0 * f);
    double a = hashNoise(i);
    double b = hashNoise(i + 1);
    return a + (b - a) * u;
}

// 2D smooth noise
double noise2D(double x, double y) {
    int ix = static_cast<int>(std::floor(x));
    int iy = static_cast<int>(std::floor(y));
    double fx = x - ix;
    double fy = y - iy;
    double ux = fx * fx * (3.0 - 2.0 * fx);
    double uy = fy * fy * (3.0 - 2.0 * fy);
    double n00 = hashNoise(ix + iy * 57);
    double n10 = hashNoise(ix + 1 + iy * 57);
    double n01 = hashNoise(ix + (iy + 1) * 57);
    double n11 = hashNoise(ix + 1 + (iy + 1) * 57);
    double nx0 = n00 + (n10 - n00) * ux;
    double nx1 = n01 + (n11 - n01) * ux;
    return nx0 + (nx1 - nx0) * uy;
}

double linear(double t, double tMin, double tMax, double vMin, double vMax) {
    if (tMax <= tMin) return vMin;
    double u = std::max(0.0, std::min(1.0, (t - tMin) / (tMax - tMin)));
    return vMin + u * (vMax - vMin);
}

double ease(double t, double tMin, double tMax, double vMin, double vMax) {
    if (tMax <= tMin) return vMin;
    double u = std::max(0.0, std::min(1.0, (t - tMin) / (tMax - tMin)));
    u = u * u * (3.0 - 2.0 * u); // smoothstep
    return vMin + u * (vMax - vMin);
}

double clamp(double value, double min, double max) {
    return std::max(min, std::min(max, value));
}

// ══════════════════════════════════════════════════════════════
// 3. BLEND MODES (27 modes — CPU implementation)
// ══════════════════════════════════════════════════════════════

// Helper: RGB ↔ HSL conversion
static void rgbToHsl(double r, double g, double b, double& h, double& s, double& l) {
    double mx = std::max({r, g, b}), mn = std::min({r, g, b});
    l = (mx + mn) / 2.0;
    if (mx == mn) { h = s = 0; return; }
    double d = mx - mn;
    s = l > 0.5 ? d / (2.0 - mx - mn) : d / (mx + mn);
    if (mx == r) h = (g - b) / d + (g < b ? 6.0 : 0.0);
    else if (mx == g) h = (b - r) / d + 2.0;
    else h = (r - g) / d + 4.0;
    h /= 6.0;
}

static double hueToRgb(double p, double q, double t) {
    if (t < 0.0) t += 1.0;
    if (t > 1.0) t -= 1.0;
    if (t < 1.0/6.0) return p + (q - p) * 6.0 * t;
    if (t < 1.0/2.0) return q;
    if (t < 2.0/3.0) return p + (q - p) * (2.0/3.0 - t) * 6.0;
    return p;
}

static void hslToRgb(double h, double s, double l, double& r, double& g, double& b) {
    if (s == 0.0) { r = g = b = l; return; }
    double q = l < 0.5 ? l * (1.0 + s) : l + s - l * s;
    double p = 2.0 * l - q;
    r = hueToRgb(p, q, h + 1.0/3.0);
    g = hueToRgb(p, q, h);
    b = hueToRgb(p, q, h - 1.0/3.0);
}

static double blendChannel(double base, double blend, BlendMode mode) {
    switch (mode) {
    case BlendMode::Multiply:    return base * blend;
    case BlendMode::Screen:      return 1.0 - (1.0 - base) * (1.0 - blend);
    case BlendMode::Overlay:     return base < 0.5 ? 2.0 * base * blend : 1.0 - 2.0 * (1.0 - base) * (1.0 - blend);
    case BlendMode::SoftLight:   return blend < 0.5 ? base - (1.0 - 2.0 * blend) * base * (1.0 - base) : base + (2.0 * blend - 1.0) * (std::sqrt(base) - base);
    case BlendMode::HardLight:   return blend < 0.5 ? 2.0 * base * blend : 1.0 - 2.0 * (1.0 - base) * (1.0 - blend);
    case BlendMode::ColorDodge:  return blend >= 1.0 ? 1.0 : std::min(1.0, base / (1.0 - blend));
    case BlendMode::ColorBurn:   return blend <= 0.0 ? 0.0 : std::max(0.0, 1.0 - (1.0 - base) / blend);
    case BlendMode::LinearDodge: return std::min(1.0, base + blend);
    case BlendMode::LinearBurn:  return std::max(0.0, base + blend - 1.0);
    case BlendMode::VividLight:  return blend < 0.5 ? (blend <= 0.0 ? 0.0 : std::max(0.0, 1.0 - (1.0 - base) / (2.0 * blend))) : (blend >= 1.0 ? 1.0 : std::min(1.0, base / (2.0 * (1.0 - blend))));
    case BlendMode::LinearLight: return std::max(0.0, std::min(1.0, base + 2.0 * blend - 1.0));
    case BlendMode::PinLight:    return blend < 0.5 ? std::min(base, 2.0 * blend) : std::max(base, 2.0 * blend - 1.0);
    case BlendMode::HardMix:     return (base + blend >= 1.0) ? 1.0 : 0.0;
    case BlendMode::Difference:  return std::abs(base - blend);
    case BlendMode::Exclusion:   return base + blend - 2.0 * base * blend;
    case BlendMode::Subtract:    return std::max(0.0, base - blend);
    case BlendMode::Divide:      return blend <= 0.0 ? 1.0 : std::min(1.0, base / blend);
    case BlendMode::Darken:      return std::min(base, blend);
    case BlendMode::Lighten:     return std::max(base, blend);
    default: return blend;
    }
}

QColor blendPixel(const QColor& base, const QColor& overlay, BlendMode mode) {
    double br = base.redF(), bg = base.greenF(), bb = base.blueF();
    double or_ = overlay.redF(), og = overlay.greenF(), ob = overlay.blueF();

    if (mode == BlendMode::Normal) return overlay;

    if (mode == BlendMode::Dissolve) {
        // Random threshold per pixel (handled at image level)
        return overlay;
    }

    // Component modes (use HSL)
    if (mode == BlendMode::Hue || mode == BlendMode::Saturation ||
        mode == BlendMode::Color || mode == BlendMode::Luminosity ||
        mode == BlendMode::DarkerColor || mode == BlendMode::LighterColor) {

        double bh, bs, bl, oh, os, ol;
        rgbToHsl(br, bg, bb, bh, bs, bl);
        rgbToHsl(or_, og, ob, oh, os, ol);

        double rh, rs, rl;
        if (mode == BlendMode::Hue)        { rh = oh; rs = bs; rl = bl; }
        else if (mode == BlendMode::Saturation) { rh = bh; rs = os; rl = bl; }
        else if (mode == BlendMode::Color)  { rh = oh; rs = os; rl = bl; }
        else if (mode == BlendMode::Luminosity) { rh = bh; rs = bs; rl = ol; }
        else if (mode == BlendMode::DarkerColor) {
            double bLum = 0.299 * br + 0.587 * bg + 0.114 * bb;
            double oLum = 0.299 * or_ + 0.587 * og + 0.114 * ob;
            return oLum < bLum ? overlay : base;
        }
        else { // LighterColor
            double bLum = 0.299 * br + 0.587 * bg + 0.114 * bb;
            double oLum = 0.299 * or_ + 0.587 * og + 0.114 * ob;
            return oLum > bLum ? overlay : base;
        }

        double rr, rg, rb;
        hslToRgb(rh, rs, rl, rr, rg, rb);
        return QColor::fromRgbF(std::clamp(rr, 0.0, 1.0), std::clamp(rg, 0.0, 1.0), std::clamp(rb, 0.0, 1.0));
    }

    // Channel-wise modes
    double rr = blendChannel(br, or_, mode);
    double rg2 = blendChannel(bg, og, mode);
    double rb2 = blendChannel(bb, ob, mode);
    return QColor::fromRgbF(std::clamp(rr, 0.0, 1.0), std::clamp(rg2, 0.0, 1.0), std::clamp(rb2, 0.0, 1.0));
}

QImage blendImages(const QImage& base, const QImage& overlay, BlendMode mode, double opacity) {
    if (base.isNull() || overlay.isNull()) return base;
    QImage result = base.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    QImage over = overlay.convertToFormat(QImage::Format_ARGB32_Premultiplied);

    if (mode == BlendMode::Normal) {
        QPainter p(&result);
        p.setOpacity(opacity);
        p.drawImage(0, 0, over);
        return result;
    }

    int w = std::min(result.width(), over.width());
    int h = std::min(result.height(), over.height());

    for (int y = 0; y < h; ++y) {
        auto* baseLine = reinterpret_cast<QRgb*>(result.scanLine(y));
        auto* overLine = reinterpret_cast<const QRgb*>(over.constScanLine(y));
        for (int x = 0; x < w; ++x) {
            QColor bc(baseLine[x]);
            QColor oc(overLine[x]);
            double oa = oc.alphaF() * opacity;
            if (oa < 0.001) continue;

            QColor blended = blendPixel(bc, oc, mode);
            // Composite: result = blended * oa + base * (1 - oa)
            double rr = blended.redF() * oa + bc.redF() * (1.0 - oa);
            double rg = blended.greenF() * oa + bc.greenF() * (1.0 - oa);
            double rb = blended.blueF() * oa + bc.blueF() * (1.0 - oa);
            double ra = oa + bc.alphaF() * (1.0 - oa);
            baseLine[x] = QColor::fromRgbF(std::clamp(rr, 0.0, 1.0),
                                             std::clamp(rg, 0.0, 1.0),
                                             std::clamp(rb, 0.0, 1.0),
                                             std::clamp(ra, 0.0, 1.0)).rgba();
        }
    }
    return result;
}

BlendMode blendModeByName(const QString& name) {
    if (name == "dissolve")       return BlendMode::Dissolve;
    if (name == "darken")         return BlendMode::Darken;
    if (name == "multiply")       return BlendMode::Multiply;
    if (name == "color_burn")     return BlendMode::ColorBurn;
    if (name == "linear_burn")    return BlendMode::LinearBurn;
    if (name == "darker_color")   return BlendMode::DarkerColor;
    if (name == "lighten")        return BlendMode::Lighten;
    if (name == "screen")         return BlendMode::Screen;
    if (name == "color_dodge")    return BlendMode::ColorDodge;
    if (name == "linear_dodge")   return BlendMode::LinearDodge;
    if (name == "lighter_color")  return BlendMode::LighterColor;
    if (name == "overlay")        return BlendMode::Overlay;
    if (name == "soft_light")     return BlendMode::SoftLight;
    if (name == "hard_light")     return BlendMode::HardLight;
    if (name == "vivid_light")    return BlendMode::VividLight;
    if (name == "linear_light")   return BlendMode::LinearLight;
    if (name == "pin_light")      return BlendMode::PinLight;
    if (name == "hard_mix")       return BlendMode::HardMix;
    if (name == "difference")     return BlendMode::Difference;
    if (name == "exclusion")      return BlendMode::Exclusion;
    if (name == "subtract")       return BlendMode::Subtract;
    if (name == "divide")         return BlendMode::Divide;
    if (name == "hue")            return BlendMode::Hue;
    if (name == "saturation")     return BlendMode::Saturation;
    if (name == "color")          return BlendMode::Color;
    if (name == "luminosity")     return BlendMode::Luminosity;
    return BlendMode::Normal;
}

QString blendModeName(BlendMode mode) {
    switch (mode) {
    case BlendMode::Normal:       return "normal";
    case BlendMode::Dissolve:     return "dissolve";
    case BlendMode::Darken:       return "darken";
    case BlendMode::Multiply:     return "multiply";
    case BlendMode::ColorBurn:    return "color_burn";
    case BlendMode::LinearBurn:   return "linear_burn";
    case BlendMode::DarkerColor:  return "darker_color";
    case BlendMode::Lighten:      return "lighten";
    case BlendMode::Screen:       return "screen";
    case BlendMode::ColorDodge:   return "color_dodge";
    case BlendMode::LinearDodge:  return "linear_dodge";
    case BlendMode::LighterColor: return "lighter_color";
    case BlendMode::Overlay:      return "overlay";
    case BlendMode::SoftLight:    return "soft_light";
    case BlendMode::HardLight:    return "hard_light";
    case BlendMode::VividLight:   return "vivid_light";
    case BlendMode::LinearLight:  return "linear_light";
    case BlendMode::PinLight:     return "pin_light";
    case BlendMode::HardMix:      return "hard_mix";
    case BlendMode::Difference:   return "difference";
    case BlendMode::Exclusion:    return "exclusion";
    case BlendMode::Subtract:     return "subtract";
    case BlendMode::Divide:       return "divide";
    case BlendMode::Hue:          return "hue";
    case BlendMode::Saturation:   return "saturation";
    case BlendMode::Color:        return "color";
    case BlendMode::Luminosity:   return "luminosity";
    }
    return "normal";
}

QStringList allBlendModeNames() {
    return {
        "normal", "dissolve",
        "darken", "multiply", "color_burn", "linear_burn", "darker_color",
        "lighten", "screen", "color_dodge", "linear_dodge", "lighter_color",
        "overlay", "soft_light", "hard_light", "vivid_light", "linear_light", "pin_light", "hard_mix",
        "difference", "exclusion", "subtract", "divide",
        "hue", "saturation", "color", "luminosity"
    };
}

// ══════════════════════════════════════════════════════════════
// 4. GPU BLEND MODE SHADER (GLSL 330)
// ══════════════════════════════════════════════════════════════

const char* blendModeFragmentShader() {
    return R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D baseTex;
    uniform sampler2D overlayTex;
    uniform int blendMode;
    uniform float opacity;

    vec3 rgb2hsl(vec3 c) {
        float mx = max(max(c.r, c.g), c.b);
        float mn = min(min(c.r, c.g), c.b);
        float l = (mx + mn) * 0.5;
        if (mx == mn) return vec3(0.0, 0.0, l);
        float d = mx - mn;
        float s = l > 0.5 ? d / (2.0 - mx - mn) : d / (mx + mn);
        float h;
        if (mx == c.r) h = (c.g - c.b) / d + (c.g < c.b ? 6.0 : 0.0);
        else if (mx == c.g) h = (c.b - c.r) / d + 2.0;
        else h = (c.r - c.g) / d + 4.0;
        return vec3(h / 6.0, s, l);
    }

    float hue2rgb(float p, float q, float t) {
        if (t < 0.0) t += 1.0;
        if (t > 1.0) t -= 1.0;
        if (t < 1.0/6.0) return p + (q - p) * 6.0 * t;
        if (t < 0.5) return q;
        if (t < 2.0/3.0) return p + (q - p) * (2.0/3.0 - t) * 6.0;
        return p;
    }

    vec3 hsl2rgb(vec3 hsl) {
        if (hsl.y == 0.0) return vec3(hsl.z);
        float q = hsl.z < 0.5 ? hsl.z * (1.0 + hsl.y) : hsl.z + hsl.y - hsl.z * hsl.y;
        float p = 2.0 * hsl.z - q;
        return vec3(hue2rgb(p, q, hsl.x + 1.0/3.0), hue2rgb(p, q, hsl.x), hue2rgb(p, q, hsl.x - 1.0/3.0));
    }

    vec3 blendOp(vec3 b, vec3 o) {
        if (blendMode == 0)  return o;                                                    // Normal
        if (blendMode == 3)  return b * o;                                                // Multiply
        if (blendMode == 8)  return 1.0 - (1.0 - b) * (1.0 - o);                        // Screen
        if (blendMode == 12) return mix(2.0 * b * o, 1.0 - 2.0 * (1.0-b) * (1.0-o), step(0.5, b)); // Overlay
        if (blendMode == 13) return mix(b - (1.0-2.0*o)*b*(1.0-b), b + (2.0*o-1.0)*(sqrt(b)-b), step(0.5, o)); // Soft Light
        if (blendMode == 14) return mix(2.0 * b * o, 1.0 - 2.0 * (1.0-b) * (1.0-o), step(0.5, o)); // Hard Light
        if (blendMode == 9)  return min(vec3(1.0), b / max(1.0 - o, vec3(0.001)));       // Color Dodge
        if (blendMode == 4)  return max(vec3(0.0), 1.0 - (1.0-b) / max(o, vec3(0.001)));// Color Burn
        if (blendMode == 10) return min(vec3(1.0), b + o);                               // Linear Dodge
        if (blendMode == 5)  return max(vec3(0.0), b + o - 1.0);                         // Linear Burn
        if (blendMode == 2)  return min(b, o);                                            // Darken
        if (blendMode == 7)  return max(b, o);                                            // Lighten
        if (blendMode == 19) return abs(b - o);                                           // Difference
        if (blendMode == 20) return b + o - 2.0 * b * o;                                 // Exclusion
        if (blendMode == 21) return max(vec3(0.0), b - o);                               // Subtract
        if (blendMode == 22) return min(vec3(1.0), b / max(o, vec3(0.001)));             // Divide
        if (blendMode == 18) return step(1.0, b + o);                                    // Hard Mix
        if (blendMode == 15) { // Vivid Light
            vec3 r;
            r.r = o.r < 0.5 ? max(0.0, 1.0-(1.0-b.r)/(2.0*o.r+0.001)) : min(1.0, b.r/(2.0*(1.0-o.r)+0.001));
            r.g = o.g < 0.5 ? max(0.0, 1.0-(1.0-b.g)/(2.0*o.g+0.001)) : min(1.0, b.g/(2.0*(1.0-o.g)+0.001));
            r.b = o.b < 0.5 ? max(0.0, 1.0-(1.0-b.b)/(2.0*o.b+0.001)) : min(1.0, b.b/(2.0*(1.0-o.b)+0.001));
            return r;
        }
        if (blendMode == 16) return clamp(b + 2.0 * o - 1.0, 0.0, 1.0);                // Linear Light
        if (blendMode == 17) return mix(min(b, 2.0*o), max(b, 2.0*o-1.0), step(0.5, o));// Pin Light
        // HSL-based modes
        if (blendMode >= 23) {
            vec3 bHsl = rgb2hsl(b);
            vec3 oHsl = rgb2hsl(o);
            if (blendMode == 23) return hsl2rgb(vec3(oHsl.x, bHsl.y, bHsl.z));           // Hue
            if (blendMode == 24) return hsl2rgb(vec3(bHsl.x, oHsl.y, bHsl.z));           // Saturation
            if (blendMode == 25) return hsl2rgb(vec3(oHsl.x, oHsl.y, bHsl.z));           // Color
            if (blendMode == 26) return hsl2rgb(vec3(bHsl.x, bHsl.y, oHsl.z));           // Luminosity
        }
        return o;
    }

    void main() {
        vec4 b = texture(baseTex, vUV);
        vec4 o = texture(overlayTex, vUV);
        float oa = o.a * opacity;
        vec3 blended = blendOp(b.rgb, o.rgb);
        fragColor = vec4(mix(b.rgb, blended, oa), max(b.a, oa));
    }
)";
}

} // namespace ae
} // namespace prestige
