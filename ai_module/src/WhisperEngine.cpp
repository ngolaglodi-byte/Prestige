// ============================================================
// Prestige AI — Whisper Speech-to-Text Engine
// Copyright (c) 2024-2026 Prestige Technologie Company
// ============================================================

#include "WhisperEngine.h"
#include <QAudioSource>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QAudioFormat>
#include <QIODevice>
#include <QDebug>

namespace prestige { namespace ai {

WhisperEngine::WhisperEngine(QObject* parent) : QObject(parent)
{
    m_processTimer = new QTimer(this);
    m_processTimer->setInterval(3000); // Process every 3 seconds
    connect(m_processTimer, &QTimer::timeout, this, &WhisperEngine::processAudioChunk);
}

WhisperEngine::~WhisperEngine()
{
    stop();
}

bool WhisperEngine::initialize(const QString& modelPath)
{
    m_modelPath = modelPath;

    // TODO: Load whisper.cpp model when available
    // For now, check if model file exists
    if (QFile::exists(modelPath)) {
        qInfo() << "[Whisper] Model file found:" << modelPath;
        // Will be loaded when whisper.cpp is integrated
        // m_modelLoaded = true;
    } else {
        qInfo() << "[Whisper] Model file not found:" << modelPath;
    }

    qInfo() << "[Whisper] Engine initialized (speech-to-text ready when model is loaded)";
    return true;
}

void WhisperEngine::setLanguage(const QString& lang)
{
    if (m_language == lang) return;
    m_language = lang;
    emit languageChanged();
}

void WhisperEngine::start()
{
    if (m_active) return;

    // Setup audio capture: 16kHz mono 16-bit (whisper.cpp native format)
    QAudioFormat format;
    format.setSampleRate(16000);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    QAudioDevice inputDevice = QMediaDevices::defaultAudioInput();
    if (inputDevice.isNull()) {
        qWarning() << "[Whisper] No audio input device available";
        return;
    }

    if (!inputDevice.isFormatSupported(format)) {
        qWarning() << "[Whisper] Audio format not supported, using nearest";
    }

    m_audioSource = new QAudioSource(inputDevice, format, this);
    m_audioDevice = m_audioSource->start();

    if (m_audioDevice) {
        connect(m_audioDevice, &QIODevice::readyRead, this, [this]() {
            m_audioBuffer.append(m_audioDevice->readAll());
        });
        m_processTimer->start();
        m_active = true;
        emit activeChanged();
        qInfo() << "[Whisper] Audio capture started -- listening on:" << inputDevice.description();
    } else {
        qWarning() << "[Whisper] Failed to start audio capture";
        delete m_audioSource;
        m_audioSource = nullptr;
    }
}

void WhisperEngine::stop()
{
    if (!m_active) return;

    m_processTimer->stop();
    if (m_audioSource) {
        m_audioSource->stop();
        delete m_audioSource;
        m_audioSource = nullptr;
        m_audioDevice = nullptr;
    }
    m_audioBuffer.clear();
    m_active = false;
    emit activeChanged();
    qInfo() << "[Whisper] Audio capture stopped";
}

void WhisperEngine::processAudioChunk()
{
    if (m_audioBuffer.isEmpty()) return;

    // TODO: When whisper.cpp is integrated:
    // 1. Convert m_audioBuffer to float32 array
    // 2. Run whisper_full()
    // 3. Extract text segments
    // 4. Emit subtitleReady(text, language, confidence)

    // For now, just clear the buffer (audio is captured but not transcribed)
    int samples = m_audioBuffer.size() / 2; // 16-bit = 2 bytes per sample
    m_audioBuffer.clear();

    // When model is loaded, this will emit real subtitles
    // emit subtitleReady("transcribed text", m_language, 0.95);

    Q_UNUSED(samples)
}

}} // namespace prestige::ai
