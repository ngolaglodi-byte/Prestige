// ============================================================
// Prestige AI — Face Tracker Implementation
// Direct port of tracker.py (pure math, no external deps)
// ============================================================

#include "FaceTracker.h"
#include <algorithm>
#include <QDebug>

namespace prestige { namespace ai {

FaceTracker::FaceTracker(float smoothing)
    : m_smoothing(smoothing)
{
}

float FaceTracker::iou(const QRectF& a, const QRectF& b) const
{
    QRectF inter = a.intersected(b);
    if (inter.isEmpty()) return 0.0f;

    double interArea = inter.width() * inter.height();
    double unionArea = a.width() * a.height() + b.width() * b.height() - interArea;
    if (unionArea <= 0.0) return 0.0f;

    return static_cast<float>(interArea / unionArea);
}

void FaceTracker::updateDetections(const QList<TrackedFace>& detections)
{
    // Mark all existing tracks as unmatched
    for (auto& track : m_tracks)
        track.matched = false;

    // Greedy IoU matching: for each detection, find best matching track
    QList<TrackedFace> unmatched;

    for (const auto& det : detections) {
        float bestIou = 0.3f; // minimum IoU threshold
        int bestIdx = -1;

        for (int i = 0; i < m_tracks.size(); ++i) {
            if (m_tracks[i].matched) continue;
            float score = iou(det.bbox, m_tracks[i].smoothedBbox);
            if (score > bestIou) {
                bestIou = score;
                bestIdx = i;
            }
        }

        if (bestIdx >= 0) {
            // Update existing track
            auto& track = m_tracks[bestIdx];
            track.matched = true;
            track.framesLost = 0;

            // Smooth bounding box with exponential moving average
            QRectF oldBbox = track.smoothedBbox;
            QRectF newBbox(
                oldBbox.x() * (1.0 - m_smoothing) + det.bbox.x() * m_smoothing,
                oldBbox.y() * (1.0 - m_smoothing) + det.bbox.y() * m_smoothing,
                oldBbox.width() * (1.0 - m_smoothing) + det.bbox.width() * m_smoothing,
                oldBbox.height() * (1.0 - m_smoothing) + det.bbox.height() * m_smoothing
            );
            track.smoothedBbox = newBbox;
            track.bbox = det.bbox;

            // Update velocity estimate
            track.velocity = QPointF(
                newBbox.center().x() - oldBbox.center().x(),
                newBbox.center().y() - oldBbox.center().y()
            );

            // Update identity info if available
            if (!det.name.isEmpty()) {
                track.name = det.name;
                track.role = det.role;
                track.confidence = det.confidence;
                track.showOverlay = det.showOverlay;
                track.overlayStyle = det.overlayStyle;
            }
        } else {
            // New track
            unmatched.append(det);
        }
    }

    // Increment lost counter for unmatched tracks
    for (auto& track : m_tracks) {
        if (!track.matched)
            track.framesLost++;
    }

    // Remove tracks that have been lost too long
    m_tracks.erase(
        std::remove_if(m_tracks.begin(), m_tracks.end(),
            [this](const TrackedFace& t) { return t.framesLost > m_maxLostFrames; }),
        m_tracks.end()
    );

    // Add new tracks for unmatched detections
    static int nextTrackId = 0;
    for (auto& det : unmatched) {
        TrackedFace newTrack = det;
        if (newTrack.id.isEmpty())
            newTrack.id = QStringLiteral("track_%1").arg(++nextTrackId);
        newTrack.smoothedBbox = det.bbox;
        newTrack.framesLost = 0;
        newTrack.matched = true;
        m_tracks.append(newTrack);
    }
}

QList<TrackedFace> FaceTracker::predict()
{
    // Apply velocity-based prediction for smooth inter-frame motion
    QList<TrackedFace> result;
    for (auto& track : m_tracks) {
        if (track.framesLost > 0) {
            // Predict position using velocity
            QRectF predicted = track.smoothedBbox;
            predicted.translate(track.velocity);
            track.smoothedBbox = predicted;

            // Decay velocity over time
            track.velocity *= 0.9;
        }
        result.append(track);
    }
    return result;
}

void FaceTracker::clear()
{
    m_tracks.clear();
}

}} // namespace prestige::ai
