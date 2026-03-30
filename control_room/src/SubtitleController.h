#pragma once

// ============================================================
// Prestige AI — Subtitle Controller
// Receives real-time subtitles from Whisper AI Engine via ZMQ.
// Exposes subtitle text and status to QML.
// ============================================================

#include <QObject>
#include <QString>

namespace prestige {

class SubtitleController : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(QString currentText READ currentText NOTIFY textChanged)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(double confidence READ confidence NOTIFY textChanged)
    Q_PROPERTY(int fontSize READ fontSize WRITE setFontSize NOTIFY fontSizeChanged)
    Q_PROPERTY(QString position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(double bgOpacity READ bgOpacity WRITE setBgOpacity NOTIFY bgOpacityChanged)
    Q_PROPERTY(QString textColor READ textColor WRITE setTextColor NOTIFY textColorChanged)
    Q_PROPERTY(QString whisperModel READ whisperModel WRITE setWhisperModel NOTIFY whisperModelChanged)
    Q_PROPERTY(bool whisperReady READ isWhisperReady NOTIFY whisperReadyChanged)

public:
    explicit SubtitleController(QObject* parent = nullptr);

    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool on);

    QString currentText() const { return m_text; }
    QString language() const { return m_language; }
    void setLanguage(const QString& lang);

    double confidence() const { return m_confidence; }

    int fontSize() const { return m_fontSize; }
    void setFontSize(int size);

    QString position() const { return m_position; }
    void setPosition(const QString& pos);

    double bgOpacity() const { return m_bgOpacity; }
    void setBgOpacity(double opacity);

    QString textColor() const { return m_textColor; }
    void setTextColor(const QString& color);

    QString whisperModel() const { return m_whisperModel; }
    void setWhisperModel(const QString& model);

    bool isWhisperReady() const { return m_whisperReady; }

    // Called from ZMQ thread via invokeMethod
    void onSubtitleReceived(const QString& text, const QString& lang, double conf);
    void setWhisperReady(bool ready);

signals:
    void enabledChanged();
    void textChanged();
    void languageChanged();
    void fontSizeChanged();
    void positionChanged();
    void bgOpacityChanged();
    void textColorChanged();
    void whisperModelChanged();
    void whisperReadyChanged();
    void configChanged();  // Emitted when any visual config changes (for publishConfig)

private:
    bool    m_enabled      = false;
    QString m_text;
    QString m_language     = "fr";
    double  m_confidence   = 0.0;
    int     m_fontSize     = 18;
    QString m_position     = "bottom";
    double  m_bgOpacity    = 0.6;
    QString m_textColor    = "#FFFFFF";
    QString m_whisperModel = "base";
    bool    m_whisperReady = false;
};

} // namespace prestige
