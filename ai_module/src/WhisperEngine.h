#pragma once
// ============================================================
// Prestige AI — Whisper Speech-to-Text Engine
// Copyright (c) 2024-2026 Prestige Technologie Company
//
// Audio capture via Qt Multimedia (QAudioSource, 16kHz mono).
// Speech-to-text via whisper.cpp when available, stub otherwise.
// Same #ifdef pattern as ONNX Runtime — compiles either way.
// ============================================================

#include <QObject>
#include <QString>
#include <QTimer>
#include <QByteArray>
#include <QFile>

// Forward declarations — Qt Multimedia may not be present on all CI
QT_BEGIN_NAMESPACE
class QAudioSource;
class QIODevice;
QT_END_NAMESPACE

namespace prestige { namespace ai {

class WhisperEngine : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool active READ isActive NOTIFY activeChanged)
    Q_PROPERTY(bool modelLoaded READ isModelLoaded NOTIFY modelLoadedChanged)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)

public:
    explicit WhisperEngine(QObject* parent = nullptr);
    ~WhisperEngine();

    bool initialize(const QString& modelPath);
    bool isActive() const { return m_active; }
    bool isModelLoaded() const { return m_modelLoaded; }
    QString language() const { return m_language; }
    void setLanguage(const QString& lang);

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();

signals:
    void activeChanged();
    void modelLoadedChanged();
    void languageChanged();
    void subtitleReady(const QString& text, const QString& language, double confidence);

private:
    void processAudioChunk();

    QAudioSource* m_audioSource = nullptr;
    QIODevice* m_audioDevice = nullptr;
    QTimer* m_processTimer = nullptr;
    QByteArray m_audioBuffer;

    bool m_active = false;
    bool m_modelLoaded = false;
    QString m_language = "fr";
    QString m_modelPath;

    // whisper.cpp handle (void* for forward compat)
    void* m_whisperCtx = nullptr;
};

}} // namespace prestige::ai
