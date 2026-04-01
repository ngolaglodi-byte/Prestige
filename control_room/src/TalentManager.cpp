// ============================================================
// Prestige AI — Talent Manager Implementation
// Direct JSON file operations (no Python/ZMQ dependency)
// ============================================================

#include "TalentManager.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDateTime>
#include <QDebug>

namespace prestige {

static QString talentsFilePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
           + "/.prestige-ai/talents.json";
}

TalentManager::TalentManager(QObject* parent)
    : QObject(parent)
{
    QDir().mkpath(photosDir());

    // Ensure the data directory exists
    QFileInfo fi(talentsFilePath());
    QDir().mkpath(fi.absolutePath());

    m_connected = true;
    emit connectionChanged();
    qInfo() << "[TalentManager] Direct file mode — talents at:" << talentsFilePath();
}

TalentManager::~TalentManager()
{
}

QString TalentManager::photosDir() const
{
    return QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
           + "/.prestige-ai/talent_photos";
}

QJsonObject TalentManager::sendRequest(const QJsonObject& request)
{
    Q_UNUSED(request)
    // Legacy ZMQ method — no longer used
    return {{"error", "ZMQ transport removed — use direct file methods"}};
}

void TalentManager::refreshTalents()
{
    QFile file(talentsFilePath());
    if (!file.open(QIODevice::ReadOnly)) {
        // No file yet — start with empty list
        m_talents.clear();
        emit talentsChanged();
        qInfo() << "[TalentManager] No talents file found — starting empty";
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        emit errorOccurred("Invalid talents.json format");
        return;
    }

    QJsonObject root = doc.object();
    QJsonArray arr = root["talents"].toArray();

    m_talents.clear();
    for (const auto& v : arr) {
        auto obj = v.toObject();
        QVariantMap talent;
        talent["id"] = obj["id"].toString();
        talent["name"] = obj["name"].toString();
        talent["role"] = obj["role"].toString();
        talent["active"] = obj["active"].toBool(true);
        talent["overlay_style"] = obj["overlay_style"].toString("default");

        QJsonArray embArr = obj["embeddings"].toArray();
        talent["has_embeddings"] = !embArr.isEmpty();
        talent["embedding_count"] = embArr.size();

        QString photoPath = photosDir() + "/" + obj["id"].toString() + ".jpg";
        talent["photo_exists"] = QFile::exists(photoPath);
        talent["photo_path"] = photoPath;

        m_talents.append(talent);
    }

    emit talentsChanged();
    qInfo() << "[TalentManager] Loaded" << m_talents.size() << "talents";
}

// Helper: read entire JSON, return root object + next_id
static QPair<QJsonObject, int> readTalentsFile()
{
    QFile file(talentsFilePath());
    if (!file.open(QIODevice::ReadOnly))
        return {QJsonObject(), 1};

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    QJsonObject root = doc.object();
    int nextId = root["next_id"].toInt(1);
    return {root, nextId};
}

static bool writeTalentsFile(const QJsonObject& root)
{
    QFile file(talentsFilePath());
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "[TalentManager] Failed to write:" << talentsFilePath();
        return false;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

QVariantMap TalentManager::addTalent(const QString& name, const QString& role, const QString& photoPath)
{
    auto [root, nextId] = readTalentsFile();
    QJsonArray arr = root["talents"].toArray();

    QJsonObject newTalent;
    QString id = QString::number(nextId);
    newTalent["id"] = id;
    newTalent["name"] = name;
    newTalent["role"] = role;
    newTalent["active"] = true;
    newTalent["overlay_style"] = "default";
    newTalent["embeddings"] = QJsonArray();

    arr.append(newTalent);
    root["talents"] = arr;
    root["next_id"] = nextId + 1;

    QVariantMap result;
    if (writeTalentsFile(root)) {
        result["success"] = true;
        result["id"] = id;
        emit talentAdded(id, name);

        // Copy photo if provided
        if (!photoPath.isEmpty() && QFile::exists(photoPath)) {
            QString dest = photosDir() + "/" + id + ".jpg";
            QFile::copy(photoPath, dest);
        }

        refreshTalents();
    } else {
        result["success"] = false;
        result["error"] = "Failed to write talents file";
        emit errorOccurred(result["error"].toString());
    }

    return result;
}

QVariantMap TalentManager::updateTalent(const QString& id, const QString& name, const QString& role)
{
    auto [root, nextId] = readTalentsFile();
    Q_UNUSED(nextId)
    QJsonArray arr = root["talents"].toArray();

    bool found = false;
    for (int i = 0; i < arr.size(); ++i) {
        QJsonObject obj = arr[i].toObject();
        if (obj["id"].toString() == id) {
            obj["name"] = name;
            obj["role"] = role;
            arr[i] = obj;
            found = true;
            break;
        }
    }

    QVariantMap result;
    if (!found) {
        result["success"] = false;
        result["error"] = "Talent not found: " + id;
        emit errorOccurred(result["error"].toString());
    } else {
        root["talents"] = arr;
        if (writeTalentsFile(root)) {
            result["success"] = true;
            refreshTalents();
        } else {
            result["success"] = false;
            result["error"] = "Failed to write talents file";
            emit errorOccurred(result["error"].toString());
        }
    }

    return result;
}

QVariantMap TalentManager::deleteTalent(const QString& id)
{
    auto [root, nextId] = readTalentsFile();
    Q_UNUSED(nextId)
    QJsonArray arr = root["talents"].toArray();

    bool found = false;
    QJsonArray newArr;
    for (const auto& v : arr) {
        QJsonObject obj = v.toObject();
        if (obj["id"].toString() == id) {
            found = true;
        } else {
            newArr.append(obj);
        }
    }

    QVariantMap result;
    if (!found) {
        result["success"] = false;
        result["error"] = "Talent not found: " + id;
        emit errorOccurred(result["error"].toString());
    } else {
        root["talents"] = newArr;
        if (writeTalentsFile(root)) {
            result["success"] = true;
            emit talentDeleted(id);

            // Remove photo if exists
            QString photo = photosDir() + "/" + id + ".jpg";
            QFile::remove(photo);

            refreshTalents();
        } else {
            result["success"] = false;
            result["error"] = "Failed to write talents file";
            emit errorOccurred(result["error"].toString());
        }
    }

    return result;
}

QVariantMap TalentManager::enrollPhoto(const QString& id, const QString& photoPath)
{
    QVariantMap result;

    if (!QFile::exists(photoPath)) {
        result["success"] = false;
        result["error"] = "Photo file not found: " + photoPath;
        emit errorOccurred(result["error"].toString());
        return result;
    }

    // Copy photo to talent photos directory
    QString dest = photosDir() + "/" + id + ".jpg";
    QFile::remove(dest); // Remove old photo if exists
    if (QFile::copy(photoPath, dest)) {
        result["success"] = true;
        result["photo_path"] = dest;
        refreshTalents();
    } else {
        result["success"] = false;
        result["error"] = "Failed to copy photo";
        emit errorOccurred(result["error"].toString());
    }

    return result;
}

QString TalentManager::capturePhotoFromCamera(int cameraIndex)
{
    Q_UNUSED(cameraIndex)

    // Save path for the capture
    QString filename = QStringLiteral("capture_%1.jpg")
        .arg(QDateTime::currentMSecsSinceEpoch());
    QString path = photosDir() + "/" + filename;

    qInfo() << "[TalentManager] Photo capture requested:" << path;
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
