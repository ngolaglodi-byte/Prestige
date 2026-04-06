// ============================================================
// Prestige AI — Preview Monitor Implementation
// Receives JPEG frames from Vision Engine via ZMQ
// ============================================================

#include "PreviewMonitor.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QBuffer>

#ifdef PRESTIGE_HAVE_ZMQ
#include <zmq.h>
#endif

namespace prestige {

PreviewMonitor::PreviewMonitor(QObject* parent)
    : QObject(parent)
{
}

PreviewMonitor::~PreviewMonitor()
{
    stop();
}

QImage PreviewMonitor::latestFrame() const
{
    QMutexLocker lock(&m_frameMutex);
    return m_latestFrame;
}

void PreviewMonitor::start(const QString& address)
{
    if (m_running)
        return;

    m_running = true;

    QObject* worker = new QObject;
    worker->moveToThread(&m_thread);

    connect(&m_thread, &QThread::started, worker, [this, address, worker]() {
        receiveLoop(address);
        worker->deleteLater();
    });
    connect(&m_thread, &QThread::finished, worker, &QObject::deleteLater);

    m_thread.start();
    qInfo() << "[PreviewMonitor] Started, connecting to" << address;
}

void PreviewMonitor::stop()
{
    m_running = false;
    if (m_thread.isRunning()) {
        m_thread.quit();
        m_thread.wait(2000);
    }
    m_active = false;
    emit activeChanged();
}

void PreviewMonitor::receiveLoop(const QString& address)
{
#ifdef PRESTIGE_HAVE_ZMQ
    void* ctx = zmq_ctx_new();
    void* socket = zmq_socket(ctx, ZMQ_SUB);

    zmq_setsockopt(socket, ZMQ_SUBSCRIBE, "", 0);
    int conflate = 1;
    zmq_setsockopt(socket, ZMQ_CONFLATE, &conflate, sizeof(conflate));
    int timeout = 200;
    zmq_setsockopt(socket, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));

    zmq_connect(socket, address.toUtf8().constData());

    QElapsedTimer fpsTimer;
    fpsTimer.start();
    int fpsCount = 0;

    constexpr int BUF_SIZE = 512 * 1024; // 512KB max JPEG
    char buffer[BUF_SIZE];

    int recvCount = 0;
    int failCount = 0;
    qInfo() << "[PreviewMonitor] Entering receive loop, address:" << address;
    while (m_running) {
        int nbytes = zmq_recv(socket, buffer, BUF_SIZE, 0);
        if (nbytes <= 0) {
            failCount++;
            if (failCount == 20)
                qWarning() << "[PreviewMonitor] No frames after 20 attempts (~4s), zmq_errno:" << zmq_errno();
            continue;
        }

        recvCount++;
        if (recvCount <= 3)
            qInfo() << "[PreviewMonitor] Received frame" << recvCount << "size:" << nbytes << "bytes";

        // Decode JPEG
        QByteArray jpegData(buffer, nbytes);
        QImage frame;
        frame.loadFromData(jpegData, "JPEG");

        if (frame.isNull())
            continue;

        {
            QMutexLocker lock(&m_frameMutex);
            m_latestFrame = frame;
        }

        if (!m_active) {
            m_active = true;
            m_width = frame.width();
            m_height = frame.height();
            QMetaObject::invokeMethod(this, [this]() {
                emit activeChanged();
                emit resolutionChanged();
            }, Qt::QueuedConnection);
        }

        m_frameCount++;
        fpsCount++;

        // Notify QML for every received frame — full refresh rate
        QMetaObject::invokeMethod(this, [this]() {
            emit frameUpdated();
        }, Qt::QueuedConnection);

        if (fpsTimer.elapsed() >= 2000) {
            m_fps = fpsCount * 1000.0 / fpsTimer.elapsed();
            fpsCount = 0;
            fpsTimer.restart();
            QMetaObject::invokeMethod(this, [this]() {
                emit fpsChanged();
            }, Qt::QueuedConnection);
        }
    }

    zmq_close(socket);
    zmq_ctx_destroy(ctx);
#else
    Q_UNUSED(address)
    qWarning() << "[PreviewMonitor] Built without ZMQ — preview disabled";
#endif

    m_active = false;
    QMetaObject::invokeMethod(this, [this]() {
        emit activeChanged();
    }, Qt::QueuedConnection);
}

} // namespace prestige
