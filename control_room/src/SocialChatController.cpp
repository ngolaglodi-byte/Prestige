// ============================================================
// Prestige AI -- Social Chat Controller Implementation
// ============================================================

#include "SocialChatController.h"
#include <QVariantMap>
#include <QDebug>

namespace prestige {

SocialChatController::SocialChatController(QObject* parent)
    : QObject(parent)
{
}

void SocialChatController::setEnabled(bool on)
{
    if (m_enabled == on) return;
    m_enabled = on;
    emit enabledChanged();
    emit configChanged();
    qInfo() << "[SocialChat] Enabled:" << on;
}

void SocialChatController::setPlatform(const QString& p)
{
    if (m_platform == p) return;
    m_platform = p;
    emit platformChanged();
    emit configChanged();
}

void SocialChatController::setChannelId(const QString& id)
{
    if (m_channelId == id) return;
    m_channelId = id;
    emit channelIdChanged();
    emit configChanged();
}

void SocialChatController::setShowOnOutput(bool show)
{
    if (m_showOnOutput == show) return;
    m_showOnOutput = show;
    emit showOnOutputChanged();
    emit configChanged();
}

void SocialChatController::setMaxMessages(int max)
{
    max = qBound(5, max, 50);
    if (m_maxMessages == max) return;
    m_maxMessages = max;
    emit maxMessagesChanged();
}

void SocialChatController::onChatMessageReceived(const QString& platform,
                                                  const QString& author,
                                                  const QString& message,
                                                  const QString& color)
{
    if (!m_enabled) return;

    QVariantMap msg;
    msg["platform"] = platform;
    msg["author"]   = author;
    msg["message"]  = message;
    msg["color"]    = color;

    m_recentMessages.prepend(msg);

    // Keep rolling list bounded
    while (m_recentMessages.size() > m_maxMessages)
        m_recentMessages.removeLast();

    emit messagesChanged();
}

QStringList SocialChatController::chatLinesForOverlay() const
{
    QStringList lines;
    int count = qMin(5, m_recentMessages.size());
    for (int i = 0; i < count; ++i) {
        auto msg = m_recentMessages[i].toMap();
        lines.append(msg["author"].toString() + ": " + msg["message"].toString());
    }
    return lines;
}

} // namespace prestige
