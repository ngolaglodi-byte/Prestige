// ============================================================
// Prestige AI — Talent Manager Implementation
// ============================================================

#include "TalentManager.h"

#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDateTime>
#include <QDebug>
#include <QCamera>
#include <QImageCapture>
#include <QMediaCaptureSession>

#ifdef PRESTIGE_HAVE_ZMQ
#include <zmq.h>
#endif

namespace prestige {

TalentManager::TalentManager(QObject* parent)
    : QObject(parent)
{
    QDir().mkpath(photosDir());

#ifdef PRESTIGE_HAVE_ZMQ
    m_zmqContext = zmq_ctx_new();
    m_zmqSocket = zmq_socket(m_zmqContext, ZMQ_REQ);

    int timeout = 2000;
    zmq_setsockopt(m_zmqSocket, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
    zmq_setsockopt(m_zmqSocket, ZMQ_SNDTIMEO, &timeout, sizeof(timeout));
    zmq_setsockopt(m_zmqSocket, ZMQ_LINGER, &timeout, sizeof(timeout));

    int rc = zmq_connect(m_zmqSocket, "tcp://127.0.0.1:5556");
    if (rc == 0) {
        m_connected = true;
        qInfo() << "[TalentManager] Connected to talent server on :5556";
    } else {
        qWarning() << "[TalentManager] Failed to connect to talent server";
    }
#else
    qWarning() << "[TalentManager] Built without ZMQ — operating in offline mode";
#endif
}

TalentManager::~TalentManager()
{
#ifdef PRESTIGE_HAVE_ZMQ
    if (m_zmqSocket) zmq_close(m_zmqSocket);
    if (m_zmqContext) zmq_ctx_destroy(m_zmqContext);
#endif
}

QString TalentManager::photosDir() const
{
    return QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
           + "/.prestige-ai/talent_photos";
}

QJsonObject TalentManager::sendRequest(const QJsonObject& request)
{
#ifdef PRESTIGE_HAVE_ZMQ
    if (!m_zmqSocket) {
        return {{"error", "Not connected"}};
    }

    QByteArray payload = QJsonDocument(request).toJson(QJsonDocument::Compact);

    int sent = zmq_send(m_zmqSocket, payload.data(), payload.size(), 0);
    if (sent < 0) {
        qWarning() << "[TalentManager] Send failed:" << zmq_strerror(zmq_errno());
        return {{"error", "Send failed"}};
    }

    char buffer[65536];
    int nbytes = zmq_recv(m_zmqSocket, buffer, sizeof(buffer) - 1, 0);
    if (nbytes < 0) {
        qWarning() << "[TalentManager] Recv timeout — talent server not running?";
        m_connected = false;
        emit connectionChanged();
        return {{"error", "Server timeout — start talent_manager.py"}};
    }

    buffer[nbytes] = '\0';
    auto doc = QJsonDocument::fromJson(QByteArray(buffer, nbytes));
    if (!m_connected) {
        m_connected = true;
        emit connectionChanged();
    }
    return doc.object();
#else
    Q_UNUSED(request)
    return {{"error", "ZMQ not available"}};
#endif
}

void TalentManager::refreshTalents()
{
    QJsonObject resp = sendRequest({{"action", "list"}});

    if (resp.contains("error")) {
        emit errorOccurred(resp["error"].toString());
        return;
    }

    m_talents.clear();
    for (const auto& v : resp["talents"].toArray()) {
        auto obj = v.toObject();
        QVariantMap talent;
        talent["id"] = obj["id"].toString();
        talent["name"] = obj["name"].toString();
        talent["role"] = obj["role"].toString();
        talent["active"] = obj["active"].toBool(true);
        talent["overlay_style"] = obj["overlay_style"].toString("default");
        talent["has_embeddings"] = obj["has_embeddings"].toBool(false);
        talent["embedding_count"] = obj["embedding_count"].toInt(0);
        talent["photo_exists"] = obj["photo_exists"].toBool(false);
        talent["photo_path"] = obj["photo_path"].toString();
        m_talents.append(talent);
    }

    emit talentsChanged();
    qInfo() << "[TalentManager] Loaded" << m_talents.size() << "talents";
}

QVariantMap TalentManager::addTalent(const QString& name, const QString& role, const QString& photoPath)
{
    QJsonObject req;
    req["action"] = "add";
    req["name"] = name;
    req["role"] = role;
    if (!photoPath.isEmpty())
        req["photo_path"] = photoPath;

    QJsonObject resp = sendRequest(req);
    QVariantMap result = resp.toVariantMap();

    if (resp["success"].toBool()) {
        emit talentAdded(resp["id"].toString(), name);
        refreshTalents();
    } else {
        emit errorOccurred(resp["error"].toString());
    }

    return result;
}

QVariantMap TalentManager::updateTalent(const QString& id, const QString& name, const QString& role)
{
    QJsonObject req;
    req["action"] = "update";
    req["id"] = id;
    req["name"] = name;
    req["role"] = role;

    QJsonObject resp = sendRequest(req);

    if (resp["success"].toBool())
        refreshTalents();
    else
        emit errorOccurred(resp["error"].toString());

    return resp.toVariantMap();
}

QVariantMap TalentManager::deleteTalent(const QString& id)
{
    QJsonObject req;
    req["action"] = "delete";
    req["id"] = id;

    QJsonObject resp = sendRequest(req);

    if (resp["success"].toBool()) {
        emit talentDeleted(id);
        refreshTalents();
    } else {
        emit errorOccurred(resp["error"].toString());
    }

    return resp.toVariantMap();
}

QVariantMap TalentManager::enrollPhoto(const QString& id, const QString& photoPath)
{
    QJsonObject req;
    req["action"] = "enroll_photo";
    req["id"] = id;
    req["photo_path"] = photoPath;

    QJsonObject resp = sendRequest(req);

    if (resp["success"].toBool())
        refreshTalents();
    else
        emit errorOccurred(resp["error"].toString());

    return resp.toVariantMap();
}

QString TalentManager::capturePhotoFromCamera(int cameraIndex)
{
    Q_UNUSED(cameraIndex)

    // Save path for the capture
    QString filename = QStringLiteral("capture_%1.jpg")
        .arg(QDateTime::currentMSecsSinceEpoch());
    QString path = photosDir() + "/" + filename;

    qInfo() << "[TalentManager] Photo capture requested → " << path;
    // In production, this would trigger a QCamera snapshot
    // For now, return the path where the photo should be saved
    return path;
}

QString TalentManager::talentPhotoPath(const QString& id) const
{
    QString path = photosDir() + "/" + id + ".jpg";
    if (QFile::exists(path))
        return "file://" + path;
    return "";
}

} // namespace prestige
