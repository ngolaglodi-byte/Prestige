// ============================================================
// Prestige AI — Lottie Animation Engine (Samsung rlottie)
// 100% compatible After Effects animations via Bodymovin JSON
// Uses Samsung rlottie for pixel-perfect rendering
// Copyright (c) 2024-2026 Prestige Technologie Company
// ============================================================

#include "LottieEngine.h"
#include <QPainter>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QFileInfo>
#include <cmath>

#ifdef PRESTIGE_HAVE_RLOTTIE
#include <rlottie.h>
#endif

namespace prestige {

LottieEngine::LottieEngine(QObject* parent) : QObject(parent) {}
LottieEngine::~LottieEngine() = default;

// ══════════════════════════════════════════════════════════════
// LOADING
// ══════════════════════════════════════════════════════════════

bool LottieEngine::loadPresets(const QString& directory) {
    QDir dir(directory);
    if (!dir.exists()) {
        qWarning() << "[Lottie] Directory not found:" << directory;
        return false;
    }

    auto files = dir.entryList({"*.json"}, QDir::Files, QDir::Name);
    for (const auto& f : files) {
        QFile file(dir.filePath(f));
        if (!file.open(QIODevice::ReadOnly)) continue;

        QJsonParseError err;
        auto doc = QJsonDocument::fromJson(file.readAll(), &err);
        if (err.error != QJsonParseError::NoError) continue;

        LottiePreset preset;
        preset.id = QFileInfo(f).baseName();
        preset.jsonPath = dir.filePath(f);
        preset.json = doc.object();
        preset.width = preset.json["w"].toInt(1920);
        preset.height = preset.json["h"].toInt(1080);
        preset.fps = preset.json["fr"].toDouble(29.97);
        preset.durationFrames = preset.json["op"].toDouble(300);

        preset.name = preset.id;
        preset.name.replace("_", " ");
        if (!preset.name.isEmpty())
            preset.name[0] = preset.name[0].toUpper();

        parsePreset(preset);
        m_presets.append(std::move(preset));
    }

    qInfo() << "[Lottie] Loaded" << m_presets.size() << "presets (Samsung rlottie renderer)";
    return !m_presets.isEmpty();
}

void LottieEngine::parsePreset(LottiePreset& preset) {
    auto assets = preset.json["assets"].toArray();
    for (int ai = 0; ai < assets.size(); ++ai) {
        auto asset = assets[ai].toObject();
        auto layers = asset["layers"].toArray();
        for (int li = 0; li < layers.size(); ++li) {
            auto layer = layers[li].toObject();
            if (layer["ty"].toInt() != 5) continue;

            LottieTextSlot slot;
            slot.id = layer["nm"].toString();
            slot.precompIndex = ai;
            slot.layerIndex = li;

            auto t = layer["t"].toObject();
            auto d = t["d"].toObject();
            auto k = d["k"].toArray();
            if (!k.isEmpty()) {
                auto first = k[0].toObject();
                auto s = first["s"].toObject();
                slot.defaultText = s["t"].toString();
                slot.currentText = slot.defaultText;
                slot.fontFamily = s["f"].toString("Kanit-ExtraBold");
                slot.fontSize = s["s"].toDouble(48);
                auto fc = s["fc"].toArray();
                if (fc.size() >= 3)
                    slot.fontColor = QColor::fromRgbF(fc[0].toDouble(), fc[1].toDouble(), fc[2].toDouble());
            }
            preset.textSlots.append(slot);
        }
    }
}

QStringList LottieEngine::presetIds() const {
    QStringList ids;
    for (const auto& p : m_presets) ids.append(p.id);
    return ids;
}

QStringList LottieEngine::presetNames() const {
    QStringList names;
    for (const auto& p : m_presets) names.append(p.name);
    return names;
}

const LottiePreset* LottieEngine::preset(const QString& id) const {
    for (const auto& p : m_presets) if (p.id == id) return &p;
    return nullptr;
}

void LottieEngine::setActivePreset(const QString& id) {
    if (m_activeId != id) { m_activeId = id; emit presetChanged(); }
}

void LottieEngine::setText(const QString& slotId, const QString& text) {
    m_textOverrides[slotId] = text;
    emit textChanged();
}

void LottieEngine::setTexts(const QMap<QString, QString>& texts) {
    m_textOverrides = texts;
    emit textChanged();
}

void LottieEngine::setTitle(const QString& title) {
    auto* p = preset(m_activeId);
    if (p && !p->textSlots.isEmpty()) {
        // First text slot = main title
        m_textOverrides[p->textSlots[0].id] = title;
    }
}

void LottieEngine::setSubtitle(const QString& subtitle) {
    auto* p = preset(m_activeId);
    if (p && p->textSlots.size() > 1) {
        m_textOverrides[p->textSlots[1].id] = subtitle;
    }
}

void LottieEngine::setTag(const QString& tag) {
    auto* p = preset(m_activeId);
    if (p && p->textSlots.size() > 2) {
        m_textOverrides[p->textSlots[2].id] = tag;
    }
}

void LottieEngine::play() { m_playing = true; }
void LottieEngine::stop() { m_playing = false; }
void LottieEngine::reset() { m_playing = false; m_startTime = 0; }

double LottieEngine::duration() const {
    auto* p = preset(m_activeId);
    return p ? p->durationFrames / p->fps : 0;
}

// ══════════════════════════════════════════════════════════════
// RENDERING via Samsung rlottie
// ══════════════════════════════════════════════════════════════

QImage LottieEngine::renderFrame(double timeSec, const QSize& outputSize) {
    auto* p = preset(m_activeId);
    if (!p) return QImage();

    double frame = timeSec * p->fps;
    if (frame >= p->durationFrames)
        frame = std::fmod(frame, p->durationFrames);

    return renderLottieFrame(*p, frame, outputSize);
}

QImage LottieEngine::renderLottieFrame(const LottiePreset& preset, double frame, const QSize& size) {
#ifdef PRESTIGE_HAVE_RLOTTIE
    // ── Samsung rlottie rendering (pixel-perfect AE reproduction) ──

    // Build modified JSON with text replacements
    QJsonObject modifiedJson = preset.json;

    // Replace text in precomp assets
    if (!m_textOverrides.isEmpty()) {
        QJsonArray assets = modifiedJson["assets"].toArray();
        for (int ai = 0; ai < assets.size(); ++ai) {
            QJsonObject asset = assets[ai].toObject();
            QJsonArray layers = asset["layers"].toArray();
            bool modified = false;
            for (int li = 0; li < layers.size(); ++li) {
                QJsonObject layer = layers[li].toObject();
                if (layer["ty"].toInt() != 5) continue;

                QString layerName = layer["nm"].toString();
                if (m_textOverrides.contains(layerName)) {
                    // Replace text value in the layer
                    QJsonObject t = layer["t"].toObject();
                    QJsonObject d = t["d"].toObject();
                    QJsonArray k = d["k"].toArray();
                    if (!k.isEmpty()) {
                        QJsonObject first = k[0].toObject();
                        QJsonObject s = first["s"].toObject();
                        s["t"] = m_textOverrides[layerName];
                        first["s"] = s;
                        k[0] = first;
                        d["k"] = k;
                        t["d"] = d;
                        layer["t"] = t;
                        layers[li] = layer;
                        modified = true;
                    }
                }
            }
            if (modified) {
                asset["layers"] = layers;
                assets[ai] = asset;
            }
        }
        modifiedJson["assets"] = assets;
    }

    // Convert to JSON string for rlottie
    QJsonDocument doc(modifiedJson);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
    std::string jsonStr(jsonData.constData(), jsonData.size());

    // Create rlottie animation from JSON data
    auto animation = rlottie::Animation::loadFromData(jsonStr, "", "", false);
    if (!animation) {
        qWarning() << "[Lottie] rlottie failed to parse animation:" << preset.id;
        return QImage();
    }

    size_t width = static_cast<size_t>(size.width());
    size_t height = static_cast<size_t>(size.height());

    // Render frame to ARGB32 buffer
    QImage result(size, QImage::Format_ARGB32_Premultiplied);
    result.fill(Qt::transparent);

    auto surface = rlottie::Surface(
        reinterpret_cast<uint32_t*>(result.bits()),
        width, height,
        static_cast<size_t>(result.bytesPerLine())
    );

    size_t frameNum = static_cast<size_t>(std::clamp(frame, 0.0, preset.durationFrames - 1));
    animation->renderSync(frameNum, surface);

    return result;

#else
    // ── Fallback: simplified renderer (when rlottie not compiled) ──
    Q_UNUSED(frame)
    QImage result(size, QImage::Format_ARGB32_Premultiplied);
    result.fill(Qt::transparent);

    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    // Simple fallback: draw colored rectangle with text
    double progress = std::clamp(frame / std::max(1.0, preset.durationFrames), 0.0, 1.0);
    double easeProgress = 1.0 - std::pow(1.0 - std::min(progress * 3, 1.0), 3);

    // Background bar
    double barW = size.width() * 0.8 * easeProgress;
    double barH = size.height() * 0.6;
    double barX = size.width() * 0.1;
    double barY = size.height() * 0.2;
    painter.setOpacity(0.85 * easeProgress);
    painter.fillRect(QRectF(barX, barY, barW, barH), QColor(30, 144, 255));

    // Text
    if (easeProgress > 0.3) {
        double textOpacity = std::min(1.0, (easeProgress - 0.3) / 0.5);
        painter.setOpacity(textOpacity);
        QFont font("Helvetica Neue", int(size.height() * 0.15));
        font.setBold(true);
        painter.setFont(font);
        painter.setPen(Qt::white);

        QString title = m_textOverrides.isEmpty() ? preset.textSlots.value(0).defaultText
                        : m_textOverrides.first();
        painter.drawText(QRectF(barX + 10, barY, barW - 20, barH * 0.5),
                         Qt::AlignVCenter | Qt::AlignLeft, title);

        if (preset.textSlots.size() > 1) {
            font.setPixelSize(int(size.height() * 0.10));
            font.setBold(false);
            painter.setFont(font);
            painter.setPen(QColor(220, 220, 220));
            QString sub = m_textOverrides.value(preset.textSlots[1].id, preset.textSlots[1].defaultText);
            painter.drawText(QRectF(barX + 10, barY + barH * 0.5, barW - 20, barH * 0.5),
                             Qt::AlignVCenter | Qt::AlignLeft, sub);
        }
    }

    return result;
#endif
}

// Unused methods from header (kept for API compatibility)
void LottieEngine::renderLayer(QPainter&, const QJsonObject&, double, double, double, const QMap<QString, QString>&) {}
void LottieEngine::renderShapeGroup(QPainter&, const QJsonArray&, double, double, double) {}
void LottieEngine::renderTextLayer(QPainter&, const QJsonObject&, double, double, double, const QMap<QString, QString>&) {}
double LottieEngine::interpolateValue(const QJsonValue&, double, double d) { return d; }
QPointF LottieEngine::interpolatePoint(const QJsonValue&, double, QPointF d) { return d; }
QColor LottieEngine::interpolateColor(const QJsonValue&, double, QColor d) { return d; }
double LottieEngine::interpolateMultidim(const QJsonArray&, double, int, double d) { return d; }
LottieEngine::Transform LottieEngine::evaluateTransform(const QJsonObject&, double) { return {}; }

} // namespace prestige
