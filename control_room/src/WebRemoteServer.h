#pragma once

// ============================================================
// Prestige AI -- Web Remote Control Server
// Simple HTTP server for remote control via browser.
// ============================================================

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QString>

namespace prestige {

class LiveController;
class OverlayController;
class MacroEngine;

class WebRemoteServer : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)
    Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)
    Q_PROPERTY(QString url READ url NOTIFY runningChanged)

public:
    explicit WebRemoteServer(LiveController* live, OverlayController* overlay,
                             MacroEngine* macros, QObject* parent = nullptr);

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();

    bool isRunning() const;
    int port() const;
    void setPort(int p);
    QString url() const;

signals:
    void runningChanged();
    void portChanged();

private:
    void handleConnection();
    void handleRequest(QTcpSocket* socket, const QByteArray& request);
    QByteArray serveHtml();
    QByteArray serveApiStatus();
    QByteArray handlePost(const QString& path);

    QTcpServer*      m_server  = nullptr;
    LiveController*  m_live    = nullptr;
    OverlayController* m_overlay = nullptr;
    MacroEngine*     m_macros  = nullptr;
    int              m_port    = 8080;
};

} // namespace prestige
