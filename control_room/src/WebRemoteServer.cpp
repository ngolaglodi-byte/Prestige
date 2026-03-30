// ============================================================
// Prestige AI — Web Remote Control Server
// Copyright (c) 2024-2026 Prestige Technologie Company
// All rights reserved.
// ============================================================

#include "WebRemoteServer.h"
#include "LiveController.h"
#include "OverlayController.h"
#include "MacroEngine.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkInterface>
#include <QDebug>

namespace prestige {

WebRemoteServer::WebRemoteServer(LiveController* live, OverlayController* overlay,
                                 MacroEngine* macros, QObject* parent)
    : QObject(parent)
    , m_server(new QTcpServer(this))
    , m_live(live)
    , m_overlay(overlay)
    , m_macros(macros)
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

void WebRemoteServer::handleRequest(QTcpSocket* socket, const QByteArray& request)
{
    QString reqStr = QString::fromUtf8(request);
    QString firstLine = reqStr.section("\r\n", 0, 0);
    QString method = firstLine.section(' ', 0, 0);
    QString path = firstLine.section(' ', 1, 1);

    QByteArray response;

    if (method == "GET" && path == "/") {
        QByteArray html = serveHtml();
        response = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n"
                   "Content-Length: " + QByteArray::number(html.size()) + "\r\n"
                   "Connection: close\r\n\r\n" + html;
    }
    else if (method == "GET" && path == "/api/status") {
        QByteArray json = serveApiStatus();
        response = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n"
                   "Access-Control-Allow-Origin: *\r\n"
                   "Content-Length: " + QByteArray::number(json.size()) + "\r\n"
                   "Connection: close\r\n\r\n" + json;
    }
    else if (method == "POST") {
        QByteArray json = handlePost(path);
        response = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n"
                   "Access-Control-Allow-Origin: *\r\n"
                   "Content-Length: " + QByteArray::number(json.size()) + "\r\n"
                   "Connection: close\r\n\r\n" + json;
    }
    else if (method == "OPTIONS") {
        response = "HTTP/1.1 204 No Content\r\n"
                   "Access-Control-Allow-Origin: *\r\n"
                   "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
                   "Access-Control-Allow-Headers: Content-Type\r\n"
                   "Connection: close\r\n\r\n";
    }
    else {
        QByteArray body = "Not Found";
        response = "HTTP/1.1 404 Not Found\r\nContent-Length: " +
                   QByteArray::number(body.size()) + "\r\nConnection: close\r\n\r\n" + body;
    }

    socket->write(response);
    socket->flush();
    socket->disconnectFromHost();
}

QByteArray WebRemoteServer::serveApiStatus()
{
    QJsonObject obj;
    obj["isLive"]        = m_live->isLive();
    obj["isRecording"]   = m_live->isRecording();
    obj["overlaysActive"] = m_live->overlaysActive();
    obj["detectedName"]  = m_live->detectedName();
    obj["fps"]           = m_live->fps();
    return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

QByteArray WebRemoteServer::handlePost(const QString& path)
{
    QJsonObject result;
    result["ok"] = true;

    if (path == "/api/overlays/toggle") {
        m_live->toggleOverlays();
        result["overlaysActive"] = m_live->overlaysActive();
    }
    else if (path == "/api/bypass/toggle") {
        bool newState = !m_live->overlaysActive();
        m_live->setOverlaysActive(newState);
        result["overlaysActive"] = newState;
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
    else {
        result["ok"] = false;
        result["error"] = "Unknown endpoint";
    }

    return QJsonDocument(result).toJson(QJsonDocument::Compact);
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
</div>

<div class="macros">
  <h3>Macros</h3>
  <div class="macro-btn" onclick="post('/api/macro/0')">Macro 1</div>
  <div class="macro-btn" onclick="post('/api/macro/1')">Macro 2</div>
  <div class="macro-btn" onclick="post('/api/macro/2')">Macro 3</div>
  <div class="macro-btn" onclick="post('/api/macro/3')">Macro 4</div>
</div>

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
