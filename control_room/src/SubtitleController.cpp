// ============================================================
// Prestige AI — Subtitle Controller Implementation
// ============================================================

#include "SubtitleController.h"
#include <QDebug>

namespace prestige {

SubtitleController::SubtitleController(QObject* parent)
    : QObject(parent)
{
}

void SubtitleController::setEnabled(bool on)
{
    if (m_enabled == on) return;
    m_enabled = on;
    emit enabledChanged();
    emit configChanged();
    if (!on) {
        m_text.clear();
        emit textChanged();
    }
}

void SubtitleController::setLanguage(const QString& lang)
{
    if (m_language == lang) return;
    m_language = lang;
    emit languageChanged();
    emit configChanged();
}

void SubtitleController::setFontSize(int size)
{
    size = qBound(12, size, 48);
    if (m_fontSize == size) return;
    m_fontSize = size;
    emit fontSizeChanged();
    emit configChanged();
}

void SubtitleController::setPosition(const QString& pos)
{
    if (m_position == pos) return;
    m_position = pos;
    emit positionChanged();
    emit configChanged();
}

void SubtitleController::setBgOpacity(double opacity)
{
    opacity = qBound(0.0, opacity, 1.0);
    if (qFuzzyCompare(m_bgOpacity, opacity)) return;
    m_bgOpacity = opacity;
    emit bgOpacityChanged();
    emit configChanged();
}

void SubtitleController::setTextColor(const QString& color)
{
    if (m_textColor == color) return;
    m_textColor = color;
    emit textColorChanged();
    emit configChanged();
}

void SubtitleController::setWhisperModel(const QString& model)
{
    if (m_whisperModel == model) return;
    m_whisperModel = model;
    emit whisperModelChanged();
}

void SubtitleController::onSubtitleReceived(const QString& text, const QString& lang, double conf)
{
    if (!m_enabled) return;

    m_text = text;
    m_confidence = conf;
    emit textChanged();

    if (!lang.isEmpty() && lang != m_language) {
        m_language = lang;
        emit languageChanged();
    }
}

void SubtitleController::setWhisperReady(bool ready)
{
    if (m_whisperReady == ready) return;
    m_whisperReady = ready;
    emit whisperReadyChanged();
}

} // namespace prestige
