// ============================================================
// Prestige AI — Virtual Studio Implementation
// Chroma Key + 8 Procedural Studio Backgrounds + Animations
// ============================================================

#include "VirtualStudio.h"
#include <QLinearGradient>
#include <QRadialGradient>
#include <QPen>
#include <QBrush>
#include <QDateTime>
#include <algorithm>

namespace prestige {

void VirtualStudio::setCustomBackgroundPath(const QString& path)
{
    if (m_customBgPath == path) return;
    m_customBgPath = path;
    if (!path.isEmpty()) {
        QImage img(path);
        if (!img.isNull()) {
            m_customBg = img;
            m_bgDirty = true;
            qInfo() << "[VirtualStudio] Custom background loaded:" << path;
        } else {
            m_customBg = QImage();
            qWarning() << "[VirtualStudio] Failed to load custom background:" << path;
        }
    } else {
        m_customBg = QImage();
        m_bgDirty = true;
    }
}

QImage VirtualStudio::process(const QImage& rawFrame)
{
    if (!m_enabled || rawFrame.isNull())
        return rawFrame.copy();

    m_frameCounter++;

    QSize sz = rawFrame.size();

    // Step 1: Chroma key — extract talent with alpha mask
    QImage keyedTalent = m_chromaKeyEnabled ? applyChromaKey(rawFrame) : rawFrame.convertToFormat(QImage::Format_ARGB32);

    // Step 2: Render studio background (cached if unchanged)
    if (m_bgDirty || m_cachedBg.size() != sz) {
        m_cachedBg = renderStudioBackground(sz);
        m_bgDirty = false;
    }
    QImage studio = m_cachedBg.copy();

    // Step 3: Animated elements on studio (if enabled)
    if (m_animEnabled) {
        QPainter bgPainter(&studio);
        bgPainter.setRenderHint(QPainter::Antialiasing, true);
        renderAnimatedElements(bgPainter, sz);
        bgPainter.end();
    }

    // Step 4: Composite talent on studio
    QImage result = compositeTalentOnStudio(studio, keyedTalent);

    return result;
}

// ══════════════════════════════════════════════════════════════
// Chroma Key — broadcast-standard green/blue screen removal
// ══════════════════════════════════════════════════════════════

QImage VirtualStudio::applyChromaKey(const QImage& frame)
{
    QImage result = frame.convertToFormat(QImage::Format_ARGB32);
    int w = result.width();
    int h = result.height();

    bool isGreen = (m_chromaKeyColor == "green");
    double tol = m_tolerance * 255.0;
    double smoothRange = m_smooth * 255.0;

    for (int y = 0; y < h; ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < w; ++x) {
            int r = qRed(line[x]);
            int g = qGreen(line[x]);
            int b = qBlue(line[x]);

            double dominance;
            if (isGreen)
                dominance = g - std::max(r, b);
            else
                dominance = b - std::max(r, g);

            int alpha;
            if (dominance > tol + smoothRange) {
                alpha = 0; // fully transparent (key color)
            } else if (dominance > tol) {
                // Smooth edge transition (feathering)
                double t = (dominance - tol) / smoothRange;
                alpha = static_cast<int>((1.0 - t) * 255.0);
            } else {
                alpha = 255; // fully opaque (talent)
            }

            // Spill suppression — reduce green/blue tint on edges
            if (alpha > 0 && alpha < 255) {
                if (isGreen)
                    g = std::min(g, static_cast<int>((r + b) * 0.5));
                else
                    b = std::min(b, static_cast<int>((r + g) * 0.5));
            }

            line[x] = qRgba(r, g, b, alpha);
        }
    }

    return result;
}

// ══════════════════════════════════════════════════════════════
// 8 Procedural Studio Backgrounds (broadcast-quality QPainter)
// ══════════════════════════════════════════════════════════════

QImage VirtualStudio::renderStudioBackground(const QSize& size)
{
    // Priority 1: Custom background image (imported by user's graphist)
    if (!m_customBg.isNull()) {
        QImage bg = m_customBg.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        return bg.convertToFormat(QImage::Format_RGB32);
    }

    // Priority 2: Procedural studio with custom colors
    QImage bg(size, QImage::Format_RGB32);
    QPainter p(&bg);
    p.setRenderHint(QPainter::Antialiasing, true);
    int w = size.width();
    int h = size.height();
    double scale = w / 1920.0;

    // Resolve colors: user custom overrides template defaults
    // Template defaults per studio (index into arrays below)
    const QColor templatePrimary[] = {
        QColor(10,15,35), QColor(255,200,140), QColor(8,8,15), QColor(45,42,40),
        QColor(30,80,160), QColor(10,15,40), QColor(5,10,20), QColor(15,12,10)
    };
    const QColor templateSecondary[] = {
        QColor(15,22,50), QColor(140,180,230), QColor(200,0,0), QColor(55,52,48),
        QColor(20,100,80), QColor(20,50,180), QColor(0,229,255), QColor(20,18,15)
    };
    const QColor templateAccent[] = {
        QColor(40,80,180), QColor(255,180,100), QColor(220,30,30), QColor(200,180,140),
        QColor(60,160,220), QColor(200,200,220), QColor(0,229,255), QColor(212,175,55)
    };
    const QColor templateFloor[] = {
        QColor(18,25,55), QColor(180,160,140), QColor(15,15,25), QColor(35,32,30),
        QColor(20,60,50), QColor(15,18,45), QColor(8,12,20), QColor(10,8,6)
    };

    int sid = qBound(0, m_studioId, 7);
    QColor cPrimary   = m_primaryColor.isValid()   ? m_primaryColor   : templatePrimary[sid];
    QColor cSecondary = m_secondaryColor.isValid() ? m_secondaryColor : templateSecondary[sid];
    QColor cAccent    = m_accentColor.isValid()    ? m_accentColor    : templateAccent[sid];
    QColor cFloor     = m_floorColor.isValid()     ? m_floorColor     : templateFloor[sid];

    switch (m_studioId) {
    case 0: { // ── News Desk ──────────────────────────────
        QLinearGradient grad(0, 0, 0, h);
        grad.setColorAt(0, cPrimary);
        grad.setColorAt(0.6, cSecondary);
        grad.setColorAt(1.0, cPrimary.darker(120));
        p.fillRect(0, 0, w, h, grad);

        // Floor with subtle reflection
        QLinearGradient floor(0, h * 0.65, 0, h);
        floor.setColorAt(0, cFloor);
        floor.setColorAt(1.0, cFloor.darker(130));
        p.fillRect(0, h * 0.65, w, h * 0.35, floor);

        // Horizontal LED grid lines
        p.setPen(QPen(QColor(cAccent.red(), cAccent.green(), cAccent.blue(), 25), 1));
        for (int i = 0; i < 20; ++i)
            p.drawLine(0, h * 0.05 * i, w, h * 0.05 * i);
        for (int i = 0; i < 30; ++i)
            p.drawLine(w * 0.033 * i, 0, w * 0.033 * i, h * 0.65);

        // Desk surface (warm)
        QLinearGradient desk(0, h * 0.78, 0, h * 0.82);
        desk.setColorAt(0, QColor(60, 45, 30));
        desk.setColorAt(1, QColor(40, 30, 20));
        p.fillRect(w * 0.15, h * 0.78, w * 0.7, h * 0.04, desk);

        // Side accent columns
        p.fillRect(0, 0, w * 0.02, h * 0.65, QColor(cAccent.red(), cAccent.green(), cAccent.blue(), 60));
        p.fillRect(w * 0.98, 0, w * 0.02, h * 0.65, QColor(cAccent.red(), cAccent.green(), cAccent.blue(), 60));
        break;
    }
    case 1: { // ── Morning Show ───────────────────────────
        QLinearGradient grad(0, 0, w, h);
        grad.setColorAt(0, QColor(255, 200, 140));
        grad.setColorAt(0.5, QColor(200, 180, 220));
        grad.setColorAt(1.0, QColor(140, 180, 230));
        p.fillRect(0, 0, w, h, grad);

        // Soft horizontal bands
        for (int i = 0; i < 8; ++i) {
            p.fillRect(0, h * 0.12 * i, w, h * 0.005, QColor(255, 255, 255, 15));
        }

        // Window light effect
        QRadialGradient light(w * 0.7, h * 0.2, w * 0.4);
        light.setColorAt(0, QColor(255, 255, 220, 50));
        light.setColorAt(1, QColor(255, 255, 220, 0));
        p.fillRect(0, 0, w, h, light);

        // Floor
        p.fillRect(0, h * 0.72, w, h * 0.28, QColor(180, 160, 140));
        break;
    }
    case 2: { // ── Sports Center ──────────────────────────
        p.fillRect(0, 0, w, h, QColor(8, 8, 15));

        // Dynamic diagonal stripes
        p.setPen(Qt::NoPen);
        for (int i = -5; i < 15; ++i) {
            QColor c = (i % 2 == 0) ? QColor(200, 0, 0, 20) : QColor(0, 100, 200, 15);
            QPolygonF stripe;
            double x0 = w * 0.1 * i;
            stripe << QPointF(x0, 0) << QPointF(x0 + w * 0.06, 0)
                   << QPointF(x0 - w * 0.1, h) << QPointF(x0 - w * 0.16, h);
            p.setBrush(c);
            p.drawPolygon(stripe);
        }

        // Stadium lights (top)
        for (int i = 0; i < 6; ++i) {
            QRadialGradient spot(w * 0.1 + w * 0.16 * i, 0, h * 0.25);
            spot.setColorAt(0, QColor(255, 255, 255, 20));
            spot.setColorAt(1, QColor(255, 255, 255, 0));
            p.fillRect(0, 0, w, h, spot);
        }

        // Floor
        QLinearGradient floor(0, h * 0.7, 0, h);
        floor.setColorAt(0, QColor(15, 15, 25));
        floor.setColorAt(1, QColor(8, 8, 15));
        p.fillRect(0, h * 0.7, w, h * 0.3, floor);
        break;
    }
    case 3: { // ── Interview ──────────────────────────────
        // Neutral warm gray
        QLinearGradient grad(0, 0, w, 0);
        grad.setColorAt(0, QColor(45, 42, 40));
        grad.setColorAt(0.5, QColor(55, 52, 48));
        grad.setColorAt(1, QColor(45, 42, 40));
        p.fillRect(0, 0, w, h, grad);

        // Vertical panels
        for (int i = 0; i < 5; ++i) {
            double x = w * 0.18 + w * 0.16 * i;
            p.fillRect(x, h * 0.05, w * 0.003, h * 0.6, QColor(80, 75, 70, 50));
        }

        // Warm side lights
        QRadialGradient leftLight(0, h * 0.4, w * 0.3);
        leftLight.setColorAt(0, QColor(255, 200, 140, 30));
        leftLight.setColorAt(1, QColor(0, 0, 0, 0));
        p.fillRect(0, 0, w, h, leftLight);

        QRadialGradient rightLight(w, h * 0.4, w * 0.3);
        rightLight.setColorAt(0, QColor(255, 200, 140, 30));
        rightLight.setColorAt(1, QColor(0, 0, 0, 0));
        p.fillRect(0, 0, w, h, rightLight);

        // Floor
        p.fillRect(0, h * 0.7, w, h * 0.3, QColor(35, 32, 30));
        break;
    }
    case 4: { // ── Weather ────────────────────────────────
        QLinearGradient grad(0, 0, 0, h);
        grad.setColorAt(0, QColor(30, 80, 160));
        grad.setColorAt(0.5, QColor(40, 120, 180));
        grad.setColorAt(1.0, QColor(20, 100, 80));
        p.fillRect(0, 0, w, h, grad);

        // Map grid
        p.setPen(QPen(QColor(255, 255, 255, 15), 1));
        for (int i = 0; i < 15; ++i) {
            p.drawLine(0, h * 0.067 * i, w, h * 0.067 * i);
            p.drawLine(w * 0.067 * i, 0, w * 0.067 * i, h);
        }

        // Globe curvature hint
        p.setPen(QPen(QColor(255, 255, 255, 20), 2 * scale));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(QPointF(w * 0.5, h * 0.5), w * 0.35, h * 0.4);
        break;
    }
    case 5: { // ── Election ───────────────────────────────
        p.fillRect(0, 0, w, h, QColor(10, 15, 40));

        // Red/Blue split
        p.fillRect(0, 0, w / 2, h, QColor(180, 20, 30, 15));
        p.fillRect(w / 2, 0, w / 2, h, QColor(20, 50, 180, 15));

        // Center divider
        p.fillRect(w / 2 - 1, 0, 2, h, QColor(255, 255, 255, 30));

        // Data frames
        p.setPen(QPen(QColor(255, 255, 255, 20), 1));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(QRectF(w * 0.05, h * 0.1, w * 0.35, h * 0.5), 8, 8);
        p.drawRoundedRect(QRectF(w * 0.6, h * 0.1, w * 0.35, h * 0.5), 8, 8);

        // Floor
        QLinearGradient floor(0, h * 0.7, 0, h);
        floor.setColorAt(0, QColor(15, 18, 45));
        floor.setColorAt(1, QColor(8, 10, 30));
        p.fillRect(0, h * 0.7, w, h * 0.3, floor);
        break;
    }
    case 6: { // ── Tech / Digital ─────────────────────────
        p.fillRect(0, 0, w, h, QColor(5, 10, 20));

        // Hexagonal grid
        p.setPen(QPen(QColor(0, 229, 255, 12), 1));
        double hexR = 40 * scale;
        for (double gy = 0; gy < h + hexR; gy += hexR * 1.5) {
            double offsetX = (static_cast<int>(gy / (hexR * 1.5)) % 2) * hexR * 0.866;
            for (double gx = offsetX; gx < w + hexR; gx += hexR * 1.732) {
                QPolygonF hex;
                for (int k = 0; k < 6; ++k) {
                    double angle = M_PI / 3.0 * k + M_PI / 6.0;
                    hex << QPointF(gx + hexR * 0.6 * std::cos(angle), gy + hexR * 0.6 * std::sin(angle));
                }
                p.drawPolygon(hex);
            }
        }

        // Circuit-board traces
        p.setPen(QPen(QColor(0, 229, 255, 20), 2 * scale));
        p.drawLine(0, h * 0.3, w * 0.3, h * 0.3);
        p.drawLine(w * 0.3, h * 0.3, w * 0.35, h * 0.4);
        p.drawLine(w * 0.35, h * 0.4, w * 0.7, h * 0.4);
        p.drawLine(w * 0.7, h * 0.6, w, h * 0.6);
        break;
    }
    case 7: { // ── Luxury / Prestige ──────────────────────
        // Deep black with subtle gradient
        QLinearGradient grad(0, 0, 0, h);
        grad.setColorAt(0, QColor(15, 12, 10));
        grad.setColorAt(0.5, QColor(20, 18, 15));
        grad.setColorAt(1, QColor(10, 8, 6));
        p.fillRect(0, 0, w, h, grad);

        // Gold trim lines
        p.setPen(QPen(QColor(212, 175, 55, 40), 1.5 * scale));
        p.drawLine(w * 0.05, h * 0.08, w * 0.95, h * 0.08);
        p.drawLine(w * 0.05, h * 0.92, w * 0.95, h * 0.92);
        p.drawLine(w * 0.03, h * 0.1, w * 0.03, h * 0.9);
        p.drawLine(w * 0.97, h * 0.1, w * 0.97, h * 0.9);

        // Marble-like subtle texture (noise dots)
        p.setPen(Qt::NoPen);
        for (int i = 0; i < 200; ++i) {
            int nx = (i * 7919 + 1) % w;
            int ny = (i * 6271 + 3) % h;
            int na = 3 + (i % 5);
            p.setBrush(QColor(255, 255, 255, na));
            p.drawEllipse(QPointF(nx, ny), 2 * scale, 2 * scale);
        }

        // Vignette
        QRadialGradient vig(w / 2, h / 2, w * 0.6);
        vig.setColorAt(0, QColor(0, 0, 0, 0));
        vig.setColorAt(1, QColor(0, 0, 0, 100));
        p.fillRect(0, 0, w, h, vig);
        break;
    }
    default: {
        p.fillRect(0, 0, w, h, QColor(20, 20, 30));
        break;
    }
    }

    p.end();
    return bg;
}

// ══════════════════════════════════════════════════════════════
// Animated Studio Elements (per-frame)
// ══════════════════════════════════════════════════════════════

void VirtualStudio::renderAnimatedElements(QPainter& painter, const QSize& size)
{
    int w = size.width();
    int h = size.height();
    double scale = w / 1920.0;
    double phase = std::sin(m_frameCounter * 0.04);       // slow pulse ~0.6Hz
    double phaseFast = std::sin(m_frameCounter * 0.08);   // faster pulse ~1.2Hz

    // Accent color — uses user's custom or template default
    const QColor templateAccents[] = {
        QColor(40,80,180), QColor(255,180,100), QColor(220,30,30), QColor(200,180,140),
        QColor(60,160,220), QColor(200,200,220), QColor(0,229,255), QColor(212,175,55)
    };
    int sid = qBound(0, m_studioId, 7);
    QColor accent = m_accentColor.isValid() ? m_accentColor : templateAccents[sid];

    // Apply light intensity
    int intensityAlpha = static_cast<int>(m_lightIntensity * 255);
    Q_UNUSED(intensityAlpha)

    // 1. Pulsing glow lines (horizontal accent bars)
    int glowAlpha = static_cast<int>(15 + 12 * phase);
    painter.setPen(QPen(QColor(accent.red(), accent.green(), accent.blue(), glowAlpha), 2 * scale));
    painter.drawLine(0, h * 0.65, w, h * 0.65);
    if (m_studioId != 7) { // no top line for luxury
        painter.drawLine(w * 0.1, h * 0.04, w * 0.9, h * 0.04);
    }

    // 2. LED screen gradient sweep (simulated screen in the background)
    if (m_studioId == 0 || m_studioId == 2 || m_studioId == 5 || m_studioId == 6) {
        double sweep = (m_frameCounter % 200) / 200.0; // 0→1 over ~8 seconds at 25fps
        QLinearGradient led(w * sweep - w * 0.15, 0, w * sweep + w * 0.15, 0);
        led.setColorAt(0, QColor(accent.red(), accent.green(), accent.blue(), 0));
        led.setColorAt(0.5, QColor(accent.red(), accent.green(), accent.blue(), 12));
        led.setColorAt(1, QColor(accent.red(), accent.green(), accent.blue(), 0));

        painter.setPen(Qt::NoPen);
        painter.setBrush(led);
        painter.drawRect(w * 0.05, h * 0.08, w * 0.9, h * 0.55);
    }

    // 3. Subtle light particles (floating ambient lights)
    painter.setPen(Qt::NoPen);
    for (int i = 0; i < 12; ++i) {
        // Deterministic position based on studioId + particle index
        int seed = m_studioId * 100 + i * 37;
        double px = ((seed * 7919) % w);
        double py = ((seed * 6271) % static_cast<int>(h * 0.65));
        double particlePhase = std::sin(m_frameCounter * 0.03 + i * 0.8);
        int particleAlpha = static_cast<int>(4 + 6 * (0.5 + 0.5 * particlePhase));
        double particleSize = (2 + (i % 3)) * scale;

        painter.setBrush(QColor(accent.red(), accent.green(), accent.blue(), particleAlpha));
        painter.drawEllipse(QPointF(px, py), particleSize, particleSize);
    }

    // 4. Floor reflection shimmer
    int shimmerAlpha = static_cast<int>(5 + 4 * phaseFast);
    QLinearGradient shimmer(0, h * 0.66, 0, h * 0.75);
    shimmer.setColorAt(0, QColor(accent.red(), accent.green(), accent.blue(), shimmerAlpha));
    shimmer.setColorAt(1, QColor(0, 0, 0, 0));
    painter.fillRect(0, h * 0.66, w, h * 0.1, shimmer);
}

// ══════════════════════════════════════════════════════════════
// Alpha Compositing — Talent on Studio
// ══════════════════════════════════════════════════════════════

QImage VirtualStudio::compositeTalentOnStudio(const QImage& studio, const QImage& keyedTalent)
{
    QImage result = studio.convertToFormat(QImage::Format_ARGB32);
    QPainter painter(&result);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawImage(0, 0, keyedTalent);
    painter.end();
    return result;
}

} // namespace prestige
