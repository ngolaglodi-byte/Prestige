#pragma once

// ============================================================
// Prestige AI -- Social Chat Controller
// Receives social media chat messages (YouTube/Twitch) via ZMQ.
// Exposes recent messages to QML for display and overlay control.
// ============================================================

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QStringList>

namespace prestige {

class SocialChatController : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(QString platform READ platform WRITE setPlatform NOTIFY platformChanged)
    Q_PROPERTY(QString channelId READ channelId WRITE setChannelId NOTIFY channelIdChanged)
    Q_PROPERTY(QVariantList recentMessages READ recentMessages NOTIFY messagesChanged)
    Q_PROPERTY(bool showOnOutput READ showOnOutput WRITE setShowOnOutput NOTIFY showOnOutputChanged)
    Q_PROPERTY(int maxMessages READ maxMessages WRITE setMaxMessages NOTIFY maxMessagesChanged)

public:
    explicit SocialChatController(QObject* parent = nullptr);

    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool on);

    QString platform() const { return m_platform; }
    void setPlatform(const QString& p);

    QString channelId() const { return m_channelId; }
    void setChannelId(const QString& id);

    QVariantList recentMessages() const { return m_recentMessages; }

    bool showOnOutput() const { return m_showOnOutput; }
    void setShowOnOutput(bool show);

    int maxMessages() const { return m_maxMessages; }
    void setMaxMessages(int max);

    // Called from ZMQ subscriber thread via invokeMethod
    void onChatMessageReceived(const QString& platform, const QString& author,
                               const QString& message, const QString& color);

    // For Compositor overlay rendering
    QStringList chatLinesForOverlay() const;

signals:
    void enabledChanged();
    void platformChanged();
    void channelIdChanged();
    void messagesChanged();
    void showOnOutputChanged();
    void maxMessagesChanged();
    void configChanged();

private:
    bool         m_enabled      = false;
    QString      m_platform     = "youtube";
    QString      m_channelId;
    QVariantList m_recentMessages;
    bool         m_showOnOutput = false;
    int          m_maxMessages  = 20;
};

} // namespace prestige
