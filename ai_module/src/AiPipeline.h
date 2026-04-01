#pragma once

// ============================================================
// Prestige AI — AI Pipeline (orchestrates detection + tracking)
// ============================================================

#include <QObject>
#include <QImage>
#include <QThread>
#include <QMutex>
#include <QList>
#include "FaceDetector.h"
#include "FaceRecognizer.h"
#include "FaceTracker.h"
#include "TalentDatabase.h"

namespace prestige { namespace ai {

class AiPipeline : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool aiReady READ isAiReady NOTIFY aiReadyChanged)
    Q_PROPERTY(bool detecting READ isDetecting NOTIFY detectingChanged)

public:
    explicit AiPipeline(QObject* parent = nullptr);
    ~AiPipeline() override;

    bool initialize(const QString& modelsDir, const QString& talentsDbPath);
    bool isAiReady() const { return m_ready; }
    bool isDetecting() const { return m_detecting; }

    TalentDatabase* talentDatabase() { return &m_talentDb; }

public slots:
    // Called with each captured frame
    void processFrame(const QImage& frame, qint64 frameId, qint64 timestampMs);

signals:
    void aiReadyChanged();
    void detectingChanged();
    // Emitted when detection results are available
    void detectionsUpdated(const QList<prestige::ai::TrackedFace>& faces,
                           const QString& sceneType, int faceCount);
    void subtitleReady(const QString& text, const QString& language, double confidence);

private:
    void detectionLoop();

    FaceDetector m_detector;
    FaceRecognizer m_recognizer;
    FaceTracker m_tracker;
    TalentDatabase m_talentDb;

    QThread m_detThread;
    QMutex m_frameMutex;
    QImage m_latestFrame;
    qint64 m_latestFrameId = 0;

    bool m_ready = false;
    bool m_detecting = false;
    bool m_running = false;
};

}} // namespace prestige::ai
