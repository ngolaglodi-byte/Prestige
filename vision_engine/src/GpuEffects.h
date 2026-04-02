#pragma once

// ============================================================
// Prestige AI — GPU Effects Engine
// Hardware-accelerated effects via OpenGL/Metal shaders
// Replaces CPU-heavy operations: chroma key, particles, glow, distortion
// Uses QOpenGLFramebufferObject for GPU → QImage capture
// ============================================================

#include <QObject>
#include <QImage>
#include <QSize>
#include <QColor>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLFramebufferObject>
#include <QOpenGLTexture>
#include <QOffscreenSurface>
#include <memory>

namespace prestige {

class GpuEffects : public QObject {
    Q_OBJECT

public:
    explicit GpuEffects(QObject* parent = nullptr);
    ~GpuEffects() override;

    bool initialize();
    bool isAvailable() const { return m_initialized; }

    // ── Chroma Key (GPU — 100x faster than CPU) ──────────
    QImage chromaKey(const QImage& frame, const QString& color,
                     double tolerance, double smooth);

    // ── Glow / Bloom (GPU multi-pass blur) ───────────────
    QImage applyGlow(const QImage& frame, const QColor& color, double intensity, double radius);
    QImage applyBloom(const QImage& frame, double threshold, double intensity);

    // ── Distortion (GPU shader) ──────────────────────────
    QImage applyGlitchRGB(const QImage& frame, double intensity, int seed);
    QImage applyChromaticAberration(const QImage& frame, double offset);
    QImage applyVHS(const QImage& frame, double intensity, int frameCount);

    // ── Blur (GPU gaussian) ──────────────────────────────
    QImage applyGaussianBlur(const QImage& frame, double radius);
    QImage applyRadialBlur(const QImage& frame, double centerX, double centerY, double strength);

private:
    bool compileShader(QOpenGLShaderProgram& program, const char* vertSrc, const char* fragSrc);
    QImage renderToImage(const QImage& input, QOpenGLShaderProgram& shader,
                         std::function<void(QOpenGLShaderProgram&)> setUniforms = nullptr);
    void uploadTexture(const QImage& image);

    bool m_initialized = false;
    std::unique_ptr<QOpenGLContext> m_context;
    std::unique_ptr<QOffscreenSurface> m_surface;
    std::unique_ptr<QOpenGLFramebufferObject> m_fbo;
    std::unique_ptr<QOpenGLTexture> m_texture;

    // Compiled shaders (cached)
    std::unique_ptr<QOpenGLShaderProgram> m_chromaKeyShader;
    std::unique_ptr<QOpenGLShaderProgram> m_glowShader;
    std::unique_ptr<QOpenGLShaderProgram> m_blurShader;
    std::unique_ptr<QOpenGLShaderProgram> m_glitchShader;
    std::unique_ptr<QOpenGLShaderProgram> m_chromaticShader;
    std::unique_ptr<QOpenGLShaderProgram> m_vhsShader;
    std::unique_ptr<QOpenGLShaderProgram> m_radialBlurShader;

    QSize m_lastSize;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
};

} // namespace prestige
