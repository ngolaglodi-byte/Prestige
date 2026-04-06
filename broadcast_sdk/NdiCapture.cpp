// ============================================================
// Prestige AI — NDI Capture Implementation
// ============================================================

#include "NdiCapture.h"
#include <QDebug>
#include <QElapsedTimer>

namespace prestige {

NdiCapture::NdiCapture(QObject* parent)
    : QObject(parent)
{
#ifdef PRESTIGE_HAVE_NDI
    if (!NDIlib_initialize()) {
        qCritical() << "[NDI] Failed to initialize NDI runtime library";
        emit errorOccurred("NDI runtime initialization failed");
    } else {
        qInfo() << "[NDI] Runtime initialized successfully";
    }
#else
    qInfo() << "[NDI] Built without NDI SDK — stub mode";
#endif
}

NdiCapture::~NdiCapture()
{
    stopCapture();
    disconnect();
#ifdef PRESTIGE_HAVE_NDI
    NDIlib_destroy();
#endif
}

QStringList NdiCapture::discoverSources(int timeoutMs)
{
    m_availableSources.clear();

#ifdef PRESTIGE_HAVE_NDI
    // Create finder if not exists
    if (!m_findInstance) {
        NDIlib_find_create_t findDesc;
        findDesc.show_local_sources = true;
        findDesc.p_groups = nullptr;
        findDesc.p_extra_ips = nullptr;
        m_findInstance = NDIlib_find_create_v2(&findDesc);

        if (!m_findInstance) {
            emit errorOccurred("Failed to create NDI finder");
            return m_availableSources;
        }
    }

    // Wait for sources to appear on network
    NDIlib_find_wait_for_sources(m_findInstance, timeoutMs);

    uint32_t count = 0;
    const NDIlib_source_t* ndiSources = NDIlib_find_get_current_sources(m_findInstance, &count);

    for (uint32_t i = 0; i < count; ++i) {
        QString name = QString::fromUtf8(ndiSources[i].p_ndi_name);
        m_availableSources.append(name);
        qInfo() << "[NDI] Found source:" << name;
    }

    qInfo() << "[NDI] Discovery complete:" << count << "sources found";
#else
    Q_UNUSED(timeoutMs)
    qWarning() << "[NDI] SDK not available — no sources";
#endif

    emit sourcesChanged();
    return m_availableSources;
}

void NdiCapture::refreshSources()
{
    discoverSources(2000);
}

bool NdiCapture::connectSource(const QString& sourceName)
{
#ifdef PRESTIGE_HAVE_NDI
    // Disconnect existing
    if (m_recvInstance) {
        disconnect();
    }

    NDIlib_recv_create_v3_t recvDesc;
    recvDesc.source_to_connect_to.p_ndi_name = nullptr;
    recvDesc.source_to_connect_to.p_url_address = nullptr;
    recvDesc.color_format = NDIlib_recv_color_format_RGBX_RGBA;
    recvDesc.bandwidth = NDIlib_recv_bandwidth_highest;
    recvDesc.allow_video_fields = false;
    recvDesc.p_ndi_recv_name = "Prestige AI";

    m_recvInstance = NDIlib_recv_create_v3(&recvDesc);
    if (!m_recvInstance) {
        emit errorOccurred("Failed to create NDI receiver");
        return false;
    }

    // Find the source by name from our last discovery
    if (!m_findInstance) {
        discoverSources(2000);
    }

    uint32_t count = 0;
    const NDIlib_source_t* ndiSources = NDIlib_find_get_current_sources(m_findInstance, &count);
    const NDIlib_source_t* targetSource = nullptr;

    QByteArray nameUtf8 = sourceName.toUtf8();
    for (uint32_t i = 0; i < count; ++i) {
        if (nameUtf8 == ndiSources[i].p_ndi_name) {
            targetSource = &ndiSources[i];
            break;
        }
    }

    if (!targetSource) {
        // Try connecting by name directly
        NDIlib_source_t src;
        src.p_ndi_name = nameUtf8.constData();
        src.p_url_address = nullptr;
        NDIlib_recv_connect(m_recvInstance, &src);
    } else {
        NDIlib_recv_connect(m_recvInstance, targetSource);
    }

    m_connected = true;
    emit connectionChanged(true);
    qInfo() << "[NDI] Connected to:" << sourceName;
    return true;
#else
    Q_UNUSED(sourceName)
    emit errorOccurred("NDI SDK not available");
    return false;
#endif
}

bool NdiCapture::disconnect()
{
    stopCapture();

    if (!m_connected)
        return true;

#ifdef PRESTIGE_HAVE_NDI
    if (m_recvInstance) {
        NDIlib_recv_destroy(m_recvInstance);
        m_recvInstance = nullptr;
    }
#endif

    m_connected = false;
    emit connectionChanged(false);
    qInfo() << "[NDI] Disconnected";
    return true;
}

void NdiCapture::startCapture()
{
    if (m_capturing || !m_connected)
        return;

    m_capturing = true;

    QObject* worker = new QObject;
    worker->moveToThread(&m_captureThread);

    connect(&m_captureThread, &QThread::started, worker, [this, worker]() {
        captureLoop();
        worker->deleteLater();
    });
    connect(&m_captureThread, &QThread::finished, worker, &QObject::deleteLater);

    m_captureThread.start();
    m_captureThread.setPriority(QThread::HighPriority);
    qInfo() << "[NDI] Capture started";
}

void NdiCapture::stopCapture()
{
    m_capturing = false;
    if (m_captureThread.isRunning()) {
        m_captureThread.quit();
        m_captureThread.wait(2000);
    }
}

void NdiCapture::captureLoop()
{
#ifdef PRESTIGE_HAVE_NDI
    if (!m_recvInstance)
        return;

    QElapsedTimer fpsTimer;
    fpsTimer.start();
    int frameCount = 0;

    while (m_capturing) {
        NDIlib_video_frame_v2_t videoFrame;
        NDIlib_audio_frame_v2_t audioFrame;
        NDIlib_metadata_frame_t metadataFrame;

        auto frameType = NDIlib_recv_capture_v2(m_recvInstance, &videoFrame, &audioFrame, &metadataFrame, 100);

        switch (frameType) {
        case NDIlib_frame_type_video: {
            // Update resolution info
            m_resolution = QSize(videoFrame.xres, videoFrame.yres);
            m_fps = static_cast<int>(videoFrame.frame_rate_N / videoFrame.frame_rate_D);

            // Convert to QImage (RGBX format)
            QImage frame(videoFrame.p_data,
                         videoFrame.xres, videoFrame.yres,
                         videoFrame.line_stride_in_bytes,
                         QImage::Format_RGBX8888);

            // Deep copy before freeing NDI buffer
            QImage copy = frame.copy();
            emit frameReady(copy);

            NDIlib_recv_free_video_v2(m_recvInstance, &videoFrame);

            frameCount++;
            m_stats.framesCapture++;
            break;
        }
        case NDIlib_frame_type_audio:
            // Forward audio data before freeing (PCM float planar → interleaved)
            if (audioFrame.p_data && audioFrame.no_samples > 0) {
                int samples = audioFrame.no_samples;
                int channels = audioFrame.no_channels;
                QByteArray pcmData(samples * channels * sizeof(float), 0);
                float* dst = reinterpret_cast<float*>(pcmData.data());
                const float* src = reinterpret_cast<const float*>(audioFrame.p_data);
                // Interleave channels
                for (int s = 0; s < samples; ++s) {
                    for (int c = 0; c < channels; ++c) {
                        dst[s * channels + c] = src[c * audioFrame.channel_stride_in_bytes / sizeof(float) + s];
                    }
                }
                emit audioReady(pcmData, audioFrame.sample_rate, channels);
            }
            NDIlib_recv_free_audio_v2(m_recvInstance, &audioFrame);
            break;
        case NDIlib_frame_type_metadata:
            NDIlib_recv_free_metadata(m_recvInstance, &metadataFrame);
            break;
        case NDIlib_frame_type_error:
            qWarning() << "[NDI] Receive error — attempting reconnection in 3s";
            emit connectionLost();
            // Auto-reconnect after 3 seconds (instead of dying permanently)
            QThread::msleep(3000);
            if (m_capturing && m_recvInstance) {
                qInfo() << "[NDI] Attempting reconnect to" << m_sourceName;
                // NDI receiver automatically reconnects if source reappears
                continue;  // Re-enter receive loop
            }
            m_capturing = false;
            break;
        default:
            break;
        }

        // FPS stats every 2 seconds
        if (fpsTimer.elapsed() >= 2000) {
            double fps = frameCount * 1000.0 / fpsTimer.elapsed();
            m_stats.currentFps = fps;
            emit statsUpdated(fps, 0);
            frameCount = 0;
            fpsTimer.restart();
        }
    }
#endif
}

} // namespace prestige
