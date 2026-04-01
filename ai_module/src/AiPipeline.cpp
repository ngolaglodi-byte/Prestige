// ============================================================
// Prestige AI — AI Pipeline Implementation
// Orchestrates detection thread + tracking at ~5fps,
// prediction at render rate (30fps).
// ============================================================

#include "AiPipeline.h"
#include <QDir>
#include <QDebug>
#include <QTimer>

namespace prestige { namespace ai {

AiPipeline::AiPipeline(QObject* parent)
    : QObject(parent)
    , m_tracker(0.3f)
{
}

AiPipeline::~AiPipeline()
{
    m_running = false;
    if (m_detThread.isRunning()) {
        m_detThread.quit();
        m_detThread.wait(3000);
    }
}

bool AiPipeline::initialize(const QString& modelsDir, const QString& talentsDbPath)
{
    // Load talent database
    if (!talentsDbPath.isEmpty()) {
        m_talentDb.setDbPath(talentsDbPath);
        m_talentDb.load();
    }

    // Try loading ONNX models (will fail gracefully in stub mode)
    bool detLoaded = m_detector.loadModel(modelsDir + "/det_10g.onnx");
    bool recLoaded = m_recognizer.loadModel(modelsDir + "/w600k_r50.onnx");

    m_ready = detLoaded && recLoaded;
    emit aiReadyChanged();

    if (m_ready) {
        // Start the detection thread
        m_running = true;
        QObject* worker = new QObject;
        worker->moveToThread(&m_detThread);
        connect(&m_detThread, &QThread::started, worker, [this, worker]() {
            detectionLoop();
            worker->deleteLater();
        });
        connect(&m_detThread, &QThread::finished, worker, &QObject::deleteLater);
        m_detThread.start();
        qInfo() << "[AI] Pipeline initialized — detection thread STARTED, models from:" << modelsDir;
    } else {
        qInfo() << "[AI] Pipeline initialized in stub mode (ONNX models not available)";
    }

    return true; // Always succeed — stub mode is valid
}

void AiPipeline::processFrame(const QImage& frame, qint64 frameId, qint64 timestampMs)
{
    Q_UNUSED(timestampMs)

    if (!m_ready) return;

    // Store latest frame for detection thread
    QMutexLocker locker(&m_frameMutex);
    m_latestFrame = frame;
    m_latestFrameId = frameId;
}

void AiPipeline::detectionLoop()
{
    // This runs on m_detThread at ~5fps
    while (m_running) {
        QImage frame;
        {
            QMutexLocker locker(&m_frameMutex);
            if (m_latestFrame.isNull()) {
                // No frame available yet, wait and retry
                QThread::msleep(50);
                continue;
            }
            frame = m_latestFrame.copy();
        }

        m_detecting = true;
        emit detectingChanged();

        // Step 1: Detect faces
        auto detections = m_detector.detect(frame, 0.5f);

        // Step 2+3: For each detection, align face, extract embedding, match talent
        QList<TrackedFace> faces;
        for (const auto& det : detections) {
            TrackedFace face;
            face.bbox = det.bbox;
            face.confidence = det.score;

            // Align face using detected landmarks
            QImage aligned = FaceRecognizer::alignFace(frame, det.landmarks);

            // Extract embedding
            auto embedding = m_recognizer.extractEmbedding(aligned);

            // Match against talent database
            if (!embedding.empty()) {
                auto match = m_talentDb.matchEmbedding(embedding, 0.4f);
                if (match.confidence > 0.0f) {
                    face.id = match.id;
                    face.name = match.name;
                    face.role = match.role;
                    face.confidence = match.confidence;
                    face.showOverlay = match.showOverlay;
                    face.overlayStyle = match.overlayStyle;
                    face.matched = true;
                }
            }

            faces.append(face);
        }

        // Step 4: Update tracker
        m_tracker.updateDetections(faces);

        // Step 5: Get predicted positions and emit
        auto tracked = m_tracker.predict();

        QString sceneType = "empty";
        if (tracked.size() == 1) sceneType = "single";
        else if (tracked.size() > 1) sceneType = "multi";

        emit detectionsUpdated(tracked, sceneType, tracked.size());

        m_detecting = false;
        emit detectingChanged();

        // Sleep to maintain ~5fps detection rate
        QThread::msleep(200);
    }
}

}} // namespace prestige::ai
