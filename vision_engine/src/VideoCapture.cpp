// ============================================================
// Prestige AI — Video Capture Implementation
// Captures real frames via Qt Multimedia QVideoSink
// ============================================================

#include "VideoCapture.h"

#include <QDateTime>
#include <QDebug>

namespace prestige {

VideoCapture::VideoCapture(QObject* parent)
    : QObject(parent)
{
    m_fpsTimer.start();
}

VideoCapture::~VideoCapture()
{
    close();
}

bool VideoCapture::openWebcam(int index)
{
    Q_UNUSED(index)
    close();

    m_camera = new QCamera(this);
    m_session = new QMediaCaptureSession(this);
    m_sink = new QVideoSink(this);

    m_session->setCamera(m_camera);
    m_session->setVideoSink(m_sink);

    // Audio passthrough — capture audio alongside video
    m_audioInput = new QAudioInput(this);
    m_session->setAudioInput(m_audioInput);
    qInfo() << "[VideoCapture] Audio input enabled";

    // Connect the sink to receive every frame
    connect(m_sink, &QVideoSink::videoFrameChanged,
            this, &VideoCapture::onVideoFrameChanged);

    m_camera->start();
    m_active = true;
    m_fpsTimer.restart();
    m_fpsFrameCount = 0;

    emit activeChanged(true);
    qInfo() << "[VideoCapture] Webcam opened";
    return true;
}

void VideoCapture::close()
{
    if (m_camera) {
        m_camera->stop();
        delete m_camera;
        m_camera = nullptr;
    }
    delete m_session; m_session = nullptr;
    delete m_sink;    m_sink = nullptr;

    m_active = false;
    emit activeChanged(false);
}

QImage VideoCapture::latestFrame() const
{
    return m_latestFrame;
}

void VideoCapture::onVideoFrameChanged(const QVideoFrame& vframe)
{
    QVideoFrame frame = vframe;
    if (!frame.isValid())
        return;

    if (!frame.map(QVideoFrame::ReadOnly))
        return;

    QImage image = frame.toImage();
    frame.unmap();

    if (image.isNull())
        return;

    // Convert to RGB32 for consistency
    if (image.format() != QImage::Format_RGB32 && image.format() != QImage::Format_ARGB32) {
        image = image.convertToFormat(QImage::Format_RGB32);
    }

    m_frameId++;
    m_captureTimestampMs = QDateTime::currentMSecsSinceEpoch();
    m_latestFrame = image;
    m_resolution = image.size();

    // FPS calculation
    m_fpsFrameCount++;
    qint64 elapsed = m_fpsTimer.elapsed();
    if (elapsed >= 2000) {
        m_currentFps = m_fpsFrameCount * 1000.0 / elapsed;
        m_fpsFrameCount = 0;
        m_fpsTimer.restart();
        emit statsUpdated();
    }

    // Emit to pipeline — FrameSender and OutputRouter connect to this
    emit frameCaptured(image, m_frameId, m_captureTimestampMs);
}

} // namespace prestige
