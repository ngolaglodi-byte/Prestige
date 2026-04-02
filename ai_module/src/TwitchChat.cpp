// ============================================================
// Prestige AI — Twitch Chat Integration (TLS + Auto-Reconnect)
// Broadcast-grade: encrypted connection, auto-recovery
// Copyright (c) 2024-2026 Prestige Technologie Company
// ============================================================

#include "TwitchChat.h"
#include <QDebug>

namespace prestige { namespace ai {

static constexpr int MAX_RECONNECT_DELAY = 30000; // 30 seconds max

TwitchChat::TwitchChat(QObject* parent) : QObject(parent)
{
    m_socket = new QSslSocket(this);
    connect(m_socket, &QSslSocket::encrypted, this, &TwitchChat::onConnected);
    connect(m_socket, &QSslSocket::disconnected, this, &TwitchChat::onDisconnected);
    connect(m_socket, &QSslSocket::readyRead, this, &TwitchChat::onDataReady);
    connect(m_socket, &QSslSocket::sslErrors, this, &TwitchChat::onSslErrors);

    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setSingleShot(true);
    connect(m_reconnectTimer, &QTimer::timeout, this, &TwitchChat::attemptReconnect);
}

TwitchChat::~TwitchChat()
{
    disconnect();
}

void TwitchChat::setChannel(const QString& ch)
{
    if (m_channel == ch) return;
    m_channel = ch;
    emit channelChanged();
}

void TwitchChat::connectToChannel(const QString& channel, const QString& oauthToken)
{
    m_channel = channel.toLower();
    m_token = oauthToken;
    m_intentionalDisconnect = false;
    m_reconnectDelay = 1000;

    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->abort();
    }

    qInfo() << "[TwitchChat] Connecting to channel:" << m_channel << "(TLS:6697)";
    // TLS encrypted connection (port 6697 — Twitch requirement since 2024)
    m_socket->connectToHostEncrypted("irc.chat.twitch.tv", 6697);
}

void TwitchChat::disconnect()
{
    m_intentionalDisconnect = true;
    m_reconnectTimer->stop();
    if (m_socket && m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->write("PART #" + m_channel.toUtf8() + "\r\n");
        m_socket->flush();
        m_socket->disconnectFromHost();
    }
}

void TwitchChat::onConnected()
{
    m_reconnectDelay = 1000; // Reset backoff on successful connect

    // Request Twitch tags for display names, badges, colors
    m_socket->write("CAP REQ :twitch.tv/tags twitch.tv/commands\r\n");

    // Anonymous login (read-only) or OAuth
    if (m_token.isEmpty()) {
        m_socket->write("NICK justinfan12345\r\n");
    } else {
        m_socket->write("PASS oauth:" + m_token.toUtf8() + "\r\n");
        m_socket->write("NICK prestige_ai\r\n");
    }
    m_socket->write("JOIN #" + m_channel.toUtf8() + "\r\n");

    m_connected = true;
    emit connectedChanged();
    qInfo() << "[TwitchChat] Connected to #" + m_channel << "(TLS encrypted)";
}

void TwitchChat::onDisconnected()
{
    bool wasConnected = m_connected;
    m_connected = false;
    if (wasConnected) emit connectedChanged();

    if (!m_intentionalDisconnect && !m_channel.isEmpty()) {
        // Auto-reconnect with exponential backoff
        qInfo() << "[TwitchChat] Disconnected — reconnecting in" << m_reconnectDelay / 1000 << "s";
        m_reconnectTimer->start(m_reconnectDelay);
        m_reconnectDelay = qMin(m_reconnectDelay * 2, MAX_RECONNECT_DELAY);
    } else {
        qInfo() << "[TwitchChat] Disconnected";
    }
}

void TwitchChat::onSslErrors(const QList<QSslError>& errors)
{
    for (const auto& err : errors) {
        qWarning() << "[TwitchChat] SSL error:" << err.errorString();
    }
    // Ignore SSL errors for Twitch (their cert chain is valid but some systems lack root CAs)
    m_socket->ignoreSslErrors();
}

void TwitchChat::attemptReconnect()
{
    if (m_intentionalDisconnect || m_channel.isEmpty()) return;
    qInfo() << "[TwitchChat] Attempting reconnect to #" + m_channel;
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->abort();
    }
    m_socket->connectToHostEncrypted("irc.chat.twitch.tv", 6697);
}

void TwitchChat::onDataReady()
{
    m_buffer.append(m_socket->readAll());

    while (m_buffer.contains('\n')) {
        int idx = m_buffer.indexOf('\n');
        QString line = QString::fromUtf8(m_buffer.left(idx)).trimmed();
        m_buffer.remove(0, idx + 1);
        parseLine(line);
    }
}

void TwitchChat::parseLine(const QString& line)
{
    // Respond to PING to keep connection alive
    if (line.startsWith("PING")) {
        m_socket->write("PONG :tmi.twitch.tv\r\n");
        return;
    }

    // Parse PRIVMSG: :username!username@username.tmi.twitch.tv PRIVMSG #channel :message
    // With tags: @badge-info=...;display-name=User;... :user!user@... PRIVMSG #channel :message
    if (line.contains("PRIVMSG")) {
        // Extract display name from tags if available
        QString author;
        if (line.startsWith("@")) {
            int tagEnd = line.indexOf(' ');
            QString tags = line.left(tagEnd);
            // Parse display-name tag
            int dnIdx = tags.indexOf("display-name=");
            if (dnIdx >= 0) {
                int dnStart = dnIdx + 13;
                int dnEnd = tags.indexOf(';', dnStart);
                author = tags.mid(dnStart, dnEnd > 0 ? dnEnd - dnStart : -1);
            }
        }

        // Fallback: extract from IRC prefix
        if (author.isEmpty()) {
            int colonIdx = line.indexOf(':', line.startsWith("@") ? line.indexOf(' ') : 0);
            int userEnd = line.indexOf('!', colonIdx);
            if (userEnd > colonIdx + 1) {
                author = line.mid(colonIdx + 1, userEnd - colonIdx - 1);
            }
        }

        // Extract message text (after the last ':' following PRIVMSG)
        int privmsgIdx = line.indexOf("PRIVMSG");
        if (privmsgIdx < 0) return;
        int msgStart = line.indexOf(':', privmsgIdx);
        if (msgStart < 0) return;
        QString message = line.mid(msgStart + 1);

        if (!author.isEmpty() && !message.isEmpty()) {
            emit messageReceived(author, message, QStringLiteral("twitch"));
        }
    }
}

}} // namespace prestige::ai
