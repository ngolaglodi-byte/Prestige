#pragma once

// ============================================================
// Prestige AI — Talent Database (JSON-backed)
// ============================================================

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <QList>
#include <vector>
#include <QMutex>

namespace prestige { namespace ai {

struct TalentRecord {
    QString id;
    QString name;
    QString role;
    std::vector<std::vector<float>> embeddings; // Multiple reference embeddings
    QString overlayStyle = "default";
    bool active = true;
};

class TalentDatabase : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList talents READ talentList NOTIFY talentsChanged)
    Q_PROPERTY(int count READ count NOTIFY talentsChanged)

public:
    explicit TalentDatabase(const QString& dbPath = "talents.json", QObject* parent = nullptr);

    void setDbPath(const QString& path) { m_dbPath = path; }
    void load();
    void save();

    int count() const { return m_talents.size(); }
    QVariantList talentList() const;

    Q_INVOKABLE QVariantMap addTalent(const QString& name, const QString& role);
    Q_INVOKABLE bool deleteTalent(const QString& id);
    Q_INVOKABLE bool updateTalent(const QString& id, const QString& name, const QString& role);
    bool enrollEmbedding(const QString& id, const std::vector<float>& embedding);

    // Recognition: find best matching talent for an embedding
    struct MatchResult {
        QString id;
        QString name;
        QString role;
        float confidence = 0.0f;
        bool showOverlay = false;
        QString overlayStyle = "default";
    };
    MatchResult matchEmbedding(const std::vector<float>& embedding, float threshold = 0.4f) const;

    const QList<TalentRecord>& records() const { return m_talents; }

signals:
    void talentsChanged();

private:
    QString m_dbPath;
    QList<TalentRecord> m_talents;
    mutable QMutex m_mutex;
    int m_nextId = 1;
};

}} // namespace prestige::ai
