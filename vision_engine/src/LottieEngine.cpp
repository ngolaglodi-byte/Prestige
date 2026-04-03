// ============================================================
// Prestige AI — Lottie Animation Engine
// Parses Bodymovin JSON, replaces text, renders frame by frame
// Copyright (c) 2024-2026 Prestige Technologie Company
// ============================================================

#include "LottieEngine.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QFontMetrics>
#include <QLinearGradient>
#include <cmath>

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
        if (err.error != QJsonParseError::NoError) {
            qWarning() << "[Lottie] Parse error in" << f << ":" << err.errorString();
            continue;
        }

        LottiePreset preset;
        preset.id = QFileInfo(f).baseName(); // "title_01"
        preset.jsonPath = dir.filePath(f);
        preset.json = doc.object();
        preset.width = preset.json["w"].toInt(1920);
        preset.height = preset.json["h"].toInt(1080);
        preset.fps = preset.json["fr"].toDouble(29.97);
        preset.durationFrames = preset.json["op"].toDouble(300);

        // Generate display name from ID
        preset.name = preset.id;
        preset.name.replace("_", " ");
        preset.name[0] = preset.name[0].toUpper();

        parsePreset(preset);
        m_presets.append(std::move(preset));
    }

    qInfo() << "[Lottie] Loaded" << m_presets.size() << "animation presets from" << directory;
    return !m_presets.isEmpty();
}

void LottieEngine::parsePreset(LottiePreset& preset) {
    // Find all text layers in precomps (assets)
    auto assets = preset.json["assets"].toArray();
    for (int ai = 0; ai < assets.size(); ++ai) {
        auto asset = assets[ai].toObject();
        auto layers = asset["layers"].toArray();
        for (int li = 0; li < layers.size(); ++li) {
            auto layer = layers[li].toObject();
            if (layer["ty"].toInt() != 5) continue; // ty=5 → text layer

            LottieTextSlot slot;
            slot.id = layer["nm"].toString();
            slot.precompIndex = ai;
            slot.layerIndex = li;

            // Extract text properties
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

                // Font color
                auto fc = s["fc"].toArray();
                if (fc.size() >= 3) {
                    slot.fontColor = QColor::fromRgbF(
                        fc[0].toDouble(), fc[1].toDouble(), fc[2].toDouble());
                }
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
    if (p && p->textSlots.size() > 0) {
        // Find the main title slot (usually the largest font or "Text 01")
        for (const auto& slot : p->textSlots) {
            if (slot.id.contains("01") || slot.id.contains("Title")) {
                m_textOverrides[slot.id] = title;
                break;
            }
        }
        // Fallback: first slot
        if (!m_textOverrides.contains(p->textSlots[0].id))
            m_textOverrides[p->textSlots[0].id] = title;
    }
}

void LottieEngine::setSubtitle(const QString& subtitle) {
    auto* p = preset(m_activeId);
    if (p && p->textSlots.size() > 1) {
        for (const auto& slot : p->textSlots) {
            if (slot.id.contains("02") || slot.id.contains("Sub")) {
                m_textOverrides[slot.id] = subtitle;
                break;
            }
        }
        if (p->textSlots.size() > 1)
            m_textOverrides[p->textSlots[1].id] = subtitle;
    }
}

void LottieEngine::setTag(const QString& tag) {
    auto* p = preset(m_activeId);
    if (p && p->textSlots.size() > 2) {
        for (const auto& slot : p->textSlots) {
            if (slot.id.contains("03") || slot.id.contains("Tag")) {
                m_textOverrides[slot.id] = tag;
                break;
            }
        }
        if (p->textSlots.size() > 2)
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
// RENDERING
// ══════════════════════════════════════════════════════════════

QImage LottieEngine::renderFrame(double timeSec, const QSize& outputSize) {
    auto* p = preset(m_activeId);
    if (!p) return QImage();

    double frame = timeSec * p->fps;
    // Loop: wrap around to duration
    if (frame >= p->durationFrames) {
        frame = std::fmod(frame, p->durationFrames);
    }

    return renderLottieFrame(*p, frame, outputSize);
}

QImage LottieEngine::renderLottieFrame(const LottiePreset& preset, double frame, const QSize& size) {
    QImage result(size, QImage::Format_ARGB32_Premultiplied);
    result.fill(Qt::transparent);

    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    double scaleX = static_cast<double>(size.width()) / preset.width;
    double scaleY = static_cast<double>(size.height()) / preset.height;

    // Render main layers (bottom to top)
    auto layers = preset.json["layers"].toArray();
    for (int i = layers.size() - 1; i >= 0; --i) {
        auto layer = layers[i].toObject();

        // Check in/out point
        double ip = layer["ip"].toDouble(0);
        double op = layer["op"].toDouble(preset.durationFrames);
        if (frame < ip || frame >= op) continue;

        int ty = layer["ty"].toInt();
        if (ty == 4) {
            // Shape layer
            renderLayer(painter, layer, frame, scaleX, scaleY, m_textOverrides);
        } else if (ty == 0) {
            // Precomp layer — render the precomp's layers
            QString refId = layer["refId"].toString();
            auto assets = preset.json["assets"].toArray();
            for (const auto& asset : assets) {
                auto aObj = asset.toObject();
                if (aObj["id"].toString() == refId) {
                    // Apply precomp transform
                    painter.save();
                    auto ks = layer["ks"].toObject();
                    Transform tr = evaluateTransform(ks, frame - ip);

                    painter.translate(tr.position.x() * scaleX, tr.position.y() * scaleY);
                    painter.translate(-tr.anchor.x() * scaleX, -tr.anchor.y() * scaleY);
                    painter.scale(tr.scale.x() / 100.0, tr.scale.y() / 100.0);
                    painter.rotate(tr.rotation);
                    painter.setOpacity(tr.opacity / 100.0);

                    // Render precomp layers
                    auto pcLayers = aObj["layers"].toArray();
                    for (int j = pcLayers.size() - 1; j >= 0; --j) {
                        auto pcLayer = pcLayers[j].toObject();
                        double pcIp = pcLayer["ip"].toDouble(0);
                        double pcOp = pcLayer["op"].toDouble(preset.durationFrames);
                        double localFrame = frame - ip;
                        if (localFrame < pcIp || localFrame >= pcOp) continue;

                        int pcTy = pcLayer["ty"].toInt();
                        if (pcTy == 5) {
                            renderTextLayer(painter, pcLayer, localFrame, scaleX, scaleY, m_textOverrides);
                        } else if (pcTy == 4) {
                            renderLayer(painter, pcLayer, localFrame, scaleX, scaleY, m_textOverrides);
                        }
                    }
                    painter.restore();
                    break;
                }
            }
        }
    }

    painter.end();
    return result;
}

// ══════════════════════════════════════════════════════════════
// LAYER RENDERING
// ══════════════════════════════════════════════════════════════

void LottieEngine::renderLayer(QPainter& painter, const QJsonObject& layer, double frame,
                                double scaleX, double scaleY, const QMap<QString, QString>&) {
    painter.save();

    // Apply transform
    auto ks = layer["ks"].toObject();
    Transform tr = evaluateTransform(ks, frame);

    painter.translate(tr.position.x() * scaleX, tr.position.y() * scaleY);
    painter.translate(-tr.anchor.x() * scaleX, -tr.anchor.y() * scaleY);
    painter.scale(tr.scale.x() / 100.0, tr.scale.y() / 100.0);
    painter.rotate(tr.rotation);
    painter.setOpacity(std::clamp(tr.opacity / 100.0, 0.0, 1.0));

    // Render shapes
    auto shapes = layer["shapes"].toArray();
    renderShapeGroup(painter, shapes, frame, scaleX, scaleY);

    painter.restore();
}

void LottieEngine::renderShapeGroup(QPainter& painter, const QJsonArray& shapes, double frame,
                                     double scaleX, double scaleY) {
    QPainterPath currentPath;
    QColor fillColor = Qt::transparent;
    QColor strokeColor = Qt::transparent;
    double strokeWidth = 0;

    for (const auto& shapeVal : shapes) {
        auto shape = shapeVal.toObject();
        QString ty = shape["ty"].toString();

        if (ty == "gr") {
            // Group — recurse
            painter.save();
            auto items = shape["it"].toArray();
            renderShapeGroup(painter, items, frame, scaleX, scaleY);
            painter.restore();
        }
        else if (ty == "rc") {
            // Rectangle
            auto p = shape["p"].toObject();  // position
            auto s = shape["s"].toObject();  // size
            auto r = shape["r"];             // roundness
            QPointF pos = interpolatePoint(p, frame);
            QPointF sz = interpolatePoint(s, frame);
            double round = interpolateValue(r, frame, 0);
            QRectF rect(
                (pos.x() - sz.x()/2) * scaleX,
                (pos.y() - sz.y()/2) * scaleY,
                sz.x() * scaleX, sz.y() * scaleY
            );
            if (round > 0)
                currentPath.addRoundedRect(rect, round * scaleX, round * scaleY);
            else
                currentPath.addRect(rect);
        }
        else if (ty == "el") {
            // Ellipse
            auto p = shape["p"].toObject();
            auto s = shape["s"].toObject();
            QPointF pos = interpolatePoint(p, frame);
            QPointF sz = interpolatePoint(s, frame);
            currentPath.addEllipse(
                QPointF(pos.x() * scaleX, pos.y() * scaleY),
                sz.x()/2 * scaleX, sz.y()/2 * scaleY
            );
        }
        else if (ty == "sh") {
            // Bezier path
            // Simplified: just use the path vertices
            auto ks = shape["ks"];
            // Handle keyframed or static path
            QJsonArray vertices;
            if (ks.isObject()) {
                auto kObj = ks.toObject();
                auto kArr = kObj["k"].toArray();
                if (!kArr.isEmpty() && kArr[0].isObject() && kArr[0].toObject().contains("v")) {
                    vertices = kArr[0].toObject()["v"].toArray();
                } else if (kArr[0].isArray()) {
                    // Keyframed — take first keyframe
                    vertices = kArr;
                }
            }
            if (!vertices.isEmpty()) {
                QPainterPath bezier;
                for (int vi = 0; vi < vertices.size(); ++vi) {
                    auto pt = vertices[vi].toArray();
                    if (pt.size() >= 2) {
                        double x = pt[0].toDouble() * scaleX;
                        double y = pt[1].toDouble() * scaleY;
                        if (vi == 0) bezier.moveTo(x, y);
                        else bezier.lineTo(x, y);
                    }
                }
                bezier.closeSubpath();
                currentPath.addPath(bezier);
            }
        }
        else if (ty == "fl") {
            // Fill
            auto c = shape["c"];
            fillColor = interpolateColor(c, frame, Qt::white);
            double opacity = interpolateValue(shape["o"], frame, 100);
            fillColor.setAlphaF(opacity / 100.0);
        }
        else if (ty == "st") {
            // Stroke
            auto c = shape["c"];
            strokeColor = interpolateColor(c, frame, Qt::white);
            strokeWidth = interpolateValue(shape["w"], frame, 2) * scaleX;
            double opacity = interpolateValue(shape["o"], frame, 100);
            strokeColor.setAlphaF(opacity / 100.0);
        }
        else if (ty == "tr") {
            // Transform within group
            Transform gtr = evaluateTransform(shape, frame);
            painter.translate(gtr.position.x() * scaleX, gtr.position.y() * scaleY);
            painter.translate(-gtr.anchor.x() * scaleX, -gtr.anchor.y() * scaleY);
            painter.scale(gtr.scale.x() / 100.0, gtr.scale.y() / 100.0);
            painter.rotate(gtr.rotation);
            painter.setOpacity(std::clamp(gtr.opacity / 100.0, 0.0, 1.0));
        }
    }

    // Draw the accumulated path
    if (!currentPath.isEmpty()) {
        if (fillColor.alpha() > 0) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(fillColor);
            painter.drawPath(currentPath);
        }
        if (strokeColor.alpha() > 0 && strokeWidth > 0) {
            painter.setPen(QPen(strokeColor, strokeWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter.setBrush(Qt::NoBrush);
            painter.drawPath(currentPath);
        }
    }
}

void LottieEngine::renderTextLayer(QPainter& painter, const QJsonObject& layer, double frame,
                                    double scaleX, double scaleY,
                                    const QMap<QString, QString>& textOverrides) {
    painter.save();

    // Apply transform
    auto ks = layer["ks"].toObject();
    Transform tr = evaluateTransform(ks, frame);
    painter.translate(tr.position.x() * scaleX, tr.position.y() * scaleY);
    painter.translate(-tr.anchor.x() * scaleX, -tr.anchor.y() * scaleY);
    painter.scale(tr.scale.x() / 100.0, tr.scale.y() / 100.0);
    painter.rotate(tr.rotation);
    painter.setOpacity(std::clamp(tr.opacity / 100.0, 0.0, 1.0));

    // Get text properties
    auto t = layer["t"].toObject();
    auto d = t["d"].toObject();
    auto kArr = d["k"].toArray();
    if (kArr.isEmpty()) { painter.restore(); return; }

    auto first = kArr[0].toObject();
    auto s = first["s"].toObject();

    // Get text — check for override
    QString layerName = layer["nm"].toString();
    QString text = textOverrides.value(layerName, s["t"].toString());

    // Font
    QString fontFamily = s["f"].toString("Helvetica Neue");
    // Map AE font names to system fonts
    if (fontFamily.contains("Kanit")) fontFamily = "Helvetica Neue";
    double fontSize = s["s"].toDouble(48) * scaleY;
    int fontWeight = fontFamily.contains("Bold") || fontFamily.contains("ExtraBold")
                     ? QFont::Bold : QFont::Normal;

    QFont font(fontFamily, static_cast<int>(fontSize));
    font.setWeight(static_cast<QFont::Weight>(fontWeight));
    font.setLetterSpacing(QFont::AbsoluteSpacing, s["ls"].toDouble(0) * scaleX);

    // Color
    QColor color = Qt::white;
    auto fc = s["fc"].toArray();
    if (fc.size() >= 3) {
        color = QColor::fromRgbF(fc[0].toDouble(), fc[1].toDouble(), fc[2].toDouble());
    }

    // Justification
    int justify = s["j"].toInt(0); // 0=left, 1=right, 2=center
    Qt::Alignment align = Qt::AlignLeft;
    if (justify == 1) align = Qt::AlignRight;
    else if (justify == 2) align = Qt::AlignHCenter;

    // Draw text
    painter.setFont(font);
    painter.setPen(color);

    QFontMetrics fm(font);
    QRectF textRect(0, -fm.ascent(), fm.horizontalAdvance(text) + 20, fm.height());
    painter.drawText(textRect, static_cast<int>(align) | Qt::AlignVCenter, text);

    painter.restore();
}

// ══════════════════════════════════════════════════════════════
// KEYFRAME INTERPOLATION
// ══════════════════════════════════════════════════════════════

double LottieEngine::interpolateValue(const QJsonValue& prop, double frame, double defaultVal) {
    if (prop.isDouble()) return prop.toDouble(defaultVal);
    if (!prop.isObject()) return defaultVal;

    auto obj = prop.toObject();
    auto k = obj["k"];

    // Static value
    if (k.isDouble()) return k.toDouble(defaultVal);
    if (k.isArray()) {
        auto arr = k.toArray();
        if (arr.isEmpty()) return defaultVal;

        // Simple static array [value]
        if (arr[0].isDouble()) return arr[0].toDouble(defaultVal);

        // Keyframed
        return interpolateMultidim(arr, frame, 0, defaultVal);
    }
    return defaultVal;
}

QPointF LottieEngine::interpolatePoint(const QJsonValue& prop, double frame, QPointF defaultVal) {
    if (!prop.isObject()) return defaultVal;
    auto obj = prop.toObject();

    // Check for separate dimensions
    if (obj.contains("x") && obj.contains("y")) {
        double x = interpolateValue(obj["x"], frame, defaultVal.x());
        double y = interpolateValue(obj["y"], frame, defaultVal.y());
        return {x, y};
    }

    auto k = obj["k"];
    if (k.isArray()) {
        auto arr = k.toArray();
        if (arr.size() >= 2 && arr[0].isDouble()) {
            return {arr[0].toDouble(defaultVal.x()), arr[1].toDouble(defaultVal.y())};
        }
        // Keyframed
        if (!arr.isEmpty() && arr[0].isObject()) {
            double x = interpolateMultidim(arr, frame, 0, defaultVal.x());
            double y = interpolateMultidim(arr, frame, 1, defaultVal.y());
            return {x, y};
        }
    }
    return defaultVal;
}

QColor LottieEngine::interpolateColor(const QJsonValue& prop, double frame, QColor defaultVal) {
    if (!prop.isObject()) return defaultVal;
    auto obj = prop.toObject();
    auto k = obj["k"];

    if (k.isArray()) {
        auto arr = k.toArray();
        if (arr.size() >= 3 && arr[0].isDouble()) {
            return QColor::fromRgbF(arr[0].toDouble(), arr[1].toDouble(), arr[2].toDouble());
        }
        // Keyframed color
        if (!arr.isEmpty() && arr[0].isObject()) {
            double r = interpolateMultidim(arr, frame, 0, defaultVal.redF());
            double g = interpolateMultidim(arr, frame, 1, defaultVal.greenF());
            double b = interpolateMultidim(arr, frame, 2, defaultVal.blueF());
            return QColor::fromRgbF(std::clamp(r, 0.0, 1.0), std::clamp(g, 0.0, 1.0), std::clamp(b, 0.0, 1.0));
        }
    }
    return defaultVal;
}

double LottieEngine::interpolateMultidim(const QJsonArray& keyframes, double frame,
                                          int component, double defaultVal) {
    if (keyframes.isEmpty()) return defaultVal;

    // Find the two keyframes surrounding the current frame
    for (int i = 0; i < keyframes.size(); ++i) {
        auto kf = keyframes[i].toObject();
        double t = kf["t"].toDouble(0);

        if (i == keyframes.size() - 1 || frame <= t) {
            // Use this keyframe's start value
            auto s = kf["s"].toArray();
            if (s.size() > component) return s[component].toDouble(defaultVal);
            return defaultVal;
        }

        double nextT = keyframes[i + 1].toObject()["t"].toDouble(t);
        if (frame >= t && frame < nextT) {
            // Interpolate between this and next keyframe
            auto s = kf["s"].toArray();
            auto e = kf["e"].toArray();
            if (e.isEmpty()) e = keyframes[i + 1].toObject()["s"].toArray();

            double startVal = (s.size() > component) ? s[component].toDouble(defaultVal) : defaultVal;
            double endVal = (e.size() > component) ? e[component].toDouble(defaultVal) : defaultVal;

            double progress = (nextT > t) ? (frame - t) / (nextT - t) : 0;

            // Apply easing (check for bezier curves in "o" and "i")
            auto o = kf["o"].toObject();
            auto in_ = kf["i"].toObject();
            if (!o.isEmpty() && !in_.isEmpty()) {
                // Cubic bezier easing
                auto ox = o["x"]; auto oy = o["y"];
                auto ix = in_["x"]; auto iy = in_["y"];
                double x1 = ox.isArray() ? ox.toArray()[0].toDouble(0.5) : ox.toDouble(0.5);
                double y1 = oy.isArray() ? oy.toArray()[0].toDouble(0) : oy.toDouble(0);
                double x2 = ix.isArray() ? ix.toArray()[0].toDouble(0.5) : ix.toDouble(0.5);
                double y2 = iy.isArray() ? iy.toArray()[0].toDouble(1) : iy.toDouble(1);

                // Simple cubic bezier approximation
                double t2 = progress;
                for (int iter = 0; iter < 8; ++iter) {
                    double cx = 3*x1, bx = 3*(x2-x1)-cx, ax = 1-cx-bx;
                    double xAtT = ((ax*t2+bx)*t2+cx)*t2;
                    double dxdt = (3*ax*t2+2*bx)*t2+cx;
                    if (std::abs(dxdt) < 1e-10) break;
                    t2 -= (xAtT - progress) / dxdt;
                    t2 = std::clamp(t2, 0.0, 1.0);
                }
                double cy = 3*y1, by = 3*(y2-y1)-cy, ay = 1-cy-by;
                progress = ((ay*t2+by)*t2+cy)*t2;
            }

            return startVal + (endVal - startVal) * progress;
        }
    }
    return defaultVal;
}

LottieEngine::Transform LottieEngine::evaluateTransform(const QJsonObject& ks, double frame) {
    Transform tr;
    if (ks.contains("p")) tr.position = interpolatePoint(ks["p"], frame);
    if (ks.contains("a")) tr.anchor = interpolatePoint(ks["a"], frame);
    if (ks.contains("s")) {
        tr.scale = interpolatePoint(ks["s"], frame, {100, 100});
    }
    if (ks.contains("r")) tr.rotation = interpolateValue(ks["r"], frame, 0);
    if (ks.contains("o")) tr.opacity = interpolateValue(ks["o"], frame, 100);
    return tr;
}

} // namespace prestige
