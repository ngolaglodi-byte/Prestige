// ============================================================
// Prestige AI — Face Recognizer Implementation
// ============================================================

#include "FaceRecognizer.h"
#include <QDebug>
#include <cmath>
#include <numeric>

#ifdef PRESTIGE_HAVE_ONNXRT
#include <onnxruntime_cxx_api.h>

namespace prestige { namespace ai {

struct FaceRecognizer::Impl {
    Ort::Env env{ORT_LOGGING_LEVEL_WARNING, "prestige_face_rec"};
    Ort::SessionOptions options;
    std::unique_ptr<Ort::Session> session;
};

FaceRecognizer::FaceRecognizer() : m_impl(new Impl) {
    m_impl->options.SetIntraOpNumThreads(2);
}

FaceRecognizer::~FaceRecognizer() { delete m_impl; }

bool FaceRecognizer::loadModel(const QString& path) {
    try {
        m_impl->session = std::make_unique<Ort::Session>(
            m_impl->env, path.toStdString().c_str(), m_impl->options);
        m_loaded = true;
        qInfo() << "[AI] Face recognizer model loaded:" << path;
        return true;
    } catch (const Ort::Exception& e) {
        qWarning() << "[AI] Failed to load recognizer:" << e.what();
        return false;
    }
}

std::vector<float> FaceRecognizer::extractEmbedding(const QImage& alignedFace) {
    if (!m_loaded || !m_impl->session) return {};
    // TODO: Preprocess 112x112 aligned face, run ArcFace inference,
    // return L2-normalized 512-d embedding
    Q_UNUSED(alignedFace)
    return {};
}

float FaceRecognizer::cosineSimilarity(const std::vector<float>& a, const std::vector<float>& b) {
    if (a.size() != b.size() || a.empty()) return 0.0f;
    float dot = std::inner_product(a.begin(), a.end(), b.begin(), 0.0f);
    float normA = std::sqrt(std::inner_product(a.begin(), a.end(), a.begin(), 0.0f));
    float normB = std::sqrt(std::inner_product(b.begin(), b.end(), b.begin(), 0.0f));
    if (normA < 1e-8f || normB < 1e-8f) return 0.0f;
    return dot / (normA * normB);
}

}} // namespace prestige::ai

#else
// Stub implementation when ONNX Runtime is not available

namespace prestige { namespace ai {

struct FaceRecognizer::Impl {};

FaceRecognizer::FaceRecognizer() : m_impl(nullptr) {}
FaceRecognizer::~FaceRecognizer() {}

bool FaceRecognizer::loadModel(const QString& path) {
    Q_UNUSED(path)
    qInfo() << "[AI] Face recognizer: ONNX Runtime not available (stub mode)";
    return false;
}

std::vector<float> FaceRecognizer::extractEmbedding(const QImage& alignedFace) {
    Q_UNUSED(alignedFace)
    return {};
}

float FaceRecognizer::cosineSimilarity(const std::vector<float>& a, const std::vector<float>& b) {
    if (a.size() != b.size() || a.empty()) return 0.0f;
    float dot = std::inner_product(a.begin(), a.end(), b.begin(), 0.0f);
    float normA = std::sqrt(std::inner_product(a.begin(), a.end(), a.begin(), 0.0f));
    float normB = std::sqrt(std::inner_product(b.begin(), b.end(), b.begin(), 0.0f));
    if (normA < 1e-8f || normB < 1e-8f) return 0.0f;
    return dot / (normA * normB);
}

}} // namespace prestige::ai

#endif
