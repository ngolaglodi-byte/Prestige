#pragma once

// ============================================================
// Prestige AI — ZeroMQ Protocol Definitions
// Copyright (c) 2024-2026 Prestige Technologie Company
// All rights reserved.
//
// JSON schema exchanged between Python AI Engine and C++ Vision Engine
//
// SYNC MODEL:
//   Python captures video + runs AI → publishes detections with frame_id + timestamp
//   C++ captures the SAME video source independently → receives detections
//   C++ uses timestamp to match detections to the correct output frame
//   Detections older than 1 frame are interpolated/predicted forward
// ============================================================

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QList>
#include <QElapsedTimer>

namespace prestige {

// ── Bounding box ───────────────────────────────────────────
struct BBox {
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;

    static BBox fromJson(const QJsonObject& obj) {
        return {
            obj["x"].toInt(),
            obj["y"].toInt(),
            obj["w"].toInt(),
            obj["h"].toInt()
        };
    }

    QJsonObject toJson() const {
        return {{"x", x}, {"y", y}, {"w", w}, {"h", h}};
    }

    // Center point
    int cx() const { return x + w / 2; }
    int cy() const { return y + h / 2; }
};

// ── Single talent detection ────────────────────────────────
struct TalentDetection {
    QString id;
    QString name;
    QString role;
    double  confidence   = 0.0;
    BBox    bbox;
    bool    showOverlay  = true;
    QString overlayStyle = QStringLiteral("default");

    static TalentDetection fromJson(const QJsonObject& obj) {
        TalentDetection t;
        t.id           = obj["id"].toString();
        t.name         = obj["name"].toString();
        t.role         = obj["role"].toString();
        t.confidence   = obj["confidence"].toDouble();
        t.bbox         = BBox::fromJson(obj["bbox"].toObject());
        t.showOverlay  = obj["show_overlay"].toBool(true);
        t.overlayStyle = obj["overlay_style"].toString(QStringLiteral("default"));
        return t;
    }

    QJsonObject toJson() const {
        return {
            {"id",            id},
            {"name",          name},
            {"role",          role},
            {"confidence",    confidence},
            {"bbox",          bbox.toJson()},
            {"show_overlay",  showOverlay},
            {"overlay_style", overlayStyle}
        };
    }
};

// ── Full frame message ─────────────────────────────────────
struct DetectionMessage {
    qint64  frameId              = 0;
    qint64  timestampMs          = 0;
    qint64  captureTimestampMs   = 0;   // When the frame was captured (for sync)
    qint64  detectionLatencyMs   = 0;   // How long detection took
    QList<TalentDetection> talents;
    QString sceneType            = QStringLiteral("unknown");
    bool    overlayGlobalVisible = true;
    int     faceCount            = 0;

    static DetectionMessage fromJson(const QByteArray& raw) {
        auto doc = QJsonDocument::fromJson(raw);
        auto obj = doc.object();

        DetectionMessage msg;
        msg.frameId              = obj["frame_id"].toInteger();
        msg.timestampMs          = obj["timestamp_ms"].toInteger();
        msg.captureTimestampMs   = obj["capture_timestamp_ms"].toInteger();
        msg.detectionLatencyMs   = obj["detection_latency_ms"].toInteger();
        msg.sceneType            = obj["scene_type"].toString(QStringLiteral("unknown"));
        msg.overlayGlobalVisible = obj["overlay_global_visible"].toBool(true);
        msg.faceCount            = obj["face_count"].toInt(0);

        for (const auto& v : obj["talents"].toArray()) {
            msg.talents.append(TalentDetection::fromJson(v.toObject()));
        }
        return msg;
    }

    QByteArray toJson() const {
        QJsonArray arr;
        for (const auto& t : talents)
            arr.append(t.toJson());

        QJsonObject obj;
        obj["frame_id"]               = frameId;
        obj["timestamp_ms"]           = timestampMs;
        obj["capture_timestamp_ms"]   = captureTimestampMs;
        obj["detection_latency_ms"]   = detectionLatencyMs;
        obj["talents"]                = arr;
        obj["scene_type"]             = sceneType;
        obj["overlay_global_visible"] = overlayGlobalVisible;
        obj["face_count"]             = faceCount;

        return QJsonDocument(obj).toJson(QJsonDocument::Compact);
    }
};

} // namespace prestige
