#pragma once

// ============================================================
// Prestige AI — Face Recognizer (ArcFace 512-d embeddings)
// ============================================================

#include <QImage>
#include <QString>
#include <vector>

namespace prestige { namespace ai {

class FaceRecognizer {
public:
    FaceRecognizer();
    ~FaceRecognizer();

    bool loadModel(const QString& modelPath); // w600k_r50.onnx
    bool isLoaded() const { return m_loaded; }

    // Extract 512-d embedding from aligned face crop
    std::vector<float> extractEmbedding(const QImage& alignedFace);

    // Compare two embeddings (cosine similarity)
    static float cosineSimilarity(const std::vector<float>& a, const std::vector<float>& b);

private:
    struct Impl;
    Impl* m_impl = nullptr;
    bool m_loaded = false;
};

}} // namespace prestige::ai
