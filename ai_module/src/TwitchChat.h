#pragma once
// ============================================================
// Prestige AI — Twitch Chat Integration
// Copyright (c) 2024-2026 Prestige Technologie Company
//
// Connects to Twitch IRC via QTcpSocket.
// Supports anonymous (read-only) or OAuth login.
// ============================================================

#include <QObject>
#include <QTcpSocket>
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

private:
    void parseLine(const QString& line);

    QTcpSocket* m_socket = nullptr;
    QString m_channel;
    QString m_token;
    bool m_connected = false;
    QByteArray m_buffer;
};

}} // namespace prestige::ai
