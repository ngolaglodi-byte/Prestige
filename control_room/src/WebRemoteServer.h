#pragma once

// ============================================================
// Prestige AI -- Web Remote Control Server
// Simple HTTP server for remote control via browser.
// Extended with full REST API.
// ============================================================

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QString>

namespace prestige {

class LiveController;
class OverlayController;
class MacroEngine;
class SetupController;
class AnalyticsEngine;
class SubtitleController;
class RssFetcher;
class GraphicsQueue;

class WebRemoteServer : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)
    Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)
    Q_PROPERTY(QString url READ url NOTIFY runningChanged)
    Q_PROPERTY(QString apiKey READ apiKey WRITE setApiKey NOTIFY apiKeyChanged)
    Q_PROPERTY(bool authEnabled READ authEnabled WRITE setAuthEnabled NOTIFY apiKeyChanged)

public:
    explicit WebRemoteServer(LiveController* live, OverlayController* overlay,
                             MacroEngine* macros, SetupController* setup,
                             AnalyticsEngine* analytics, SubtitleController* subtitles,
                             RssFetcher* rss, QObject* parent = nullptr);

    void setGraphicsQueue(GraphicsQueue* queue) { m_queue = queue; }

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();

    bool isRunning() const;
    int port() const;
    void setPort(int p);
    QString url() const;

    QString apiKey() const { return m_apiKey; }
    void setApiKey(const QString& key) { if (m_apiKey != key) { m_apiKey = key; emit apiKeyChanged(); } }
    bool authEnabled() const { return m_authEnabled; }
    void setAuthEnabled(bool v) { if (m_authEnabled != v) { m_authEnabled = v; emit apiKeyChanged(); } }

signals:
    void runningChanged();
    void portChanged();
    void apiKeyChanged();

private:
    void handleConnection();
    void handleRequest(QTcpSocket* socket, const QByteArray& request);
    QByteArray serveHtml();
    QByteArray serveApiStatus();
    QByteArray serveApiDocs();
    QByteArray serveApiAnalytics();
    QByteArray serveApiConfig();
    QByteArray serveApiPrograms();
    QByteArray serveApiTalents();
    QByteArray handlePost(const QString& path, const QByteArray& body);
    QByteArray serveApiQueue();

    QTcpServer*        m_server     = nullptr;
    LiveController*    m_live       = nullptr;
    OverlayController* m_overlay    = nullptr;
    MacroEngine*       m_macros     = nullptr;
    SetupController*   m_setup      = nullptr;
    AnalyticsEngine*   m_analytics  = nullptr;
    SubtitleController* m_subtitles = nullptr;
    RssFetcher*        m_rss        = nullptr;
    GraphicsQueue*     m_queue      = nullptr;
    int                m_port       = 8080;
    QString            m_apiKey;         // If empty + auth disabled, all requests pass
    bool               m_authEnabled = false;
};

} // namespace prestige
