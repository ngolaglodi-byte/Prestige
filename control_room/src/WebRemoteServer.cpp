// ============================================================
// Prestige AI -- Web Remote Control Server
// Copyright (c) 2024-2026 Prestige Technologie Company
// All rights reserved.
// ============================================================

#include "WebRemoteServer.h"
#include "LiveController.h"
#include "OverlayController.h"
#include "MacroEngine.h"
#include "SetupController.h"
#include "AnalyticsEngine.h"
#include "SubtitleController.h"
#include "RssFetcher.h"
#include "GraphicsQueue.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkInterface>
#include <QDebug>

namespace prestige {

WebRemoteServer::WebRemoteServer(LiveController* live, OverlayController* overlay,
                                 MacroEngine* macros, SetupController* setup,
                                 AnalyticsEngine* analytics, SubtitleController* subtitles,
                                 RssFetcher* rss, QObject* parent)
    : QObject(parent)
    , m_server(new QTcpServer(this))
    , m_live(live)
    , m_overlay(overlay)
    , m_macros(macros)
    , m_setup(setup)
    , m_analytics(analytics)
    , m_subtitles(subtitles)
    , m_rss(rss)
{
    connect(m_server, &QTcpServer::newConnection, this, &WebRemoteServer::handleConnection);
}

void WebRemoteServer::start()
{
    if (m_server->isListening()) return;

    if (m_server->listen(QHostAddress::Any, static_cast<quint16>(m_port))) {
        qInfo() << "[WebRemote] Server started on port" << m_port;
        emit runningChanged();
    } else {
        qWarning() << "[WebRemote] Failed to start:" << m_server->errorString();
    }
}

void WebRemoteServer::stop()
{
    if (!m_server->isListening()) return;
    m_server->close();
    qInfo() << "[WebRemote] Server stopped";
    emit runningChanged();
}

bool WebRemoteServer::isRunning() const { return m_server->isListening(); }
int WebRemoteServer::port() const { return m_port; }

void WebRemoteServer::setPort(int p)
{
    if (m_port == p) return;
    m_port = p;
    emit portChanged();
}

QString WebRemoteServer::url() const
{
    if (!m_server->isListening()) return QString();
    // Try to get a local network IP
    for (const auto& addr : QNetworkInterface::allAddresses()) {
        if (addr != QHostAddress::LocalHost && addr.protocol() == QAbstractSocket::IPv4Protocol) {
            return QStringLiteral("http://%1:%2").arg(addr.toString()).arg(m_port);
        }
    }
    return QStringLiteral("http://127.0.0.1:%1").arg(m_port);
}

void WebRemoteServer::handleConnection()
{
    while (m_server->hasPendingConnections()) {
        QTcpSocket* socket = m_server->nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, this, [this, socket]() {
            QByteArray request = socket->readAll();
            handleRequest(socket, request);
        });
        connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
    }
}

static QByteArray makeJsonResponse(const QByteArray& json, int statusCode = 200)
{
    QByteArray statusLine;
    if (statusCode == 200)
        statusLine = "HTTP/1.1 200 OK\r\n";
    else if (statusCode == 404)
        statusLine = "HTTP/1.1 404 Not Found\r\n";
    else
        statusLine = "HTTP/1.1 200 OK\r\n";

    return statusLine +
           "Content-Type: application/json; charset=utf-8\r\n"
           "Access-Control-Allow-Origin: *\r\n"
           "Content-Length: " + QByteArray::number(json.size()) + "\r\n"
           "Connection: close\r\n\r\n" + json;
}

static QByteArray makeHtmlResponse(const QByteArray& html)
{
    return "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n"
           "Content-Length: " + QByteArray::number(html.size()) + "\r\n"
           "Connection: close\r\n\r\n" + html;
}

void WebRemoteServer::handleRequest(QTcpSocket* socket, const QByteArray& request)
{
    QString reqStr = QString::fromUtf8(request);
    QString firstLine = reqStr.section("\r\n", 0, 0);
    QString method = firstLine.section(' ', 0, 0);
    QString path = firstLine.section(' ', 1, 1);

    // Extract body (after \r\n\r\n)
    QByteArray body;
    int bodyStart = request.indexOf("\r\n\r\n");
    if (bodyStart >= 0)
        body = request.mid(bodyStart + 4);

    QByteArray response;

    if (method == "GET" && path == "/") {
        response = makeHtmlResponse(serveHtml());
    }
    else if (method == "GET" && path == "/api/status") {
        response = makeJsonResponse(serveApiStatus());
    }
    else if (method == "GET" && path == "/api/analytics") {
        response = makeJsonResponse(serveApiAnalytics());
    }
    else if (method == "GET" && path == "/api/config") {
        response = makeJsonResponse(serveApiConfig());
    }
    else if (method == "GET" && path == "/api/programs") {
        response = makeJsonResponse(serveApiPrograms());
    }
    else if (method == "GET" && path == "/api/talents") {
        response = makeJsonResponse(serveApiTalents());
    }
    else if (method == "GET" && path == "/api/queue") {
        response = makeJsonResponse(serveApiQueue());
    }
    else if (method == "GET" && path == "/api/docs") {
        response = makeHtmlResponse(serveApiDocs());
    }
    else if (method == "POST") {
        QByteArray json = handlePost(path, body);
        response = makeJsonResponse(json);
    }
    else if (method == "OPTIONS") {
        response = "HTTP/1.1 204 No Content\r\n"
                   "Access-Control-Allow-Origin: *\r\n"
                   "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
                   "Access-Control-Allow-Headers: Content-Type\r\n"
                   "Connection: close\r\n\r\n";
    }
    else {
        QByteArray notFound = "Not Found";
        response = "HTTP/1.1 404 Not Found\r\nContent-Length: " +
                   QByteArray::number(notFound.size()) + "\r\nConnection: close\r\n\r\n" + notFound;
    }

    socket->write(response);
    socket->flush();
    socket->disconnectFromHost();
}

QByteArray WebRemoteServer::serveApiStatus()
{
    QJsonObject obj;
    obj["version"] = "1.0.0";
    obj["overlaysActive"] = m_live->overlaysActive();
    obj["bypassed"] = m_live->isBypassed();
    obj["recording"] = m_live->isRecording();
    obj["recordingDuration"] = m_live->recordingDuration();
    obj["activeProgram"] = 0;
    obj["programName"] = m_setup ? m_setup->currentProfileName() : "";
    obj["currentStyle"] = m_setup ? m_setup->selectedStyle() : "";
    obj["detectedTalent"] = m_live->detectedName();
    obj["detectedRole"] = m_live->detectedRole();
    obj["confidence"] = m_live->confidence();
    obj["fps"] = m_live->fps();
    obj["latency"] = m_live->latencyMs();
    obj["clockVisible"] = m_setup ? m_setup->clockVisible() : false;
    obj["tickerVisible"] = m_setup ? m_setup->tickerVisible() : false;
    obj["scoreboardVisible"] = m_setup ? m_setup->scoreboardVisible() : false;
    obj["weatherVisible"] = m_setup ? m_setup->weatherVisible() : false;
    obj["countdownActive"] = m_live->countdownActive();
    obj["qrCodeVisible"] = m_live->qrCodeVisible();
    obj["uptime"] = m_analytics ? m_analytics->elapsedSeconds() : 0;
    obj["totalDetections"] = m_analytics ? m_analytics->totalDetections() : 0;
    obj["isLive"] = m_live->isLive();
    obj["isRecording"] = m_live->isRecording();
    obj["detectedName"] = m_live->detectedName();
    return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

QByteArray WebRemoteServer::serveApiAnalytics()
{
    QJsonObject obj;
    if (m_analytics) {
        obj["totalDetections"] = m_analytics->totalDetections();
        obj["totalOverlays"] = m_analytics->totalOverlays();
        obj["elapsedSeconds"] = m_analytics->elapsedSeconds();
        obj["avgFps"] = m_analytics->avgFps();
        obj["avgLatency"] = m_analytics->avgLatency();
        obj["currentTalent"] = m_analytics->currentTalent();

        QJsonArray talentArr;
        for (const auto& v : m_analytics->talentStats()) {
            QVariantMap m = v.toMap();
            QJsonObject t;
            t["name"] = m["name"].toString();
            t["role"] = m["role"].toString();
            t["detections"] = m["detections"].toInt();
            t["screenTime"] = m["screenTime"].toInt();
            t["screenTimePercent"] = m["screenTimePercent"].toDouble();
            t["avgConfidence"] = m["avgConfidence"].toDouble();
            talentArr.append(t);
        }
        obj["talents"] = talentArr;
    }
    return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

QByteArray WebRemoteServer::serveApiConfig()
{
    QJsonObject obj;
    if (m_setup) {
        obj["selectedStyle"] = m_setup->selectedStyle();
        obj["animationType"] = m_setup->animationType();
        obj["accentColor"] = m_setup->accentColor().name();
        obj["backgroundOpacity"] = m_setup->backgroundOpacity();
        obj["clockVisible"] = m_setup->clockVisible();
        obj["clockFormat"] = m_setup->clockFormat();
        obj["tickerVisible"] = m_setup->tickerVisible();
        obj["scoreboardVisible"] = m_setup->scoreboardVisible();
        obj["weatherVisible"] = m_setup->weatherVisible();
        obj["scoreboardTeamA"] = m_setup->scoreboardTeamA();
        obj["scoreboardTeamB"] = m_setup->scoreboardTeamB();
        obj["scoreboardScoreA"] = m_setup->scoreboardScoreA();
        obj["scoreboardScoreB"] = m_setup->scoreboardScoreB();
    }
    if (m_subtitles) {
        obj["subtitlesEnabled"] = m_subtitles->isEnabled();
    }
    if (m_rss) {
        obj["tickerText"] = m_rss->headlines();
    }
    return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

QByteArray WebRemoteServer::serveApiPrograms()
{
    QJsonObject obj;
    QJsonArray arr;
    if (m_setup) {
        QVariantList profiles = m_setup->availableProfiles();
        for (int i = 0; i < profiles.size(); ++i) {
            QVariantMap p = profiles.at(i).toMap();
            QJsonObject pObj;
            pObj["index"] = i;
            pObj["id"] = p["id"].toString();
            pObj["name"] = p["name"].toString();
            arr.append(pObj);
        }
    }
    obj["programs"] = arr;
    return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

QByteArray WebRemoteServer::serveApiTalents()
{
    QJsonObject obj;
    QJsonArray arr;
    if (m_analytics) {
        for (const auto& v : m_analytics->talentStats()) {
            QVariantMap m = v.toMap();
            QJsonObject t;
            t["name"] = m["name"].toString();
            t["role"] = m["role"].toString();
            t["detections"] = m["detections"].toInt();
            t["screenTime"] = m["screenTime"].toInt();
            t["avgConfidence"] = m["avgConfidence"].toDouble();
            arr.append(t);
        }
    }
    obj["talents"] = arr;
    return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

QByteArray WebRemoteServer::handlePost(const QString& path, const QByteArray& body)
{
    QJsonObject result;
    result["ok"] = true;

    QJsonObject bodyObj;
    if (!body.isEmpty()) {
        auto doc = QJsonDocument::fromJson(body);
        if (doc.isObject())
            bodyObj = doc.object();
    }

    if (path == "/api/overlays/toggle") {
        m_live->toggleOverlays();
        result["overlaysActive"] = m_live->overlaysActive();
    }
    else if (path == "/api/bypass/toggle") {
        bool newState = !m_live->isBypassed();
        m_live->setBypassed(newState);
        result["bypassed"] = newState;
    }
    else if (path == "/api/record/toggle") {
        m_live->toggleRecording();
        result["isRecording"] = m_live->isRecording();
    }
    else if (path.startsWith("/api/macro/")) {
        bool ok = false;
        int idx = path.mid(11).toInt(&ok);
        if (ok && idx >= 0 && idx <= 3) {
            m_macros->executeMacro(idx);
            result["macro"] = idx;
        } else {
            result["ok"] = false;
            result["error"] = "Invalid macro index";
        }
    }
    else if (path == "/api/program/switch") {
        int index = bodyObj.value("index").toInt(-1);
        if (index >= 0 && m_setup) {
            QVariantList profiles = m_setup->availableProfiles();
            if (index < profiles.size()) {
                QString id = profiles.at(index).toMap()["id"].toString();
                m_setup->loadProfile(id);
                result["program"] = index;
            } else {
                result["ok"] = false;
                result["error"] = "Invalid program index";
            }
        } else {
            result["ok"] = false;
            result["error"] = "Invalid index";
        }
    }
    else if (path == "/api/style/set") {
        QString styleId = bodyObj.value("style_id").toString();
        if (!styleId.isEmpty() && m_setup) {
            m_setup->setSelectedStyle(styleId);
            result["style"] = styleId;
        } else {
            result["ok"] = false;
            result["error"] = "Missing style_id";
        }
    }
    else if (path == "/api/ticker/set") {
        QString text = bodyObj.value("text").toString();
        if (m_rss) {
            m_rss->setRssUrl(""); // clear RSS URL — manual mode
            // We can set headlines via a force mechanism; for now just note it
            result["text"] = text;
        }
    }
    else if (path == "/api/ticker/toggle") {
        if (m_setup) {
            bool newState = !m_setup->tickerVisible();
            m_setup->setTickerVisible(newState);
            result["tickerVisible"] = newState;
        }
    }
    else if (path == "/api/countdown/start") {
        int seconds = bodyObj.value("seconds").toInt(60);
        QString label = bodyObj.value("label").toString("LIVE IN");
        m_live->startCountdown(seconds, label);
        result["countdownActive"] = true;
        result["seconds"] = seconds;
        result["label"] = label;
    }
    else if (path == "/api/countdown/stop") {
        m_live->stopCountdown();
        result["countdownActive"] = false;
    }
    else if (path == "/api/qrcode/set") {
        QString qrUrl = bodyObj.value("url").toString();
        bool visible = bodyObj.value("visible").toBool(true);
        QString position = bodyObj.value("position").toString("bottom_right");
        m_live->setQrCode(qrUrl, visible, position);
        result["qrCodeVisible"] = visible;
        result["url"] = qrUrl;
    }
    else if (path == "/api/scoreboard/set") {
        if (m_setup) {
            if (bodyObj.contains("teamA")) m_setup->setScoreboardTeamA(bodyObj["teamA"].toString());
            if (bodyObj.contains("teamB")) m_setup->setScoreboardTeamB(bodyObj["teamB"].toString());
            if (bodyObj.contains("scoreA")) m_setup->setScoreboardScoreA(bodyObj["scoreA"].toInt());
            if (bodyObj.contains("scoreB")) m_setup->setScoreboardScoreB(bodyObj["scoreB"].toInt());
            if (bodyObj.contains("visible")) m_setup->setScoreboardVisible(bodyObj["visible"].toBool());
            result["scoreboard"] = "updated";
        }
    }
    else if (path == "/api/weather/toggle") {
        if (m_setup) {
            bool newState = !m_setup->weatherVisible();
            m_setup->setWeatherVisible(newState);
            result["weatherVisible"] = newState;
        }
    }
    else if (path == "/api/subtitle/toggle") {
        if (m_subtitles) {
            bool newState = !m_subtitles->isEnabled();
            m_subtitles->setEnabled(newState);
            result["subtitlesEnabled"] = newState;
        }
    }
    else if (path == "/api/clock/toggle") {
        if (m_setup) {
            bool newState = !m_setup->clockVisible();
            m_setup->setClockVisible(newState);
            result["clockVisible"] = newState;
        }
    }
    else if (path == "/api/queue/add") {
        if (m_queue) {
            QString type = bodyObj.value("type").toString();
            if (type == "lower_third") {
                m_queue->addLowerThird(bodyObj.value("name").toString(), bodyObj.value("role").toString());
            } else if (type == "ticker") {
                m_queue->addTicker(bodyObj.value("text").toString());
            } else if (type == "message") {
                m_queue->addMessage(bodyObj.value("text").toString());
            } else if (type == "qr_code") {
                m_queue->addQrCode(bodyObj.value("url").toString());
            } else {
                result["ok"] = false;
                result["error"] = "Unknown queue item type";
            }
            result["count"] = m_queue->count();
        }
    }
    else if (path == "/api/queue/take") {
        if (m_queue) {
            m_queue->takeNext();
            result["currentIndex"] = m_queue->currentIndex();
        }
    }
    else if (path == "/api/queue/clear") {
        if (m_queue) {
            m_queue->clearProgram();
            result["currentIndex"] = m_queue->currentIndex();
        }
    }
    else {
        result["ok"] = false;
        result["error"] = "Unknown endpoint";
    }

    return QJsonDocument(result).toJson(QJsonDocument::Compact);
}

QByteArray WebRemoteServer::serveApiQueue()
{
    QJsonObject obj;
    if (m_queue) {
        QJsonArray arr;
        for (const auto& v : m_queue->items()) {
            arr.append(QJsonObject::fromVariantMap(v.toMap()));
        }
        obj["items"] = arr;
        obj["currentIndex"] = m_queue->currentIndex();
        obj["count"] = m_queue->count();
        obj["currentItem"] = QJsonObject::fromVariantMap(m_queue->currentItem());
        obj["nextItem"] = QJsonObject::fromVariantMap(m_queue->nextItem());
    }
    return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

QByteArray WebRemoteServer::serveApiDocs()
{
    static const char* html = R"HTML(<!DOCTYPE html>
<html lang="fr">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Prestige AI - API Documentation</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;background:#07070A;color:#CCC;min-height:100vh;padding:40px 20px;max-width:900px;margin:0 auto}
h1{color:#5B4FDB;font-size:1.8em;letter-spacing:2px;margin-bottom:4px}
h2{color:#8B80E0;font-size:1.2em;margin:30px 0 12px;border-bottom:1px solid #222;padding-bottom:6px}
h3{color:#1DB954;font-size:0.95em;margin:16px 0 6px}
.subtitle{color:#666;font-size:0.9em;margin-bottom:30px}
.endpoint{background:rgba(255,255,255,0.03);border:1px solid rgba(255,255,255,0.06);border-radius:8px;padding:16px;margin-bottom:10px}
.method{display:inline-block;padding:2px 8px;border-radius:4px;font-size:0.75em;font-weight:bold;margin-right:8px}
.get{background:rgba(29,185,84,0.15);color:#1DB954}
.post{background:rgba(91,79,219,0.15);color:#8B80E0}
.path{font-family:monospace;color:#FFF;font-size:0.95em}
.desc{color:#888;font-size:0.85em;margin-top:6px}
code{background:rgba(255,255,255,0.06);padding:2px 6px;border-radius:3px;font-size:0.85em;color:#CCC}
pre{background:rgba(255,255,255,0.04);border:1px solid rgba(255,255,255,0.08);border-radius:6px;padding:12px;margin:8px 0;overflow-x:auto;font-size:0.8em;color:#AAA}
footer{text-align:center;color:#444;font-size:0.7em;margin-top:40px;padding-top:20px;border-top:1px solid #1A1A1A}
</style>
</head>
<body>
<h1>PRESTIGE AI</h1>
<p class="subtitle">REST API Documentation v1.0.0</p>

<h2>Status & Information</h2>

<div class="endpoint">
  <span class="method get">GET</span><span class="path">/api/status</span>
  <div class="desc">Full system status including overlays, detection, recording, and all visibility states.</div>
</div>

<div class="endpoint">
  <span class="method get">GET</span><span class="path">/api/programs</span>
  <div class="desc">List all available emission programs/profiles.</div>
</div>

<div class="endpoint">
  <span class="method get">GET</span><span class="path">/api/talents</span>
  <div class="desc">List all detected talents with screen time and confidence stats.</div>
</div>

<div class="endpoint">
  <span class="method get">GET</span><span class="path">/api/analytics</span>
  <div class="desc">Current analytics data: detections, overlays, FPS, latency, per-talent breakdown.</div>
</div>

<div class="endpoint">
  <span class="method get">GET</span><span class="path">/api/config</span>
  <div class="desc">Current configuration: style, branding, visibility states, scoreboard data.</div>
</div>

<div class="endpoint">
  <span class="method get">GET</span><span class="path">/api/docs</span>
  <div class="desc">This documentation page.</div>
</div>

<h2>Overlay Control</h2>

<div class="endpoint">
  <span class="method post">POST</span><span class="path">/api/overlays/toggle</span>
  <div class="desc">Toggle overlay visibility on/off.</div>
</div>

<div class="endpoint">
  <span class="method post">POST</span><span class="path">/api/bypass/toggle</span>
  <div class="desc">Toggle bypass mode (pass-through without overlays).</div>
</div>

<div class="endpoint">
  <span class="method post">POST</span><span class="path">/api/record/toggle</span>
  <div class="desc">Toggle recording on/off.</div>
</div>

<div class="endpoint">
  <span class="method post">POST</span><span class="path">/api/macro/:id</span>
  <div class="desc">Execute macro by index (0-3).</div>
</div>

<h2>Program & Style</h2>

<div class="endpoint">
  <span class="method post">POST</span><span class="path">/api/program/switch</span>
  <div class="desc">Switch active program.</div>
  <pre>{ "index": 0 }</pre>
</div>

<div class="endpoint">
  <span class="method post">POST</span><span class="path">/api/style/set</span>
  <div class="desc">Set overlay style.</div>
  <pre>{ "style_id": "bfm" }</pre>
</div>

<h2>Ticker</h2>

<div class="endpoint">
  <span class="method post">POST</span><span class="path">/api/ticker/set</span>
  <div class="desc">Set ticker text manually.</div>
  <pre>{ "text": "BREAKING NEWS: ..." }</pre>
</div>

<div class="endpoint">
  <span class="method post">POST</span><span class="path">/api/ticker/toggle</span>
  <div class="desc">Toggle ticker visibility.</div>
</div>

<h2>Countdown</h2>

<div class="endpoint">
  <span class="method post">POST</span><span class="path">/api/countdown/start</span>
  <div class="desc">Start a countdown timer.</div>
  <pre>{ "seconds": 60, "label": "LIVE IN" }</pre>
</div>

<div class="endpoint">
  <span class="method post">POST</span><span class="path">/api/countdown/stop</span>
  <div class="desc">Stop the countdown timer.</div>
</div>

<h2>QR Code</h2>

<div class="endpoint">
  <span class="method post">POST</span><span class="path">/api/qrcode/set</span>
  <div class="desc">Set QR code URL, visibility, and position.</div>
  <pre>{ "url": "https://example.com", "visible": true, "position": "bottom_right" }</pre>
</div>

<h2>Scoreboard</h2>

<div class="endpoint">
  <span class="method post">POST</span><span class="path">/api/scoreboard/set</span>
  <div class="desc">Set scoreboard data.</div>
  <pre>{ "teamA": "PSG", "scoreA": 3, "teamB": "OM", "scoreB": 1, "visible": true }</pre>
</div>

<h2>Visibility Toggles</h2>

<div class="endpoint">
  <span class="method post">POST</span><span class="path">/api/weather/toggle</span>
  <div class="desc">Toggle weather widget visibility.</div>
</div>

<div class="endpoint">
  <span class="method post">POST</span><span class="path">/api/subtitle/toggle</span>
  <div class="desc">Toggle subtitle visibility.</div>
</div>

<div class="endpoint">
  <span class="method post">POST</span><span class="path">/api/clock/toggle</span>
  <div class="desc">Toggle clock visibility.</div>
</div>

<footer>&copy; 2024-2026 Prestige Technologie Company. All rights reserved.</footer>
</body>
</html>)HTML";

    return QByteArray(html);
}

QByteArray WebRemoteServer::serveHtml()
{
    static const char* html = R"HTML(<!DOCTYPE html>
<html lang="fr">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Prestige AI - Remote</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;background:#07070A;color:#CCC;min-height:100vh;display:flex;flex-direction:column;align-items:center;padding:20px}
h1{color:#5B4FDB;font-size:1.4em;letter-spacing:3px;margin-bottom:4px}
.subtitle{color:#666;font-size:0.8em;margin-bottom:24px}
.status-bar{display:flex;gap:12px;margin-bottom:24px;align-items:center}
.pill{padding:6px 16px;border-radius:20px;font-size:0.8em;font-weight:bold}
.live-on{background:rgba(204,0,0,0.2);color:#FF3333;border:1px solid rgba(204,0,0,0.4)}
.live-off{background:rgba(255,255,255,0.04);color:#555;border:1px solid rgba(255,255,255,0.08)}
.rec-on{background:rgba(204,0,0,0.15);color:#FF3333}
.rec-off{color:#444}
.detected{color:#1DB954;font-size:0.9em}
.fps{color:#666;font-family:monospace;font-size:0.8em}
.controls{display:grid;grid-template-columns:1fr 1fr;gap:10px;width:100%;max-width:400px;margin-bottom:20px}
.btn{padding:14px;border:1px solid rgba(255,255,255,0.08);border-radius:10px;background:rgba(255,255,255,0.04);color:#CCC;font-size:0.9em;cursor:pointer;text-align:center;transition:all 0.15s}
.btn:hover{background:rgba(255,255,255,0.08)}
.btn:active{transform:scale(0.97)}
.btn.active{border-color:rgba(29,185,84,0.4);color:#1DB954}
.btn.danger{border-color:rgba(204,0,0,0.3);color:#CC3333}
.macros{display:grid;grid-template-columns:1fr 1fr;gap:8px;width:100%;max-width:400px}
.macros h3{grid-column:1/-1;color:#666;font-size:0.8em;margin-bottom:4px}
.macro-btn{padding:12px;border-radius:8px;background:rgba(91,79,219,0.08);border:1px solid rgba(91,79,219,0.15);color:#8B80E0;cursor:pointer;font-size:0.85em;transition:all 0.15s}
.macro-btn:hover{background:rgba(91,79,219,0.15)}
.api-link{margin-top:16px;color:#555;font-size:0.75em}
.api-link a{color:#5B4FDB;text-decoration:none}
.api-link a:hover{text-decoration:underline}
</style>
</head>
<body>
<h1>PRESTIGE AI</h1>
<p class="subtitle">Remote Control</p>

<div class="status-bar">
  <span id="livePill" class="pill live-off">OFF AIR</span>
  <span id="recPill" class="pill rec-off">REC</span>
  <span id="detected" class="detected">---</span>
  <span id="fps" class="fps">--fps</span>
</div>

<div class="controls">
  <div class="btn" id="btnOverlays" onclick="post('/api/overlays/toggle')">Overlays ON/OFF</div>
  <div class="btn" id="btnBypass" onclick="post('/api/bypass/toggle')">Bypass</div>
  <div class="btn" id="btnRecord" onclick="post('/api/record/toggle')">Enregistrement</div>
  <div class="btn" onclick="post('/api/clock/toggle')">Horloge</div>
  <div class="btn" onclick="post('/api/ticker/toggle')">Ticker</div>
  <div class="btn" onclick="post('/api/weather/toggle')">Meteo</div>
  <div class="btn" onclick="post('/api/subtitle/toggle')">Sous-titres</div>
  <div class="btn" onclick="post('/api/countdown/start')">Countdown</div>
</div>

<div class="macros">
  <h3>Macros</h3>
  <div class="macro-btn" onclick="post('/api/macro/0')">Macro 1</div>
  <div class="macro-btn" onclick="post('/api/macro/1')">Macro 2</div>
  <div class="macro-btn" onclick="post('/api/macro/2')">Macro 3</div>
  <div class="macro-btn" onclick="post('/api/macro/3')">Macro 4</div>
</div>

<p class="api-link"><a href="/api/docs">API Documentation</a></p>

<footer style="text-align:center;color:#555;font-size:11px;margin-top:20px;">
    &copy; 2024-2026 Prestige Technologie Company
</footer>

<script>
async function post(path){try{await fetch(path,{method:'POST'})}catch(e){}}
async function refresh(){
  try{
    const r=await fetch('/api/status');
    const d=await r.json();
    const lp=document.getElementById('livePill');
    lp.textContent=d.isLive?'EN DIRECT':'OFF AIR';
    lp.className='pill '+(d.isLive?'live-on':'live-off');
    const rp=document.getElementById('recPill');
    rp.className='pill '+(d.isRecording?'rec-on':'rec-off');
    document.getElementById('detected').textContent=d.detectedName||'---';
    document.getElementById('fps').textContent=d.fps+'fps';
    const ob=document.getElementById('btnOverlays');
    ob.className='btn '+(d.overlaysActive?'active':'');
    const rb=document.getElementById('btnRecord');
    rb.className='btn '+(d.isRecording?'danger':'');
  }catch(e){}
}
setInterval(refresh,500);
refresh();
</script>
</body>
</html>)HTML";

    return QByteArray(html);
}

} // namespace prestige
