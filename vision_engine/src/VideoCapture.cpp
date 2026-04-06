// ============================================================
// Prestige AI — Video Capture Implementation
// Captures real frames via Qt Multimedia QVideoSink
// ============================================================

#include "VideoCapture.h"

#include <QDateTime>
#include <QDebug>
#include <QMediaDevices>
#include <QCameraDevice>
#include <QPainter>
#include <QCoreApplication>
#include <QPermissions>
#include <QFont>

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

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    // Request camera permission on macOS/iOS (required since macOS 10.14)
    QCameraPermission cameraPermission;
    switch (qApp->checkPermission(cameraPermission)) {
    case Qt::PermissionStatus::Undetermined:
        qInfo() << "[VideoCapture] Requesting camera permission...";
        qApp->requestPermission(cameraPermission, this, [this, index](const QPermission& permission) {
            if (permission.status() == Qt::PermissionStatus::Granted) {
                qInfo() << "[VideoCapture] Camera permission GRANTED";
                openWebcam(index); // Retry after permission granted
            } else {
                qWarning() << "[VideoCapture] Camera permission DENIED by user";
            }
        });
        return false; // Will retry via callback
    case Qt::PermissionStatus::Denied:
        qWarning() << "[VideoCapture] Camera permission DENIED — open System Settings > Privacy > Camera";
        return false;
    case Qt::PermissionStatus::Granted:
        break; // Continue to open camera
    }
#endif

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
    m_currentType = "webcam";
    m_currentSource = "default";
    m_fpsTimer.restart();
    m_fpsFrameCount = 0;

    emit activeChanged(true);
    qInfo() << "[VideoCapture] Webcam opened";
    return true;
}

bool VideoCapture::openSource(const QString& type, const QString& source)
{
    // Skip if same source already active
    if (m_active && m_currentType == type && m_currentSource == source)
        return true;

    close();

    if (type == "webcam") {
        // Find camera by name, fallback to default
        QCameraDevice selectedDevice;
        const auto devices = QMediaDevices::videoInputs();
        for (const auto& dev : devices) {
            if (dev.description().contains(source, Qt::CaseInsensitive)) {
                selectedDevice = dev;
                break;
            }
        }

        m_session = new QMediaCaptureSession(this);
        m_sink = new QVideoSink(this);
        m_session->setVideoSink(m_sink);

        if (selectedDevice.isNull()) {
            // Default camera
            m_camera = new QCamera(this);
            qInfo() << "[VideoCapture] Opening default webcam";
        } else {
            m_camera = new QCamera(selectedDevice, this);
            qInfo() << "[VideoCapture] Opening webcam:" << selectedDevice.description();
        }

        m_session->setCamera(m_camera);

        // Audio passthrough
        m_audioInput = new QAudioInput(this);
        m_session->setAudioInput(m_audioInput);

        connect(m_sink, &QVideoSink::videoFrameChanged,
                this, &VideoCapture::onVideoFrameChanged);

        m_camera->start();
        m_active = true;
        m_currentType = type;
        m_currentSource = source;
        m_fpsTimer.restart();
        m_fpsFrameCount = 0;
        emit activeChanged(true);
        return true;
    }

    // DeckLink SDI capture
    if (type == "decklink") {
        if (!DecklinkCapture::isSdkAvailable()) {
            qWarning() << "[VideoCapture] DeckLink SDK not available — install Blackmagic Desktop Video";
            return false;
        }
        m_decklink = new DecklinkCapture(this);
        connect(m_decklink, &DecklinkCapture::frameReady, this, [this](const QImage& frame, qint64 ts) {
            m_frameId++;
            m_captureTimestampMs = ts;
            m_latestFrame = frame;
            m_resolution = frame.size();
            m_fpsFrameCount++;
            qint64 elapsed = m_fpsTimer.elapsed();
            if (elapsed >= 2000) { m_currentFps = m_fpsFrameCount * 1000.0 / elapsed; m_fpsFrameCount = 0; m_fpsTimer.restart(); emit statsUpdated(); }
            emit frameCaptured(frame, m_frameId, ts);
        });
        if (!m_decklink->open(source)) {
            qWarning() << "[VideoCapture] DeckLink open failed:" << source;
            delete m_decklink; m_decklink = nullptr;
            return false;
        }
        m_active = true; m_currentType = type; m_currentSource = source;
        m_fpsTimer.restart(); m_fpsFrameCount = 0;
        emit activeChanged(true);
        qInfo() << "[VideoCapture] DeckLink capture started:" << source;
        return true;
    }

    // AJA SDI capture
    if (type == "aja") {
        if (!AjaCapture::isSdkAvailable()) {
            qWarning() << "[VideoCapture] AJA SDK not available — install AJA NTV2 drivers";
            return false;
        }
        m_aja = new AjaCapture(this);
        connect(m_aja, &AjaCapture::frameReady, this, [this](const QImage& frame, qint64 ts) {
            m_frameId++;
            m_captureTimestampMs = ts;
            m_latestFrame = frame;
            m_resolution = frame.size();
            m_fpsFrameCount++;
            qint64 elapsed = m_fpsTimer.elapsed();
            if (elapsed >= 2000) { m_currentFps = m_fpsFrameCount * 1000.0 / elapsed; m_fpsFrameCount = 0; m_fpsTimer.restart(); emit statsUpdated(); }
            emit frameCaptured(frame, m_frameId, ts);
        });
        if (!m_aja->open(source)) {
            qWarning() << "[VideoCapture] AJA open failed:" << source;
            delete m_aja; m_aja = nullptr;
            return false;
        }
        m_active = true; m_currentType = type; m_currentSource = source;
        m_fpsTimer.restart(); m_fpsFrameCount = 0;
        emit activeChanged(true);
        qInfo() << "[VideoCapture] AJA capture started:" << source;
        return true;
    }

    // Magewell USB/PCIe capture
    if (type == "magewell") {
        if (!MagewellCapture::isSdkAvailable()) {
            qWarning() << "[VideoCapture] Magewell SDK not available — install Magewell drivers";
            return false;
        }
        m_magewell = new MagewellCapture(this);
        connect(m_magewell, &MagewellCapture::frameReady, this, [this](const QImage& frame, qint64 ts) {
            m_frameId++;
            m_captureTimestampMs = ts;
            m_latestFrame = frame;
            m_resolution = frame.size();
            m_fpsFrameCount++;
            qint64 elapsed = m_fpsTimer.elapsed();
            if (elapsed >= 2000) { m_currentFps = m_fpsFrameCount * 1000.0 / elapsed; m_fpsFrameCount = 0; m_fpsTimer.restart(); emit statsUpdated(); }
            emit frameCaptured(frame, m_frameId, ts);
        });
        if (!m_magewell->open(source)) {
            qWarning() << "[VideoCapture] Magewell open failed:" << source;
            delete m_magewell; m_magewell = nullptr;
            return false;
        }
        m_active = true; m_currentType = type; m_currentSource = source;
        m_fpsTimer.restart(); m_fpsFrameCount = 0;
        emit activeChanged(true);
        qInfo() << "[VideoCapture] Magewell capture started:" << source;
        return true;
    }

    // NDI network capture
    if (type == "ndi") {
        m_ndi = new NdiCapture(this);
        connect(m_ndi, &NdiCapture::frameReady, this, [this](const QImage& frame) {
            m_frameId++;
            m_captureTimestampMs = QDateTime::currentMSecsSinceEpoch();
            m_latestFrame = frame;
            m_resolution = frame.size();
            m_fpsFrameCount++;
            qint64 elapsed = m_fpsTimer.elapsed();
            if (elapsed >= 2000) { m_currentFps = m_fpsFrameCount * 1000.0 / elapsed; m_fpsFrameCount = 0; m_fpsTimer.restart(); emit statsUpdated(); }
            emit frameCaptured(frame, m_frameId, m_captureTimestampMs);
        });
        if (!m_ndi->connectSource(source)) {
            qWarning() << "[VideoCapture] NDI connect failed:" << source;
            delete m_ndi; m_ndi = nullptr;
            return false;
        }
        m_ndi->startCapture();
        m_active = true; m_currentType = type; m_currentSource = source;
        m_fpsTimer.restart(); m_fpsFrameCount = 0;
        emit activeChanged(true);
        qInfo() << "[VideoCapture] NDI capture started:" << source;
        return true;
    }

    // Test pattern — SMPTE color bars generated internally
    if (type == "test" || type == "test_pattern") {
        close();
        m_currentType = type;
        m_currentSource = "SMPTE Color Bars";
        m_active = true;
        m_resolution = {1920, 1080};
        m_fpsTimer.restart();
        m_fpsFrameCount = 0;
        emit activeChanged(true);

        // Generate SMPTE color bars at 25fps via QTimer
        m_testPatternTimer = new QTimer(this);
        connect(m_testPatternTimer, &QTimer::timeout, this, [this]() {
            QImage bars(1920, 1080, QImage::Format_RGB32);
            QPainter p(&bars);
            // SMPTE color bars: white, yellow, cyan, green, magenta, red, blue
            const QColor colors[] = {
                QColor(180, 180, 180), QColor(180, 180, 16), QColor(16, 180, 180),
                QColor(16, 180, 16), QColor(180, 16, 180), QColor(180, 16, 16), QColor(16, 16, 180)
            };
            int barW = 1920 / 7;
            for (int i = 0; i < 7; ++i)
                p.fillRect(i * barW, 0, barW + (i == 6 ? 1920 - 7 * barW : 0), 810, colors[i]);
            // Bottom strip: darker bars
            const QColor dark[] = {
                QColor(16, 16, 180), QColor(16, 16, 16), QColor(180, 16, 180),
                QColor(16, 16, 16), QColor(16, 180, 180), QColor(16, 16, 16), QColor(180, 180, 180)
            };
            for (int i = 0; i < 7; ++i)
                p.fillRect(i * barW, 810, barW + (i == 6 ? 1920 - 7 * barW : 0), 270, dark[i]);
            // Label
            p.setPen(Qt::white);
            p.setFont(QFont("Menlo", 24, QFont::Bold));
            p.drawText(QRect(0, 900, 1920, 100), Qt::AlignCenter, "PRESTIGE AI — TEST PATTERN");
            p.end();

            m_frameId++;
            m_captureTimestampMs = QDateTime::currentMSecsSinceEpoch();
            m_latestFrame = bars;
            m_fpsFrameCount++;
            qint64 elapsed = m_fpsTimer.elapsed();
            if (elapsed >= 2000) { m_currentFps = m_fpsFrameCount * 1000.0 / elapsed; m_fpsFrameCount = 0; m_fpsTimer.restart(); emit statsUpdated(); }
            emit frameCaptured(bars, m_frameId, m_captureTimestampMs);
        });
        m_testPatternTimer->start(40); // 25fps
        qInfo() << "[VideoCapture] Test pattern started (SMPTE 1920x1080 @25fps)";
        return true;
    }

    qWarning() << "[VideoCapture] Unknown source type:" << type;
    return false;
}

void VideoCapture::close()
{
    // Stop test pattern timer if running
    if (m_testPatternTimer) {
        m_testPatternTimer->stop();
        delete m_testPatternTimer;
        m_testPatternTimer = nullptr;
    }

    if (m_camera) {
        m_camera->stop();
        delete m_camera;
        m_camera = nullptr;
    }
    delete m_session; m_session = nullptr;
    delete m_sink;    m_sink = nullptr;

    // Close hardware capture cards
    if (m_decklink) { m_decklink->close();      delete m_decklink; m_decklink = nullptr; }
    if (m_aja)      { m_aja->close();           delete m_aja;      m_aja = nullptr; }
    if (m_magewell) { m_magewell->close();      delete m_magewell; m_magewell = nullptr; }
    if (m_ndi)      { m_ndi->stopCapture(); m_ndi->disconnect(); delete m_ndi; m_ndi = nullptr; }

    m_active = false;
    m_currentType.clear();
    m_currentSource.clear();
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

// ── Deinterlace (Bob method — SMPTE RP 220) ─────────────────
QImage VideoCapture::deinterlace(const QImage& interlacedFrame, bool topFieldFirst) {
    if (interlacedFrame.isNull()) return interlacedFrame;
    int w = interlacedFrame.width(), h = interlacedFrame.height();
    QImage result(w, h, interlacedFrame.format());

    // Bob deinterlacing: take one field and interpolate missing lines
    int startLine = topFieldFirst ? 0 : 1;
    for (int y = 0; y < h; ++y) {
        int srcY;
        if (y % 2 == startLine) {
            // This field's line — copy directly
            srcY = y;
        } else {
            // Missing line — average of above and below
            srcY = y; // Will be interpolated below
        }
        memcpy(result.scanLine(y), interlacedFrame.constScanLine(srcY), interlacedFrame.bytesPerLine());
    }

    // Interpolate missing field lines
    for (int y = (startLine == 0 ? 1 : 0); y < h; y += 2) {
        if (y == 0 || y >= h - 1) continue;
        const uchar* above = interlacedFrame.constScanLine(y - 1);
        const uchar* below = interlacedFrame.constScanLine(y + 1);
        uchar* dst = result.scanLine(y);
        int bytes = interlacedFrame.bytesPerLine();
        for (int b = 0; b < bytes; ++b) {
            dst[b] = static_cast<uchar>((above[b] + below[b]) / 2);
        }
    }
    return result;
}

} // namespace prestige
