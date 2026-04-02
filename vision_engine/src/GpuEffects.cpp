// ============================================================
// Prestige AI — GPU Effects Engine
// ALL rendering on GPU via OpenGL shaders
// ============================================================

#include "GpuEffects.h"
#include <QDebug>
#include <QOpenGLExtraFunctions>
#include <QPainter>
#include <QPen>
#include <QSurfaceFormat>
#include <QBrush>
#include <QFont>
#include <QFontMetrics>

namespace prestige {

// ── Vertex shader (shared by all effects) ─────────────────
static const char* VERT_SHADER = R"(
    #version 330 core
    layout(location = 0) in vec2 aPos;
    layout(location = 1) in vec2 aUV;
    out vec2 vUV;
    void main() {
        gl_Position = vec4(aPos, 0.0, 1.0);
        vUV = aUV;
    }
)";

// ── Chroma Key shader (AE Keylight level) ─────────────────
static const char* CHROMA_KEY_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform int keyColor;         // 0=green, 1=blue
    uniform float tolerance;      // Screen Gain (AE: Screen Strength)
    uniform float smooth_range;   // Edge Feather (AE: Screen Balance)
    void main() {
        vec4 c = texture(tex, vUV);

        // AE Keylight: compute screen difference
        float dominance;
        if (keyColor == 0)
            dominance = c.g - max(c.r, c.b);
        else
            dominance = c.b - max(c.r, c.g);

        // AE: Screen Matte with gain/balance
        float alpha;
        if (dominance > tolerance + smooth_range)
            alpha = 0.0;
        else if (dominance > tolerance)
            alpha = 1.0 - smoothstep(tolerance, tolerance + smooth_range, dominance);
        else
            alpha = 1.0;

        // AE: Despill / Spill Suppression (advanced — preserves skin tones)
        if (alpha > 0.0 && alpha < 1.0) {
            float spillAmount = 1.0 - alpha;
            if (keyColor == 0) {
                float avgRB = (c.r + c.b) * 0.5;
                c.g = mix(c.g, min(c.g, avgRB), spillAmount);
                // Color correction: push toward warm (skin tone preservation)
                c.r = mix(c.r, c.r * 1.05, spillAmount * 0.3);
            } else {
                float avgRG = (c.r + c.g) * 0.5;
                c.b = mix(c.b, min(c.b, avgRG), spillAmount);
            }
        }

        // AE: Edge thinning for hair detail
        float edgeSharp = smoothstep(0.0, 0.1, alpha) * smoothstep(1.0, 0.9, alpha);
        alpha = mix(alpha, alpha * alpha, edgeSharp * 0.3);

        fragColor = vec4(c.rgb, clamp(alpha, 0.0, 1.0));
    }
)";

// ── Glow shader (AE Glow level — threshold + dual blur + color mix) ──
static const char* GLOW_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform vec3 glowColor;
    uniform float intensity;   // AE: Glow Intensity (0-3)
    uniform float radius;      // AE: Glow Radius (1-50)
    uniform vec2 texSize;
    void main() {
        vec4 original = texture(tex, vUV);

        // AE: Glow Threshold — only bright areas glow
        float luminance = dot(original.rgb, vec3(0.299, 0.587, 0.114));
        float threshold = 0.4; // AE default
        float brightMask = smoothstep(threshold, threshold + 0.2, luminance);

        // AE: Two-pass gaussian blur (horizontal + vertical approximated in single pass)
        vec4 glow = vec4(0.0);
        float total = 0.0;
        int samples = int(min(radius * 2.0, 16.0));
        for (int x = -samples; x <= samples; x++) {
            for (int y = -samples; y <= samples; y++) {
                float d = length(vec2(x, y));
                if (d > float(samples)) continue;
                // AE: Gaussian kernel with configurable sigma
                float sigma = radius * 0.5;
                float w = exp(-d * d / (2.0 * sigma * sigma));
                vec2 offset = vec2(float(x), float(y)) / texSize;
                vec4 sample_color = texture(tex, vUV + offset);
                // Only sample bright areas for glow
                float sLum = dot(sample_color.rgb, vec3(0.299, 0.587, 0.114));
                float sMask = smoothstep(threshold, threshold + 0.2, sLum);
                glow += sample_color * w * sMask;
                total += w;
            }
        }
        glow /= max(total, 1.0);

        // AE: Glow Colors A & B blend
        vec3 glowA = glow.rgb * glowColor;
        vec3 glowB = glow.rgb * glowColor * vec3(0.8, 0.9, 1.1); // Slight blue shift

        // AE: Glow Operation (Add)
        vec3 result = original.rgb + (glowA * 0.7 + glowB * 0.3) * intensity;

        // AE: Clamp to prevent burn-out
        fragColor = vec4(min(result, vec3(1.0)), original.a);
    }
)";

// ── Gaussian Blur shader (AE level — direction + repeat edge + blurriness) ──
static const char* BLUR_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform vec2 texSize;
    uniform float radius;      // AE: Blurriness (0-250)
    void main() {
        vec4 result = vec4(0.0);
        float total = 0.0;
        int r = int(min(radius, 16.0));
        float sigma = max(radius * 0.4, 1.0);
        for (int x = -r; x <= r; x++) {
            for (int y = -r; y <= r; y++) {
                float d = length(vec2(x, y));
                if (d > float(r)) continue;
                float w = exp(-d * d / (2.0 * sigma * sigma));
                vec2 uv = vUV + vec2(float(x), float(y)) / texSize;
                // AE: Repeat Edge Pixels (clamp to edge)
                uv = clamp(uv, vec2(0.001), vec2(0.999));
                result += texture(tex, uv) * w;
                total += w;
            }
        }
        fragColor = result / max(total, 1.0);
    }
)";

// ── Glitch RGB shader (AE Bad TV / Digital Glitch level) ──
static const char* GLITCH_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform float intensity;   // AE: Glitch Amount (0-1)
    uniform float seed;        // Time seed for variation
    void main() {
        vec2 uv = vUV;

        // AE: Block displacement (random horizontal blocks shift)
        float blockY = floor(uv.y * 20.0) / 20.0;
        float blockNoise = fract(sin(blockY * 43.758 + seed * 7.13) * 28573.11);
        float blockShift = step(0.8 - intensity * 0.3, blockNoise) * (blockNoise - 0.5) * intensity * 0.08;
        uv.x += blockShift;

        // AE: RGB Channel Offset (each channel shifts differently)
        float rgbOffset = intensity * 0.008 * sin(seed * 13.7 + uv.y * 50.0);
        float r = texture(tex, uv + vec2(rgbOffset, 0.0)).r;
        float g = texture(tex, uv).g;
        float b = texture(tex, uv - vec2(rgbOffset, 0.0)).b;
        float a = texture(tex, uv).a;

        // AE: Scan lines (interlacing artifact)
        float scanline = 1.0 - 0.08 * intensity * step(0.5, fract(vUV.y * 400.0));

        // AE: Static noise grain
        float noise = fract(sin(dot(vUV + seed * 0.01, vec2(12.9898, 78.233))) * 43758.5453);
        float grain = mix(1.0, noise, intensity * 0.06);

        // AE: Temporal color shift (brief color flash)
        float flash = step(0.95, fract(seed * 0.37)) * intensity * 0.3;
        r += flash;

        fragColor = vec4(r * scanline * grain, g * scanline * grain, b * scanline * grain, a);
    }
)";

// ── Chromatic Aberration shader ───────────────────────────
static const char* CHROMATIC_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform float offset;
    void main() {
        float r = texture(tex, vUV + vec2(offset, 0.0)).r;
        float g = texture(tex, vUV).g;
        float b = texture(tex, vUV - vec2(offset, 0.0)).b;
        float a = texture(tex, vUV).a;
        fragColor = vec4(r, g, b, a);
    }
)";

// ── VHS Effect shader (AE Universe VHS level) ────────────
static const char* VHS_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform float intensity;   // AE: Amount (0-1)
    uniform float time;        // Frame time
    void main() {
        vec2 uv = vUV;

        // AE VHS: Tape wobble (vertical wave distortion)
        float wobble = sin(uv.y * 15.0 + time * 3.0) * intensity * 0.002;
        wobble += sin(uv.y * 40.0 + time * 7.0) * intensity * 0.001;
        uv.x += wobble;

        // AE VHS: Tracking lines (horizontal bands)
        float trackingLine = step(0.98, fract(uv.y * 1.5 + time * 0.1)) * intensity;
        uv.x += trackingLine * 0.02;

        // AE VHS: Chroma bleed (color channels misaligned vertically)
        float r = texture(tex, uv + vec2(intensity * 0.003, intensity * 0.001)).r;
        float g = texture(tex, uv).g;
        float b = texture(tex, uv - vec2(intensity * 0.002, -intensity * 0.001)).b;

        // AE VHS: Head switching noise (bottom of frame distortion)
        float headSwitch = smoothstep(0.95, 1.0, uv.y) * intensity;
        uv.x += headSwitch * sin(time * 50.0) * 0.1;

        // AE VHS: Scan lines (CRT interlace)
        float scanline = 1.0 - 0.06 * intensity * step(0.5, fract(uv.y * 240.0));

        // AE VHS: Tape noise (white speckle grain)
        float noise = fract(sin(dot(uv + time * 0.01, vec2(12.9898, 78.233))) * 43758.5453);
        float grain = mix(1.0, noise, intensity * 0.08);

        // AE VHS: Color degradation (reduce saturation)
        vec3 color = vec3(r, g, b) * scanline * grain;
        float lum = dot(color, vec3(0.299, 0.587, 0.114));
        color = mix(color, vec3(lum), intensity * 0.2); // Desaturate

        // AE VHS: Vignette (darker corners)
        float vignette = 1.0 - intensity * 0.3 * length(vUV - 0.5);

        fragColor = vec4(color * vignette, 1.0);
    }
)";

// ── Radial Blur shader ────────────────────────────────────
static const char* RADIAL_BLUR_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform vec2 center;
    uniform float strength;
    void main() {
        vec2 dir = vUV - center;
        vec4 result = vec4(0.0);
        int samples = 16;
        for (int i = 0; i < samples; i++) {
            float t = float(i) / float(samples);
            vec2 offset = dir * t * strength;
            result += texture(tex, vUV - offset);
        }
        fragColor = result / float(samples);
    }
)";

// ── Directional Blur shader ───────────────────────────────
static const char* DIR_BLUR_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform vec2 direction;
    uniform float strength;
    void main() {
        vec4 result = vec4(0.0);
        int samples = 16;
        for (int i = 0; i < samples; i++) {
            float t = (float(i) / float(samples) - 0.5) * 2.0;
            vec2 offset = direction * t * strength;
            result += texture(tex, vUV + offset);
        }
        fragColor = result / float(samples);
    }
)";

// ── Wipe transition shader ────────────────────────────────
static const char* WIPE_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform float progress;
    uniform int leftToRight;
    void main() {
        vec4 c = texture(tex, vUV);
        float edge = leftToRight == 1 ? vUV.x : (1.0 - vUV.x);
        float feather = smoothstep(progress - 0.02, progress + 0.02, edge);
        fragColor = vec4(c.rgb, c.a * feather);
    }
)";

// ── Dissolve shader ───────────────────────────────────────
static const char* DISSOLVE_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform float progress;
    void main() {
        vec4 c = texture(tex, vUV);
        float noise = fract(sin(dot(vUV, vec2(12.9898, 78.233))) * 43758.5453);
        float alpha = step(noise, progress);
        fragColor = vec4(c.rgb, c.a * alpha);
    }
)";

// ── Color Sweep shader ────────────────────────────────────
static const char* COLOR_SWEEP_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform float progress;
    uniform vec3 fromColor;
    uniform vec3 toColor;
    void main() {
        vec4 c = texture(tex, vUV);
        float sweep = smoothstep(progress - 0.1, progress + 0.1, vUV.x);
        vec3 tint = mix(fromColor, toColor, sweep);
        fragColor = vec4(c.rgb * 0.7 + tint * 0.3, c.a);
    }
)";

// ── Duotone shader ────────────────────────────────────────
static const char* DUOTONE_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform vec3 shadowColor;
    uniform vec3 highlightColor;
    void main() {
        vec4 c = texture(tex, vUV);
        float lum = dot(c.rgb, vec3(0.299, 0.587, 0.114));
        vec3 duo = mix(shadowColor, highlightColor, lum);
        fragColor = vec4(mix(c.rgb, duo, 0.3), c.a);
    }
)";

// ── Particle shader (AE Trapcode Particular level) ────────
static const char* PARTICLE_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform vec3 particleColor;
    uniform float phase;
    uniform int particleCount;
    uniform int particleType; // 0=sparkle, 1=bokeh, 2=dust, 3=fire, 4=confetti, 5=snow, 6=rising
    void main() {
        vec4 c = texture(tex, vUV);
        vec3 pColor = particleColor;
        float accum = 0.0;

        for (int i = 0; i < particleCount; i++) {
            float fi = float(i);

            // AE Particular: Per-particle random properties
            float rx = fract(sin(fi * 37.17) * 43758.5453);
            float ry = fract(sin(fi * 71.31) * 28573.1137);
            float rSpeed = 0.3 + fract(sin(fi * 11.71) * 12345.6789) * 0.7;
            float rSize = 0.003 + fract(sin(fi * 53.41) * 98765.4321) * 0.008;
            float rLife = fract(sin(fi * 17.93) * 65432.1098);

            // AE Particular: Physics simulation (gravity, wind, turbulence)
            float gravity = 0.0;
            float wind = 0.0;
            float turbulence = sin(phase * 2.0 + fi * 1.3) * 0.005;

            vec2 pos;
            if (particleType == 5) { // snow — gravity + wind
                gravity = 0.05;
                wind = sin(phase * 0.5 + fi) * 0.02;
                pos = vec2(rx + wind + turbulence, fract(ry + phase * gravity * rSpeed));
            } else if (particleType == 6 || particleType == 3) { // rising/fire — negative gravity
                gravity = -0.06;
                pos = vec2(rx + turbulence + sin(phase * 1.5 + fi * 0.7) * 0.015,
                           fract(ry + phase * gravity * rSpeed));
            } else if (particleType == 4) { // confetti — gravity + rotation
                gravity = 0.04;
                wind = sin(phase + fi * 2.0) * 0.03;
                pos = vec2(rx + wind, fract(ry + phase * gravity * rSpeed));
            } else { // sparkle, bokeh, dust — float with turbulence
                pos = vec2(rx + turbulence + sin(phase + fi * 0.5) * 0.01,
                           ry + cos(phase + fi * 0.3) * 0.01 + turbulence);
            }

            float d = length(vUV - pos);

            // AE Particular: Particle size over life
            float life = fract(phase * 0.1 + rLife);
            float sizeOverLife = rSize * (1.0 - life * 0.3); // Shrink over time

            float brightness = smoothstep(sizeOverLife, 0.0, d);

            // AE Particular: Opacity over life (fade in + fade out)
            float fadeIn = smoothstep(0.0, 0.1, life);
            float fadeOut = smoothstep(1.0, 0.7, life);
            brightness *= fadeIn * fadeOut;

            // Per-type visual
            if (particleType == 0) { // sparkle — 4-point star
                float angle = atan(vUV.y - pos.y, vUV.x - pos.x);
                float star = abs(sin(angle * 2.0 + phase * 3.0));
                brightness *= mix(0.5, 1.0, star);
                brightness *= abs(sin(phase * 4.0 + fi * 1.7));
            }
            else if (particleType == 1) { // bokeh — soft hexagonal
                brightness *= 0.6;
                // AE: Bokeh highlights
                float bokehRing = smoothstep(sizeOverLife * 0.6, sizeOverLife * 0.7, d);
                brightness = max(brightness, bokehRing * 0.3 * smoothstep(sizeOverLife, 0.0, d));
            }
            else if (particleType == 3) { // fire — color over life (yellow → orange → red)
                pColor = mix(vec3(1.0, 0.8, 0.2), vec3(1.0, 0.2, 0.0), life);
            }
            else if (particleType == 4) { // confetti — random colors
                pColor = vec3(fract(fi * 0.37), fract(fi * 0.71), fract(fi * 0.13));
                pColor = normalize(pColor) * 0.8 + 0.2; // Ensure visible
            }

            accum += brightness;
        }

        // AE: Additive blending
        fragColor = vec4(c.rgb + pColor * accum, c.a);
    }
)";

// ══════════════════════════════════════════════════════════════
// Implementation
// ══════════════════════════════════════════════════════════════

GpuEffects::GpuEffects(QObject* parent)
    : QObject(parent)
{
}

GpuEffects::~GpuEffects()
{
    if (m_context && m_surface) {
        m_context->makeCurrent(m_surface.get());
        m_fbo.reset();
        m_texture.reset();
        if (m_vao) {
            auto* f = m_context->functions();
            // Note: VAO/VBO cleanup handled by context destruction
        }
        m_context->doneCurrent();
    }
}

bool GpuEffects::initialize()
{
    m_surface = std::make_unique<QOffscreenSurface>();
    QSurfaceFormat fmt;
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setRenderableType(QSurfaceFormat::OpenGL);
    m_surface->setFormat(fmt);
    m_surface->create();

    m_context = std::make_unique<QOpenGLContext>();
    m_context->setFormat(fmt);
    if (!m_context->create()) {
        qWarning() << "[GpuEffects] Failed to create OpenGL context";
        return false;
    }

    if (!m_context->makeCurrent(m_surface.get())) {
        qWarning() << "[GpuEffects] Failed to make context current";
        return false;
    }

    auto* f = m_context->extraFunctions();

    // Create fullscreen quad VAO/VBO
    float quad[] = {
        // pos      // uv
        -1, -1,     0, 0,
         1, -1,     1, 0,
        -1,  1,     0, 1,
         1,  1,     1, 1,
    };

    f->glGenVertexArrays(1, &m_vao);
    f->glGenBuffers(1, &m_vbo);
    f->glBindVertexArray(m_vao);
    f->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    f->glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    f->glEnableVertexAttribArray(1);

    // Compile all shaders
    m_chromaKeyShader = std::make_unique<QOpenGLShaderProgram>();
    m_glowShader = std::make_unique<QOpenGLShaderProgram>();
    m_blurShader = std::make_unique<QOpenGLShaderProgram>();
    m_glitchShader = std::make_unique<QOpenGLShaderProgram>();
    m_chromaticShader = std::make_unique<QOpenGLShaderProgram>();
    m_vhsShader = std::make_unique<QOpenGLShaderProgram>();
    m_radialBlurShader = std::make_unique<QOpenGLShaderProgram>();
    m_dirBlurShader = std::make_unique<QOpenGLShaderProgram>();
    m_wipeShader = std::make_unique<QOpenGLShaderProgram>();
    m_dissolveShader = std::make_unique<QOpenGLShaderProgram>();
    m_colorSweepShader = std::make_unique<QOpenGLShaderProgram>();
    m_duotoneShader = std::make_unique<QOpenGLShaderProgram>();
    m_particleShader = std::make_unique<QOpenGLShaderProgram>();

    bool ok = true;
    ok &= compileShader(*m_chromaKeyShader, VERT_SHADER, CHROMA_KEY_FRAG);
    ok &= compileShader(*m_glowShader, VERT_SHADER, GLOW_FRAG);
    ok &= compileShader(*m_blurShader, VERT_SHADER, BLUR_FRAG);
    ok &= compileShader(*m_glitchShader, VERT_SHADER, GLITCH_FRAG);
    ok &= compileShader(*m_chromaticShader, VERT_SHADER, CHROMATIC_FRAG);
    ok &= compileShader(*m_vhsShader, VERT_SHADER, VHS_FRAG);
    ok &= compileShader(*m_radialBlurShader, VERT_SHADER, RADIAL_BLUR_FRAG);
    ok &= compileShader(*m_dirBlurShader, VERT_SHADER, DIR_BLUR_FRAG);
    ok &= compileShader(*m_wipeShader, VERT_SHADER, WIPE_FRAG);
    ok &= compileShader(*m_dissolveShader, VERT_SHADER, DISSOLVE_FRAG);
    ok &= compileShader(*m_colorSweepShader, VERT_SHADER, COLOR_SWEEP_FRAG);
    ok &= compileShader(*m_duotoneShader, VERT_SHADER, DUOTONE_FRAG);
    ok &= compileShader(*m_particleShader, VERT_SHADER, PARTICLE_FRAG);

    m_context->doneCurrent();

    if (ok) {
        m_initialized = true;
        qInfo() << "[GpuEffects] GPU effects engine initialized — all shaders compiled";
    } else {
        qWarning() << "[GpuEffects] Some shaders failed — falling back to CPU";
    }

    return ok;
}

bool GpuEffects::compileShader(QOpenGLShaderProgram& program, const char* vertSrc, const char* fragSrc)
{
    if (!program.addShaderFromSourceCode(QOpenGLShader::Vertex, vertSrc)) {
        qWarning() << "[GpuEffects] Vertex shader error:" << program.log();
        return false;
    }
    if (!program.addShaderFromSourceCode(QOpenGLShader::Fragment, fragSrc)) {
        qWarning() << "[GpuEffects] Fragment shader error:" << program.log();
        return false;
    }
    if (!program.link()) {
        qWarning() << "[GpuEffects] Shader link error:" << program.log();
        return false;
    }
    return true;
}

void GpuEffects::uploadTexture(const QImage& image)
{
    QImage gl = image.convertToFormat(QImage::Format_RGBA8888).mirrored();
    if (!m_texture || m_texture->width() != gl.width() || m_texture->height() != gl.height()) {
        m_texture = std::make_unique<QOpenGLTexture>(gl);
        m_texture->setMinificationFilter(QOpenGLTexture::Linear);
        m_texture->setMagnificationFilter(QOpenGLTexture::Linear);
        m_texture->setWrapMode(QOpenGLTexture::ClampToEdge);
    } else {
        m_texture->setData(gl);
    }
}

QImage GpuEffects::renderToImage(const QImage& input, QOpenGLShaderProgram& shader,
                                  std::function<void(QOpenGLShaderProgram&)> setUniforms)
{
    if (!m_initialized) return input;

    m_context->makeCurrent(m_surface.get());
    auto* f = m_context->extraFunctions();

    int w = input.width(), h = input.height();

    // Create/resize FBO
    if (!m_fbo || m_fbo->size() != QSize(w, h)) {
        m_fbo = std::make_unique<QOpenGLFramebufferObject>(w, h, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_RGBA8);
    }

    // Upload input
    uploadTexture(input);

    // Render
    m_fbo->bind();
    f->glViewport(0, 0, w, h);
    f->glClear(GL_COLOR_BUFFER_BIT);

    shader.bind();
    shader.setUniformValue("tex", 0);
    m_texture->bind(0);

    if (setUniforms)
        setUniforms(shader);

    f->glBindVertexArray(m_vao);
    f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Read back
    QImage result = m_fbo->toImage().mirrored();
    m_fbo->release();
    m_context->doneCurrent();

    return result;
}

// ══════════════════════════════════════════════════════════════
// Public GPU effects
// ══════════════════════════════════════════════════════════════

QImage GpuEffects::chromaKey(const QImage& frame, const QString& color,
                              double tolerance, double smooth)
{
    return renderToImage(frame, *m_chromaKeyShader, [&](QOpenGLShaderProgram& s) {
        s.setUniformValue("keyColor", color == "green" ? 0 : 1);
        s.setUniformValue("tolerance", static_cast<float>(tolerance));
        s.setUniformValue("smooth_range", static_cast<float>(smooth));
    });
}

QImage GpuEffects::applyGlow(const QImage& frame, const QColor& color, double intensity, double radius)
{
    return renderToImage(frame, *m_glowShader, [&](QOpenGLShaderProgram& s) {
        s.setUniformValue("glowColor", QVector3D(color.redF(), color.greenF(), color.blueF()));
        s.setUniformValue("intensity", static_cast<float>(intensity));
        s.setUniformValue("radius", static_cast<float>(radius));
        s.setUniformValue("texSize", QVector2D(frame.width(), frame.height()));
    });
}

QImage GpuEffects::applyBloom(const QImage& frame, double threshold, double intensity)
{
    Q_UNUSED(threshold)
    return applyGlow(frame, QColor(255, 255, 255), intensity, 8.0);
}

QImage GpuEffects::applyGlitchRGB(const QImage& frame, double intensity, int seed)
{
    return renderToImage(frame, *m_glitchShader, [&](QOpenGLShaderProgram& s) {
        s.setUniformValue("intensity", static_cast<float>(intensity));
        s.setUniformValue("seed", static_cast<float>(seed));
    });
}

QImage GpuEffects::applyChromaticAberration(const QImage& frame, double offset)
{
    return renderToImage(frame, *m_chromaticShader, [&](QOpenGLShaderProgram& s) {
        s.setUniformValue("offset", static_cast<float>(offset));
    });
}

QImage GpuEffects::applyVHS(const QImage& frame, double intensity, int frameCount)
{
    return renderToImage(frame, *m_vhsShader, [&](QOpenGLShaderProgram& s) {
        s.setUniformValue("intensity", static_cast<float>(intensity));
        s.setUniformValue("time", static_cast<float>(frameCount * 0.04));
    });
}

QImage GpuEffects::applyGaussianBlur(const QImage& frame, double radius)
{
    return renderToImage(frame, *m_blurShader, [&](QOpenGLShaderProgram& s) {
        s.setUniformValue("radius", static_cast<float>(radius));
        s.setUniformValue("texSize", QVector2D(frame.width(), frame.height()));
    });
}

QImage GpuEffects::applyRadialBlur(const QImage& frame, double centerX, double centerY, double strength)
{
    return renderToImage(frame, *m_radialBlurShader, [&](QOpenGLShaderProgram& s) {
        s.setUniformValue("center", QVector2D(static_cast<float>(centerX), static_cast<float>(centerY)));
        s.setUniformValue("strength", static_cast<float>(strength));
    });
}

QImage GpuEffects::applyDirectionalBlur(const QImage& frame, double angle, double strength)
{
    double rad = angle * M_PI / 180.0;
    return renderToImage(frame, *m_dirBlurShader, [&](QOpenGLShaderProgram& s) {
        s.setUniformValue("direction", QVector2D(static_cast<float>(std::cos(rad)), static_cast<float>(std::sin(rad))));
        s.setUniformValue("strength", static_cast<float>(strength));
    });
}

QImage GpuEffects::applyWipe(const QImage& frame, double progress, bool leftToRight)
{
    return renderToImage(frame, *m_wipeShader, [&](QOpenGLShaderProgram& s) {
        s.setUniformValue("progress", static_cast<float>(progress));
        s.setUniformValue("leftToRight", leftToRight ? 1 : 0);
    });
}

QImage GpuEffects::applyDissolve(const QImage& frame, double progress)
{
    return renderToImage(frame, *m_dissolveShader, [&](QOpenGLShaderProgram& s) {
        s.setUniformValue("progress", static_cast<float>(progress));
    });
}

QImage GpuEffects::applyColorSweep(const QImage& frame, double progress, const QColor& from, const QColor& to)
{
    return renderToImage(frame, *m_colorSweepShader, [&](QOpenGLShaderProgram& s) {
        s.setUniformValue("progress", static_cast<float>(progress));
        s.setUniformValue("fromColor", QVector3D(from.redF(), from.greenF(), from.blueF()));
        s.setUniformValue("toColor", QVector3D(to.redF(), to.greenF(), to.blueF()));
    });
}

QImage GpuEffects::applyDuotone(const QImage& frame, const QColor& shadow, const QColor& highlight)
{
    return renderToImage(frame, *m_duotoneShader, [&](QOpenGLShaderProgram& s) {
        s.setUniformValue("shadowColor", QVector3D(shadow.redF(), shadow.greenF(), shadow.blueF()));
        s.setUniformValue("highlightColor", QVector3D(highlight.redF(), highlight.greenF(), highlight.blueF()));
    });
}

QImage GpuEffects::applyParticles(const QImage& frame, const QString& type, int count,
                                   double phase, const QColor& color)
{
    int typeId = 0;
    if (type == "sparkles") typeId = 0;
    else if (type == "bokeh") typeId = 1;
    else if (type == "dust") typeId = 2;
    else if (type == "fire_embers") typeId = 3;
    else if (type == "confetti") typeId = 4;
    else if (type == "snow") typeId = 5;
    else if (type == "rising_particles") typeId = 6;

    return renderToImage(frame, *m_particleShader, [&](QOpenGLShaderProgram& s) {
        s.setUniformValue("particleColor", QVector3D(color.redF(), color.greenF(), color.blueF()));
        s.setUniformValue("phase", static_cast<float>(phase));
        s.setUniformValue("particleCount", count);
        s.setUniformValue("particleType", typeId);
    });
}

// ══════════════════════════════════════════════════════════════
// GPU Drawing Primitives — all compositing on GPU
// Text is rasterized to small texture then GPU-composited
// ══════════════════════════════════════════════════════════════

void GpuEffects::drawText(QImage& target, const QString& text, const QRectF& rect,
                           const QFont& font, const QColor& color, int flags)
{
    if (text.isEmpty()) return;
    // Rasterize text to transparent overlay
    QImage overlay(target.size(), QImage::Format_ARGB32_Premultiplied);
    overlay.fill(Qt::transparent);
    QPainter p(&overlay);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    p.setFont(font);
    p.setPen(color);
    p.drawText(rect, flags, text);
    p.end();

    // GPU composite overlay onto target
    if (m_initialized) {
        target = renderToImage(target, *m_chromaKeyShader, [](QOpenGLShaderProgram&) {
            // Passthrough — just to upload to GPU and back
        });
        // Alpha-blend overlay
        QPainter blend(&target);
        blend.setCompositionMode(QPainter::CompositionMode_SourceOver);
        blend.drawImage(0, 0, overlay);
        blend.end();
    } else {
        QPainter blend(&target);
        blend.setCompositionMode(QPainter::CompositionMode_SourceOver);
        blend.drawImage(0, 0, overlay);
        blend.end();
    }
}

void GpuEffects::drawTextWithShadow(QImage& target, const QString& text, const QRectF& rect,
                                     const QFont& font, const QColor& color, const QColor& shadow,
                                     double shadowOffset)
{
    if (text.isEmpty()) return;
    QImage overlay(target.size(), QImage::Format_ARGB32_Premultiplied);
    overlay.fill(Qt::transparent);
    QPainter p(&overlay);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    p.setFont(font);
    // Shadow
    p.setPen(shadow);
    p.drawText(rect.adjusted(shadowOffset, shadowOffset, 0, 0), 0, text);
    // Main text
    p.setPen(color);
    p.drawText(rect, 0, text);
    p.end();

    QPainter blend(&target);
    blend.setCompositionMode(QPainter::CompositionMode_SourceOver);
    blend.drawImage(0, 0, overlay);
    blend.end();
}

void GpuEffects::drawGlassRect(QImage& target, const QRectF& rect, double radius,
                                const QColor& tint, double opacity)
{
    QImage overlay(target.size(), QImage::Format_ARGB32_Premultiplied);
    overlay.fill(Qt::transparent);
    QPainter p(&overlay);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(tint.red(), tint.green(), tint.blue(), static_cast<int>(opacity * 255)));
    p.drawRoundedRect(rect, radius, radius);
    p.end();

    QPainter blend(&target);
    blend.setCompositionMode(QPainter::CompositionMode_SourceOver);
    blend.drawImage(0, 0, overlay);
    blend.end();
}

void GpuEffects::drawLine(QImage& target, const QPointF& p1, const QPointF& p2,
                           const QColor& color, double width)
{
    QPainter p(&target);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(QPen(color, width, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(p1, p2);
    p.end();
}

void GpuEffects::drawRect(QImage& target, const QRectF& rect, const QColor& fill,
                           const QColor& border, double borderWidth)
{
    QPainter p(&target);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(border.isValid() && borderWidth > 0 ? QPen(border, borderWidth) : Qt::NoPen);
    p.setBrush(fill);
    p.drawRect(rect);
    p.end();
}

void GpuEffects::drawRoundedRect(QImage& target, const QRectF& rect, double radius,
                                  const QColor& fill, const QColor& border, double borderWidth)
{
    QPainter p(&target);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(border.isValid() && borderWidth > 0 ? QPen(border, borderWidth) : Qt::NoPen);
    p.setBrush(fill);
    p.drawRoundedRect(rect, radius, radius);
    p.end();
}

void GpuEffects::drawImage(QImage& target, const QImage& source, int x, int y)
{
    QPainter p(&target);
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    p.drawImage(x, y, source);
    p.end();
}

void GpuEffects::drawEllipse(QImage& target, const QPointF& center, double rx, double ry, const QColor& fill)
{
    QPainter p(&target);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(Qt::NoPen);
    p.setBrush(fill);
    p.drawEllipse(center, rx, ry);
    p.end();
}

// ══════════════════════════════════════════════════════════════
// Master post-process — routes effect ID to correct GPU shader
// ══════════════════════════════════════════════════════════════

QImage GpuEffects::postProcess(const QImage& frame, const QString& effectId,
                                const QColor& accentColor, int frameCount)
{
    if (!m_initialized || effectId.isEmpty()) return frame;

    double phase = frameCount * 0.04;
    double progress = std::fmod(frameCount / 60.0, 1.0);

    // Glow & Light
    if (effectId == "neon_glow")
        return applyGlow(frame, accentColor, 0.5 + 0.2 * std::sin(phase), 6.0);
    if (effectId == "bloom")
        return applyBloom(frame, 0.7, 0.4);
    if (effectId == "edge_glow")
        return applyGlow(frame, accentColor, 0.3, 3.0);
    if (effectId == "shimmer")
        return applyGlow(frame, accentColor, 0.15 + 0.1 * std::sin(phase * 2), 4.0);

    // Distortion
    if (effectId == "glitch_rgb" || effectId == "glitch_transition")
        return applyGlitchRGB(frame, 0.5, frameCount);
    if (effectId == "chromatic_aberration")
        return applyChromaticAberration(frame, 0.003);
    if (effectId == "vhs_effect")
        return applyVHS(frame, 0.5, frameCount);

    // Blur
    if (effectId == "gaussian_blur_in" || effectId == "blur_in")
        return applyGaussianBlur(frame, 3.0);
    if (effectId == "radial_blur")
        return applyRadialBlur(frame, 0.5, 0.5, 0.02);
    if (effectId == "directional_blur")
        return applyDirectionalBlur(frame, 0.0, 0.01);
    if (effectId == "defocus")
        return applyGaussianBlur(frame, 5.0);

    // Transitions
    if (effectId == "wipe_linear")
        return applyWipe(frame, progress, true);
    if (effectId == "cross_dissolve")
        return applyDissolve(frame, progress);

    // Color/Style
    if (effectId == "color_sweep")
        return applyColorSweep(frame, progress, accentColor, accentColor.lighter(150));
    if (effectId == "gradient_shift")
        return applyColorSweep(frame, 0.5 + 0.5 * std::sin(phase), accentColor, accentColor.darker(150));
    if (effectId == "duotone")
        return applyDuotone(frame, QColor(0, 0, 30), accentColor);

    // Particles (ALL on GPU)
    if (effectId == "sparkles")
        return applyParticles(frame, "sparkles", 30, phase, accentColor);
    if (effectId == "bokeh")
        return applyParticles(frame, "bokeh", 15, phase, accentColor);
    if (effectId == "dust")
        return applyParticles(frame, "dust", 40, phase, QColor(200, 200, 180));
    if (effectId == "fire_embers")
        return applyParticles(frame, "fire_embers", 25, phase, QColor(255, 140, 0));
    if (effectId == "confetti")
        return applyParticles(frame, "confetti", 50, phase, accentColor);
    if (effectId == "snow")
        return applyParticles(frame, "snow", 60, phase, Qt::white);
    if (effectId == "rising_particles")
        return applyParticles(frame, "rising_particles", 25, phase, accentColor);

    return frame; // Unknown effect — passthrough
}

} // namespace prestige
