#pragma once

// ============================================================
// Prestige AI — Qt 3D PBR Studio Renderer
// Broadcast-grade 3D virtual studio with:
//   - PBR Materials (metallic-roughness workflow)
//   - HDR Environment Lighting (IBL)
//   - Shadow Mapping
//   - Screen-Space Reflections on floor
//   - Animated elements (LED screens, light rigs)
// Renders offscreen to QImage for compositing pipeline.
// Copyright (c) 2024-2026 Prestige Technologie Company
// ============================================================

#include <QObject>
#include <QImage>
#include <QSize>
#include <QColor>
#include <QUrl>
#include <memory>

namespace prestige {

// ── Studio Preset Definition ─────────────────────────────────
struct StudioPreset {
    QString id;
    QString name;
    QString description;

    // Geometry
    double deskWidth    = 3.0;    // meters
    double deskDepth    = 1.2;
    double deskHeight   = 0.9;
    double wallHeight   = 4.0;
    double floorDepth   = 8.0;
    double screenWidth  = 5.0;
    double screenHeight = 2.8;
    bool   hasDesk      = true;
    bool   hasBackScreen = true;
    bool   hasSideScreens = false;
    int    numLightRigs = 3;

    // PBR Materials
    QColor deskColor    = QColor(40, 35, 30);
    double deskMetallic = 0.1;
    double deskRoughness = 0.4;

    QColor wallColor    = QColor(15, 15, 25);
    double wallMetallic = 0.0;
    double wallRoughness = 0.8;

    QColor floorColor   = QColor(20, 20, 30);
    double floorMetallic = 0.3;
    double floorRoughness = 0.15; // Reflective floor

    QColor screenColor  = QColor(10, 20, 40);
    double screenEmissive = 0.5;  // LED screen glow

    // Lighting
    QColor keyLightColor    = QColor(255, 248, 240);  // Warm key
    double keyLightIntensity = 1.0;
    QColor fillLightColor   = QColor(200, 220, 255);  // Cool fill
    double fillLightIntensity = 0.4;
    QColor backLightColor   = QColor(180, 200, 255);  // Rim light
    double backLightIntensity = 0.3;
    QColor ambientColor     = QColor(30, 30, 50);

    // HDR Environment
    QString hdriPath;  // Empty = use procedural sky
};

// ── 3D Renderer ──────────────────────────────────────────────
class Studio3DRenderer : public QObject {
    Q_OBJECT

public:
    explicit Studio3DRenderer(QObject* parent = nullptr);
    ~Studio3DRenderer() override;

    // Initialize the 3D render pipeline (offscreen)
    bool initialize(const QSize& resolution);
    bool isInitialized() const { return m_initialized; }

    // Set which studio preset to render
    void setPreset(int presetId);
    void setPreset(const StudioPreset& preset);
    int currentPresetId() const { return m_currentPresetId; }

    // Customization (override preset colors)
    void setPrimaryColor(const QColor& c);
    void setSecondaryColor(const QColor& c);
    void setAccentColor(const QColor& c);
    void setFloorColor(const QColor& c);
    void setLightIntensity(double intensity);
    void setLightColor(const QColor& c);
    void setAnimationsEnabled(bool enabled);

    // Load custom HDRI environment map
    void setHdriEnvironment(const QString& path);

    // Render one frame (call per composite frame)
    QImage render(double timeSec);

    // Get all available presets
    static QList<StudioPreset> allPresets();
    static StudioPreset preset(int id);

private:
    void buildScene();
    void updateAnimations(double timeSec);
    QImage renderOffscreen();

    struct Impl;
    std::unique_ptr<Impl> m_impl;
    bool m_initialized = false;
    int  m_currentPresetId = 0;
    StudioPreset m_activePreset;
    QSize m_resolution = {1920, 1080};

    // Customization overrides
    QColor m_customPrimary;
    QColor m_customSecondary;
    QColor m_customAccent;
    QColor m_customFloor;
    double m_customLightIntensity = -1; // -1 = use preset
    QColor m_customLightColor;
    bool   m_animEnabled = true;
    bool   m_sceneDirty = true;
};

} // namespace prestige
