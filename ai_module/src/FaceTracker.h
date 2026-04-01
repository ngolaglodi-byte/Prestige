#pragma once

// ============================================================
// Prestige AI — Face Tracker (IoU-based multi-object tracker)
// ============================================================

#include <QRectF>
#include <QPointF>
#include <QList>
#include <QString>
#include <vector>

namespace prestige { namespace ai {

struct TrackedFace {
    QString id;
    QString name;
    QString role;
    float confidence = 0.0f;
    QRectF bbox;
    QRectF smoothedBbox;
    bool showOverlay = true;
    QString overlayStyle = "default";

    // Internal tracking state
    QPointF velocity;
    int framesLost = 0;
    bool matched = false;
};

class FaceTracker {
public:
    explicit FaceTracker(float smoothing = 0.3f);

    // Update with new detections (called at ~5fps from detector)
    void updateDetections(const QList<TrackedFace>& detections);

    // Predict positions (called at 30fps)
    QList<TrackedFace> predict();

    void clear();

private:
    float iou(const QRectF& a, const QRectF& b) const;
    QList<TrackedFace> m_tracks;
    float m_smoothing;
    int m_maxLostFrames = 45; // 1.5s at 30fps
};

}} // namespace prestige::ai
