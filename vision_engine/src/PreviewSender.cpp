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

    // Send every 2nd frame to reduce bandwidth (15fps preview is enough)
    m_skipCounter++;
    if (m_skipCounter % 2 != 0)
        return;

    // Downscale to 960x540 for preview
    QImage preview = compositedFrame.scaled(960, 540, Qt::KeepAspectRatio, Qt::FastTransformation);

    // Encode JPEG
    QByteArray jpegData;
    QBuffer buffer(&jpegData);
    buffer.open(QIODevice::WriteOnly);
    preview.save(&buffer, "JPEG", 75);

    zmq_send(m_zmqSocket, jpegData.constData(), jpegData.size(), ZMQ_NOBLOCK);
    m_framesSent++;
#else
    Q_UNUSED(compositedFrame)
#endif
}

} // namespace prestige
