// ============================================================
// Prestige AI — Whisper Speech-to-Text Engine
// Real-time transcription using whisper.cpp (C++ native)
// No Python dependency — runs directly in the Vision Engine process
// Copyright (c) 2024-2026 Prestige Technologie Company
// ============================================================

#include "WhisperEngine.h"
#include <QAudioSource>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QAudioFormat>
#include <QIODevice>
#include <QDebug>
#include <QFile>

#ifdef PRESTIGE_HAVE_WHISPER
#include "whisper.h"
#endif

namespace prestige { namespace ai {

WhisperEngine::WhisperEngine(QObject* parent) : QObject(parent)
{
    m_processTimer = new QTimer(this);
    m_processTimer->setInterval(3000); // Process every 3 seconds (matches Python Whisper chunk size)
    connect(m_processTimer, &QTimer::timeout, this, &WhisperEngine::processAudioChunk);
}

WhisperEngine::~WhisperEngine()
{
    stop();
#ifdef PRESTIGE_HAVE_WHISPER
    if (m_whisperCtx) {
        whisper_free(m_whisperCtx);
        m_whisperCtx = nullptr;
    }
#endif
}

bool WhisperEngine::initialize(const QString& modelPath)
{
    m_modelPath = modelPath;

#ifdef PRESTIGE_HAVE_WHISPER
    if (!QFile::exists(modelPath)) {
        qWarning() << "[Whisper] Model file not found:" << modelPath;
        qInfo() << "[Whisper] Engine initialized in stub mode (no model)";
        return true;
    }

    // Load whisper.cpp model
    struct whisper_context_params cparams = whisper_context_default_params();
    m_whisperCtx = whisper_init_from_file_with_params(
        modelPath.toUtf8().constData(), cparams);

    if (m_whisperCtx) {
        m_modelLoaded = true;
        emit modelLoadedChanged();
        qInfo() << "[Whisper] Model loaded successfully:" << modelPath;
    } else {
        qWarning() << "[Whisper] Failed to load model:" << modelPath;
    }
#else
    if (QFile::exists(modelPath)) {
        qInfo() << "[Whisper] Model file found but whisper.cpp not compiled in:" << modelPath;
    }
    qInfo() << "[Whisper] Engine initialized (stub — rebuild with whisper.cpp for transcription)";
#endif

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
            QByteArray data = m_audioDevice->readAll();
            m_audioBuffer.append(data);
            // Cap buffer at 30 seconds (16000 samples/s * 2 bytes * 30 = 960KB)
            static constexpr int MAX_BUFFER = 16000 * 2 * 30;
            if (m_audioBuffer.size() > MAX_BUFFER) {
                m_audioBuffer.remove(0, m_audioBuffer.size() - MAX_BUFFER);
            }
        });
        m_processTimer->start();
        m_active = true;
        emit activeChanged();
        qInfo() << "[Whisper] Audio capture started — listening on:" << inputDevice.description();
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

    // Minimum 1 second of audio needed (16000 samples * 2 bytes = 32000 bytes)
    if (m_audioBuffer.size() < 32000) return;

#ifdef PRESTIGE_HAVE_WHISPER
    if (!m_modelLoaded || !m_whisperCtx) {
        m_audioBuffer.clear();
        return;
    }

    // Convert S16 PCM buffer to float32 (whisper.cpp native format)
    int numSamples = m_audioBuffer.size() / 2;
    std::vector<float> pcmf32(numSamples);
    const int16_t* pcm16 = reinterpret_cast<const int16_t*>(m_audioBuffer.constData());
    for (int i = 0; i < numSamples; ++i) {
        pcmf32[i] = static_cast<float>(pcm16[i]) / 32768.0f;
    }

    // Configure whisper inference
    struct whisper_full_params wparams = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    wparams.print_progress   = false;
    wparams.print_special    = false;
    wparams.print_realtime   = false;
    wparams.print_timestamps = false;
    wparams.single_segment   = true;   // Single segment for real-time
    wparams.no_timestamps    = true;
    wparams.n_threads        = 4;      // Use 4 threads for inference

    // Language setting
    if (m_language == "auto" || m_language.isEmpty()) {
        wparams.language = "auto";
        wparams.detect_language = true;
    } else {
        QByteArray langBytes = m_language.toUtf8();
        wparams.language = langBytes.constData();
        wparams.detect_language = false;
    }

    // Run inference
    int result = whisper_full(m_whisperCtx, wparams, pcmf32.data(), static_cast<int>(pcmf32.size()));

    if (result == 0) {
        int nSegments = whisper_full_n_segments(m_whisperCtx);
        for (int i = 0; i < nSegments; ++i) {
            const char* text = whisper_full_get_segment_text(m_whisperCtx, i);
            if (text && text[0] != '\0') {
                QString transcription = QString::fromUtf8(text).trimmed();
                if (!transcription.isEmpty() && transcription != m_lastText) {
                    m_lastText = transcription;

                    // Detect language if auto
                    QString detectedLang = m_language;
                    if (wparams.detect_language) {
                        int langId = whisper_full_lang_id(m_whisperCtx);
                        if (langId >= 0) {
                            detectedLang = QString::fromUtf8(whisper_lang_str(langId));
                        }
                    }

                    emit subtitleReady(transcription, detectedLang, 0.92);
                }
            }
        }
    } else {
        qWarning() << "[Whisper] Inference failed with code:" << result;
    }

    // Keep 0.5s overlap for context continuity (same as Python implementation)
    int overlapBytes = 16000 * 2; // 0.5 seconds
    if (m_audioBuffer.size() > overlapBytes) {
        m_audioBuffer = m_audioBuffer.right(overlapBytes);
    } else {
        m_audioBuffer.clear();
    }

#else
    // Stub mode: discard audio
    m_audioBuffer.clear();
#endif
}

}} // namespace prestige::ai
