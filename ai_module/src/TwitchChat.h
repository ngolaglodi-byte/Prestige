#pragma once
// ============================================================
// Prestige AI — Twitch Chat Integration (TLS)
// Copyright (c) 2024-2026 Prestige Technologie Company
//
// Connects to Twitch IRC via QSslSocket (TLS on port 6697).
// Supports anonymous (read-only) or OAuth login.
// Auto-reconnects on disconnect with exponential backoff.
// ============================================================

#include <QObject>
#include <QSslSocket>
#include <QTimer>

namespace prestige { namespace ai {

class TwitchChat : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)
    Q_PROPERTY(QString channel READ channel WRITE setChannel NOTIFY channelChanged)

public:
    explicit TwitchChat(QObject* parent = nullptr);
    ~TwitchChat();

    bool isConnected() const { return m_connected; }
    QString channel() const { return m_channel; }
    void setChannel(const QString& ch);

    Q_INVOKABLE void connectToChannel(const QString& channel, const QString& oauthToken = QString());
    Q_INVOKABLE void disconnect();

signals:
    void connectedChanged();
    void channelChanged();
    void messageReceived(const QString& author, const QString& message, const QString& platform);

private slots:
    void onDataReady();
    void onConnected();
    void onDisconnected();
    void onSslErrors(const QList<QSslError>& errors);
    void attemptReconnect();

private:
    void parseLine(const QString& line);

    QSslSocket* m_socket = nullptr;
    QTimer*     m_reconnectTimer = nullptr;
    QString     m_channel;
    QString     m_token;
    bool        m_connected = false;
    bool        m_intentionalDisconnect = false;
    int         m_reconnectDelay = 1000;  // Start at 1s, max 30s
    QByteArray  m_buffer;
};

}} // namespace prestige::ai
