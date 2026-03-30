// ============================================================
// Prestige AI — Audio Meter Implementation
// ============================================================

#include "AudioMeter.h"
#include <QMediaDevices>
#include <QAudioDevice>
#include <QAudioFormat>
#include <QDebug>
#include <cmath>

namespace prestige {

AudioMeter::AudioMeter(QObject* parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
{
    connect(m_timer, &QTimer::timeout, this, &AudioMeter::processAudio);
}

AudioMeter::~AudioMeter()
{
    stop();
}

void AudioMeter::start()
{
    if (m_active) return;

    QAudioDevice inputDevice = QMediaDevices::defaultAudioInput();
    if (inputDevice.isNull()) {
        qWarning() << "[AudioMeter] No audio input device available";
        return;
    }

    QAudioFormat format;
    format.setSampleRate(16000);
    format.setChannelCount(2);
    format.setSampleFormat(QAudioFormat::Int16);

    if (!inputDevice.isFormatSupported(format)) {
        // Fall back to mono
        format.setChannelCount(1);
        if (!inputDevice.isFormatSupported(format)) {
            qWarning() << "[AudioMeter] Audio format not supported";
            return;
        }
    }

    m_source = new QAudioSource(inputDevice, format, this);
    m_device = m_source->start();

    if (!m_device) {
        qWarning() << "[AudioMeter] Failed to start audio capture";
        delete m_source;
        m_source = nullptr;
        return;
    }

    m_active = true;
    m_timer->start(30);
    emit activeChanged();
    qInfo() << "[AudioMeter] Started — channels:" << format.channelCount();
}

void AudioMeter::stop()
{
    if (!m_active) return;

    m_timer->stop();
    if (m_source) {
        m_source->stop();
        delete m_source;
        m_source = nullptr;
    }
    m_device = nullptr;
    m_active = false;
    m_levelL = 0;
    m_levelR = 0;
    m_peakL = 0;
    m_peakR = 0;
    emit levelChanged();
    emit activeChanged();
    qInfo() << "[AudioMeter] Stopped";
}

void AudioMeter::processAudio()
{
    if (!m_device || !m_source) return;

    QByteArray data = m_device->readAll();
    if (data.isEmpty()) {
        // Decay peaks even without data
        m_peakL *= 0.95;
        m_peakR *= 0.95;
        m_levelL *= 0.8;
        m_levelR *= 0.8;
        emit levelChanged();
        return;
    }

    const auto* samples = reinterpret_cast<const qint16*>(data.constData());
    int sampleCount = data.size() / static_cast<int>(sizeof(qint16));
    int channels = m_source->format().channelCount();

    double sumL = 0, sumR = 0;
    int countL = 0, countR = 0;

    for (int i = 0; i < sampleCount; ++i) {
        double val = std::abs(static_cast<double>(samples[i])) / 32768.0;
        if (channels == 1) {
            sumL += val * val;
            sumR += val * val;
            countL++;
            countR++;
        } else {
            if (i % 2 == 0) {
                sumL += val * val;
                countL++;
            } else {
                sumR += val * val;
                countR++;
            }
        }
    }

    double rmsL = countL > 0 ? std::sqrt(sumL / countL) : 0.0;
    double rmsR = countR > 0 ? std::sqrt(sumR / countR) : 0.0;

    // Scale to 0.0-1.0 range (RMS of speech is typically 0.01-0.3)
    m_levelL = qBound(0.0, rmsL * 3.0, 1.0);
    m_levelR = qBound(0.0, rmsR * 3.0, 1.0);

    // Peak hold with decay
    if (m_levelL > m_peakL) m_peakL = m_levelL;
    else m_peakL *= 0.95;

    if (m_levelR > m_peakR) m_peakR = m_levelR;
    else m_peakR *= 0.95;

    emit levelChanged();
}

} // namespace prestige
