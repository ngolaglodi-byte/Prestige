// ============================================================
// Prestige AI — ZeroMQ Subscriber Implementation
// With temporal sync compensation
// ============================================================

#include "ZmqSubscriber.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>

#ifdef PRESTIGE_HAVE_ZMQ
#include <zmq.h>
#endif

namespace prestige {

ZmqSubscriber::ZmqSubscriber(TalentStore& store, QObject* parent)
    : QObject(parent)
    , m_store(store)
{
    m_appTimer.start();
}

ZmqSubscriber::~ZmqSubscriber()
{
    stop();
}

void ZmqSubscriber::start(const QString& address)
{
    if (m_running)
        return;

    m_running = true;

    QObject* worker = new QObject;
    worker->moveToThread(&m_thread);

    connect(&m_thread, &QThread::started, worker, [this, address, worker]() {
        run(address);
        worker->deleteLater();
    });
    connect(&m_thread, &QThread::finished, worker, &QObject::deleteLater);

    m_thread.start();
    m_thread.setPriority(QThread::HighPriority);
}

void ZmqSubscriber::stop()
{
    m_running = false;
    if (m_thread.isRunning()) {
        m_thread.quit();
        m_thread.wait(2000);
    }
}

bool ZmqSubscriber::isConnected() const
{
    return m_connected;
}

void ZmqSubscriber::run(const QString& address)
{
#ifdef PRESTIGE_HAVE_ZMQ
    void* ctx = zmq_ctx_new();
    void* socket = zmq_socket(ctx, ZMQ_SUB);

    zmq_setsockopt(socket, ZMQ_SUBSCRIBE, "", 0);

    int timeout = 100;
    zmq_setsockopt(socket, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));

    // Conflate: only keep the latest message (drop old ones)
    int conflate = 1;
    zmq_setsockopt(socket, ZMQ_CONFLATE, &conflate, sizeof(conflate));

    int rc = zmq_connect(socket, address.toUtf8().constData());
    if (rc != 0) {
        emit errorOccurred(QStringLiteral("ZMQ connect failed: %1").arg(zmq_strerror(zmq_errno())));
        zmq_close(socket);
        zmq_ctx_destroy(ctx);
        return;
    }

    m_connected = true;
    emit connectedChanged(true);
    qInfo() << "[ZmqSubscriber] Connected to" << address;

    QElapsedTimer perfTimer;

    while (m_running) {
        // Receive first frame (could be topic or plain detection)
        zmq_msg_t frame1;
        zmq_msg_init(&frame1);
        int nbytes = zmq_msg_recv(&frame1, socket, 0);
        if (nbytes <= 0) {
            zmq_msg_close(&frame1);
            continue;
        }

        QByteArray firstPart(static_cast<char*>(zmq_msg_data(&frame1)), zmq_msg_size(&frame1));
        zmq_msg_close(&frame1);

        // Check for multipart (subtitle messages)
        int more = 0;
        size_t moreSize = sizeof(more);
        zmq_getsockopt(socket, ZMQ_RCVMORE, &more, &moreSize);

        if (more) {
            zmq_msg_t frame2;
            zmq_msg_init(&frame2);
            zmq_msg_recv(&frame2, socket, 0);
            QByteArray payload(static_cast<char*>(zmq_msg_data(&frame2)), zmq_msg_size(&frame2));
            zmq_msg_close(&frame2);

            if (firstPart == "subtitle") {
                auto doc = QJsonDocument::fromJson(payload);
                auto obj = doc.object();
                emit subtitleReceived(
                    obj["text"].toString(),
                    obj["language"].toString(),
                    obj["confidence"].toDouble(0.9)
                );
            }
            continue;
        }

        perfTimer.start();

        QByteArray raw = firstPart;
        DetectionMessage msg = DetectionMessage::fromJson(raw);

        // ── Temporal sync ──────────────────────────────────
        // Calculate how old this detection is relative to now
        qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
        if (msg.captureTimestampMs > 0) {
            m_syncOffsetMs = static_cast<double>(nowMs - msg.captureTimestampMs);
        }

        // ── Update talent store with bbox prediction ───────
        QList<TalentOverlay> overlays;
        overlays.reserve(msg.talents.size());

        for (const auto& t : msg.talents) {
            TalentOverlay ov;
            ov.id           = t.id;
            ov.name         = t.name;
            ov.role         = t.role;
            ov.confidence   = t.confidence;
            ov.showOverlay  = t.showOverlay;
            ov.overlayStyle = t.overlayStyle;

            // Use bbox as-is — the Python tracker already provides
            // smoothed, up-to-date positions at 30fps+
            ov.bbox = QRectF(t.bbox.x, t.bbox.y, t.bbox.w, t.bbox.h);

            overlays.append(ov);
        }

        m_store.update(overlays);
        emit messageReceived(msg);

        qint64 elapsed = perfTimer.nsecsElapsed() / 1000000;
        if (elapsed > 10) {
            qWarning() << "[ZmqSubscriber] Parse took" << elapsed << "ms (> 10ms budget)";
        }
    }

    m_connected = false;
    emit connectedChanged(false);

    zmq_close(socket);
    zmq_ctx_destroy(ctx);
#else
    Q_UNUSED(address)
    qWarning() << "[ZmqSubscriber] Built without ZMQ support — running in stub mode";
    m_connected = false;
    emit connectedChanged(false);
#endif
}

} // namespace prestige
