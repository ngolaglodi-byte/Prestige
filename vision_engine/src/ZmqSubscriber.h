#pragma once

// ============================================================
// Prestige AI — ZeroMQ Subscriber
// Receives detection JSON from Python AI Engine
// Applies temporal compensation for video/overlay sync
// ============================================================

#include <QObject>
#include <QThread>
#include <QByteArray>
#include <QElapsedTimer>
#include "Protocol.h"
#include "TalentData.h"

namespace prestige {

class ZmqSubscriber : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)
    Q_PROPERTY(double syncOffsetMs READ syncOffsetMs NOTIFY syncUpdated)

public:
    explicit ZmqSubscriber(TalentStore& store, QObject* parent = nullptr);
    ~ZmqSubscriber() override;

    void start(const QString& address = QStringLiteral("tcp://127.0.0.1:5555"));
    void stop();
    bool isConnected() const;

    // Sync info
    double syncOffsetMs() const { return m_syncOffsetMs; }

signals:
    void messageReceived(const prestige::DetectionMessage& msg);
    void subtitleReceived(const QString& text, const QString& language, double confidence);
    void connectedChanged(bool connected);
    void syncUpdated();
    void errorOccurred(const QString& error);

private:
    void run(const QString& address);

    TalentStore&  m_store;
    QThread       m_thread;
    bool          m_running     = false;
    bool          m_connected   = false;
    double        m_syncOffsetMs = 0.0;
    QElapsedTimer m_appTimer;   // Shared clock for sync
};

} // namespace prestige
