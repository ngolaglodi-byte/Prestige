#pragma once

// ============================================================
// Prestige AI — Lottie Animation Engine
// Renders Bodymovin/Lottie JSON animations with dynamic text
// replacement. Used for professional title overlays.
//
// The réalisateur selects a title style → types name/role →
// the animation plays with their text, identical to AE original.
//
// Copyright (c) 2024-2026 Prestige Technologie Company
// ============================================================

#include <QObject>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QRectF>
#include <QColor>
#include <QFont>
#include <QPainterPath>
#include <memory>
#include <vector>

namespace prestige {

// ── Lottie Text Slot ─────────────────────────────────────────
// Describes a replaceable text field in a Lottie animation
struct LottieTextSlot {
    QString id;          // e.g. "Text 01"
    QString defaultText; // Original text from AE
    QString currentText; // Replaced text (from réalisateur)
    QString fontFamily;
    double  fontSize = 48;
    QColor  fontColor = Qt::white;
    int     precompIndex = -1;  // Which precomp asset contains this text
    int     layerIndex = -1;    // Layer index within the precomp
};

// ── Lottie Animation Preset ──────────────────────────────────
struct LottiePreset {
    QString id;            // "title_01"
    QString name;          // "Simple Tag"
    QString jsonPath;      // Path to the .json file
    QJsonObject json;      // Parsed JSON (cached)
    int width = 1920;
    int height = 1080;
    double fps = 29.97;
    double durationFrames = 300;
    QList<LottieTextSlot> textSlots; // All replaceable text fields
};

// ── Lottie Engine ────────────────────────────────────────────
class LottieEngine : public QObject {
    Q_OBJECT

public:
    explicit LottieEngine(QObject* parent = nullptr);
    ~LottieEngine() override;

    // Load all .json presets from a directory
    bool loadPresets(const QString& directory);

    // Get available presets
    QStringList presetIds() const;
    QStringList presetNames() const;
    const LottiePreset* preset(const QString& id) const;

    // Select active preset
    void setActivePreset(const QString& id);
    QString activePresetId() const { return m_activeId; }

    // Replace text dynamically (key = slot id like "Text 01")
    void setText(const QString& slotId, const QString& text);
    void setTexts(const QMap<QString, QString>& texts);

    // Convenience: set by position (1=main title, 2=subtitle, 3=tag)
    void setTitle(const QString& title);
    void setSubtitle(const QString& subtitle);
    void setTag(const QString& tag);
    void setAccentColor(const QColor& c) { m_accentColor = c; }
    void setFontSize(int s) { m_fontSize = s; }
    void setTextColor(const QColor& c) { m_textColor = c; }

    // Render a frame at a given time (seconds from start)
    QImage renderFrame(double timeSec, const QSize& outputSize = {1920, 1080});

    // Animation state
    bool isPlaying() const { return m_playing; }
    void play();
    void stop();
    void reset();
    double duration() const; // in seconds

signals:
    void presetChanged();
    void textChanged();

private:
    // Internal Lottie rendering
    void parsePreset(LottiePreset& preset);
    QImage renderLottieFrame(const LottiePreset& preset, double frame, const QSize& size);

    // Shape rendering from Lottie JSON
    void renderLayer(QPainter& painter, const QJsonObject& layer, double frame,
                     double scaleX, double scaleY, const QMap<QString, QString>& textOverrides);
    void renderShapeGroup(QPainter& painter, const QJsonArray& shapes, double frame,
                          double scaleX, double scaleY);
    void renderTextLayer(QPainter& painter, const QJsonObject& layer, double frame,
                         double scaleX, double scaleY, const QMap<QString, QString>& textOverrides);

    // Keyframe interpolation
    double interpolateValue(const QJsonValue& prop, double frame, double defaultVal = 0);
    QPointF interpolatePoint(const QJsonValue& prop, double frame, QPointF defaultVal = {0, 0});
    QColor interpolateColor(const QJsonValue& prop, double frame, QColor defaultVal = Qt::white);
    double interpolateMultidim(const QJsonArray& keyframes, double frame, int component, double defaultVal);

    // Transform helpers
    struct Transform {
        QPointF position = {0, 0};
        QPointF anchor = {0, 0};
        QPointF scale = {100, 100};
        double rotation = 0;
        double opacity = 100;
    };
    Transform evaluateTransform(const QJsonObject& ks, double frame);

    QList<LottiePreset> m_presets;
    QString m_activeId;
    QMap<QString, QString> m_textOverrides;
    QColor m_accentColor;
    QColor m_textColor = Qt::white;
    int m_fontSize = 28;
    bool m_playing = false;
    double m_startTime = 0;
};

} // namespace prestige
