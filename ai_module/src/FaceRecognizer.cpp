// ============================================================
// Prestige AI — Face Recognizer Implementation
// ============================================================

#include "FaceRecognizer.h"
#include <QDebug>
#include <QTransform>
#include <cmath>
#include <numeric>
#include <array>
#include <vector>

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

    const int faceSize = 112;

    // Ensure 112x112 RGB
    QImage face = alignedFace.scaled(faceSize, faceSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
                             .convertToFormat(QImage::Format_RGB888);

    // Convert to float tensor [1, 3, 112, 112], normalize to [-1, 1]
    std::vector<float> inputTensor(1 * 3 * faceSize * faceSize);
    const uchar* pixels = face.constBits();
    int stride = face.bytesPerLine();

    for (int y = 0; y < faceSize; ++y) {
        for (int x = 0; x < faceSize; ++x) {
            int srcIdx = y * stride + x * 3;
            float r = (static_cast<float>(pixels[srcIdx + 0]) / 255.0f - 0.5f) / 0.5f;
            float g = (static_cast<float>(pixels[srcIdx + 1]) / 255.0f - 0.5f) / 0.5f;
            float b = (static_cast<float>(pixels[srcIdx + 2]) / 255.0f - 0.5f) / 0.5f;
            inputTensor[0 * faceSize * faceSize + y * faceSize + x] = r;
            inputTensor[1 * faceSize * faceSize + y * faceSize + x] = g;
            inputTensor[2 * faceSize * faceSize + y * faceSize + x] = b;
        }
    }

    // Run inference
    auto memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    std::array<int64_t, 4> inputShape = {1, 3, faceSize, faceSize};
    Ort::Value inputOrt = Ort::Value::CreateTensor<float>(
        memoryInfo, inputTensor.data(), inputTensor.size(),
        inputShape.data(), inputShape.size());

    const char* inputName = "input.1";
    const char* outputName = "683"; // ArcFace output name

    std::vector<Ort::Value> outputs;
    try {
        outputs = m_impl->session->Run(
            Ort::RunOptions{nullptr},
            &inputName, &inputOrt, 1,
            &outputName, 1);
    } catch (const Ort::Exception& e) {
        qWarning() << "[AI] Recognition inference failed:" << e.what();
        return {};
    }

    // Extract 512-d embedding
    float* data = outputs[0].GetTensorMutableData<float>();
    auto shape = outputs[0].GetTensorTypeAndShapeInfo().GetShape();
    int embSize = static_cast<int>(shape.back());

    std::vector<float> embedding(data, data + embSize);

    // L2 normalize
    float norm = std::sqrt(std::inner_product(embedding.begin(), embedding.end(), embedding.begin(), 0.0f));
    if (norm > 1e-8f) {
        for (auto& v : embedding) v /= norm;
    }

    return embedding;
}

float FaceRecognizer::cosineSimilarity(const std::vector<float>& a, const std::vector<float>& b) {
    if (a.size() != b.size() || a.empty()) return 0.0f;
    float dot = std::inner_product(a.begin(), a.end(), b.begin(), 0.0f);
    float normA = std::sqrt(std::inner_product(a.begin(), a.end(), a.begin(), 0.0f));
    float normB = std::sqrt(std::inner_product(b.begin(), b.end(), b.begin(), 0.0f));
    if (normA < 1e-8f || normB < 1e-8f) return 0.0f;
    return dot / (normA * normB);
}

QImage FaceRecognizer::alignFace(const QImage& frame, const std::vector<float>& landmarks) {
    if (landmarks.size() < 10) {
        // Fallback: just return scaled frame
        return frame;
    }

    // Reference landmarks for 112x112 aligned face (standard ArcFace alignment)
    const float dst[5][2] = {
        {38.2946f, 51.6963f},
        {73.5318f, 51.5014f},
        {56.0252f, 71.7366f},
        {41.5493f, 92.3655f},
        {70.7299f, 92.2041f}
    };

    // Source landmarks
    float src[5][2];
    for (int i = 0; i < 5; ++i) {
        src[i][0] = landmarks[i * 2 + 0];
        src[i][1] = landmarks[i * 2 + 1];
    }

    // Compute simple similarity transform (translation + scale + rotation)
    // Using first two points (eye centers)
    float srcCx = (src[0][0] + src[1][0]) / 2.0f;
    float srcCy = (src[0][1] + src[1][1]) / 2.0f;
    float dstCx = (dst[0][0] + dst[1][0]) / 2.0f;
    float dstCy = (dst[0][1] + dst[1][1]) / 2.0f;

    float srcDx = src[1][0] - src[0][0];
    float srcDy = src[1][1] - src[0][1];
    float dstDx = dst[1][0] - dst[0][0];
    float dstDy = dst[1][1] - dst[0][1];

    float srcDist = std::sqrt(srcDx * srcDx + srcDy * srcDy);
    float dstDist = std::sqrt(dstDx * dstDx + dstDy * dstDy);

    if (srcDist < 1.0f) return frame.scaled(112, 112);

    float scale = dstDist / srcDist;
    float angle = std::atan2(dstDy, dstDx) - std::atan2(srcDy, srcDx);

    // Apply transform using QTransform
    QTransform transform;
    transform.translate(dstCx, dstCy);
    transform.rotateRadians(angle);
    transform.scale(scale, scale);
    transform.translate(-srcCx, -srcCy);

    QImage aligned = frame.transformed(transform, Qt::SmoothTransformation);

    // Crop to 112x112 from center
    int cx = aligned.width() / 2;
    int cy = aligned.height() / 2;
    QRect cropRect(cx - 56, cy - 56, 112, 112);
    cropRect = cropRect.intersected(aligned.rect());

    if (cropRect.width() < 112 || cropRect.height() < 112) {
        return frame.scaled(112, 112, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    return aligned.copy(cropRect);
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

QImage FaceRecognizer::alignFace(const QImage& frame, const std::vector<float>& landmarks) {
    Q_UNUSED(landmarks)
    return frame;
}

}} // namespace prestige::ai

#endif
