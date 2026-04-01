// ============================================================
// Prestige AI — Face Detector Implementation
// ============================================================

#include "FaceDetector.h"
#include <QDebug>
#include <QPainter>
#include <algorithm>
#include <cmath>
#include <array>
#include <vector>

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

    const int inputSize = 640;

    // 1. Preprocess: resize to 640x640, convert to float NCHW
    QImage rgb = frame.scaled(inputSize, inputSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)
                      .convertToFormat(QImage::Format_RGB888);

    // Letterbox padding (center the image)
    QImage padded(inputSize, inputSize, QImage::Format_RGB888);
    padded.fill(Qt::black);
    int offsetX = (inputSize - rgb.width()) / 2;
    int offsetY = (inputSize - rgb.height()) / 2;
    {
        QPainter painter(&padded);
        painter.drawImage(offsetX, offsetY, rgb);
        painter.end();
    }

    // Scale factors for mapping back to original coordinates
    float scaleX = static_cast<float>(frame.width()) / rgb.width();
    float scaleY = static_cast<float>(frame.height()) / rgb.height();

    // Convert to float tensor [1, 3, 640, 640]
    std::vector<float> inputTensor(1 * 3 * inputSize * inputSize);
    const uchar* pixels = padded.constBits();
    int stride = padded.bytesPerLine();
    for (int y = 0; y < inputSize; ++y) {
        for (int x = 0; x < inputSize; ++x) {
            int srcIdx = y * stride + x * 3;
            float r = (static_cast<float>(pixels[srcIdx + 0]) - 127.5f) / 128.0f;
            float g = (static_cast<float>(pixels[srcIdx + 1]) - 127.5f) / 128.0f;
            float b = (static_cast<float>(pixels[srcIdx + 2]) - 127.5f) / 128.0f;
            inputTensor[0 * inputSize * inputSize + y * inputSize + x] = r;
            inputTensor[1 * inputSize * inputSize + y * inputSize + x] = g;
            inputTensor[2 * inputSize * inputSize + y * inputSize + x] = b;
        }
    }

    // 2. Run ONNX inference
    auto memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    std::array<int64_t, 4> inputShape = {1, 3, inputSize, inputSize};
    Ort::Value inputOrt = Ort::Value::CreateTensor<float>(
        memoryInfo, inputTensor.data(), inputTensor.size(),
        inputShape.data(), inputShape.size());

    const char* inputName = "input.1";
    // SCRFD outputs: 9 tensors (3 strides x 3 outputs each)
    std::vector<const char*> outputNames = {
        "score_8", "score_16", "score_32",
        "bbox_8", "bbox_16", "bbox_32",
        "kps_8", "kps_16", "kps_32"
    };

    std::vector<Ort::Value> outputs;
    try {
        outputs = m_impl->session->Run(
            Ort::RunOptions{nullptr},
            &inputName, &inputOrt, 1,
            outputNames.data(), outputNames.size());
    } catch (const Ort::Exception& e) {
        qWarning() << "[AI] Detection inference failed:" << e.what();
        return results;
    }

    // 3. Postprocess: decode anchors for each stride
    const int strides[] = {8, 16, 32};

    struct RawDetection {
        float x1, y1, x2, y2, score;
        float landmarks[10];
    };
    std::vector<RawDetection> allDets;

    for (int s = 0; s < 3; ++s) {
        int gridSize = inputSize / strides[s];

        float* scores = outputs[s].GetTensorMutableData<float>();
        float* bboxes = outputs[s + 3].GetTensorMutableData<float>();
        float* kps = outputs[s + 6].GetTensorMutableData<float>();

        int numAnchors = gridSize * gridSize * 2; // 2 anchors per grid cell

        for (int i = 0; i < numAnchors; ++i) {
            float score = scores[i];
            if (score < threshold) continue;

            // Anchor position
            int anchorIdx = i / 2;
            int anchorY = anchorIdx / gridSize;
            int anchorX = anchorIdx % gridSize;
            float cx = (anchorX + 0.5f) * strides[s];
            float cy = (anchorY + 0.5f) * strides[s];

            // Decode bbox
            float dx = bboxes[i * 4 + 0] * strides[s];
            float dy = bboxes[i * 4 + 1] * strides[s];
            float dw = bboxes[i * 4 + 2] * strides[s];
            float dh = bboxes[i * 4 + 3] * strides[s];

            float x1 = cx - dx;
            float y1 = cy - dy;
            float x2 = cx + dw;
            float y2 = cy + dh;

            RawDetection det;
            det.x1 = x1; det.y1 = y1; det.x2 = x2; det.y2 = y2;
            det.score = score;

            // Decode landmarks
            for (int k = 0; k < 5; ++k) {
                det.landmarks[k * 2 + 0] = cx + kps[i * 10 + k * 2 + 0] * strides[s];
                det.landmarks[k * 2 + 1] = cy + kps[i * 10 + k * 2 + 1] * strides[s];
            }

            allDets.push_back(det);
        }
    }

    // 4. NMS (Non-Maximum Suppression)
    std::sort(allDets.begin(), allDets.end(), [](const RawDetection& a, const RawDetection& b) {
        return a.score > b.score;
    });

    std::vector<bool> suppressed(allDets.size(), false);
    float nmsThreshold = 0.4f;

    for (size_t i = 0; i < allDets.size(); ++i) {
        if (suppressed[i]) continue;
        for (size_t j = i + 1; j < allDets.size(); ++j) {
            if (suppressed[j]) continue;
            // IoU
            float xx1 = std::max(allDets[i].x1, allDets[j].x1);
            float yy1 = std::max(allDets[i].y1, allDets[j].y1);
            float xx2 = std::min(allDets[i].x2, allDets[j].x2);
            float yy2 = std::min(allDets[i].y2, allDets[j].y2);
            float w = std::max(0.0f, xx2 - xx1);
            float h = std::max(0.0f, yy2 - yy1);
            float inter = w * h;
            float areaI = (allDets[i].x2 - allDets[i].x1) * (allDets[i].y2 - allDets[i].y1);
            float areaJ = (allDets[j].x2 - allDets[j].x1) * (allDets[j].y2 - allDets[j].y1);
            float iou = inter / (areaI + areaJ - inter + 1e-6f);
            if (iou > nmsThreshold) suppressed[j] = true;
        }
    }

    // 5. Convert to output format (map back to original image coordinates)
    for (size_t i = 0; i < allDets.size(); ++i) {
        if (suppressed[i]) continue;
        const auto& d = allDets[i];

        DetectedFace face;
        face.score = d.score;
        face.bbox = QRectF(
            (d.x1 - offsetX) * scaleX,
            (d.y1 - offsetY) * scaleY,
            (d.x2 - d.x1) * scaleX,
            (d.y2 - d.y1) * scaleY
        );

        face.landmarks.resize(10);
        for (int k = 0; k < 5; ++k) {
            face.landmarks[k * 2 + 0] = (d.landmarks[k * 2 + 0] - offsetX) * scaleX;
            face.landmarks[k * 2 + 1] = (d.landmarks[k * 2 + 1] - offsetY) * scaleY;
        }

        results.append(face);
    }

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
