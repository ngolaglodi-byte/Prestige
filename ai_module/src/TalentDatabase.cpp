// ============================================================
// Prestige AI — Talent Database Implementation
// JSON load/save with cosine similarity matching
// ============================================================

#include "TalentDatabase.h"
#include "FaceRecognizer.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMutexLocker>
#include <QDebug>
#include <QUuid>

namespace prestige { namespace ai {

TalentDatabase::TalentDatabase(const QString& dbPath, QObject* parent)
    : QObject(parent)
    , m_dbPath(dbPath)
{
}

void TalentDatabase::load()
{
    QMutexLocker locker(&m_mutex);

    QFile file(m_dbPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qInfo() << "[AI] Talent database not found at:" << m_dbPath << "— starting empty";
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        qWarning() << "[AI] Invalid talent database format";
        return;
    }

    QJsonObject root = doc.object();
    m_nextId = root["next_id"].toInt(1);

    QJsonArray arr = root["talents"].toArray();
    m_talents.clear();

    for (const auto& v : arr) {
        QJsonObject obj = v.toObject();
        TalentRecord rec;
        rec.id = obj["id"].toString();
        rec.name = obj["name"].toString();
        rec.role = obj["role"].toString();
        rec.overlayStyle = obj["overlay_style"].toString("default");
        rec.active = obj["active"].toBool(true);

        // Load embeddings (array of arrays of floats)
        QJsonArray embArr = obj["embeddings"].toArray();
        for (const auto& embVal : embArr) {
            QJsonArray floatArr = embVal.toArray();
            std::vector<float> emb;
            emb.reserve(floatArr.size());
            for (const auto& f : floatArr)
                emb.push_back(static_cast<float>(f.toDouble()));
            rec.embeddings.push_back(std::move(emb));
        }

        m_talents.append(rec);
    }

    qInfo() << "[AI] Loaded" << m_talents.size() << "talents from" << m_dbPath;
    emit talentsChanged();
}

void TalentDatabase::save()
{
    QMutexLocker locker(&m_mutex);

    QJsonArray arr;
    for (const auto& rec : m_talents) {
        QJsonObject obj;
        obj["id"] = rec.id;
        obj["name"] = rec.name;
        obj["role"] = rec.role;
        obj["overlay_style"] = rec.overlayStyle;
        obj["active"] = rec.active;

        QJsonArray embArr;
        for (const auto& emb : rec.embeddings) {
            QJsonArray floatArr;
            for (float f : emb)
                floatArr.append(static_cast<double>(f));
            embArr.append(floatArr);
        }
        obj["embeddings"] = embArr;

        arr.append(obj);
    }

    QJsonObject root;
    root["next_id"] = m_nextId;
    root["talents"] = arr;

    QFile file(m_dbPath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "[AI] Failed to save talent database:" << m_dbPath;
        return;
    }

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();

    qInfo() << "[AI] Saved" << m_talents.size() << "talents to" << m_dbPath;
}

QVariantList TalentDatabase::talentList() const
{
    QMutexLocker locker(&m_mutex);
    QVariantList list;
    for (const auto& rec : m_talents) {
        QVariantMap map;
        map["id"] = rec.id;
        map["name"] = rec.name;
        map["role"] = rec.role;
        map["overlay_style"] = rec.overlayStyle;
        map["active"] = rec.active;
        map["has_embeddings"] = !rec.embeddings.empty();
        map["embedding_count"] = static_cast<int>(rec.embeddings.size());
        list.append(map);
    }
    return list;
}

QVariantMap TalentDatabase::addTalent(const QString& name, const QString& role)
{
    QMutexLocker locker(&m_mutex);

    TalentRecord rec;
    rec.id = QString::number(m_nextId++);
    rec.name = name;
    rec.role = role;
    m_talents.append(rec);

    locker.unlock();
    save();
    emit talentsChanged();

    QVariantMap result;
    result["success"] = true;
    result["id"] = rec.id;
    return result;
}

bool TalentDatabase::deleteTalent(const QString& id)
{
    QMutexLocker locker(&m_mutex);

    for (int i = 0; i < m_talents.size(); ++i) {
        if (m_talents[i].id == id) {
            m_talents.removeAt(i);
            locker.unlock();
            save();
            emit talentsChanged();
            return true;
        }
    }
    return false;
}

bool TalentDatabase::updateTalent(const QString& id, const QString& name, const QString& role)
{
    QMutexLocker locker(&m_mutex);

    for (auto& rec : m_talents) {
        if (rec.id == id) {
            rec.name = name;
            rec.role = role;
            locker.unlock();
            save();
            emit talentsChanged();
            return true;
        }
    }
    return false;
}

bool TalentDatabase::enrollEmbedding(const QString& id, const std::vector<float>& embedding)
{
    QMutexLocker locker(&m_mutex);

    for (auto& rec : m_talents) {
        if (rec.id == id) {
            rec.embeddings.push_back(embedding);
            locker.unlock();
            save();
            return true;
        }
    }
    return false;
}

TalentDatabase::MatchResult TalentDatabase::matchEmbedding(
    const std::vector<float>& embedding, float threshold) const
{
    QMutexLocker locker(&m_mutex);

    MatchResult best;
    float bestScore = threshold;

    for (const auto& rec : m_talents) {
        if (!rec.active) continue;

        for (const auto& refEmb : rec.embeddings) {
            float sim = FaceRecognizer::cosineSimilarity(embedding, refEmb);
            if (sim > bestScore) {
                bestScore = sim;
                best.id = rec.id;
                best.name = rec.name;
                best.role = rec.role;
                best.confidence = sim;
                best.showOverlay = true;
                best.overlayStyle = rec.overlayStyle;
            }
        }
    }

    return best;
}

}} // namespace prestige::ai
