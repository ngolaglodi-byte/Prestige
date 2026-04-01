// ============================================================
// Prestige AI — Twitch Chat Integration
// Copyright (c) 2024-2026 Prestige Technologie Company
// ============================================================

#include "TwitchChat.h"
#include <QDebug>

namespace prestige { namespace ai {

TwitchChat::TwitchChat(QObject* parent) : QObject(parent)
{
    m_socket = new QTcpSocket(this);
    connect(m_socket, &QTcpSocket::connected, this, &TwitchChat::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &TwitchChat::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &TwitchChat::onDataReady);
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

    if (m_socket->state() != QAbstractSocket::UnconnectedState)
        m_socket->disconnectFromHost();

    qInfo() << "[TwitchChat] Connecting to channel:" << m_channel;
    m_socket->connectToHost("irc.chat.twitch.tv", 6667);
}

void TwitchChat::disconnect()
{
    if (m_socket && m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->write("PART #" + m_channel.toUtf8() + "\r\n");
        m_socket->disconnectFromHost();
    }
}

void TwitchChat::onConnected()
{
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
    qInfo() << "[TwitchChat] Connected to #" + m_channel;
}

void TwitchChat::onDisconnected()
{
    m_connected = false;
    emit connectedChanged();
    qInfo() << "[TwitchChat] Disconnected";
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
    if (line.contains("PRIVMSG")) {
        int userEnd = line.indexOf('!');
        if (userEnd < 1) return;
        QString author = line.mid(1, userEnd - 1); // Skip leading ':'

        int msgStart = line.indexOf(':', line.indexOf("PRIVMSG"));
        if (msgStart < 0) return;
        msgStart = line.indexOf(':', msgStart + 1);
        if (msgStart < 0) return;
        QString message = line.mid(msgStart + 1);

        emit messageReceived(author, message, QStringLiteral("twitch"));
    }
}

}} // namespace prestige::ai
