// ============================================================
// Prestige AI — Frame Sender Implementation
// ============================================================

#include "FrameSender.h"

#include <QBuffer>
#include <QByteArray>
#include <QDebug>
#include <QElapsedTimer>

#ifdef PRESTIGE_HAVE_ZMQ
#include <zmq.h>
#endif

namespace prestige {

FrameSender::FrameSender(QObject* parent)
    : QObject(parent)
{
}

FrameSender::~FrameSender()
{
    stop();
}

void FrameSender::start(const QString& address)
{
#ifdef PRESTIGE_HAVE_ZMQ
    m_zmqContext = zmq_ctx_new();
    m_zmqSocket = zmq_socket(m_zmqContext, ZMQ_PUB);

    // Low HWM: if Python is slow, drop frames rather than queue them
    int hwm = 1;
    zmq_setsockopt(m_zmqSocket, ZMQ_SNDHWM, &hwm, sizeof(hwm));
    int linger = 0;
    zmq_setsockopt(m_zmqSocket, ZMQ_LINGER, &linger, sizeof(linger));

    int rc = zmq_bind(m_zmqSocket, address.toUtf8().constData());
    if (rc == 0) {
        m_connected = true;
        emit connectedChanged(true);
        qInfo() << "[FrameSender] Bound on" << address;
    } else {
        qCritical() << "[FrameSender] Bind failed:" << zmq_strerror(zmq_errno());
    }
#else
    Q_UNUSED(address)
    qWarning() << "[FrameSender] Built without ZMQ — frame sending disabled";
#endif
}

void FrameSender::stop()
{
#ifdef PRESTIGE_HAVE_ZMQ
    if (m_zmqSocket) {
        zmq_close(m_zmqSocket);
        m_zmqSocket = nullptr;
    }
    if (m_zmqContext) {
        zmq_ctx_destroy(m_zmqContext);
        m_zmqContext = nullptr;
    }
#endif
    m_connected = false;
    emit connectedChanged(false);
}

void FrameSender::sendFrame(const QImage& frame, qint64 frameId, qint64 captureTimestampMs)
{
#ifdef PRESTIGE_HAVE_ZMQ
    if (!m_zmqSocket || frame.isNull())
        return;

    // Downscale for AI analysis (Python doesn't need full resolution)
    QImage scaled = frame;
    if (frame.width() > m_sendResolution.width()) {
        scaled = frame.scaled(m_sendResolution, Qt::KeepAspectRatio, Qt::FastTransformation);
    }

    // Encode as JPEG (fast, ~2-4ms for 640x480)
    QByteArray jpegData;
    QBuffer buffer(&jpegData);
    buffer.open(QIODevice::WriteOnly);
    scaled.save(&buffer, "JPEG", m_jpegQuality);

    // Build message: [8 bytes frameId][8 bytes timestamp][JPEG data]
    QByteArray msg;
    msg.reserve(16 + jpegData.size());
    msg.append(reinterpret_cast<const char*>(&frameId), 8);
    msg.append(reinterpret_cast<const char*>(&captureTimestampMs), 8);
    msg.append(jpegData);

    int rc = zmq_send(m_zmqSocket, msg.constData(), msg.size(), ZMQ_NOBLOCK);
    if (rc > 0) {
        m_framesSent++;
    } else {
        m_framesDropped++;
    }
#else
    Q_UNUSED(frame) Q_UNUSED(frameId) Q_UNUSED(captureTimestampMs)
#endif
}

} // namespace prestige
