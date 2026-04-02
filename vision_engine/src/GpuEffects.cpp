// ============================================================
// Prestige AI — GPU Effects Engine
// ALL rendering on GPU via OpenGL shaders
// ============================================================

#include "GpuEffects.h"
#include <QDebug>
#include <QOpenGLExtraFunctions>

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

// ── Chroma Key shader ─────────────────────────────────────
static const char* CHROMA_KEY_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform int keyColor; // 0=green, 1=blue
    uniform float tolerance;
    uniform float smooth_range;
    void main() {
        vec4 c = texture(tex, vUV);
        float dominance;
        if (keyColor == 0)
            dominance = c.g - max(c.r, c.b);
        else
            dominance = c.b - max(c.r, c.g);
        float alpha;
        if (dominance > tolerance + smooth_range)
            alpha = 0.0;
        else if (dominance > tolerance)
            alpha = 1.0 - (dominance - tolerance) / smooth_range;
        else
            alpha = 1.0;
        // Spill suppression
        if (alpha > 0.0 && alpha < 1.0) {
            if (keyColor == 0)
                c.g = min(c.g, (c.r + c.b) * 0.5);
            else
                c.b = min(c.b, (c.r + c.g) * 0.5);
        }
        fragColor = vec4(c.rgb, alpha);
    }
)";

// ── Glow shader (single-pass radial glow) ─────────────────
static const char* GLOW_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform vec3 glowColor;
    uniform float intensity;
    uniform float radius;
    uniform vec2 texSize;
    void main() {
        vec4 original = texture(tex, vUV);
        vec4 glow = vec4(0.0);
        float total = 0.0;
        int samples = 12;
        for (int x = -samples; x <= samples; x++) {
            for (int y = -samples; y <= samples; y++) {
                float d = length(vec2(x, y));
                if (d > float(samples)) continue;
                float w = exp(-d * d / (2.0 * radius * radius));
                vec2 offset = vec2(float(x), float(y)) / texSize;
                glow += texture(tex, vUV + offset) * w;
                total += w;
            }
        }
        glow /= total;
        vec3 glowResult = glow.rgb * glowColor * intensity;
        fragColor = vec4(original.rgb + glowResult, original.a);
    }
)";

// ── Gaussian Blur shader ──────────────────────────────────
static const char* BLUR_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform vec2 texSize;
    uniform float radius;
    void main() {
        vec4 result = vec4(0.0);
        float total = 0.0;
        int r = int(radius);
        for (int x = -r; x <= r; x++) {
            for (int y = -r; y <= r; y++) {
                float d = length(vec2(x, y));
                float w = exp(-d * d / (2.0 * radius * radius));
                vec2 offset = vec2(float(x), float(y)) / texSize;
                result += texture(tex, vUV + offset) * w;
                total += w;
            }
        }
        fragColor = result / total;
    }
)";

// ── Glitch RGB shader ─────────────────────────────────────
static const char* GLITCH_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform float intensity;
    uniform float seed;
    void main() {
        float offset = intensity * 0.01 * sin(seed * 13.7 + vUV.y * 50.0);
        float r = texture(tex, vUV + vec2(offset, 0.0)).r;
        float g = texture(tex, vUV).g;
        float b = texture(tex, vUV - vec2(offset, 0.0)).b;
        float a = texture(tex, vUV).a;
        // Scan line effect
        float scanline = 1.0 - 0.1 * intensity * step(0.5, fract(vUV.y * 300.0));
        fragColor = vec4(r * scanline, g * scanline, b * scanline, a);
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

// ── VHS Effect shader ─────────────────────────────────────
static const char* VHS_FRAG = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 fragColor;
    uniform sampler2D tex;
    uniform float intensity;
    uniform float time;
    void main() {
        vec2 uv = vUV;
        // Horizontal shake
        uv.x += intensity * 0.003 * sin(time * 7.0 + uv.y * 20.0);
        // Color bleed
        float r = texture(tex, uv + vec2(intensity * 0.002, 0.0)).r;
        float g = texture(tex, uv).g;
        float b = texture(tex, uv - vec2(intensity * 0.002, 0.0)).b;
        // Scan lines
        float scanline = 1.0 - 0.08 * intensity * step(0.5, fract(uv.y * 240.0));
        // Noise
        float noise = fract(sin(dot(uv + time, vec2(12.9898, 78.233))) * 43758.5453);
        float n = mix(1.0, noise, intensity * 0.05);
        fragColor = vec4(r * scanline * n, g * scanline * n, b * scanline * n, 1.0);
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
    m_surface->create();

    m_context = std::make_unique<QOpenGLContext>();
    m_context->setFormat(m_surface->requestedFormat());
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

    bool ok = true;
    ok &= compileShader(*m_chromaKeyShader, VERT_SHADER, CHROMA_KEY_FRAG);
    ok &= compileShader(*m_glowShader, VERT_SHADER, GLOW_FRAG);
    ok &= compileShader(*m_blurShader, VERT_SHADER, BLUR_FRAG);
    ok &= compileShader(*m_glitchShader, VERT_SHADER, GLITCH_FRAG);
    ok &= compileShader(*m_chromaticShader, VERT_SHADER, CHROMATIC_FRAG);
    ok &= compileShader(*m_vhsShader, VERT_SHADER, VHS_FRAG);
    ok &= compileShader(*m_radialBlurShader, VERT_SHADER, RADIAL_BLUR_FRAG);

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

} // namespace prestige
