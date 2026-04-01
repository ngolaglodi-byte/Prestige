#pragma once

// ============================================================
// Prestige AI — Face Detector (ONNX Runtime RetinaFace)
// ============================================================

#include <QImage>
#include <QRectF>
#include <QList>
#include <QString>
#include <vector>

namespace prestige { namespace ai {

struct DetectedFace {
    QRectF bbox;
    float score = 0.0f;
    std::vector<float> landmarks; // 5 points x 2 = 10 floats
};

class FaceDetector {
public:
    FaceDetector();
    ~FaceDetector();

    bool loadModel(const QString& modelPath); // det_10g.onnx
    bool isLoaded() const { return m_loaded; }

    QList<DetectedFace> detect(const QImage& frame, float scoreThreshold = 0.5f);

private:
    struct Impl;
    Impl* m_impl = nullptr;
    bool m_loaded = false;
};

}} // namespace prestige::ai
