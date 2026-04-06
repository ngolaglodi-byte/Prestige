// ============================================================
// Prestige AI — Preview Sender Implementation
// Sends JPEG-compressed composited frames to Control Room
// ============================================================

#include "PreviewSender.h"

#include <QBuffer>
#include <QByteArray>
#include <QDebug>

#ifdef PRESTIGE_HAVE_ZMQ
#include <zmq.h>
#endif

namespace prestige {

PreviewSender::PreviewSender(QObject* parent)
    : QObject(parent)
{
}

PreviewSender::~PreviewSender()
{
    stop();
}

void PreviewSender::start(const QString& address)
{
#ifdef PRESTIGE_HAVE_ZMQ
    m_zmqContext = zmq_ctx_new();
    m_zmqSocket = zmq_socket(m_zmqContext, ZMQ_PUB);

    int hwm = 1;
    zmq_setsockopt(m_zmqSocket, ZMQ_SNDHWM, &hwm, sizeof(hwm));
    int linger = 0;
    zmq_setsockopt(m_zmqSocket, ZMQ_LINGER, &linger, sizeof(linger));

    int rc = zmq_bind(m_zmqSocket, address.toUtf8().constData());
    if (rc == 0) {
        m_connected = true;
        qInfo() << "[PreviewSender] Bound on" << address;
    } else {
        qWarning() << "[PreviewSender] Bind failed";
    }
#else
    Q_UNUSED(address)
#endif
}

void PreviewSender::stop()
{
#ifdef PRESTIGE_HAVE_ZMQ
    if (m_zmqSocket) { zmq_close(m_zmqSocket); m_zmqSocket = nullptr; }
    if (m_zmqContext) { zmq_ctx_destroy(m_zmqContext); m_zmqContext = nullptr; }
#endif
    m_connected = false;
}

void PreviewSender::sendFrame(const QImage& compositedFrame)
{
#ifdef PRESTIGE_HAVE_ZMQ
    if (!m_zmqSocket || compositedFrame.isNull())
        return;

    // Send every 2nd frame for smooth ~15fps preview
    m_skipCounter++;
    if (m_skipCounter % 2 != 0)
        return;

    // Broadcast monitoring: scale to 1280x720 if source is larger
    // This is standard practice — monitoring at 720p, output at native resolution
    QImage previewFrame = compositedFrame;
    if (compositedFrame.width() > 1280)
        previewFrame = compositedFrame.scaled(1280, 720, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // JPEG 88 — broadcast monitoring quality (visually clean, efficient bandwidth)
    QByteArray jpegData;
    QBuffer buffer(&jpegData);
    buffer.open(QIODevice::WriteOnly);
    previewFrame.save(&buffer, "JPEG", 88);

    int rc = zmq_send(m_zmqSocket, jpegData.constData(), jpegData.size(), ZMQ_NOBLOCK);
    m_framesSent++;
    if (m_framesSent % 150 == 1)
        qInfo() << "[PreviewSender] Frame" << m_framesSent << "size:" << jpegData.size() << "bytes, rc:" << rc;
#else
    Q_UNUSED(compositedFrame)
#endif
}

} // namespace prestige
