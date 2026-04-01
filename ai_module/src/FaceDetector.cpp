// ============================================================
// Prestige AI — Face Detector Implementation
// ============================================================

#include "FaceDetector.h"
#include <QDebug>

#ifdef PRESTIGE_HAVE_ONNXRT
#include <onnxruntime_cxx_api.h>

namespace prestige { namespace ai {

struct FaceDetector::Impl {
    Ort::Env env{ORT_LOGGING_LEVEL_WARNING, "prestige_face_det"};
    Ort::SessionOptions options;
    std::unique_ptr<Ort::Session> session;
};

FaceDetector::FaceDetector() : m_impl(new Impl) {
    m_impl->options.SetIntraOpNumThreads(2);
}

FaceDetector::~FaceDetector() { delete m_impl; }

bool FaceDetector::loadModel(const QString& path) {
    try {
        m_impl->session = std::make_unique<Ort::Session>(
            m_impl->env, path.toStdString().c_str(), m_impl->options);
        m_loaded = true;
        qInfo() << "[AI] Face detector model loaded:" << path;
        return true;
    } catch (const Ort::Exception& e) {
        qWarning() << "[AI] Failed to load detector:" << e.what();
        return false;
    }
}

QList<DetectedFace> FaceDetector::detect(const QImage& frame, float threshold) {
    QList<DetectedFace> results;
    if (!m_loaded || !m_impl->session) return results;
    // TODO: Full RetinaFace preprocessing + inference + postprocessing
    // This requires: resize to 640x640, normalize, NCHW transpose,
    // anchor decoding with strides [8,16,32], NMS
    Q_UNUSED(frame)
    Q_UNUSED(threshold)
    return results;
}

}} // namespace prestige::ai

#else
// Stub implementation when ONNX Runtime is not available

namespace prestige { namespace ai {

struct FaceDetector::Impl {};

FaceDetector::FaceDetector() : m_impl(nullptr) {}
FaceDetector::~FaceDetector() {}

bool FaceDetector::loadModel(const QString& path) {
    Q_UNUSED(path)
    qInfo() << "[AI] Face detector: ONNX Runtime not available (stub mode)";
    return false;
}

QList<DetectedFace> FaceDetector::detect(const QImage& frame, float threshold) {
    Q_UNUSED(frame)
    Q_UNUSED(threshold)
    return {};
}

}} // namespace prestige::ai

#endif
