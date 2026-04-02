// ============================================================
// Prestige AI — After Effects GPU Shader Effects
// 90+ GPU-accelerated effects across 7 AE categories
// All shaders: GLSL 330 Core — runs on macOS Metal via Qt RHI
// ============================================================

#include "GpuEffects.h"
#include "AEEasing.h"
#include <QDebug>
#include <QOpenGLExtraFunctions>
#include <QPainter>
#include <cmath>

namespace prestige {

// ── Shared vertex shader (already in GpuEffects.cpp) ────────
extern const char* VERT_SHADER;

// ══════════════════════════════════════════════════════════════
// AE DISTORTION SHADERS
// ══════════════════════════════════════════════════════════════

static const char* TURBULENT_DISPLACE_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform float amount;
    uniform float size;
    uniform float time;

    float hash(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }

    float vnoise(vec2 p) {
        vec2 i = floor(p), f = fract(p);
        f = f * f * (3.0 - 2.0 * f);
        return mix(mix(hash(i), hash(i + vec2(1,0)), f.x),
                   mix(hash(i + vec2(0,1)), hash(i + vec2(1,1)), f.x), f.y);
    }

    float fbm(vec2 p) {
        float v = 0.0, a = 0.5;
        for (int i = 0; i < 6; i++) { v += a * vnoise(p); p *= 2.0; a *= 0.5; }
        return v;
    }

    void main() {
        vec2 p = vUV * size + time * 0.5;
        float dx = fbm(p + vec2(1.7, 9.2)) - 0.5;
        float dy = fbm(p + vec2(8.3, 2.8)) - 0.5;
        vec2 displaced = vUV + vec2(dx, dy) * amount * 0.01;
        displaced = clamp(displaced, 0.0, 1.0);
        fragColor = texture(tex, displaced);
    }
)";

static const char* TWIRL_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform float angle;
    uniform float radius;
    uniform vec2 center;

    void main() {
        vec2 d = vUV - center;
        float dist = length(d);
        float factor = max(0.0, 1.0 - dist / radius);
        float a = angle * factor * factor; // Quadratic falloff like AE
        float s = sin(a), c = cos(a);
        vec2 rotated = center + vec2(c * d.x - s * d.y, s * d.x + c * d.y);
        rotated = clamp(rotated, 0.0, 1.0);
        fragColor = texture(tex, rotated);
    }
)";

static const char* SPHERIZE_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform float radius;
    uniform vec2 center;

    void main() {
        vec2 d = (vUV - center) / radius;
        float dist = length(d);
        if (dist < 1.0) {
            float z = sqrt(1.0 - dist * dist);
            vec2 refracted = center + d * z * radius;
            refracted = clamp(refracted, 0.0, 1.0);
            fragColor = texture(tex, refracted);
        } else {
            fragColor = texture(tex, vUV);
        }
    }
)";

static const char* RIPPLE_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform float amplitude;
    uniform float frequency;
    uniform float phase;

    void main() {
        vec2 uv = vUV;
        uv.x += sin(uv.y * frequency + phase) * amplitude;
        uv.y += cos(uv.x * frequency + phase * 0.7) * amplitude * 0.5;
        uv = clamp(uv, 0.0, 1.0);
        fragColor = texture(tex, uv);
    }
)";

static const char* WAVE_WARP_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform float amplitude;
    uniform float frequency;
    uniform float phase;
    uniform float direction; // Radians

    void main() {
        vec2 dir = vec2(cos(direction), sin(direction));
        vec2 perp = vec2(-dir.y, dir.x);
        float d = dot(vUV - 0.5, dir);
        float wave = sin(d * frequency + phase) * amplitude;
        vec2 uv = vUV + perp * wave;
        uv = clamp(uv, 0.0, 1.0);
        fragColor = texture(tex, uv);
    }
)";

static const char* KALEIDOSCOPE_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform int segments;
    uniform float rotation;

    void main() {
        vec2 p = vUV - 0.5;
        float angle = atan(p.y, p.x) + rotation;
        float segAngle = 6.28318530718 / float(segments);
        angle = mod(angle, segAngle);
        if (angle > segAngle * 0.5) angle = segAngle - angle; // Mirror
        float r = length(p);
        vec2 uv = 0.5 + r * vec2(cos(angle), sin(angle));
        uv = clamp(uv, 0.0, 1.0);
        fragColor = texture(tex, uv);
    }
)";

// ══════════════════════════════════════════════════════════════
// AE COLOR CORRECTION SHADERS
// ══════════════════════════════════════════════════════════════

static const char* CURVES_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform float shadows;    // -1 to 1
    uniform float midtones;   // -1 to 1
    uniform float highlights; // -1 to 1

    void main() {
        vec4 c = texture(tex, vUV);
        // S-curve with 3 control points (shadows, midtones, highlights)
        vec3 result;
        for (int i = 0; i < 3; i++) {
            float v = c[i];
            // Shadow region (0-0.33)
            if (v < 0.333) v += shadows * 0.15 * (1.0 - v / 0.333);
            // Midtone region (0.33-0.66)
            else if (v < 0.666) v += midtones * 0.15 * sin((v - 0.333) * 3.0 * 3.14159);
            // Highlight region (0.66-1.0)
            else v += highlights * 0.15 * ((v - 0.666) / 0.334);
            result[i] = clamp(v, 0.0, 1.0);
        }
        fragColor = vec4(result, c.a);
    }
)";

static const char* LEVELS_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform float inBlack;    // 0-1 (AE: 0-255)
    uniform float inWhite;    // 0-1
    uniform float gamma;      // 0.1-10
    uniform float outBlack;   // 0-1
    uniform float outWhite;   // 0-1

    void main() {
        vec4 c = texture(tex, vUV);
        // Input levels: remap
        vec3 v = clamp((c.rgb - inBlack) / max(inWhite - inBlack, 0.001), 0.0, 1.0);
        // Gamma
        v = pow(v, vec3(1.0 / gamma));
        // Output levels
        v = outBlack + v * (outWhite - outBlack);
        fragColor = vec4(clamp(v, 0.0, 1.0), c.a);
    }
)";

static const char* HUE_SAT_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform float hueShift;      // -180 to 180 (degrees / 360)
    uniform float saturation;    // -1 to 1
    uniform float lightness;     // -1 to 1

    vec3 rgb2hsl(vec3 c) {
        float mx = max(max(c.r, c.g), c.b), mn = min(min(c.r, c.g), c.b);
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
        if (t < 0.0) t += 1.0; if (t > 1.0) t -= 1.0;
        if (t < 1.0/6.0) return p + (q-p) * 6.0 * t;
        if (t < 0.5) return q;
        if (t < 2.0/3.0) return p + (q-p) * (2.0/3.0 - t) * 6.0;
        return p;
    }

    vec3 hsl2rgb(vec3 hsl) {
        if (hsl.y == 0.0) return vec3(hsl.z);
        float q = hsl.z < 0.5 ? hsl.z*(1.0+hsl.y) : hsl.z+hsl.y-hsl.z*hsl.y;
        float p = 2.0*hsl.z - q;
        return vec3(hue2rgb(p,q,hsl.x+1.0/3.0), hue2rgb(p,q,hsl.x), hue2rgb(p,q,hsl.x-1.0/3.0));
    }

    void main() {
        vec4 c = texture(tex, vUV);
        vec3 hsl = rgb2hsl(c.rgb);
        hsl.x = fract(hsl.x + hueShift);
        hsl.y = clamp(hsl.y + saturation, 0.0, 1.0);
        hsl.z = clamp(hsl.z + lightness, 0.0, 1.0);
        fragColor = vec4(hsl2rgb(hsl), c.a);
    }
)";

static const char* BRIGHT_CONTRAST_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform float brightness;  // -1 to 1
    uniform float contrast;    // -1 to 1

    void main() {
        vec4 c = texture(tex, vUV);
        vec3 v = c.rgb + brightness;
        v = (v - 0.5) * (1.0 + contrast) + 0.5;
        fragColor = vec4(clamp(v, 0.0, 1.0), c.a);
    }
)";

static const char* TINT_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform vec3 mapBlack;
    uniform vec3 mapWhite;
    uniform float amount;

    void main() {
        vec4 c = texture(tex, vUV);
        float lum = dot(c.rgb, vec3(0.299, 0.587, 0.114));
        vec3 tinted = mix(mapBlack, mapWhite, lum);
        fragColor = vec4(mix(c.rgb, tinted, amount), c.a);
    }
)";

static const char* TRITONE_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform vec3 shadowCol;
    uniform vec3 midtoneCol;
    uniform vec3 highlightCol;

    void main() {
        vec4 c = texture(tex, vUV);
        float lum = dot(c.rgb, vec3(0.299, 0.587, 0.114));
        vec3 result;
        if (lum < 0.5) result = mix(shadowCol, midtoneCol, lum * 2.0);
        else result = mix(midtoneCol, highlightCol, (lum - 0.5) * 2.0);
        fragColor = vec4(result, c.a);
    }
)";

static const char* LEAVE_COLOR_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform vec3 targetColor;
    uniform float tolerance;
    uniform float feather;

    void main() {
        vec4 c = texture(tex, vUV);
        float diff = length(c.rgb - targetColor);
        float mask = smoothstep(tolerance, tolerance + feather, diff);
        float lum = dot(c.rgb, vec3(0.299, 0.587, 0.114));
        vec3 gray = vec3(lum);
        fragColor = vec4(mix(c.rgb, gray, mask), c.a);
    }
)";

static const char* VIBRANCE_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform float amount;

    void main() {
        vec4 c = texture(tex, vUV);
        float mx = max(max(c.r, c.g), c.b);
        float mn = min(min(c.r, c.g), c.b);
        float sat = mx - mn;
        // Vibrance: boost low-saturation more, high-saturation less
        float boost = amount * (1.0 - sat) * (1.0 - sat);
        float lum = dot(c.rgb, vec3(0.299, 0.587, 0.114));
        fragColor = vec4(mix(vec3(lum), c.rgb, 1.0 + boost), c.a);
    }
)";

// ══════════════════════════════════════════════════════════════
// AE GENERATE SHADERS
// ══════════════════════════════════════════════════════════════

static const char* FRACTAL_NOISE_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform float scale;
    uniform float complexity;
    uniform float evolution;
    uniform float noiseOpacity;

    float hash(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }

    float vnoise(vec2 p) {
        vec2 i = floor(p), f = fract(p);
        f = f * f * (3.0 - 2.0 * f);
        return mix(mix(hash(i), hash(i+vec2(1,0)), f.x),
                   mix(hash(i+vec2(0,1)), hash(i+vec2(1,1)), f.x), f.y);
    }

    void main() {
        vec4 c = texture(tex, vUV);
        vec2 p = vUV * scale + evolution;
        float n = 0.0, amp = 1.0, freq = 1.0;
        int octaves = int(complexity);
        for (int i = 0; i < 8; i++) {
            if (i >= octaves) break;
            n += amp * vnoise(p * freq);
            freq *= 2.0; amp *= 0.5;
        }
        n = n * 0.5 + 0.5;
        fragColor = vec4(mix(c.rgb, vec3(n), noiseOpacity), c.a);
    }
)";

static const char* CELL_PATTERN_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform float size;
    uniform float scatter;
    uniform float time;

    float hash(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }

    void main() {
        vec4 c = texture(tex, vUV);
        vec2 p = vUV * size;
        vec2 ip = floor(p);
        float minDist = 10.0;

        for (int y = -1; y <= 1; y++) {
            for (int x = -1; x <= 1; x++) {
                vec2 neighbor = ip + vec2(x, y);
                vec2 point = neighbor + hash(neighbor) * scatter;
                point += sin(time + 6.28 * hash(neighbor + 0.5)) * 0.1;
                float d = length(p - point);
                minDist = min(minDist, d);
            }
        }

        float cell = smoothstep(0.0, 0.6, minDist);
        fragColor = vec4(mix(c.rgb, vec3(cell), 0.5), c.a);
    }
)";

// ══════════════════════════════════════════════════════════════
// AE STYLIZE SHADERS
// ══════════════════════════════════════════════════════════════

static const char* EMBOSS_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform vec2 texSize;
    uniform float angle;   // Radians
    uniform float height;
    uniform float amount;

    void main() {
        vec2 dir = vec2(cos(angle), sin(angle)) / texSize;
        vec4 tl = texture(tex, vUV - dir * height);
        vec4 br = texture(tex, vUV + dir * height);
        vec4 c = texture(tex, vUV);
        vec3 embossed = vec3(0.5) + (tl.rgb - br.rgb) * amount;
        fragColor = vec4(mix(c.rgb, embossed, amount), c.a);
    }
)";

static const char* FIND_EDGES_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform vec2 texSize;
    uniform int invertEdges;

    void main() {
        vec2 px = 1.0 / texSize;
        // Sobel operator
        vec3 tl = texture(tex, vUV + vec2(-px.x, -px.y)).rgb;
        vec3 t  = texture(tex, vUV + vec2(0, -px.y)).rgb;
        vec3 tr = texture(tex, vUV + vec2(px.x, -px.y)).rgb;
        vec3 l  = texture(tex, vUV + vec2(-px.x, 0)).rgb;
        vec3 r  = texture(tex, vUV + vec2(px.x, 0)).rgb;
        vec3 bl = texture(tex, vUV + vec2(-px.x, px.y)).rgb;
        vec3 b  = texture(tex, vUV + vec2(0, px.y)).rgb;
        vec3 br_ = texture(tex, vUV + vec2(px.x, px.y)).rgb;

        vec3 gx = -tl - 2.0*l - bl + tr + 2.0*r + br_;
        vec3 gy = -tl - 2.0*t - tr + bl + 2.0*b + br_;
        vec3 edge = sqrt(gx*gx + gy*gy);
        float v = dot(edge, vec3(0.333));
        if (invertEdges == 1) v = 1.0 - v;
        fragColor = vec4(vec3(v), texture(tex, vUV).a);
    }
)";

static const char* HALFTONE_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform float dotSize;
    uniform float angle;

    void main() {
        vec4 c = texture(tex, vUV);
        float lum = dot(c.rgb, vec3(0.299, 0.587, 0.114));

        float s = sin(angle), co = cos(angle);
        vec2 rotUV = vec2(co * vUV.x - s * vUV.y, s * vUV.x + co * vUV.y);
        vec2 cell = fract(rotUV * dotSize) - 0.5;
        float dist = length(cell);
        float dot_threshold = (1.0 - lum) * 0.5;
        float alpha = step(dist, dot_threshold);
        fragColor = vec4(c.rgb * alpha, c.a);
    }
)";

static const char* STAINED_GLASS_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform float cellSize;
    uniform float edgeWidth;
    uniform vec3 edgeColor;

    float hash(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }

    void main() {
        vec2 p = vUV * cellSize;
        vec2 ip = floor(p);
        float minDist = 10.0;
        float secondDist = 10.0;
        vec2 closestCell = ip;

        for (int y = -1; y <= 1; y++) {
            for (int x = -1; x <= 1; x++) {
                vec2 neighbor = ip + vec2(x, y);
                vec2 point = neighbor + vec2(hash(neighbor), hash(neighbor + 1.0)) * 0.8 + 0.1;
                float d = length(p - point);
                if (d < minDist) { secondDist = minDist; minDist = d; closestCell = neighbor; }
                else if (d < secondDist) { secondDist = d; }
            }
        }

        float edge = smoothstep(edgeWidth, edgeWidth + 0.02, secondDist - minDist);
        vec2 cellUV = (closestCell + 0.5) / cellSize;
        vec4 cellColor = texture(tex, clamp(cellUV, 0.0, 1.0));
        fragColor = vec4(mix(edgeColor, cellColor.rgb, edge), cellColor.a);
    }
)";

// ══════════════════════════════════════════════════════════════
// AE PERSPECTIVE SHADERS
// ══════════════════════════════════════════════════════════════

static const char* CC_SPHERE_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform float rotX;
    uniform float rotY;
    uniform float radius;

    void main() {
        vec2 p = (vUV - 0.5) * 2.0;
        float r = length(p) / radius;
        if (r > 1.0) { fragColor = vec4(0.0); return; }

        float z = sqrt(1.0 - r * r);
        vec3 normal = normalize(vec3(p.x / radius, p.y / radius, z));

        // Apply rotation
        float cx = cos(rotX), sx = sin(rotX);
        float cy = cos(rotY), sy = sin(rotY);
        vec3 rotNormal;
        rotNormal.x = cy * normal.x + sy * normal.z;
        rotNormal.y = sx * sy * normal.x + cx * normal.y - sx * cy * normal.z;
        rotNormal.z = -cx * sy * normal.x + sx * normal.y + cx * cy * normal.z;

        // Equirectangular mapping
        vec2 uv;
        uv.x = atan(rotNormal.x, rotNormal.z) / 6.28318 + 0.5;
        uv.y = asin(clamp(rotNormal.y, -1.0, 1.0)) / 3.14159 + 0.5;

        // Lighting
        vec3 lightDir = normalize(vec3(0.5, 0.7, 1.0));
        float diffuse = max(dot(normal, lightDir), 0.0);
        float ambient = 0.3;

        vec4 c = texture(tex, uv);
        fragColor = vec4(c.rgb * (ambient + diffuse * 0.7), c.a * step(r, 1.0));
    }
)";

static const char* DROP_SHADOW_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform vec2 texSize;
    uniform float angle;
    uniform float distance_;
    uniform float softness;
    uniform vec4 shadowColor;

    void main() {
        vec2 offset = vec2(cos(angle), sin(angle)) * distance_ / texSize;
        vec4 c = texture(tex, vUV);

        // Sample alpha at offset for shadow
        float shadowAlpha = 0.0;
        int samples = int(max(softness * 3.0, 1.0));
        float total = 0.0;
        for (int x = -samples; x <= samples; x++) {
            for (int y = -samples; y <= samples; y++) {
                float w = exp(-float(x*x + y*y) / max(softness * softness, 0.5));
                vec2 sampleUV = vUV - offset + vec2(float(x), float(y)) / texSize;
                shadowAlpha += texture(tex, clamp(sampleUV, 0.0, 1.0)).a * w;
                total += w;
            }
        }
        shadowAlpha /= total;

        vec3 shadow = shadowColor.rgb;
        float sa = shadowAlpha * shadowColor.a;
        // Composite: shadow behind, original on top
        vec3 result = mix(shadow * sa, c.rgb, c.a);
        float resultA = max(c.a, sa);
        fragColor = vec4(result, resultA);
    }
)";

// ══════════════════════════════════════════════════════════════
// IMPLEMENTATIONS — AE Distortions
// ══════════════════════════════════════════════════════════════

QImage GpuEffects::applyTurbulentDisplace(const QImage& frame, double amount, double size, double time) {
    if (!m_initialized) return frame;
    if (!m_turbulentDisplaceShader) {
        m_turbulentDisplaceShader = std::make_unique<QOpenGLShaderProgram>();
        if (!compileShader(*m_turbulentDisplaceShader, VERT_SHADER, TURBULENT_DISPLACE_FRAG))
            return frame;
    }
    return renderToImage(frame, *m_turbulentDisplaceShader, [&](QOpenGLShaderProgram& s) {
        s.setUniformValue("amount", static_cast<float>(amount));
        s.setUniformValue("size", static_cast<float>(size));
        s.setUniformValue("time", static_cast<float>(time));
    });
}

QImage GpuEffects::applyTwirl(const QImage& frame, double angle, double radius, double cx, double cy) {
    if (!m_initialized) return frame;
    if (!m_twirlShader) {
        m_twirlShader = std::make_unique<QOpenGLShaderProgram>();
        if (!compileShader(*m_twirlShader, VERT_SHADER, TWIRL_FRAG)) return frame;
    }
    return renderToImage(frame, *m_twirlShader, [&](QOpenGLShaderProgram& s) {
        s.setUniformValue("angle", static_cast<float>(angle));
        s.setUniformValue("radius", static_cast<float>(radius));
        s.setUniformValue("center", QVector2D(cx, cy));
    });
}

QImage GpuEffects::applySpherize(const QImage& frame, double radius, double cx, double cy) {
    if (!m_initialized) return frame;
    if (!m_spherizeShader) {
        m_spherizeShader = std::make_unique<QOpenGLShaderProgram>();
        if (!compileShader(*m_spherizeShader, VERT_SHADER, SPHERIZE_FRAG)) return frame;
    }
    return renderToImage(frame, *m_spherizeShader, [&](QOpenGLShaderProgram& s) {
        s.setUniformValue("radius", static_cast<float>(radius));
        s.setUniformValue("center", QVector2D(cx, cy));
    });
}

QImage GpuEffects::applyRipple(const QImage& frame, double amplitude, double frequency, double phase) {
    if (!m_initialized) return frame;
    if (!m_rippleShader) {
        m_rippleShader = std::make_unique<QOpenGLShaderProgram>();
        if (!compileShader(*m_rippleShader, VERT_SHADER, RIPPLE_FRAG)) return frame;
    }
    return renderToImage(frame, *m_rippleShader, [&](QOpenGLShaderProgram& s) {
        s.setUniformValue("amplitude", static_cast<float>(amplitude));
        s.setUniformValue("frequency", static_cast<float>(frequency));
        s.setUniformValue("phase", static_cast<float>(phase));
    });
}

QImage GpuEffects::applyWaveWarp(const QImage& frame, double amplitude, double frequency, double phase, double direction) {
    if (!m_initialized) return frame;
    if (!m_waveWarpShader) {
        m_waveWarpShader = std::make_unique<QOpenGLShaderProgram>();
        if (!compileShader(*m_waveWarpShader, VERT_SHADER, WAVE_WARP_FRAG)) return frame;
    }
    return renderToImage(frame, *m_waveWarpShader, [&](QOpenGLShaderProgram& s) {
        s.setUniformValue("amplitude", static_cast<float>(amplitude));
        s.setUniformValue("frequency", static_cast<float>(frequency));
        s.setUniformValue("phase", static_cast<float>(phase));
        s.setUniformValue("direction", static_cast<float>(direction));
    });
}

QImage GpuEffects::applyKaleidoscope(const QImage& frame, int segments, double rotation) {
    if (!m_initialized) return frame;
    if (!m_kaleidoscopeShader) {
        m_kaleidoscopeShader = std::make_unique<QOpenGLShaderProgram>();
        if (!compileShader(*m_kaleidoscopeShader, VERT_SHADER, KALEIDOSCOPE_FRAG)) return frame;
    }
    return renderToImage(frame, *m_kaleidoscopeShader, [&](QOpenGLShaderProgram& s) {
        s.setUniformValue("segments", segments);
        s.setUniformValue("rotation", static_cast<float>(rotation));
    });
}

// ── Unique distortion implementations ────────────────────────

QImage GpuEffects::applyMeshWarp(const QImage& frame, double intensity, double time) {
    // AE Mesh Warp: grid-based deformation with smooth Bezier interpolation
    // Uses a 4x4 control grid where each point displaces smoothly
    QImage result(frame.size(), QImage::Format_ARGB32_Premultiplied);
    result.fill(Qt::transparent);
    int w = frame.width(), h = frame.height();
    int gridSize = 4;
    // Generate animated control point offsets
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            double gx = static_cast<double>(x) / w * gridSize;
            double gy = static_cast<double>(y) / h * gridSize;
            int ix = std::min(static_cast<int>(gx), gridSize - 1);
            int iy = std::min(static_cast<int>(gy), gridSize - 1);
            double fx = gx - ix, fy = gy - iy;
            // Smooth interpolation (bicubic-like via smoothstep)
            fx = fx * fx * (3.0 - 2.0 * fx);
            fy = fy * fy * (3.0 - 2.0 * fy);
            // Per-grid-point displacement (animated)
            auto gridDisp = [&](int gi, int gj) -> std::pair<double, double> {
                double seed = gi * 7.13 + gj * 13.37 + time * 0.8;
                double dx = std::sin(seed * 2.7) * intensity * 0.01;
                double dy = std::cos(seed * 3.1) * intensity * 0.01;
                return {dx, dy};
            };
            auto [d00x, d00y] = gridDisp(ix, iy);
            auto [d10x, d10y] = gridDisp(ix + 1, iy);
            auto [d01x, d01y] = gridDisp(ix, iy + 1);
            auto [d11x, d11y] = gridDisp(ix + 1, iy + 1);
            double dispX = d00x * (1 - fx) * (1 - fy) + d10x * fx * (1 - fy) + d01x * (1 - fx) * fy + d11x * fx * fy;
            double dispY = d00y * (1 - fx) * (1 - fy) + d10y * fx * (1 - fy) + d01y * (1 - fx) * fy + d11y * fx * fy;
            int sx = std::clamp(x + static_cast<int>(dispX * w), 0, w - 1);
            int sy = std::clamp(y + static_cast<int>(dispY * h), 0, h - 1);
            result.setPixelColor(x, y, frame.pixelColor(sx, sy));
        }
    }
    return result;
}

QImage GpuEffects::applyLiquify(const QImage& frame, double cx, double cy, double radius, double strength) {
    // AE Liquify: push distortion from center point outward with falloff
    QImage result(frame.size(), QImage::Format_ARGB32_Premultiplied);
    int w = frame.width(), h = frame.height();
    double centerX = cx * w, centerY = cy * h;
    double rad = radius * std::min(w, h);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            double dx = x - centerX, dy = y - centerY;
            double dist = std::sqrt(dx * dx + dy * dy);
            if (dist < rad && dist > 0.001) {
                // Push/pull with quadratic falloff (like AE Liquify brush)
                double falloff = 1.0 - (dist / rad);
                falloff = falloff * falloff; // Quadratic
                double pushX = dx / dist * strength * falloff * rad * 0.3;
                double pushY = dy / dist * strength * falloff * rad * 0.3;
                int sx = std::clamp(x - static_cast<int>(pushX), 0, w - 1);
                int sy = std::clamp(y - static_cast<int>(pushY), 0, h - 1);
                result.setPixelColor(x, y, frame.pixelColor(sx, sy));
            } else {
                result.setPixelColor(x, y, frame.pixelColor(x, y));
            }
        }
    }
    return result;
}

QImage GpuEffects::applyPolarCoords(const QImage& frame, bool toPolar) {
    // CPU fallback — polar transform
    QImage result(frame.size(), QImage::Format_ARGB32_Premultiplied);
    result.fill(Qt::transparent);
    int w = frame.width(), h = frame.height();
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            double nx = (x - w / 2.0) / (w / 2.0);
            double ny = (y - h / 2.0) / (h / 2.0);
            int sx, sy;
            if (toPolar) {
                double r = std::sqrt(nx * nx + ny * ny);
                double theta = std::atan2(ny, nx) / M_PI * 0.5 + 0.5;
                sx = static_cast<int>(theta * w) % w;
                sy = static_cast<int>(r * h) % h;
            } else {
                double theta = nx * M_PI;
                double r = ny * 0.5 + 0.5;
                sx = static_cast<int>((std::cos(theta) * r + 1.0) * 0.5 * w) % w;
                sy = static_cast<int>((std::sin(theta) * r + 1.0) * 0.5 * h) % h;
            }
            if (sx >= 0 && sx < w && sy >= 0 && sy < h)
                result.setPixelColor(x, y, frame.pixelColor(sx, sy));
        }
    }
    return result;
}

QImage GpuEffects::applyBulge(const QImage& frame, double radius, double strength, double cx, double cy) {
    // AE Bulge: barrel (positive) / pincushion (negative) distortion
    // Distinct from Spherize: supports negative strength (inward push)
    QImage result(frame.size(), QImage::Format_ARGB32_Premultiplied);
    int w = frame.width(), h = frame.height();
    double centerX = cx * w, centerY = cy * h;
    double rad = radius * std::min(w, h);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            double dx = (x - centerX) / rad, dy = (y - centerY) / rad;
            double dist = std::sqrt(dx * dx + dy * dy);
            if (dist < 1.0) {
                // Barrel/pincushion: r' = r + strength * r * (1 - r^2)
                double newDist = dist + strength * dist * (1.0 - dist * dist);
                double scale = (dist > 0.001) ? newDist / dist : 1.0;
                int sx = std::clamp(static_cast<int>(centerX + dx * rad * scale), 0, w - 1);
                int sy = std::clamp(static_cast<int>(centerY + dy * rad * scale), 0, h - 1);
                result.setPixelColor(x, y, frame.pixelColor(sx, sy));
            } else {
                result.setPixelColor(x, y, frame.pixelColor(x, y));
            }
        }
    }
    return result;
}

QImage GpuEffects::applyDisplacementMap(const QImage& frame, const QImage& map, double horizontal, double vertical) {
    if (frame.isNull() || map.isNull()) return frame;
    QImage result(frame.size(), QImage::Format_ARGB32_Premultiplied);
    int w = frame.width(), h = frame.height();
    QImage scaledMap = map.scaled(w, h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            QColor mc = scaledMap.pixelColor(x, y);
            int dx = x + static_cast<int>((mc.redF() - 0.5) * horizontal);
            int dy = y + static_cast<int>((mc.greenF() - 0.5) * vertical);
            dx = std::clamp(dx, 0, w - 1);
            dy = std::clamp(dy, 0, h - 1);
            result.setPixelColor(x, y, frame.pixelColor(dx, dy));
        }
    }
    return result;
}

QImage GpuEffects::applyMirror(const QImage& frame, double angle, double offset) {
    QImage result = frame.copy();
    QPainter p(&result);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    QTransform t;
    t.translate(frame.width() / 2.0, frame.height() / 2.0);
    t.rotate(angle);
    t.scale(-1, 1);
    t.rotate(-angle);
    t.translate(-frame.width() / 2.0 + offset, -frame.height() / 2.0);
    p.setTransform(t);
    p.setOpacity(0.5);
    p.drawImage(0, 0, frame);
    return result;
}

QImage GpuEffects::applyMagnify(const QImage& frame, double cx, double cy, double radius, double magnification) {
    // AE Magnify: flat zoom (loupe) within a circular region — no distortion
    QImage result = frame.copy();
    int w = frame.width(), h = frame.height();
    double centerX = cx * w, centerY = cy * h;
    double rad = radius * std::min(w, h) * 0.3;
    for (int y = std::max(0, static_cast<int>(centerY - rad)); y < std::min(h, static_cast<int>(centerY + rad)); ++y) {
        for (int x = std::max(0, static_cast<int>(centerX - rad)); x < std::min(w, static_cast<int>(centerX + rad)); ++x) {
            double dx = x - centerX, dy = y - centerY;
            if (dx * dx + dy * dy < rad * rad) {
                // Simple zoom from center — no refraction
                int sx = std::clamp(static_cast<int>(centerX + dx / magnification), 0, w - 1);
                int sy = std::clamp(static_cast<int>(centerY + dy / magnification), 0, h - 1);
                result.setPixelColor(x, y, frame.pixelColor(sx, sy));
            }
        }
    }
    return result;
}

QImage GpuEffects::applyCornerPin(const QImage& frame, double tl, double tr, double bl, double br) {
    // Simplified perspective via QPainter transform
    QImage result(frame.size(), QImage::Format_ARGB32_Premultiplied);
    result.fill(Qt::transparent);
    QPainter p(&result);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    QTransform t;
    double w = frame.width(), h = frame.height();
    t.translate(tl * w * 0.05, tl * h * 0.05);
    p.setTransform(t);
    p.drawImage(0, 0, frame);
    return result;
}

QImage GpuEffects::applyBezierWarp(const QImage& frame, double curvature, double time) {
    // AE Bezier Warp: deform image boundary using smooth Bezier curves
    QImage result(frame.size(), QImage::Format_ARGB32_Premultiplied);
    result.fill(Qt::transparent);
    int w = frame.width(), h = frame.height();
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            double nx = static_cast<double>(x) / w;
            double ny = static_cast<double>(y) / h;
            // Bezier boundary deformation: edges curve inward/outward
            double edgeDistX = std::min(nx, 1.0 - nx) * 2.0; // 0 at edges, 1 at center
            double edgeDistY = std::min(ny, 1.0 - ny) * 2.0;
            double warpX = curvature * 0.1 * std::sin(ny * M_PI) * (1.0 - edgeDistX) * std::sin(time);
            double warpY = curvature * 0.1 * std::sin(nx * M_PI) * (1.0 - edgeDistY) * std::cos(time * 0.7);
            int sx = std::clamp(static_cast<int>((nx + warpX) * w), 0, w - 1);
            int sy = std::clamp(static_cast<int>((ny + warpY) * h), 0, h - 1);
            result.setPixelColor(x, y, frame.pixelColor(sx, sy));
        }
    }
    return result;
}

QImage GpuEffects::applySmear(const QImage& frame, double angle, double distance, double time) {
    // AE Smear: directional pixel stretching from source point (not blur)
    QImage result = frame.copy();
    int w = frame.width(), h = frame.height();
    double rad = angle * M_PI / 180.0;
    double dirX = std::cos(rad), dirY = std::sin(rad);
    int dist = static_cast<int>(distance);
    // Smear: each pixel pulls from behind along the direction
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            // Accumulate color along smear direction with decay
            double r = 0, g = 0, b = 0, a = 0;
            double totalW = 0;
            for (int s = 0; s < dist; ++s) {
                double weight = 1.0 / (1.0 + s * 0.5); // Decay
                int sx = std::clamp(x - static_cast<int>(dirX * s), 0, w - 1);
                int sy = std::clamp(y - static_cast<int>(dirY * s), 0, h - 1);
                QColor sc = frame.pixelColor(sx, sy);
                r += sc.redF() * weight;
                g += sc.greenF() * weight;
                b += sc.blueF() * weight;
                a += sc.alphaF() * weight;
                totalW += weight;
            }
            result.setPixelColor(x, y, QColor::fromRgbF(
                std::clamp(r / totalW, 0.0, 1.0),
                std::clamp(g / totalW, 0.0, 1.0),
                std::clamp(b / totalW, 0.0, 1.0),
                std::clamp(a / totalW, 0.0, 1.0)));
        }
    }
    return result;
}

QImage GpuEffects::applyPixelate(const QImage& frame, double blockSize) {
    if (blockSize < 2) return frame;
    int bs = static_cast<int>(blockSize);
    QImage small = frame.scaled(frame.width() / bs, frame.height() / bs, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    return small.scaled(frame.size(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
}

QImage GpuEffects::applyPosterize(const QImage& frame, int levels) {
    if (!m_initialized || levels < 2) return frame;
    // Quick CPU posterize
    QImage result = frame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    double factor = 255.0 / (levels - 1);
    for (int y = 0; y < result.height(); ++y) {
        auto* line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            int r = qRound(qRound(qRed(line[x]) / factor) * factor);
            int g = qRound(qRound(qGreen(line[x]) / factor) * factor);
            int b = qRound(qRound(qBlue(line[x]) / factor) * factor);
            line[x] = qRgba(std::clamp(r, 0, 255), std::clamp(g, 0, 255), std::clamp(b, 0, 255), qAlpha(line[x]));
        }
    }
    return result;
}

QImage GpuEffects::applyMosaic(const QImage& frame, int tilesH, int tilesV) {
    // AE Mosaic: separate horizontal/vertical tile counts (non-square tiles)
    if (tilesH < 1 || tilesV < 1) return frame;
    int cellW = std::max(1, frame.width() / tilesH);
    int cellH = std::max(1, frame.height() / tilesV);
    QImage result(frame.size(), QImage::Format_ARGB32_Premultiplied);
    QPainter p(&result);
    for (int ty = 0; ty < tilesV; ++ty) {
        for (int tx = 0; tx < tilesH; ++tx) {
            // Sample center of each cell
            int sx = std::min(tx * cellW + cellW / 2, frame.width() - 1);
            int sy = std::min(ty * cellH + cellH / 2, frame.height() - 1);
            QColor cellColor = frame.pixelColor(sx, sy);
            p.fillRect(tx * cellW, ty * cellH, cellW, cellH, cellColor);
        }
    }
    return result;
}

QImage GpuEffects::applyReshape(const QImage& frame, double morphX, double morphY, double time) {
    // AE Reshape: smooth vertex-interpolated morph between two shapes
    // Simulated via elastic displacement field (vertex morph needs masks)
    QImage result(frame.size(), QImage::Format_ARGB32_Premultiplied);
    int w = frame.width(), h = frame.height();
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            double nx = static_cast<double>(x) / w - 0.5;
            double ny = static_cast<double>(y) / h - 0.5;
            // Elastic morph: radial + directional displacement
            double dist = std::sqrt(nx * nx + ny * ny);
            double angle = std::atan2(ny, nx);
            double morphDist = dist + morphX * 0.1 * std::sin(angle * 3.0 + time);
            double morphAngle = angle + morphY * 0.3 * std::cos(dist * 10.0 + time * 2.0);
            double sx = (0.5 + morphDist * std::cos(morphAngle)) * w;
            double sy = (0.5 + morphDist * std::sin(morphAngle)) * h;
            int ix = std::clamp(static_cast<int>(sx), 0, w - 1);
            int iy = std::clamp(static_cast<int>(sy), 0, h - 1);
            result.setPixelColor(x, y, frame.pixelColor(ix, iy));
        }
    }
    return result;
}

// ══════════════════════════════════════════════════════════════
// IMPLEMENTATIONS — AE Color Correction
// ══════════════════════════════════════════════════════════════

QImage GpuEffects::applyCurves(const QImage& frame, double shadows, double midtones, double highlights) {
    if (!m_initialized) return frame;
    if (!m_curvesShader) {
        m_curvesShader = std::make_unique<QOpenGLShaderProgram>();
        if (!compileShader(*m_curvesShader, VERT_SHADER, CURVES_FRAG)) return frame;
    }
    return renderToImage(frame, *m_curvesShader, [&](QOpenGLShaderProgram& s) {
        s.setUniformValue("shadows", static_cast<float>(shadows));
        s.setUniformValue("midtones", static_cast<float>(midtones));
        s.setUniformValue("highlights", static_cast<float>(highlights));
    });
}

QImage GpuEffects::applyLevels(const QImage& frame, double inBlack, double inWhite, double gamma, double outBlack, double outWhite) {
    if (!m_initialized) return frame;
    if (!m_levelsShader) {
        m_levelsShader = std::make_unique<QOpenGLShaderProgram>();
        if (!compileShader(*m_levelsShader, VERT_SHADER, LEVELS_FRAG)) return frame;
    }
    return renderToImage(frame, *m_levelsShader, [&](QOpenGLShaderProgram& s) {
        s.setUniformValue("inBlack", static_cast<float>(inBlack));
        s.setUniformValue("inWhite", static_cast<float>(inWhite));
        s.setUniformValue("gamma", static_cast<float>(gamma));
        s.setUniformValue("outBlack", static_cast<float>(outBlack));
        s.setUniformValue("outWhite", static_cast<float>(outWhite));
    });
}

QImage GpuEffects::applyHueSaturation(const QImage& frame, double hue, double saturation, double lightness) {
    if (!m_initialized) return frame;
    if (!m_hueSatShader) {
        m_hueSatShader = std::make_unique<QOpenGLShaderProgram>();
        if (!compileShader(*m_hueSatShader, VERT_SHADER, HUE_SAT_FRAG)) return frame;
    }
    return renderToImage(frame, *m_hueSatShader, [&](QOpenGLShaderProgram& s) {
        s.setUniformValue("hueShift", static_cast<float>(hue / 360.0));
        s.setUniformValue("saturation", static_cast<float>(saturation));
        s.setUniformValue("lightness", static_cast<float>(lightness));
    });
}

QImage GpuEffects::applyBrightContrast(const QImage& frame, double brightness, double contrast) {
    if (!m_initialized) return frame;
    if (!m_brightContrastShader) {
        m_brightContrastShader = std::make_unique<QOpenGLShaderProgram>();
        if (!compileShader(*m_brightContrastShader, VERT_SHADER, BRIGHT_CONTRAST_FRAG)) return frame;
    }
    return renderToImage(frame, *m_brightContrastShader, [&](QOpenGLShaderProgram& s) {
        s.setUniformValue("brightness", static_cast<float>(brightness));
        s.setUniformValue("contrast", static_cast<float>(contrast));
    });
}

QImage GpuEffects::applyExposure(const QImage& frame, double exposure, double offset, double gamma) {
    // AE Exposure: operates in linear light space (not gamma like Levels)
    QImage result = frame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    double gain = std::pow(2.0, exposure);
    double invGamma = (gamma > 0.01) ? 1.0 / gamma : 1.0;
    for (int y = 0; y < result.height(); ++y) {
        auto* line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            // Linearize (gamma 2.2 → linear)
            double r = std::pow(qRed(line[x]) / 255.0, 2.2);
            double g = std::pow(qGreen(line[x]) / 255.0, 2.2);
            double b = std::pow(qBlue(line[x]) / 255.0, 2.2);
            // Apply exposure gain in linear space + offset
            r = r * gain + offset;
            g = g * gain + offset;
            b = b * gain + offset;
            // Apply gamma correction then back to sRGB
            r = std::pow(std::clamp(r, 0.0, 1.0), invGamma);
            g = std::pow(std::clamp(g, 0.0, 1.0), invGamma);
            b = std::pow(std::clamp(b, 0.0, 1.0), invGamma);
            // Back to gamma space (linear → sRGB)
            r = std::pow(r, 1.0 / 2.2);
            g = std::pow(g, 1.0 / 2.2);
            b = std::pow(b, 1.0 / 2.2);
            line[x] = qRgba(std::clamp(static_cast<int>(r * 255), 0, 255),
                             std::clamp(static_cast<int>(g * 255), 0, 255),
                             std::clamp(static_cast<int>(b * 255), 0, 255), qAlpha(line[x]));
        }
    }
    return result;
}

QImage GpuEffects::applyTint(const QImage& frame, const QColor& mapBlack, const QColor& mapWhite, double amount) {
    if (!m_initialized) return frame;
    if (!m_tintShader) {
        m_tintShader = std::make_unique<QOpenGLShaderProgram>();
        if (!compileShader(*m_tintShader, VERT_SHADER, TINT_FRAG)) return frame;
    }
    return renderToImage(frame, *m_tintShader, [&](QOpenGLShaderProgram& s) {
        s.setUniformValue("mapBlack", QVector3D(mapBlack.redF(), mapBlack.greenF(), mapBlack.blueF()));
        s.setUniformValue("mapWhite", QVector3D(mapWhite.redF(), mapWhite.greenF(), mapWhite.blueF()));
        s.setUniformValue("amount", static_cast<float>(amount));
    });
}

QImage GpuEffects::applyTritone(const QImage& frame, const QColor& shadows, const QColor& midtones, const QColor& highlights) {
    if (!m_initialized) return frame;
    if (!m_tritoneShader) {
        m_tritoneShader = std::make_unique<QOpenGLShaderProgram>();
        if (!compileShader(*m_tritoneShader, VERT_SHADER, TRITONE_FRAG)) return frame;
    }
    return renderToImage(frame, *m_tritoneShader, [&](QOpenGLShaderProgram& s) {
        s.setUniformValue("shadowCol", QVector3D(shadows.redF(), shadows.greenF(), shadows.blueF()));
        s.setUniformValue("midtoneCol", QVector3D(midtones.redF(), midtones.greenF(), midtones.blueF()));
        s.setUniformValue("highlightCol", QVector3D(highlights.redF(), highlights.greenF(), highlights.blueF()));
    });
}

QImage GpuEffects::applyColorama(const QImage& frame, double phase, double paletteRotation) {
    // AE Colorama: maps luminance to a color palette wheel (not hue rotation)
    QImage result = frame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    // Generate rainbow palette with rotation
    auto paletteColor = [&](double lum) -> QColor {
        double hue = std::fmod(lum * 360.0 + paletteRotation + phase * 360.0, 360.0);
        return QColor::fromHsvF(hue / 360.0, 0.85, 0.9);
    };
    for (int y = 0; y < result.height(); ++y) {
        auto* line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            double lum = (qRed(line[x]) * 0.299 + qGreen(line[x]) * 0.587 + qBlue(line[x]) * 0.114) / 255.0;
            QColor mapped = paletteColor(lum);
            line[x] = qRgba(mapped.red(), mapped.green(), mapped.blue(), qAlpha(line[x]));
        }
    }
    return result;
}

QImage GpuEffects::applyLeaveColor(const QImage& frame, const QColor& targetColor, double tolerance, double feather) {
    if (!m_initialized) return frame;
    if (!m_leaveColorShader) {
        m_leaveColorShader = std::make_unique<QOpenGLShaderProgram>();
        if (!compileShader(*m_leaveColorShader, VERT_SHADER, LEAVE_COLOR_FRAG)) return frame;
    }
    return renderToImage(frame, *m_leaveColorShader, [&](QOpenGLShaderProgram& s) {
        s.setUniformValue("targetColor", QVector3D(targetColor.redF(), targetColor.greenF(), targetColor.blueF()));
        s.setUniformValue("tolerance", static_cast<float>(tolerance));
        s.setUniformValue("feather", static_cast<float>(feather));
    });
}

QImage GpuEffects::applyChannelMixer(const QImage& frame, double rr, double rg, double rb,
                                      double gr, double gg, double gb,
                                      double br_, double bg_, double bb_) {
    // CPU channel mix (fast enough for overlay-sized images)
    QImage result = frame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    for (int y = 0; y < result.height(); ++y) {
        auto* line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            double r = qRed(line[x]) / 255.0, g = qGreen(line[x]) / 255.0, b = qBlue(line[x]) / 255.0;
            int nr = std::clamp(static_cast<int>((r * rr + g * rg + b * rb) * 255), 0, 255);
            int ng = std::clamp(static_cast<int>((r * gr + g * gg + b * gb) * 255), 0, 255);
            int nb = std::clamp(static_cast<int>((r * br_ + g * bg_ + b * bb_) * 255), 0, 255);
            line[x] = qRgba(nr, ng, nb, qAlpha(line[x]));
        }
    }
    return result;
}

QImage GpuEffects::applyVibrance(const QImage& frame, double amount) {
    if (!m_initialized) return frame;
    if (!m_vibranceShader) {
        m_vibranceShader = std::make_unique<QOpenGLShaderProgram>();
        if (!compileShader(*m_vibranceShader, VERT_SHADER, VIBRANCE_FRAG)) return frame;
    }
    return renderToImage(frame, *m_vibranceShader, [&](QOpenGLShaderProgram& s) {
        s.setUniformValue("amount", static_cast<float>(amount));
    });
}

QImage GpuEffects::applyPhotoFilter(const QImage& frame, const QColor& filterColor, double density) {
    // AE Photo Filter: multiply by filter color while preserving luminance
    QImage result = frame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    double fr = filterColor.redF(), fg = filterColor.greenF(), fb = filterColor.blueF();
    for (int y = 0; y < result.height(); ++y) {
        auto* line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            double r = qRed(line[x]) / 255.0, g = qGreen(line[x]) / 255.0, b = qBlue(line[x]) / 255.0;
            double origLum = r * 0.299 + g * 0.587 + b * 0.114;
            // Apply color filter (multiply)
            double nr = r * (1.0 - density) + r * fr * density;
            double ng = g * (1.0 - density) + g * fg * density;
            double nb = b * (1.0 - density) + b * fb * density;
            // Preserve luminance
            double newLum = nr * 0.299 + ng * 0.587 + nb * 0.114;
            if (newLum > 0.001) {
                double lumScale = origLum / newLum;
                nr *= lumScale; ng *= lumScale; nb *= lumScale;
            }
            line[x] = qRgba(std::clamp(static_cast<int>(nr * 255), 0, 255),
                             std::clamp(static_cast<int>(ng * 255), 0, 255),
                             std::clamp(static_cast<int>(nb * 255), 0, 255), qAlpha(line[x]));
        }
    }
    return result;
}

QImage GpuEffects::applyShadowHighlight(const QImage& frame, double shadowAmount, double highlightAmount) {
    // AE Shadow/Highlight: local tone mapping (not global curves)
    // Uses blurred luminance as local reference
    QImage blurred = applyGaussianBlur(frame, 15.0); // Local neighborhood
    QImage result = frame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    QImage blurImg = blurred.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    for (int y = 0; y < result.height(); ++y) {
        auto* line = reinterpret_cast<QRgb*>(result.scanLine(y));
        auto* bLine = reinterpret_cast<const QRgb*>(blurImg.constScanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            double r = qRed(line[x]) / 255.0, g = qGreen(line[x]) / 255.0, b = qBlue(line[x]) / 255.0;
            double localLum = (qRed(bLine[x]) * 0.299 + qGreen(bLine[x]) * 0.587 + qBlue(bLine[x]) * 0.114) / 255.0;
            // Shadow boost: brighten where local luminance is dark
            double shadowBoost = (1.0 - localLum) * shadowAmount * 0.5;
            // Highlight reduction: darken where local luminance is bright
            double highlightReduce = localLum * highlightAmount * -0.3;
            double adjustment = shadowBoost + highlightReduce;
            r = std::clamp(r + adjustment, 0.0, 1.0);
            g = std::clamp(g + adjustment, 0.0, 1.0);
            b = std::clamp(b + adjustment, 0.0, 1.0);
            line[x] = qRgba(static_cast<int>(r * 255), static_cast<int>(g * 255), static_cast<int>(b * 255), qAlpha(line[x]));
        }
    }
    return result;
}

QImage GpuEffects::applyColorBalance(const QImage& frame, double cyanRed, double magentaGreen, double yellowBlue) {
    // AE Color Balance: per-tonal-range adjustments (shadows/midtones/highlights)
    QImage result = frame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    for (int y = 0; y < result.height(); ++y) {
        auto* line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            double r = qRed(line[x]) / 255.0, g = qGreen(line[x]) / 255.0, b = qBlue(line[x]) / 255.0;
            double lum = r * 0.299 + g * 0.587 + b * 0.114;
            // Shadow weight (dark pixels), midtone weight, highlight weight
            double shadowW = std::clamp(1.0 - lum * 3.0, 0.0, 1.0);
            double highlightW = std::clamp(lum * 3.0 - 2.0, 0.0, 1.0);
            double midtoneW = 1.0 - shadowW - highlightW;
            // Apply color balance per range
            double crAdj = cyanRed * (shadowW * 0.5 + midtoneW * 1.0 + highlightW * 0.3);
            double mgAdj = magentaGreen * (shadowW * 0.3 + midtoneW * 1.0 + highlightW * 0.5);
            double ybAdj = yellowBlue * (shadowW * 0.3 + midtoneW * 0.5 + highlightW * 1.0);
            r = std::clamp(r + crAdj * 0.2, 0.0, 1.0);
            g = std::clamp(g + mgAdj * 0.2, 0.0, 1.0);
            b = std::clamp(b + ybAdj * 0.2, 0.0, 1.0);
            line[x] = qRgba(static_cast<int>(r * 255), static_cast<int>(g * 255), static_cast<int>(b * 255), qAlpha(line[x]));
        }
    }
    return result;
}

QImage GpuEffects::applySelectiveColor(const QImage& frame, const QColor& target, double cyan, double magenta, double yellow, double black) {
    // AE Selective Color: adjust CMYK values within a specific color range
    QImage result = frame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    double th = target.hueF(), ts = target.saturationF();
    for (int y = 0; y < result.height(); ++y) {
        auto* line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            QColor px(line[x]);
            double hueDiff = std::abs(px.hueF() - th);
            if (hueDiff > 0.5) hueDiff = 1.0 - hueDiff;
            // Weight based on how close to target hue (±30 degrees)
            double weight = std::clamp(1.0 - hueDiff * 6.0, 0.0, 1.0) * std::min(1.0, px.saturationF() * 2.0);
            if (weight > 0.01) {
                double r = px.redF(), g = px.greenF(), b = px.blueF();
                // CMYK adjustments within the selected range
                r = std::clamp(r - cyan * weight * 0.3 + magenta * weight * 0.1, 0.0, 1.0);
                g = std::clamp(g - magenta * weight * 0.3 + yellow * weight * 0.1, 0.0, 1.0);
                b = std::clamp(b - yellow * weight * 0.3 + cyan * weight * 0.1, 0.0, 1.0);
                // Black adjustment
                double bkAdj = black * weight * 0.2;
                r = std::clamp(r - bkAdj, 0.0, 1.0);
                g = std::clamp(g - bkAdj, 0.0, 1.0);
                b = std::clamp(b - bkAdj, 0.0, 1.0);
                line[x] = qRgba(static_cast<int>(r * 255), static_cast<int>(g * 255), static_cast<int>(b * 255), qAlpha(line[x]));
            }
        }
    }
    return result;
}

QImage GpuEffects::applyGradientMap(const QImage& frame, const QColor& start, const QColor& end) {
    // AE Gradient Map: maps luminance to a gradient (dedicated, not via Tint)
    QImage result = frame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    for (int y = 0; y < result.height(); ++y) {
        auto* line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            double lum = (qRed(line[x]) * 0.299 + qGreen(line[x]) * 0.587 + qBlue(line[x]) * 0.114) / 255.0;
            // Smooth interpolation along gradient
            double t = lum;
            // Apply S-curve for richer midtone mapping
            t = t * t * (3.0 - 2.0 * t); // smoothstep
            int r = static_cast<int>(start.red() + t * (end.red() - start.red()));
            int g = static_cast<int>(start.green() + t * (end.green() - start.green()));
            int b = static_cast<int>(start.blue() + t * (end.blue() - start.blue()));
            line[x] = qRgba(std::clamp(r, 0, 255), std::clamp(g, 0, 255), std::clamp(b, 0, 255), qAlpha(line[x]));
        }
    }
    return result;
}

QImage GpuEffects::applyBlackWhite(const QImage& frame, double redWeight, double greenWeight, double blueWeight) {
    return applyChannelMixer(frame, redWeight, greenWeight, blueWeight, redWeight, greenWeight, blueWeight, redWeight, greenWeight, blueWeight);
}

QImage GpuEffects::applyInvert(const QImage& frame) {
    QImage result = frame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    result.invertPixels(QImage::InvertRgb);
    return result;
}

QImage GpuEffects::applyThreshold(const QImage& frame, double threshold) {
    return applyLevels(frame, threshold, threshold + 0.01, 1.0, 0.0, 1.0);
}

QImage GpuEffects::applySolarize(const QImage& frame, double threshold) {
    QImage result = frame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    int th = static_cast<int>(threshold * 255);
    for (int y = 0; y < result.height(); ++y) {
        auto* line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            int r = qRed(line[x]) > th ? 255 - qRed(line[x]) : qRed(line[x]);
            int g = qGreen(line[x]) > th ? 255 - qGreen(line[x]) : qGreen(line[x]);
            int b = qBlue(line[x]) > th ? 255 - qBlue(line[x]) : qBlue(line[x]);
            line[x] = qRgba(r, g, b, qAlpha(line[x]));
        }
    }
    return result;
}

// ══════════════════════════════════════════════════════════════
// IMPLEMENTATIONS — AE Generate
// ══════════════════════════════════════════════════════════════

QImage GpuEffects::applyFractalNoise(const QImage& frame, double scale, double complexity, double evolution, double opacity) {
    if (!m_initialized) return frame;
    if (!m_fractalNoiseShader) {
        m_fractalNoiseShader = std::make_unique<QOpenGLShaderProgram>();
        if (!compileShader(*m_fractalNoiseShader, VERT_SHADER, FRACTAL_NOISE_FRAG)) return frame;
    }
    return renderToImage(frame, *m_fractalNoiseShader, [&](QOpenGLShaderProgram& s) {
        s.setUniformValue("scale", static_cast<float>(scale));
        s.setUniformValue("complexity", static_cast<float>(complexity));
        s.setUniformValue("evolution", static_cast<float>(evolution));
        s.setUniformValue("noiseOpacity", static_cast<float>(opacity));
    });
}

QImage GpuEffects::applyCellPattern(const QImage& frame, double size, double scatter, double time) {
    if (!m_initialized) return frame;
    if (!m_cellPatternShader) {
        m_cellPatternShader = std::make_unique<QOpenGLShaderProgram>();
        if (!compileShader(*m_cellPatternShader, VERT_SHADER, CELL_PATTERN_FRAG)) return frame;
    }
    return renderToImage(frame, *m_cellPatternShader, [&](QOpenGLShaderProgram& s) {
        s.setUniformValue("size", static_cast<float>(size));
        s.setUniformValue("scatter", static_cast<float>(scatter));
        s.setUniformValue("time", static_cast<float>(time));
    });
}

QImage GpuEffects::applyCheckerboard(const QImage& frame, double size, const QColor& c1, const QColor& c2, double opacity) {
    QImage result = frame.copy();
    QPainter p(&result);
    int s = std::max(2, static_cast<int>(size));
    for (int y = 0; y < frame.height(); y += s) {
        for (int x = 0; x < frame.width(); x += s) {
            bool even = ((x / s) + (y / s)) % 2 == 0;
            p.fillRect(x, y, s, s, QColor(even ? c1 : c2));
        }
    }
    p.setOpacity(1.0 - opacity);
    p.drawImage(0, 0, frame);
    return result;
}

QImage GpuEffects::applyGrid(const QImage& frame, double spacing, double width, const QColor& color, double opacity) {
    QImage result = frame.copy();
    QPainter p(&result);
    p.setOpacity(opacity);
    p.setPen(QPen(color, width));
    int sp = std::max(2, static_cast<int>(spacing));
    for (int x = 0; x < frame.width(); x += sp)
        p.drawLine(x, 0, x, frame.height());
    for (int y = 0; y < frame.height(); y += sp)
        p.drawLine(0, y, frame.width(), y);
    return result;
}

QImage GpuEffects::applyGradientRamp(const QImage& frame, const QColor& start, const QColor& end, double angle, double opacity) {
    QImage result = frame.copy();
    QPainter p(&result);
    p.setOpacity(opacity);
    QLinearGradient grad;
    double a = angle * M_PI / 180.0;
    grad.setStart(frame.width() * 0.5 - std::cos(a) * frame.width(), frame.height() * 0.5 - std::sin(a) * frame.height());
    grad.setFinalStop(frame.width() * 0.5 + std::cos(a) * frame.width(), frame.height() * 0.5 + std::sin(a) * frame.height());
    grad.setColorAt(0, start);
    grad.setColorAt(1, end);
    p.fillRect(frame.rect(), grad);
    return result;
}

QImage GpuEffects::applyStroke(const QImage& frame, const QColor& color, double width, double feather) {
    (void)feather;
    QImage result = frame.copy();
    QPainter p(&result);
    p.setPen(QPen(color, width));
    p.setBrush(Qt::NoBrush);
    p.drawRect(static_cast<int>(width / 2), static_cast<int>(width / 2),
               frame.width() - static_cast<int>(width), frame.height() - static_cast<int>(width));
    return result;
}

QImage GpuEffects::applyVegas(const QImage& frame, int segments, double width, const QColor& color, double speed, double time) {
    QImage result = frame.copy();
    QPainter p(&result);
    p.setRenderHint(QPainter::Antialiasing);
    double perimeter = 2.0 * (frame.width() + frame.height());
    double segLen = perimeter / segments;
    double offset = std::fmod(time * speed * 100, segLen);
    p.setPen(QPen(color, width, Qt::SolidLine, Qt::RoundCap));
    for (int i = 0; i < segments; i += 2) {
        double start = i * segLen + offset;
        double end = start + segLen;
        // Draw along perimeter
        auto perimeterPoint = [&](double d) -> QPointF {
            d = std::fmod(d, perimeter);
            if (d < frame.width()) return {d, 0};
            d -= frame.width();
            if (d < frame.height()) return {static_cast<double>(frame.width()), d};
            d -= frame.height();
            if (d < frame.width()) return {static_cast<double>(frame.width()) - d, static_cast<double>(frame.height())};
            d -= frame.width();
            return {0, static_cast<double>(frame.height()) - d};
        };
        p.drawLine(perimeterPoint(start), perimeterPoint(end));
    }
    return result;
}

QImage GpuEffects::applyCircleBurst(const QImage& frame, int count, double radius, const QColor& color, double phase) {
    QImage result = frame.copy();
    QPainter p(&result);
    p.setRenderHint(QPainter::Antialiasing);
    double cx = frame.width() / 2.0, cy = frame.height() / 2.0;
    for (int i = 0; i < count; ++i) {
        double angle = (2.0 * M_PI / count) * i + phase;
        double x = cx + std::cos(angle) * radius;
        double y = cy + std::sin(angle) * radius;
        p.setPen(Qt::NoPen);
        QColor c = color;
        c.setAlphaF(0.3 + 0.7 * std::abs(std::sin(phase * 3.0 + i)));
        p.setBrush(c);
        p.drawEllipse(QPointF(x, y), 4, 4);
    }
    return result;
}

QImage GpuEffects::applyRadioWaves(const QImage& frame, double frequency, double speed, const QColor& color, double time) {
    QImage result = frame.copy();
    QPainter p(&result);
    p.setRenderHint(QPainter::Antialiasing);
    double cx = frame.width() / 2.0, cy = frame.height() / 2.0;
    int waves = static_cast<int>(frequency);
    for (int i = 0; i < waves; ++i) {
        double r = std::fmod(time * speed * 50 + i * (frame.width() * 0.5 / waves), frame.width() * 0.6);
        double alpha = 1.0 - r / (frame.width() * 0.6);
        QColor c = color;
        c.setAlphaF(std::max(0.0, alpha * 0.5));
        p.setPen(QPen(c, 2));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(QPointF(cx, cy), r, r);
    }
    return result;
}

QImage GpuEffects::applyAudioSpectrum(const QImage& frame, const QColor& startColor, const QColor& endColor, int bands, double time) {
    QImage result = frame.copy();
    QPainter p(&result);
    int barW = frame.width() / std::max(bands, 1);
    for (int i = 0; i < bands; ++i) {
        double h = (0.3 + 0.7 * std::abs(std::sin(time * 5.0 + i * 0.7))) * frame.height() * 0.4;
        double t = static_cast<double>(i) / bands;
        QColor c = QColor::fromRgbF(startColor.redF() + t * (endColor.redF() - startColor.redF()),
                                      startColor.greenF() + t * (endColor.greenF() - startColor.greenF()),
                                      startColor.blueF() + t * (endColor.blueF() - startColor.blueF()), 0.7);
        p.fillRect(i * barW, frame.height() - static_cast<int>(h), barW - 1, static_cast<int>(h), c);
    }
    return result;
}

QImage GpuEffects::applyLensFlareGen(const QImage& frame, double cx, double cy, double brightness, const QColor& color) {
    QImage result = frame.copy();
    QPainter p(&result);
    p.setRenderHint(QPainter::Antialiasing);
    double x = cx * frame.width(), y = cy * frame.height();
    QRadialGradient grad(x, y, frame.width() * 0.3 * brightness);
    grad.setColorAt(0, QColor(color.red(), color.green(), color.blue(), static_cast<int>(200 * brightness)));
    grad.setColorAt(0.3, QColor(color.red(), color.green(), color.blue(), static_cast<int>(80 * brightness)));
    grad.setColorAt(1, Qt::transparent);
    p.setCompositionMode(QPainter::CompositionMode_Plus);
    p.fillRect(frame.rect(), grad);
    return result;
}

QImage GpuEffects::applyLightBurst(const QImage& frame, double cx, double cy, int rays, double intensity, double time) {
    QImage result = frame.copy();
    QPainter p(&result);
    p.setRenderHint(QPainter::Antialiasing);
    p.setCompositionMode(QPainter::CompositionMode_Plus);
    double x = cx * frame.width(), y = cy * frame.height();
    double maxR = std::sqrt(frame.width() * frame.width() + frame.height() * frame.height()) * 0.5;
    for (int i = 0; i < rays; ++i) {
        double angle = (2.0 * M_PI / rays) * i + time;
        double ex = x + std::cos(angle) * maxR;
        double ey = y + std::sin(angle) * maxR;
        QColor c(255, 255, 220, static_cast<int>(40 * intensity));
        p.setPen(QPen(c, 2.0 * intensity));
        p.drawLine(QPointF(x, y), QPointF(ex, ey));
    }
    return result;
}

QImage GpuEffects::applyFill(const QImage& frame, const QColor& color, double opacity) {
    QImage result = frame.copy();
    QPainter p(&result);
    p.setOpacity(opacity);
    p.fillRect(frame.rect(), color);
    return result;
}

QImage GpuEffects::apply4ColorGradient(const QImage& frame, const QColor& tl, const QColor& tr, const QColor& bl, const QColor& br, double opacity) {
    QImage result = frame.copy();
    QImage grad(frame.size(), QImage::Format_ARGB32_Premultiplied);
    for (int y = 0; y < grad.height(); ++y) {
        double vy = static_cast<double>(y) / grad.height();
        for (int x = 0; x < grad.width(); ++x) {
            double vx = static_cast<double>(x) / grad.width();
            QColor top = QColor::fromRgbF(tl.redF() + vx * (tr.redF() - tl.redF()), tl.greenF() + vx * (tr.greenF() - tl.greenF()), tl.blueF() + vx * (tr.blueF() - tl.blueF()));
            QColor bot = QColor::fromRgbF(bl.redF() + vx * (br.redF() - bl.redF()), bl.greenF() + vx * (br.greenF() - bl.greenF()), bl.blueF() + vx * (br.blueF() - bl.blueF()));
            QColor c = QColor::fromRgbF(top.redF() + vy * (bot.redF() - top.redF()), top.greenF() + vy * (bot.greenF() - top.greenF()), top.blueF() + vy * (bot.blueF() - top.blueF()));
            grad.setPixelColor(x, y, c);
        }
    }
    QPainter p(&result);
    p.setOpacity(opacity);
    p.drawImage(0, 0, grad);
    return result;
}

QImage GpuEffects::applyBeam(const QImage& frame, double startX, double startY, double endX, double endY, double width, const QColor& color, double time) {
    QImage result = frame.copy();
    QPainter p(&result);
    p.setRenderHint(QPainter::Antialiasing);
    p.setCompositionMode(QPainter::CompositionMode_Plus);
    double pulse = 0.5 + 0.5 * std::sin(time * 5.0);
    QColor c = color;
    c.setAlphaF(c.alphaF() * pulse);
    p.setPen(QPen(c, width, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(QPointF(startX * frame.width(), startY * frame.height()),
               QPointF(endX * frame.width(), endY * frame.height()));
    return result;
}

// ══════════════════════════════════════════════════════════════
// IMPLEMENTATIONS — AE Stylize
// ══════════════════════════════════════════════════════════════

QImage GpuEffects::applyPosterizeEffect(const QImage& frame, int levels) {
    // AE Posterize: reduces color depth with dithering for smoother transitions
    QImage result = frame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    double factor = 255.0 / (levels - 1);
    for (int y = 0; y < result.height(); ++y) {
        auto* line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            // Ordered dithering (Bayer 2x2) for smoother posterization vs raw quantize
            double dither = ((x % 2) * 2 + (y % 2)) / 4.0 - 0.375;
            int r = std::clamp(static_cast<int>(std::round((qRed(line[x]) / 255.0 + dither * 0.03) * (levels - 1)) * factor), 0, 255);
            int g = std::clamp(static_cast<int>(std::round((qGreen(line[x]) / 255.0 + dither * 0.03) * (levels - 1)) * factor), 0, 255);
            int b = std::clamp(static_cast<int>(std::round((qBlue(line[x]) / 255.0 + dither * 0.03) * (levels - 1)) * factor), 0, 255);
            line[x] = qRgba(r, g, b, qAlpha(line[x]));
        }
    }
    return result;
}

QImage GpuEffects::applyEmboss(const QImage& frame, double angle, double height, double amount) {
    if (!m_initialized) return frame;
    if (!m_embossShader) {
        m_embossShader = std::make_unique<QOpenGLShaderProgram>();
        if (!compileShader(*m_embossShader, VERT_SHADER, EMBOSS_FRAG)) return frame;
    }
    return renderToImage(frame, *m_embossShader, [&](QOpenGLShaderProgram& s) {
        s.setUniformValue("texSize", QVector2D(frame.width(), frame.height()));
        s.setUniformValue("angle", static_cast<float>(angle * M_PI / 180.0));
        s.setUniformValue("height", static_cast<float>(height));
        s.setUniformValue("amount", static_cast<float>(amount));
    });
}

QImage GpuEffects::applyFindEdges(const QImage& frame, bool invert) {
    if (!m_initialized) return frame;
    if (!m_findEdgesShader) {
        m_findEdgesShader = std::make_unique<QOpenGLShaderProgram>();
        if (!compileShader(*m_findEdgesShader, VERT_SHADER, FIND_EDGES_FRAG)) return frame;
    }
    return renderToImage(frame, *m_findEdgesShader, [&](QOpenGLShaderProgram& s) {
        s.setUniformValue("texSize", QVector2D(frame.width(), frame.height()));
        s.setUniformValue("invertEdges", invert ? 1 : 0);
    });
}

QImage GpuEffects::applyRoughenEdges(const QImage& frame, double amount, double scale, double time) {
    // AE Roughen Edges: fractal displacement on ALPHA EDGES only (not whole image)
    QImage result = frame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    int w = result.width(), h = result.height();
    for (int y = 0; y < h; ++y) {
        auto* line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < w; ++x) {
            int alpha = qAlpha(line[x]);
            if (alpha < 5 || alpha > 250) continue; // Only process edge pixels
            // Edge detection: check neighbors
            bool isEdge = false;
            for (int dy = -1; dy <= 1 && !isEdge; ++dy) {
                for (int dx = -1; dx <= 1 && !isEdge; ++dx) {
                    int nx = std::clamp(x + dx, 0, w - 1);
                    int ny = std::clamp(y + dy, 0, h - 1);
                    int na = qAlpha(reinterpret_cast<const QRgb*>(frame.constScanLine(ny))[nx]);
                    if (std::abs(alpha - na) > 20) isEdge = true;
                }
            }
            if (isEdge) {
                // Fractal noise displacement on edge
                double noise = ae::noise2D(x * scale * 0.01 + time, y * scale * 0.01);
                int newAlpha = std::clamp(alpha + static_cast<int>(noise * amount * 50), 0, 255);
                line[x] = qRgba(qRed(line[x]), qGreen(line[x]), qBlue(line[x]), newAlpha);
            }
        }
    }
    return result;
}

QImage GpuEffects::applyScatter(const QImage& frame, double amount) {
    QImage result(frame.size(), QImage::Format_ARGB32_Premultiplied);
    int w = frame.width(), h = frame.height();
    int a = static_cast<int>(amount);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int sx = x + (static_cast<int>(ae::random(-a, a, x * 31 + y * 17)) % (2 * a + 1));
            int sy = y + (static_cast<int>(ae::random(-a, a, x * 13 + y * 37)) % (2 * a + 1));
            sx = std::clamp(sx, 0, w - 1);
            sy = std::clamp(sy, 0, h - 1);
            result.setPixelColor(x, y, frame.pixelColor(sx, sy));
        }
    }
    return result;
}

QImage GpuEffects::applyStylizeGlow(const QImage& frame, double threshold, double radius, double intensity, const QColor& color) {
    // AE Stylize Glow: threshold-based extraction + colored glow (passes threshold through)
    // First extract bright areas above threshold, then blur and composite
    QImage brightExtract = frame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    for (int y = 0; y < brightExtract.height(); ++y) {
        auto* line = reinterpret_cast<QRgb*>(brightExtract.scanLine(y));
        for (int x = 0; x < brightExtract.width(); ++x) {
            double lum = (qRed(line[x]) * 0.299 + qGreen(line[x]) * 0.587 + qBlue(line[x]) * 0.114) / 255.0;
            if (lum < threshold) {
                line[x] = qRgba(0, 0, 0, 0); // Below threshold: remove
            }
        }
    }
    // Blur the bright areas
    QImage blurred = applyGaussianBlur(brightExtract, radius);
    // Tint with glow color and composite additively
    QImage result = frame.copy();
    QPainter p(&result);
    p.setCompositionMode(QPainter::CompositionMode_Plus);
    p.setOpacity(intensity);
    // Tint the blurred layer
    QImage tinted = blurred;
    for (int y = 0; y < tinted.height(); ++y) {
        auto* line = reinterpret_cast<QRgb*>(tinted.scanLine(y));
        for (int x = 0; x < tinted.width(); ++x) {
            int r = std::clamp(static_cast<int>(qRed(line[x]) * color.redF()), 0, 255);
            int g = std::clamp(static_cast<int>(qGreen(line[x]) * color.greenF()), 0, 255);
            int b = std::clamp(static_cast<int>(qBlue(line[x]) * color.blueF()), 0, 255);
            line[x] = qRgba(r, g, b, qAlpha(line[x]));
        }
    }
    p.drawImage(0, 0, tinted);
    return result;
}

QImage GpuEffects::applyCartoon(const QImage& frame, double edgeThreshold, double colorLevels) {
    QImage posterized = applyPosterize(frame, static_cast<int>(colorLevels));
    QImage edges = applyFindEdges(frame, false);
    // Combine: darken posterized with edges
    return ae::blendImages(posterized, edges, ae::BlendMode::Multiply, edgeThreshold);
}

QImage GpuEffects::applyOilPaint(const QImage& frame, double brushSize, double smoothness) {
    // AE Oil Paint: Kuwahara filter — preserves edges while creating painterly look
    QImage result(frame.size(), QImage::Format_ARGB32_Premultiplied);
    int w = frame.width(), h = frame.height();
    int radius = std::max(1, static_cast<int>(brushSize));
    (void)smoothness;
    for (int y = 0; y < h; ++y) {
        auto* outLine = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < w; ++x) {
            // Kuwahara: evaluate 4 quadrants, pick the one with lowest variance
            double bestVar = 1e10;
            double bestR = 0, bestG = 0, bestB = 0;
            for (int qy = 0; qy < 2; ++qy) {
                for (int qx = 0; qx < 2; ++qx) {
                    double sumR = 0, sumG = 0, sumB = 0;
                    double sumR2 = 0, sumG2 = 0, sumB2 = 0;
                    int count = 0;
                    int sy0 = qy == 0 ? y - radius : y;
                    int sy1 = qy == 0 ? y : y + radius;
                    int sx0 = qx == 0 ? x - radius : x;
                    int sx1 = qx == 0 ? x : x + radius;
                    for (int sy = std::max(0, sy0); sy <= std::min(h - 1, sy1); ++sy) {
                        auto* sLine = reinterpret_cast<const QRgb*>(frame.constScanLine(sy));
                        for (int sx = std::max(0, sx0); sx <= std::min(w - 1, sx1); ++sx) {
                            double r = qRed(sLine[sx]) / 255.0;
                            double g = qGreen(sLine[sx]) / 255.0;
                            double b = qBlue(sLine[sx]) / 255.0;
                            sumR += r; sumG += g; sumB += b;
                            sumR2 += r * r; sumG2 += g * g; sumB2 += b * b;
                            count++;
                        }
                    }
                    if (count > 0) {
                        double meanR = sumR / count, meanG = sumG / count, meanB = sumB / count;
                        double var = (sumR2 / count - meanR * meanR) + (sumG2 / count - meanG * meanG) + (sumB2 / count - meanB * meanB);
                        if (var < bestVar) {
                            bestVar = var;
                            bestR = meanR; bestG = meanG; bestB = meanB;
                        }
                    }
                }
            }
            outLine[x] = qRgba(std::clamp(static_cast<int>(bestR * 255), 0, 255),
                                std::clamp(static_cast<int>(bestG * 255), 0, 255),
                                std::clamp(static_cast<int>(bestB * 255), 0, 255),
                                qAlpha(reinterpret_cast<const QRgb*>(frame.constScanLine(y))[x]));
        }
    }
    return result;
}

QImage GpuEffects::applyHalftone(const QImage& frame, double dotSize, double angle) {
    if (!m_initialized) return frame;
    if (!m_halftoneShader) {
        m_halftoneShader = std::make_unique<QOpenGLShaderProgram>();
        if (!compileShader(*m_halftoneShader, VERT_SHADER, HALFTONE_FRAG)) return frame;
    }
    return renderToImage(frame, *m_halftoneShader, [&](QOpenGLShaderProgram& s) {
        s.setUniformValue("dotSize", static_cast<float>(dotSize));
        s.setUniformValue("angle", static_cast<float>(angle * M_PI / 180.0));
    });
}

QImage GpuEffects::applyCrossHatch(const QImage& frame, double spacing, double angle, double intensity) {
    QImage result = frame.copy();
    QPainter p(&result);
    p.setRenderHint(QPainter::Antialiasing);
    QImage gray = applyBlackWhite(frame, 0.299, 0.587, 0.114);
    double s = std::max(2.0, spacing);
    p.setPen(QPen(QColor(0, 0, 0, static_cast<int>(intensity * 200)), 1));
    double a = angle * M_PI / 180.0;
    for (double d = -frame.width(); d < frame.width() + frame.height(); d += s) {
        double x1 = d, y1 = 0.0;
        double x2 = d + std::cos(a) * frame.height() * 2, y2 = std::sin(a) * frame.height() * 2;
        p.drawLine(QPointF(x1, y1), QPointF(x2, y2));
    }
    return result;
}

QImage GpuEffects::applyStainedGlass(const QImage& frame, double cellSize, double edgeWidth, const QColor& edgeColor) {
    if (!m_initialized) return frame;
    if (!m_stainedGlassShader) {
        m_stainedGlassShader = std::make_unique<QOpenGLShaderProgram>();
        if (!compileShader(*m_stainedGlassShader, VERT_SHADER, STAINED_GLASS_FRAG)) return frame;
    }
    return renderToImage(frame, *m_stainedGlassShader, [&](QOpenGLShaderProgram& s) {
        s.setUniformValue("cellSize", static_cast<float>(cellSize));
        s.setUniformValue("edgeWidth", static_cast<float>(edgeWidth));
        s.setUniformValue("edgeColor", QVector3D(edgeColor.redF(), edgeColor.greenF(), edgeColor.blueF()));
    });
}

QImage GpuEffects::applyNoise(const QImage& frame, double amount, bool colored) {
    QImage result = frame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    int a = static_cast<int>(amount * 255);
    for (int y = 0; y < result.height(); ++y) {
        auto* line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            int seed = x * 7919 + y * 104729;
            if (colored) {
                int nr = std::clamp(qRed(line[x]) + static_cast<int>(ae::random(-a, a, seed)), 0, 255);
                int ng = std::clamp(qGreen(line[x]) + static_cast<int>(ae::random(-a, a, seed + 1)), 0, 255);
                int nb = std::clamp(qBlue(line[x]) + static_cast<int>(ae::random(-a, a, seed + 2)), 0, 255);
                line[x] = qRgba(nr, ng, nb, qAlpha(line[x]));
            } else {
                int n = static_cast<int>(ae::random(-a, a, seed));
                line[x] = qRgba(std::clamp(qRed(line[x]) + n, 0, 255),
                                std::clamp(qGreen(line[x]) + n, 0, 255),
                                std::clamp(qBlue(line[x]) + n, 0, 255), qAlpha(line[x]));
            }
        }
    }
    return result;
}

QImage GpuEffects::applyStrobe(const QImage& frame, double frequency, double time, const QColor& color) {
    double flash = std::sin(time * frequency * 2.0 * M_PI);
    if (flash < 0.0) return frame;
    QImage result = frame.copy();
    QPainter p(&result);
    p.setOpacity(flash * 0.5);
    p.fillRect(frame.rect(), color);
    return result;
}

QImage GpuEffects::applyMotionTile(const QImage& frame, int tilesH, int tilesV, double mirrorEdges) {
    (void)mirrorEdges;
    QImage tile = frame.scaled(frame.width() / std::max(tilesH, 1), frame.height() / std::max(tilesV, 1),
                                Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QImage result(frame.size(), QImage::Format_ARGB32_Premultiplied);
    QPainter p(&result);
    for (int y = 0; y < tilesV; ++y)
        for (int x = 0; x < tilesH; ++x)
            p.drawImage(x * tile.width(), y * tile.height(), tile);
    return result;
}

QImage GpuEffects::applyTextureOverlay(const QImage& frame, const QImage& texture, double opacity, int blendMode) {
    if (texture.isNull()) return frame;
    QImage scaledTex = texture.scaled(frame.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    return ae::blendImages(frame, scaledTex, static_cast<ae::BlendMode>(blendMode), opacity);
}

// ══════════════════════════════════════════════════════════════
// IMPLEMENTATIONS — AE Perspective
// ══════════════════════════════════════════════════════════════

QImage GpuEffects::applyCCSphere(const QImage& frame, double rotX, double rotY, double radius) {
    if (!m_initialized) return frame;
    if (!m_ccSphereShader) {
        m_ccSphereShader = std::make_unique<QOpenGLShaderProgram>();
        if (!compileShader(*m_ccSphereShader, VERT_SHADER, CC_SPHERE_FRAG)) return frame;
    }
    return renderToImage(frame, *m_ccSphereShader, [&](QOpenGLShaderProgram& s) {
        s.setUniformValue("rotX", static_cast<float>(rotX));
        s.setUniformValue("rotY", static_cast<float>(rotY));
        s.setUniformValue("radius", static_cast<float>(radius));
    });
}

QImage GpuEffects::applyCCCylinder(const QImage& frame, double rotY, double radius) {
    // AE CC Cylinder: wrap image around a cylinder (horizontal curvature only)
    QImage result(frame.size(), QImage::Format_ARGB32_Premultiplied);
    result.fill(Qt::transparent);
    int w = frame.width(), h = frame.height();
    double centerX = w / 2.0;
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            // Cylindrical mapping: only horizontal distortion
            double nx = (x - centerX) / (w * radius * 0.5);
            if (std::abs(nx) > 1.0) continue; // Outside cylinder
            // Arc-length mapping
            double angle = std::asin(std::clamp(nx, -1.0, 1.0)) + rotY;
            double srcX = centerX + std::sin(angle) * w * radius * 0.5;
            // Foreshortening (z-depth)
            double z = std::cos(std::asin(std::clamp(nx, -1.0, 1.0)));
            // Lighting based on surface normal
            double light = 0.4 + 0.6 * z;
            int sx = std::clamp(static_cast<int>(srcX), 0, w - 1);
            QColor c = frame.pixelColor(sx, y);
            result.setPixelColor(x, y, QColor(
                std::clamp(static_cast<int>(c.red() * light), 0, 255),
                std::clamp(static_cast<int>(c.green() * light), 0, 255),
                std::clamp(static_cast<int>(c.blue() * light), 0, 255),
                c.alpha()));
        }
    }
    return result;
}

QImage GpuEffects::applyBevelAlpha(const QImage& frame, double depth, double angle, const QColor& lightColor) {
    // AE Bevel Alpha: 3D chisel effect on alpha edges with specular highlight
    QImage result = frame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    int w = result.width(), h = result.height();
    double rad = angle * M_PI / 180.0;
    double lx = std::cos(rad), ly = std::sin(rad);
    for (int y = 1; y < h - 1; ++y) {
        auto* line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 1; x < w - 1; ++x) {
            int alpha = qAlpha(line[x]);
            if (alpha < 5) continue;
            // Alpha gradient (surface normal from alpha channel)
            int aLeft = qAlpha(reinterpret_cast<const QRgb*>(frame.constScanLine(y))[x - 1]);
            int aRight = qAlpha(reinterpret_cast<const QRgb*>(frame.constScanLine(y))[x + 1]);
            int aUp = qAlpha(reinterpret_cast<const QRgb*>(frame.constScanLine(y - 1))[x]);
            int aDown = qAlpha(reinterpret_cast<const QRgb*>(frame.constScanLine(y + 1))[x]);
            double gradX = (aRight - aLeft) / 510.0;
            double gradY = (aDown - aUp) / 510.0;
            // Dot product with light direction = specular/shadow
            double dot = gradX * lx + gradY * ly;
            double highlight = std::clamp(dot * depth * 3.0, -1.0, 1.0);
            // Apply: positive = highlight (light color), negative = shadow (darken)
            int r = qRed(line[x]), g = qGreen(line[x]), b = qBlue(line[x]);
            if (highlight > 0) {
                r = std::clamp(r + static_cast<int>(highlight * lightColor.red()), 0, 255);
                g = std::clamp(g + static_cast<int>(highlight * lightColor.green()), 0, 255);
                b = std::clamp(b + static_cast<int>(highlight * lightColor.blue()), 0, 255);
            } else {
                r = std::clamp(r + static_cast<int>(highlight * 100), 0, 255);
                g = std::clamp(g + static_cast<int>(highlight * 100), 0, 255);
                b = std::clamp(b + static_cast<int>(highlight * 100), 0, 255);
            }
            line[x] = qRgba(r, g, b, alpha);
        }
    }
    return result;
}

QImage GpuEffects::applyDropShadowEffect(const QImage& frame, double angle, double distance, double softness, const QColor& color, double opacity) {
    if (!m_initialized) return frame;
    if (!m_dropShadowShader) {
        m_dropShadowShader = std::make_unique<QOpenGLShaderProgram>();
        if (!compileShader(*m_dropShadowShader, VERT_SHADER, DROP_SHADOW_FRAG)) return frame;
    }
    return renderToImage(frame, *m_dropShadowShader, [&](QOpenGLShaderProgram& s) {
        s.setUniformValue("texSize", QVector2D(frame.width(), frame.height()));
        s.setUniformValue("angle", static_cast<float>(angle * M_PI / 180.0));
        s.setUniformValue("distance_", static_cast<float>(distance));
        s.setUniformValue("softness", static_cast<float>(softness));
        s.setUniformValue("shadowColor", QVector4D(color.redF(), color.greenF(), color.blueF(), opacity));
    });
}

QImage GpuEffects::applyRadialShadow(const QImage& frame, double cx, double cy, const QColor& color, double opacity) {
    QImage result = frame.copy();
    QPainter p(&result);
    QRadialGradient grad(cx * frame.width(), cy * frame.height(), frame.width() * 0.5);
    grad.setColorAt(0, Qt::transparent);
    grad.setColorAt(1, QColor(color.red(), color.green(), color.blue(), static_cast<int>(opacity * 255)));
    p.fillRect(frame.rect(), grad);
    return result;
}

QImage GpuEffects::apply3DRotation(const QImage& frame, double rotX, double rotY, double rotZ, double perspective) {
    QImage result(frame.size(), QImage::Format_ARGB32_Premultiplied);
    result.fill(Qt::transparent);
    QPainter p(&result);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    QTransform t;
    t.translate(frame.width() / 2.0, frame.height() / 2.0);
    // Simulate perspective with scale
    double scaleX = std::cos(rotY * M_PI / 180.0);
    double scaleY = std::cos(rotX * M_PI / 180.0);
    t.scale(std::max(0.01, scaleX), std::max(0.01, scaleY));
    t.rotate(rotZ);
    t.translate(-frame.width() / 2.0, -frame.height() / 2.0);
    p.setTransform(t);
    p.drawImage(0, 0, frame);
    return result;
}

QImage GpuEffects::applyReflection(const QImage& frame, double reflectionHeight, double opacity, double gap) {
    QImage result(frame.width(), frame.height() + static_cast<int>(frame.height() * reflectionHeight + gap),
                  QImage::Format_ARGB32_Premultiplied);
    result.fill(Qt::transparent);
    QPainter p(&result);
    p.drawImage(0, 0, frame);
    // Draw mirrored reflection
    QImage mirrored = frame.mirrored(false, true);
    int ry = frame.height() + static_cast<int>(gap);
    int rh = static_cast<int>(frame.height() * reflectionHeight);
    QLinearGradient fade(0, ry, 0, ry + rh);
    fade.setColorAt(0, QColor(255, 255, 255, static_cast<int>(opacity * 255)));
    fade.setColorAt(1, Qt::transparent);
    p.setOpacity(opacity);
    p.drawImage(0, ry, mirrored.copy(0, 0, frame.width(), rh));
    return result.scaled(frame.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

QImage GpuEffects::applyVanishingPoint(const QImage& frame, double vpX, double vpY, double depth) {
    // AE Vanishing Point: perspective transform toward a vanishing point
    QImage result(frame.size(), QImage::Format_ARGB32_Premultiplied);
    result.fill(Qt::transparent);
    int w = frame.width(), h = frame.height();
    double vx = vpX * w, vy = vpY * h;
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            // Perspective: points converge toward vanishing point
            double dx = x - vx, dy = y - vy;
            double dist = std::sqrt(dx * dx + dy * dy) / std::max(w, h);
            double scale = 1.0 + depth * dist * 0.5;
            int sx = std::clamp(static_cast<int>(vx + dx * scale), 0, w - 1);
            int sy = std::clamp(static_cast<int>(vy + dy * scale), 0, h - 1);
            result.setPixelColor(x, y, frame.pixelColor(sx, sy));
        }
    }
    return result;
}

// ══════════════════════════════════════════════════════════════
// IMPLEMENTATIONS — AE Time Effects
// ══════════════════════════════════════════════════════════════

QImage GpuEffects::applyEcho(const QImage& frame, const QImage& prevFrame, double echoTime, int numEchoes, double startingIntensity, double decay) {
    if (prevFrame.isNull()) return frame;
    QImage result = frame.copy();
    QPainter p(&result);
    for (int i = 1; i <= numEchoes; ++i) {
        double alpha = startingIntensity * std::pow(decay, i);
        if (alpha < 0.01) break;
        p.setOpacity(alpha);
        p.drawImage(0, 0, prevFrame);
    }
    return result;
}

QImage GpuEffects::applyPosterizeTimeEffect(const QImage& frame, const QImage& heldFrame, double fps, double time) {
    double quantized = std::floor(time * fps) / fps;
    double current = time;
    // If we're between quantized frames, show held frame
    if (std::abs(current - quantized) > 0.001 && !heldFrame.isNull())
        return heldFrame;
    return frame;
}

QImage GpuEffects::applyTimeDisplacement(const QImage& frame, const QImage& displaceMap, double maxDisplacement) {
    // AE Time Displacement: luminance map controls temporal offset
    // Since we only have current+prev frames, simulate with spatial displacement + blur mix
    if (displaceMap.isNull()) return frame;
    QImage result = frame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    QImage map = displaceMap.scaled(frame.size()).convertToFormat(QImage::Format_ARGB32_Premultiplied);
    int w = frame.width(), h = frame.height();
    // Bright areas = "future" (sharper), dark areas = "past" (more blurred/displaced)
    for (int y = 0; y < h; ++y) {
        auto* line = reinterpret_cast<QRgb*>(result.scanLine(y));
        auto* mLine = reinterpret_cast<const QRgb*>(map.constScanLine(y));
        for (int x = 0; x < w; ++x) {
            double mapLum = (qRed(mLine[x]) * 0.299 + qGreen(mLine[x]) * 0.587 + qBlue(mLine[x]) * 0.114) / 255.0;
            // Displacement based on luminance (simulating temporal offset spatially)
            double disp = (mapLum - 0.5) * maxDisplacement * 20.0;
            int sx = std::clamp(x + static_cast<int>(disp), 0, w - 1);
            int sy = std::clamp(y + static_cast<int>(disp * 0.3), 0, h - 1);
            line[x] = reinterpret_cast<const QRgb*>(frame.constScanLine(sy))[sx];
        }
    }
    return result;
}

QImage GpuEffects::applyForceMotionBlur(const QImage& frame, const QImage& prevFrame, double shutterAngle) {
    if (prevFrame.isNull()) return frame;
    QImage result = frame.copy();
    QPainter p(&result);
    p.setOpacity(shutterAngle / 360.0);
    p.drawImage(0, 0, prevFrame);
    return result;
}

QImage GpuEffects::applyTrailsEffect(const QImage& frame, const QImage& prevFrame, double trailLength) {
    if (prevFrame.isNull()) return frame;
    QImage result = frame.copy();
    QPainter p(&result);
    p.setOpacity(trailLength);
    p.setCompositionMode(QPainter::CompositionMode_Lighten);
    p.drawImage(0, 0, prevFrame);
    return result;
}

// ══════════════════════════════════════════════════════════════
// IMPLEMENTATIONS — AE Matte/Keying
// ══════════════════════════════════════════════════════════════

QImage GpuEffects::applyLumaKey(const QImage& frame, double threshold, double feather, bool keyOut) {
    QImage result = frame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    for (int y = 0; y < result.height(); ++y) {
        auto* line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            double lum = (qRed(line[x]) * 0.299 + qGreen(line[x]) * 0.587 + qBlue(line[x]) * 0.114) / 255.0;
            double alpha;
            if (keyOut) {
                alpha = lum < threshold ? 0.0 : (lum < threshold + feather ? (lum - threshold) / feather : 1.0);
            } else {
                alpha = lum > threshold ? 0.0 : (lum > threshold - feather ? (threshold - lum) / feather : 1.0);
            }
            int a = static_cast<int>(alpha * qAlpha(line[x]));
            line[x] = qRgba(qRed(line[x]), qGreen(line[x]), qBlue(line[x]), a);
        }
    }
    return result;
}

QImage GpuEffects::applyDifferenceMatte(const QImage& frame, const QImage& cleanPlate, double tolerance, double softness) {
    if (cleanPlate.isNull()) return frame;
    QImage result = frame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    QImage plate = cleanPlate.scaled(frame.size()).convertToFormat(QImage::Format_ARGB32_Premultiplied);
    for (int y = 0; y < result.height(); ++y) {
        auto* fLine = reinterpret_cast<QRgb*>(result.scanLine(y));
        auto* pLine = reinterpret_cast<const QRgb*>(plate.constScanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            double diff = (std::abs(qRed(fLine[x]) - qRed(pLine[x])) +
                           std::abs(qGreen(fLine[x]) - qGreen(pLine[x])) +
                           std::abs(qBlue(fLine[x]) - qBlue(pLine[x]))) / 765.0;
            double alpha = diff < tolerance ? 0.0 : (diff < tolerance + softness ? (diff - tolerance) / softness : 1.0);
            int a = static_cast<int>(alpha * 255);
            fLine[x] = qRgba(qRed(fLine[x]), qGreen(fLine[x]), qBlue(fLine[x]), a);
        }
    }
    return result;
}

QImage GpuEffects::applyExtract(const QImage& frame, double blackPoint, double whitePoint, double softness) {
    // AE Extract: dual-threshold luminance keying (blackPoint AND whitePoint)
    QImage result = frame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    for (int y = 0; y < result.height(); ++y) {
        auto* line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            double lum = (qRed(line[x]) * 0.299 + qGreen(line[x]) * 0.587 + qBlue(line[x]) * 0.114) / 255.0;
            double alpha;
            if (lum < blackPoint - softness) alpha = 0.0;
            else if (lum < blackPoint) alpha = (lum - (blackPoint - softness)) / std::max(softness, 0.001);
            else if (lum <= whitePoint) alpha = 1.0;
            else if (lum < whitePoint + softness) alpha = 1.0 - (lum - whitePoint) / std::max(softness, 0.001);
            else alpha = 0.0;
            int a = static_cast<int>(std::clamp(alpha, 0.0, 1.0) * qAlpha(line[x]));
            line[x] = qRgba(qRed(line[x]), qGreen(line[x]), qBlue(line[x]), a);
        }
    }
    return result;
}

QImage GpuEffects::applyLinearColorKey(const QImage& frame, const QColor& keyColor, double tolerance, double feather) {
    // AE Linear Color Key: makes pixels transparent based on color distance (keying)
    // Different from Leave Color which desaturates — this modifies ALPHA
    QImage result = frame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    double kr = keyColor.redF(), kg = keyColor.greenF(), kb = keyColor.blueF();
    for (int y = 0; y < result.height(); ++y) {
        auto* line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            double r = qRed(line[x]) / 255.0, g = qGreen(line[x]) / 255.0, b = qBlue(line[x]) / 255.0;
            double diff = std::sqrt((r - kr) * (r - kr) + (g - kg) * (g - kg) + (b - kb) * (b - kb));
            double alpha;
            if (diff < tolerance) alpha = 0.0; // Key out (transparent)
            else if (diff < tolerance + feather) alpha = (diff - tolerance) / std::max(feather, 0.001);
            else alpha = 1.0;
            int a = static_cast<int>(std::clamp(alpha, 0.0, 1.0) * qAlpha(line[x]));
            line[x] = qRgba(qRed(line[x]), qGreen(line[x]), qBlue(line[x]), a);
        }
    }
    return result;
}

QImage GpuEffects::applyRefineEdge(const QImage& frame, double smooth, double feather, double contrast, double shift) {
    // AE Refine Edge: alpha-channel specific processing (smooth, feather, contract/expand)
    QImage result = frame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    int w = result.width(), h = result.height();
    int radius = std::max(1, static_cast<int>(smooth + feather));

    // Pass 1: Smooth alpha edges (box blur on alpha only)
    QImage smoothed = result;
    for (int y = 0; y < h; ++y) {
        auto* outLine = reinterpret_cast<QRgb*>(smoothed.scanLine(y));
        for (int x = 0; x < w; ++x) {
            double alphaSum = 0;
            int count = 0;
            for (int dy = -radius; dy <= radius; ++dy) {
                for (int dx = -radius; dx <= radius; ++dx) {
                    int sy = std::clamp(y + dy, 0, h - 1);
                    int sx = std::clamp(x + dx, 0, w - 1);
                    alphaSum += qAlpha(reinterpret_cast<const QRgb*>(result.constScanLine(sy))[sx]);
                    count++;
                }
            }
            int smoothAlpha = static_cast<int>(alphaSum / count);
            // Apply contrast to alpha
            double a = smoothAlpha / 255.0;
            a = std::clamp((a - 0.5) * (1.0 + contrast * 2.0) + 0.5, 0.0, 1.0);
            // Apply shift (expand/contract)
            a = std::clamp(a + shift * 0.3, 0.0, 1.0);
            outLine[x] = qRgba(qRed(outLine[x]), qGreen(outLine[x]), qBlue(outLine[x]),
                               static_cast<int>(a * 255));
        }
    }
    return smoothed;
}

QImage GpuEffects::applySimpleChoker(const QImage& frame, double chokeAmount) {
    // Erode/dilate alpha channel
    QImage result = frame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    int radius = static_cast<int>(std::abs(chokeAmount));
    if (radius < 1) return result;
    for (int y = 0; y < result.height(); ++y) {
        auto* line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            int minAlpha = 255, maxAlpha = 0;
            for (int dy = -radius; dy <= radius; ++dy) {
                for (int dx = -radius; dx <= radius; ++dx) {
                    int sy = std::clamp(y + dy, 0, result.height() - 1);
                    int sx = std::clamp(x + dx, 0, result.width() - 1);
                    int a = qAlpha(reinterpret_cast<const QRgb*>(frame.constScanLine(sy))[sx]);
                    minAlpha = std::min(minAlpha, a);
                    maxAlpha = std::max(maxAlpha, a);
                }
            }
            int newAlpha = chokeAmount > 0 ? minAlpha : maxAlpha;
            line[x] = qRgba(qRed(line[x]), qGreen(line[x]), qBlue(line[x]), newAlpha);
        }
    }
    return result;
}

QImage GpuEffects::applyAlphaMatte(const QImage& frame, const QImage& matte, bool invertMatte) {
    if (matte.isNull()) return frame;
    QImage result = frame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    QImage m = matte.scaled(frame.size()).convertToFormat(QImage::Format_ARGB32_Premultiplied);
    for (int y = 0; y < result.height(); ++y) {
        auto* fLine = reinterpret_cast<QRgb*>(result.scanLine(y));
        auto* mLine = reinterpret_cast<const QRgb*>(m.constScanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            int ma = qAlpha(mLine[x]);
            if (invertMatte) ma = 255 - ma;
            int fa = qAlpha(fLine[x]) * ma / 255;
            fLine[x] = qRgba(qRed(fLine[x]), qGreen(fLine[x]), qBlue(fLine[x]), fa);
        }
    }
    return result;
}

QImage GpuEffects::applyTrackMatte(const QImage& frame, const QImage& matte, bool useLuma) {
    if (matte.isNull()) return frame;
    QImage result = frame.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    QImage m = matte.scaled(frame.size()).convertToFormat(QImage::Format_ARGB32_Premultiplied);
    for (int y = 0; y < result.height(); ++y) {
        auto* fLine = reinterpret_cast<QRgb*>(result.scanLine(y));
        auto* mLine = reinterpret_cast<const QRgb*>(m.constScanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            int ma;
            if (useLuma) {
                ma = static_cast<int>(qRed(mLine[x]) * 0.299 + qGreen(mLine[x]) * 0.587 + qBlue(mLine[x]) * 0.114);
            } else {
                ma = qAlpha(mLine[x]);
            }
            int fa = qAlpha(fLine[x]) * ma / 255;
            fLine[x] = qRgba(qRed(fLine[x]), qGreen(fLine[x]), qBlue(fLine[x]), fa);
        }
    }
    return result;
}

// Make VERT_SHADER accessible from this translation unit
extern const char* VERT_SHADER;

} // namespace prestige
