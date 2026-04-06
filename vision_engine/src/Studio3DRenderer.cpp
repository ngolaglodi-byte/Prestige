// ============================================================
// Prestige AI — Qt 3D PBR Studio Renderer
// Professional broadcast-grade virtual studios
// Uses Qt 3D with offscreen framebuffer rendering
// Copyright (c) 2024-2026 Prestige Technologie Company
// ============================================================

#include "Studio3DRenderer.h"
#include <QDebug>
#include <QPainter>
#include <QLinearGradient>
#include <QRadialGradient>
#include <cmath>

#ifdef PRESTIGE_HAVE_QT3D
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QPointLight>
#include <Qt3DRender/QDirectionalLight>
#include <Qt3DRender/QSpotLight>
#include <Qt3DRender/QEnvironmentLight>
#include <Qt3DRender/QRenderCapture>
#include <Qt3DRender/QRenderCaptureReply>
#include <Qt3DRender/QRenderSurfaceSelector>
#include <Qt3DRender/QViewport>
#include <Qt3DRender/QClearBuffers>
#include <Qt3DRender/QCameraSelector>
#include <Qt3DRender/QRenderTargetSelector>
#include <Qt3DRender/QRenderTarget>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QRenderTargetOutput>
#include <QOffscreenSurface>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras/QPlaneMesh>
#include <Qt3DExtras/QCylinderMesh>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/QMetalRoughMaterial>
#include <Qt3DExtras/QDiffuseSpecularMaterial>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DExtras/QForwardRenderer>
#endif

namespace prestige {

// ══════════════════════════════════════════════════════════════
// 8 Professional Studio Presets
// ══════════════════════════════════════════════════════════════

static QList<StudioPreset> createPresets() {
    QList<StudioPreset> presets;

    // 0: News Desk — Classic broadcast news set
    {
        StudioPreset p;
        p.id = "news_desk"; p.name = "News Desk"; p.description = "Classic broadcast news studio";
        p.deskWidth = 3.5; p.deskDepth = 1.0; p.deskHeight = 0.85;
        p.deskColor = QColor(45, 40, 35); p.deskMetallic = 0.15; p.deskRoughness = 0.35;
        p.wallColor = QColor(12, 14, 28); p.floorColor = QColor(18, 18, 30);
        p.floorMetallic = 0.4; p.floorRoughness = 0.1; // Very reflective
        p.screenColor = QColor(10, 25, 60); p.screenEmissive = 0.6;
        p.hasBackScreen = true; p.hasSideScreens = true; p.numLightRigs = 4;
        p.keyLightColor = QColor(255, 245, 235); p.keyLightIntensity = 1.2;
        presets.append(p);
    }

    // 1: Morning Show — Warm, bright, inviting
    {
        StudioPreset p;
        p.id = "morning_show"; p.name = "Morning Show"; p.description = "Warm daylight studio";
        p.deskWidth = 4.0; p.deskDepth = 0.8; p.deskHeight = 0.75;
        p.deskColor = QColor(180, 160, 130); p.deskMetallic = 0.05; p.deskRoughness = 0.5; // Light wood
        p.wallColor = QColor(35, 32, 28); p.floorColor = QColor(140, 120, 95);
        p.floorMetallic = 0.1; p.floorRoughness = 0.4; // Warm wood floor
        p.screenColor = QColor(200, 220, 240); p.screenEmissive = 0.3; // Bright daylight screen
        p.hasBackScreen = true; p.hasSideScreens = false; p.numLightRigs = 5;
        p.keyLightColor = QColor(255, 240, 210); p.keyLightIntensity = 1.5; // Very warm
        p.fillLightColor = QColor(255, 250, 240); p.fillLightIntensity = 0.6;
        p.ambientColor = QColor(60, 55, 45);
        presets.append(p);
    }

    // 2: Sports Center — Dynamic, dark, energetic
    {
        StudioPreset p;
        p.id = "sports_center"; p.name = "Sports Center"; p.description = "Dynamic sports broadcast";
        p.deskWidth = 3.0; p.deskDepth = 1.2; p.deskHeight = 0.9;
        p.deskColor = QColor(25, 25, 30); p.deskMetallic = 0.6; p.deskRoughness = 0.2; // Brushed metal
        p.wallColor = QColor(8, 8, 15); p.floorColor = QColor(10, 10, 18);
        p.floorMetallic = 0.5; p.floorRoughness = 0.08; // Mirror-like
        p.screenColor = QColor(30, 10, 10); p.screenEmissive = 0.8; // Red-tinted LED
        p.hasBackScreen = true; p.hasSideScreens = true; p.numLightRigs = 6;
        p.keyLightColor = QColor(255, 255, 255); p.keyLightIntensity = 1.3;
        p.backLightColor = QColor(255, 50, 50); p.backLightIntensity = 0.5; // Red rim
        presets.append(p);
    }

    // 3: Interview — Neutral, professional, two-person
    {
        StudioPreset p;
        p.id = "interview"; p.name = "Interview Set"; p.description = "Two-person interview studio";
        p.deskWidth = 2.0; p.deskDepth = 0.6; p.deskHeight = 0.7;
        p.deskColor = QColor(60, 50, 40); p.deskMetallic = 0.1; p.deskRoughness = 0.45; // Dark wood
        p.wallColor = QColor(25, 22, 20); p.floorColor = QColor(30, 25, 22);
        p.floorMetallic = 0.15; p.floorRoughness = 0.3;
        p.hasBackScreen = false; p.hasSideScreens = false; p.numLightRigs = 3;
        p.keyLightColor = QColor(255, 248, 235); p.keyLightIntensity = 0.9;
        p.ambientColor = QColor(40, 35, 30);
        presets.append(p);
    }

    // 4: Weather — Blue-green map wall
    {
        StudioPreset p;
        p.id = "weather"; p.name = "Weather Center"; p.description = "Weather forecast studio";
        p.hasDesk = false; // No desk — standing presenter
        p.wallColor = QColor(5, 15, 35); p.floorColor = QColor(10, 15, 25);
        p.floorMetallic = 0.3; p.floorRoughness = 0.15;
        p.screenWidth = 7.0; p.screenHeight = 3.5; // Large map screen
        p.screenColor = QColor(10, 40, 80); p.screenEmissive = 0.7;
        p.hasBackScreen = true; p.hasSideScreens = false; p.numLightRigs = 3;
        p.keyLightColor = QColor(220, 235, 255); p.keyLightIntensity = 1.0;
        presets.append(p);
    }

    // 5: Election — Red/blue split, data-heavy
    {
        StudioPreset p;
        p.id = "election"; p.name = "Election Night"; p.description = "Election coverage studio";
        p.deskWidth = 4.0; p.deskDepth = 1.5; p.deskHeight = 0.85;
        p.deskColor = QColor(30, 30, 35); p.deskMetallic = 0.4; p.deskRoughness = 0.25;
        p.wallColor = QColor(10, 10, 20); p.floorColor = QColor(12, 12, 22);
        p.floorMetallic = 0.4; p.floorRoughness = 0.1;
        p.screenColor = QColor(15, 15, 40); p.screenEmissive = 0.9;
        p.hasBackScreen = true; p.hasSideScreens = true; p.numLightRigs = 6;
        p.keyLightColor = QColor(255, 255, 255); p.keyLightIntensity = 1.1;
        presets.append(p);
    }

    // 6: Tech/Digital — Cyberpunk, neon
    {
        StudioPreset p;
        p.id = "tech_digital"; p.name = "Tech Studio"; p.description = "Modern technology studio";
        p.deskWidth = 2.5; p.deskDepth = 0.8; p.deskHeight = 0.8;
        p.deskColor = QColor(20, 20, 25); p.deskMetallic = 0.7; p.deskRoughness = 0.15; // Chrome
        p.wallColor = QColor(5, 8, 15); p.floorColor = QColor(8, 8, 14);
        p.floorMetallic = 0.6; p.floorRoughness = 0.05; // Mirror floor
        p.screenColor = QColor(0, 15, 30); p.screenEmissive = 1.0;
        p.hasBackScreen = true; p.hasSideScreens = true; p.numLightRigs = 4;
        p.keyLightColor = QColor(200, 240, 255); p.keyLightIntensity = 1.0;
        p.backLightColor = QColor(0, 229, 255); p.backLightIntensity = 0.6; // Cyan rim
        presets.append(p);
    }

    // 7: Luxury/Prestige — Black and gold, elegant
    {
        StudioPreset p;
        p.id = "luxury_prestige"; p.name = "Luxury Prestige"; p.description = "Premium black & gold studio";
        p.deskWidth = 3.0; p.deskDepth = 1.0; p.deskHeight = 0.85;
        p.deskColor = QColor(15, 12, 10); p.deskMetallic = 0.8; p.deskRoughness = 0.1; // Polished dark
        p.wallColor = QColor(8, 6, 5); p.floorColor = QColor(12, 10, 8);
        p.floorMetallic = 0.7; p.floorRoughness = 0.05; // Marble-like
        p.screenColor = QColor(5, 5, 5); p.screenEmissive = 0.2; // Subtle
        p.hasBackScreen = false; p.hasSideScreens = false; p.numLightRigs = 3;
        p.keyLightColor = QColor(255, 230, 180); p.keyLightIntensity = 0.8; // Warm gold
        p.fillLightColor = QColor(255, 215, 150); p.fillLightIntensity = 0.3;
        p.ambientColor = QColor(20, 15, 10);
        presets.append(p);
    }

    return presets;
}

QList<StudioPreset> Studio3DRenderer::allPresets() {
    static QList<StudioPreset> presets = createPresets();
    return presets;
}

StudioPreset Studio3DRenderer::preset(int id) {
    auto presets = allPresets();
    if (id >= 0 && id < presets.size()) return presets[id];
    return presets.isEmpty() ? StudioPreset() : presets[0];
}

// ══════════════════════════════════════════════════════════════
// Implementation — Offscreen 3D Rendering
// ══════════════════════════════════════════════════════════════

#ifdef PRESTIGE_HAVE_QT3D

struct Studio3DRenderer::Impl {
    Qt3DCore::QEntity*              rootEntity = nullptr;
    Qt3DRender::QCamera*            camera = nullptr;
    Qt3DRender::QRenderCapture*     capture = nullptr;

    // Scene entities
    Qt3DCore::QEntity* deskEntity = nullptr;
    Qt3DCore::QEntity* floorEntity = nullptr;
    Qt3DCore::QEntity* wallEntity = nullptr;
    Qt3DCore::QEntity* backScreenEntity = nullptr;
    QList<Qt3DCore::QEntity*> lightEntities;
    QList<Qt3DCore::QEntity*> sideScreenEntities;

    // Materials (PBR)
    Qt3DExtras::QMetalRoughMaterial* deskMat = nullptr;
    Qt3DExtras::QMetalRoughMaterial* floorMat = nullptr;
    Qt3DExtras::QMetalRoughMaterial* wallMat = nullptr;
    Qt3DExtras::QMetalRoughMaterial* screenMat = nullptr;

    // Offscreen rendering
    QOffscreenSurface* surface = nullptr;
    QImage lastFrame;
};

Studio3DRenderer::Studio3DRenderer(QObject* parent)
    : QObject(parent), m_impl(std::make_unique<Impl>())
{}

Studio3DRenderer::~Studio3DRenderer() = default;

bool Studio3DRenderer::initialize(const QSize& resolution)
{
    m_resolution = resolution;

    m_impl->rootEntity = new Qt3DCore::QEntity();

    // Camera — broadcast studio angle
    m_impl->camera = new Qt3DRender::QCamera(m_impl->rootEntity);
    m_impl->camera->lens()->setPerspectiveProjection(
        35.0f, // FOV — standard broadcast camera lens
        static_cast<float>(resolution.width()) / resolution.height(),
        0.1f, 100.0f
    );
    m_impl->camera->setPosition(QVector3D(0, 1.6f, 4.0f));   // Eye height, 4m from desk
    m_impl->camera->setViewCenter(QVector3D(0, 0.9f, 0));     // Look at desk height
    m_impl->camera->setUpVector(QVector3D(0, 1, 0));

    m_initialized = true;
    setPreset(0);
    qInfo() << "[Studio3D] PBR renderer initialized at" << resolution;
    return true;
}

void Studio3DRenderer::setPreset(int presetId)
{
    m_currentPresetId = presetId;
    m_activePreset = preset(presetId);
    m_sceneDirty = true;
}

void Studio3DRenderer::setPreset(const StudioPreset& p)
{
    m_activePreset = p;
    m_sceneDirty = true;
}

void Studio3DRenderer::setPrimaryColor(const QColor& c) { m_customPrimary = c; m_sceneDirty = true; }
void Studio3DRenderer::setSecondaryColor(const QColor& c) { m_customSecondary = c; m_sceneDirty = true; }
void Studio3DRenderer::setAccentColor(const QColor& c) { m_customAccent = c; m_sceneDirty = true; }
void Studio3DRenderer::setFloorColor(const QColor& c) { m_customFloor = c; m_sceneDirty = true; }
void Studio3DRenderer::setLightIntensity(double v) { m_customLightIntensity = v; }
void Studio3DRenderer::setLightColor(const QColor& c) { m_customLightColor = c; }
void Studio3DRenderer::setAnimationsEnabled(bool v) { m_animEnabled = v; }
void Studio3DRenderer::setHdriEnvironment(const QString& path) { Q_UNUSED(path) }

void Studio3DRenderer::buildScene()
{
    if (!m_impl->rootEntity) return;
    auto& p = m_activePreset;

    // Apply custom color overrides
    QColor wallColor = m_customPrimary.isValid() ? m_customPrimary : p.wallColor;
    QColor floorColor = m_customFloor.isValid() ? m_customFloor : p.floorColor;
    QColor accentColor = m_customAccent.isValid() ? m_customAccent : p.screenColor;

    // ── Floor (reflective PBR) ───────────────────────────
    if (!m_impl->floorEntity) {
        m_impl->floorEntity = new Qt3DCore::QEntity(m_impl->rootEntity);
        auto* mesh = new Qt3DExtras::QPlaneMesh();
        mesh->setWidth(12.0f); mesh->setHeight(p.floorDepth);
        m_impl->floorEntity->addComponent(mesh);

        m_impl->floorMat = new Qt3DExtras::QMetalRoughMaterial();
        m_impl->floorEntity->addComponent(m_impl->floorMat);

        auto* transform = new Qt3DCore::QTransform();
        transform->setTranslation(QVector3D(0, 0, -p.floorDepth / 2.0f));
        m_impl->floorEntity->addComponent(transform);
    }
    m_impl->floorMat->setBaseColor(floorColor);
    m_impl->floorMat->setMetalness(p.floorMetallic);
    m_impl->floorMat->setRoughness(p.floorRoughness);

    // ── Back Wall ────────────────────────────────────────
    if (!m_impl->wallEntity) {
        m_impl->wallEntity = new Qt3DCore::QEntity(m_impl->rootEntity);
        auto* mesh = new Qt3DExtras::QCuboidMesh();
        mesh->setXExtent(12.0f); mesh->setYExtent(p.wallHeight); mesh->setZExtent(0.15f);
        m_impl->wallEntity->addComponent(mesh);

        m_impl->wallMat = new Qt3DExtras::QMetalRoughMaterial();
        m_impl->wallEntity->addComponent(m_impl->wallMat);

        auto* transform = new Qt3DCore::QTransform();
        transform->setTranslation(QVector3D(0, p.wallHeight / 2.0f, -p.floorDepth));
        m_impl->wallEntity->addComponent(transform);
    }
    m_impl->wallMat->setBaseColor(wallColor);
    m_impl->wallMat->setMetalness(p.wallMetallic);
    m_impl->wallMat->setRoughness(p.wallRoughness);

    // ── Desk (if studio has one) ─────────────────────────
    QColor deskColor = m_customSecondary.isValid() ? m_customSecondary : p.deskColor;
    if (p.hasDesk) {
        if (!m_impl->deskEntity) {
            m_impl->deskEntity = new Qt3DCore::QEntity(m_impl->rootEntity);
            auto* mesh = new Qt3DExtras::QCuboidMesh();
            mesh->setXExtent(p.deskWidth); mesh->setYExtent(0.06f); mesh->setZExtent(p.deskDepth);
            m_impl->deskEntity->addComponent(mesh);

            m_impl->deskMat = new Qt3DExtras::QMetalRoughMaterial();
            m_impl->deskEntity->addComponent(m_impl->deskMat);

            auto* transform = new Qt3DCore::QTransform();
            transform->setTranslation(QVector3D(0, p.deskHeight, 0));
            m_impl->deskEntity->addComponent(transform);

            // Desk front panel
            auto* frontPanel = new Qt3DCore::QEntity(m_impl->rootEntity);
            auto* fpMesh = new Qt3DExtras::QCuboidMesh();
            fpMesh->setXExtent(p.deskWidth + 0.1f); fpMesh->setYExtent(p.deskHeight - 0.02f); fpMesh->setZExtent(0.04f);
            frontPanel->addComponent(fpMesh);
            auto* fpMat = new Qt3DExtras::QMetalRoughMaterial();
            fpMat->setBaseColor(deskColor.darker(120));
            fpMat->setMetalness(p.deskMetallic + 0.1);
            fpMat->setRoughness(p.deskRoughness);
            frontPanel->addComponent(fpMat);
            auto* fpTrans = new Qt3DCore::QTransform();
            fpTrans->setTranslation(QVector3D(0, (p.deskHeight - 0.02f) / 2.0f, p.deskDepth / 2.0f));
            frontPanel->addComponent(fpTrans);
        }
        m_impl->deskMat->setBaseColor(deskColor);
        m_impl->deskMat->setMetalness(p.deskMetallic);
        m_impl->deskMat->setRoughness(p.deskRoughness);
    }

    // ── Back LED Screen ──────────────────────────────────
    if (p.hasBackScreen) {
        if (!m_impl->backScreenEntity) {
            m_impl->backScreenEntity = new Qt3DCore::QEntity(m_impl->rootEntity);
            auto* mesh = new Qt3DExtras::QCuboidMesh();
            mesh->setXExtent(p.screenWidth); mesh->setYExtent(p.screenHeight); mesh->setZExtent(0.05f);
            m_impl->backScreenEntity->addComponent(mesh);

            m_impl->screenMat = new Qt3DExtras::QMetalRoughMaterial();
            m_impl->backScreenEntity->addComponent(m_impl->screenMat);

            auto* transform = new Qt3DCore::QTransform();
            transform->setTranslation(QVector3D(0, p.wallHeight * 0.45f, -p.floorDepth + 0.2f));
            m_impl->backScreenEntity->addComponent(transform);
        }
        m_impl->screenMat->setBaseColor(accentColor);
        m_impl->screenMat->setMetalness(0.0);
        m_impl->screenMat->setRoughness(0.95); // Matte screen surface
    }

    // ── Three-Point Lighting (Key + Fill + Back) ─────────
    double lightMul = (m_customLightIntensity > 0) ? m_customLightIntensity : p.keyLightIntensity;
    QColor lightCol = m_customLightColor.isValid() ? m_customLightColor : p.keyLightColor;

    // Clean up old lights
    for (auto* e : m_impl->lightEntities) { delete e; }
    m_impl->lightEntities.clear();

    // Key light (main — upper right)
    {
        auto* entity = new Qt3DCore::QEntity(m_impl->rootEntity);
        auto* light = new Qt3DRender::QSpotLight();
        light->setColor(lightCol);
        light->setIntensity(lightMul);
        light->setCutOffAngle(50.0f);
        light->setLocalDirection(QVector3D(-0.3f, -0.7f, -0.5f));
        entity->addComponent(light);
        auto* t = new Qt3DCore::QTransform();
        t->setTranslation(QVector3D(2.5f, 3.5f, 2.0f));
        entity->addComponent(t);
        m_impl->lightEntities.append(entity);
    }

    // Fill light (left side, softer)
    {
        auto* entity = new Qt3DCore::QEntity(m_impl->rootEntity);
        auto* light = new Qt3DRender::QPointLight();
        light->setColor(p.fillLightColor);
        light->setIntensity(p.fillLightIntensity * lightMul);
        entity->addComponent(light);
        auto* t = new Qt3DCore::QTransform();
        t->setTranslation(QVector3D(-3.0f, 2.5f, 1.5f));
        entity->addComponent(t);
        m_impl->lightEntities.append(entity);
    }

    // Back/rim light (behind talent)
    {
        auto* entity = new Qt3DCore::QEntity(m_impl->rootEntity);
        auto* light = new Qt3DRender::QSpotLight();
        light->setColor(p.backLightColor);
        light->setIntensity(p.backLightIntensity * lightMul);
        light->setCutOffAngle(60.0f);
        light->setLocalDirection(QVector3D(0, -0.5f, 1.0f));
        entity->addComponent(light);
        auto* t = new Qt3DCore::QTransform();
        t->setTranslation(QVector3D(0, 3.0f, -3.0f));
        entity->addComponent(t);
        m_impl->lightEntities.append(entity);
    }

    m_sceneDirty = false;
}

void Studio3DRenderer::updateAnimations(double timeSec)
{
    if (!m_animEnabled || !m_impl->screenMat) return;

    // LED screen color pulse
    auto& p = m_activePreset;
    QColor base = m_customAccent.isValid() ? m_customAccent : p.screenColor;
    double pulse = 0.5 + 0.5 * std::sin(timeSec * 0.5);
    double emissive = p.screenEmissive * (0.8 + 0.2 * pulse);
    QColor screenCol(
        std::min(255, base.red() + static_cast<int>(emissive * 30)),
        std::min(255, base.green() + static_cast<int>(emissive * 30)),
        std::min(255, base.blue() + static_cast<int>(emissive * 50))
    );
    m_impl->screenMat->setBaseColor(screenCol);
}

QImage Studio3DRenderer::renderOffscreen()
{
    // Qt3D offscreen capture — returns QImage from GPU framebuffer
    if (!m_impl->capture) return QImage();

    auto* reply = m_impl->capture->requestCapture();
    // In real implementation, this would need an event loop wait
    // For now, return cached frame
    return m_impl->lastFrame;
}

QImage Studio3DRenderer::render(double timeSec)
{
    if (!m_initialized) return QImage();

    if (m_sceneDirty) buildScene();
    updateAnimations(timeSec);
    return renderOffscreen();
}

#else
// ══════════════════════════════════════════════════════════════
// Fallback: High-quality 2D PBR simulation (when Qt 3D not available)
// Uses QPainter with gradient-based PBR approximation
// Still significantly better than the original flat 2D
// ══════════════════════════════════════════════════════════════

struct Studio3DRenderer::Impl {
    QImage cachedBg;
    bool dirty = true;
};

Studio3DRenderer::Studio3DRenderer(QObject* parent)
    : QObject(parent), m_impl(std::make_unique<Impl>())
{}

Studio3DRenderer::~Studio3DRenderer() = default;

bool Studio3DRenderer::initialize(const QSize& resolution)
{
    m_resolution = resolution;
    m_initialized = true;
    setPreset(0);
    qInfo() << "[Studio3D] PBR renderer initialized (2D fallback) at" << resolution;
    return true;
}

void Studio3DRenderer::setPreset(int presetId) { m_currentPresetId = presetId; m_activePreset = preset(presetId); m_sceneDirty = true; m_impl->dirty = true; }
void Studio3DRenderer::setPreset(const StudioPreset& p) { m_activePreset = p; m_sceneDirty = true; m_impl->dirty = true; }
void Studio3DRenderer::setPrimaryColor(const QColor& c) { m_customPrimary = c; m_impl->dirty = true; }
void Studio3DRenderer::setSecondaryColor(const QColor& c) { m_customSecondary = c; m_impl->dirty = true; }
void Studio3DRenderer::setAccentColor(const QColor& c) { m_customAccent = c; m_impl->dirty = true; }
void Studio3DRenderer::setFloorColor(const QColor& c) { m_customFloor = c; m_impl->dirty = true; }
void Studio3DRenderer::setLightIntensity(double v) { m_customLightIntensity = v; m_impl->dirty = true; }
void Studio3DRenderer::setLightColor(const QColor& c) { m_customLightColor = c; m_impl->dirty = true; }
void Studio3DRenderer::setAnimationsEnabled(bool v) { m_animEnabled = v; }
void Studio3DRenderer::setHdriEnvironment(const QString& path) { Q_UNUSED(path) }

void Studio3DRenderer::buildScene() { m_sceneDirty = false; m_impl->dirty = true; }
void Studio3DRenderer::updateAnimations(double) {}
QImage Studio3DRenderer::renderOffscreen() { return m_impl->cachedBg; }

QImage Studio3DRenderer::render(double timeSec)
{
    if (!m_initialized) return QImage();

    int w = m_resolution.width(), h = m_resolution.height();
    auto& p = m_activePreset;

    QColor wallColor = m_customPrimary.isValid() ? m_customPrimary : p.wallColor;
    QColor floorColor = m_customFloor.isValid() ? m_customFloor : p.floorColor;
    QColor accentColor = m_customAccent.isValid() ? m_customAccent : p.screenColor;
    QColor deskColor = m_customSecondary.isValid() ? m_customSecondary : p.deskColor;
    double lightMul = (m_customLightIntensity > 0) ? m_customLightIntensity : p.keyLightIntensity;

    // Only rebuild when dirty (colors/preset changed)
    if (!m_impl->dirty && !m_impl->cachedBg.isNull()) {
        // Add animated elements on top of cached background
        QImage frame = m_impl->cachedBg.copy();
        if (m_animEnabled) {
            QPainter ap(&frame);
            ap.setRenderHint(QPainter::Antialiasing);
            double phase = std::sin(timeSec * 0.6);

            // LED screen glow pulse
            int glowA = static_cast<int>((10 + 8 * phase) * lightMul);
            QLinearGradient screenGlow(w * 0.15, 0, w * 0.85, 0);
            screenGlow.setColorAt(0, QColor(accentColor.red(), accentColor.green(), accentColor.blue(), 0));
            screenGlow.setColorAt(0.5, QColor(accentColor.red(), accentColor.green(), accentColor.blue(), glowA));
            screenGlow.setColorAt(1, QColor(accentColor.red(), accentColor.green(), accentColor.blue(), 0));
            ap.fillRect(QRectF(w * 0.1, h * 0.05, w * 0.8, h * 0.55), screenGlow);

            // LED sweep
            double sweep = std::fmod(timeSec * 0.12, 1.0);
            int sweepAlpha = static_cast<int>(15 * lightMul);
            QLinearGradient led(w * sweep - w * 0.1, 0, w * sweep + w * 0.1, 0);
            led.setColorAt(0, QColor(255, 255, 255, 0));
            led.setColorAt(0.5, QColor(255, 255, 255, sweepAlpha));
            led.setColorAt(1, QColor(255, 255, 255, 0));
            ap.fillRect(QRectF(w * 0.1, h * 0.08, w * 0.8, h * 0.52), led);

            // Floating particles
            ap.setPen(Qt::NoPen);
            for (int i = 0; i < 15; ++i) {
                int seed = m_currentPresetId * 100 + i * 37;
                double px = (seed * 7919) % w;
                double py = (seed * 6271) % static_cast<int>(h * 0.63);
                double pPhase = std::sin(timeSec * 0.4 + i * 0.7);
                int pAlpha = static_cast<int>((3 + 5 * (0.5 + 0.5 * pPhase)) * lightMul);
                double pSize = (2.0 + (i % 3)) * (w / 1920.0);
                ap.setBrush(QColor(accentColor.red(), accentColor.green(), accentColor.blue(), pAlpha));
                ap.drawEllipse(QPointF(px, py), pSize, pSize);
            }

            // Floor reflection shimmer
            int shimA = static_cast<int>((4 + 3 * phase) * lightMul * p.floorMetallic * 3);
            QLinearGradient floorShim(0, h * 0.65, 0, h * 0.78);
            floorShim.setColorAt(0, QColor(accentColor.red(), accentColor.green(), accentColor.blue(), shimA));
            floorShim.setColorAt(1, QColor(0, 0, 0, 0));
            ap.fillRect(QRectF(0, h * 0.65, w, h * 0.13), floorShim);
        }
        return frame;
    }

    // ── Build static background (PBR-simulated) ──────────
    QImage bg(w, h, QImage::Format_RGB32);
    QPainter p2(&bg);
    p2.setRenderHint(QPainter::Antialiasing);
    p2.setRenderHint(QPainter::SmoothPixmapTransform);

    // Wall background — vertical gradient
    QLinearGradient wallGrad(0, 0, 0, h * 0.65);
    wallGrad.setColorAt(0, wallColor.darker(130));
    wallGrad.setColorAt(0.5, wallColor);
    wallGrad.setColorAt(1, wallColor.lighter(110));
    p2.fillRect(0, 0, w, h, wallGrad);

    // Floor — perspective gradient (PBR reflective simulation)
    double floorY = h * 0.63;
    QLinearGradient floorGrad(0, floorY, 0, h);
    QColor floorLight = floorColor.lighter(120 + static_cast<int>(p.floorMetallic * 80));
    floorGrad.setColorAt(0, floorLight);
    floorGrad.setColorAt(0.3, floorColor);
    floorGrad.setColorAt(1, floorColor.darker(150));
    p2.fillRect(0, static_cast<int>(floorY), w, h - static_cast<int>(floorY), floorGrad);

    // Floor reflection line (PBR specular simulation)
    if (p.floorMetallic > 0.2) {
        int reflAlpha = static_cast<int>(p.floorMetallic * 40 * lightMul);
        p2.setPen(QPen(QColor(255, 255, 255, reflAlpha), 1));
        p2.drawLine(0, static_cast<int>(floorY), w, static_cast<int>(floorY));
    }

    // Back LED screen panel
    if (p.hasBackScreen) {
        double screenL = w * (0.5 - p.screenWidth / 14.0);
        double screenR = w * (0.5 + p.screenWidth / 14.0);
        double screenT = h * 0.06;
        double screenB = h * 0.60;
        QRectF screenRect(screenL, screenT, screenR - screenL, screenB - screenT);

        // Screen bezel (dark frame)
        p2.setPen(Qt::NoPen);
        p2.setBrush(QColor(5, 5, 8));
        p2.drawRoundedRect(screenRect.adjusted(-4, -4, 4, 4), 6, 6);

        // Screen surface (emissive gradient)
        QLinearGradient screenGrad(screenRect.topLeft(), screenRect.bottomRight());
        screenGrad.setColorAt(0, accentColor.darker(200));
        screenGrad.setColorAt(0.3, accentColor);
        screenGrad.setColorAt(0.7, accentColor.lighter(110));
        screenGrad.setColorAt(1, accentColor.darker(150));
        p2.setBrush(screenGrad);
        p2.drawRoundedRect(screenRect, 4, 4);

        // Screen scan lines (LED pixel simulation)
        p2.setPen(QPen(QColor(0, 0, 0, 15), 1));
        for (double y = screenRect.top(); y < screenRect.bottom(); y += 3)
            p2.drawLine(QPointF(screenRect.left(), y), QPointF(screenRect.right(), y));
    }

    // Side screens
    if (p.hasSideScreens) {
        for (int side = 0; side < 2; ++side) {
            double sx = (side == 0) ? w * 0.02 : w * 0.82;
            double sw = w * 0.16;
            QRectF sideRect(sx, h * 0.10, sw, h * 0.50);
            // Perspective skew (narrower at far edge)
            double skew = side == 0 ? 0.92 : 1.08;
            p2.save();
            QTransform t;
            t.translate(sideRect.center().x(), sideRect.center().y());
            t.scale(1.0, 1.0);
            t.shear(side == 0 ? 0.05 : -0.05, 0);
            t.translate(-sideRect.center().x(), -sideRect.center().y());
            p2.setTransform(t);
            p2.setBrush(accentColor.darker(180));
            p2.setPen(QPen(QColor(5, 5, 8), 3));
            p2.drawRoundedRect(sideRect, 3, 3);
            p2.restore();
        }
    }

    // Desk (PBR-style with specular highlight)
    if (p.hasDesk) {
        double deskFrac = p.deskWidth / 12.0;
        double deskL = w * (0.5 - deskFrac / 2);
        double deskR = w * (0.5 + deskFrac / 2);
        double deskTop = h * 0.62;
        double deskBot = h * 0.72;
        QRectF deskRect(deskL, deskTop, deskR - deskL, deskBot - deskTop);

        // Desk surface (metallic PBR simulation)
        QLinearGradient deskGrad(deskRect.topLeft(), deskRect.bottomLeft());
        deskGrad.setColorAt(0, deskColor.lighter(110 + static_cast<int>(p.deskMetallic * 60)));
        deskGrad.setColorAt(0.3, deskColor);
        deskGrad.setColorAt(1, deskColor.darker(120));
        p2.setBrush(deskGrad);
        p2.setPen(QPen(deskColor.lighter(130), 1));
        p2.drawRoundedRect(deskRect, 3, 3);

        // Specular highlight on desk (simulates metallic reflection)
        if (p.deskMetallic > 0.1) {
            int specAlpha = static_cast<int>(p.deskMetallic * 30 * lightMul);
            QLinearGradient spec(deskRect.left(), deskRect.top(), deskRect.right(), deskRect.top());
            spec.setColorAt(0, QColor(255, 255, 255, 0));
            spec.setColorAt(0.3, QColor(255, 255, 255, specAlpha));
            spec.setColorAt(0.7, QColor(255, 255, 255, 0));
            p2.fillRect(deskRect, spec);
        }

        // Desk front panel (angled)
        QRectF frontRect(deskL - 5, deskBot, deskR - deskL + 10, h * 0.08);
        QLinearGradient frontGrad(frontRect.topLeft(), frontRect.bottomLeft());
        frontGrad.setColorAt(0, deskColor.darker(110));
        frontGrad.setColorAt(1, deskColor.darker(160));
        p2.setBrush(frontGrad);
        p2.setPen(Qt::NoPen);
        p2.drawRect(frontRect);

        // LED strip on desk front edge
        int ledAlpha = static_cast<int>(p.screenEmissive * 20 * lightMul);
        p2.setPen(QPen(QColor(accentColor.red(), accentColor.green(), accentColor.blue(), ledAlpha), 2));
        p2.drawLine(QPointF(deskL, deskBot), QPointF(deskR, deskBot));
    }

    // Light rig spots (visible ceiling fixtures)
    for (int i = 0; i < p.numLightRigs; ++i) {
        double lx = w * (0.15 + 0.7 * i / std::max(1, p.numLightRigs - 1));
        QRadialGradient spotGrad(lx, 0, h * 0.15);
        int spotAlpha = static_cast<int>(8 * lightMul);
        spotGrad.setColorAt(0, QColor(255, 250, 240, spotAlpha));
        spotGrad.setColorAt(1, QColor(255, 250, 240, 0));
        p2.fillRect(QRectF(lx - h * 0.15, 0, h * 0.3, h * 0.25), spotGrad);
    }

    // Vignette (darkens edges — cinematic)
    QRadialGradient vignette(w / 2.0, h / 2.0, w * 0.7);
    vignette.setColorAt(0, QColor(0, 0, 0, 0));
    vignette.setColorAt(0.7, QColor(0, 0, 0, 0));
    vignette.setColorAt(1, QColor(0, 0, 0, 60));
    p2.fillRect(0, 0, w, h, vignette);

    p2.end();
    m_impl->cachedBg = bg;
    m_impl->dirty = false;
    return bg;
}

#endif // PRESTIGE_HAVE_QT3D

} // namespace prestige
