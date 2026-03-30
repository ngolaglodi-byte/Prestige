#pragma once

// ============================================================
// Prestige AI — Talent Manager
// C++ client for Python talent management server (ZMQ REQ/REP)
// ============================================================

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <QJsonObject>
#include <QImage>

namespace prestige {

class TalentManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList talents READ talents NOTIFY talentsChanged)
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectionChanged)

public:
    explicit TalentManager(QObject* parent = nullptr);
    ~TalentManager() override;

    QVariantList talents() const { return m_talents; }
    bool isConnected() const { return m_connected; }

    // Core CRUD — all callable from QML
    Q_INVOKABLE void refreshTalents();
    Q_INVOKABLE QVariantMap addTalent(const QString& name, const QString& role, const QString& photoPath);
    Q_INVOKABLE QVariantMap updateTalent(const QString& id, const QString& name, const QString& role);
    Q_INVOKABLE QVariantMap deleteTalent(const QString& id);
    Q_INVOKABLE QVariantMap enrollPhoto(const QString& id, const QString& photoPath);

    // Photo capture helper
    Q_INVOKABLE QString capturePhotoFromCamera(int cameraIndex = 0);
    Q_INVOKABLE QString talentPhotoPath(const QString& id) const;

signals:
    void talentsChanged();
    void connectionChanged();
    void talentAdded(const QString& id, const QString& name);
    void talentDeleted(const QString& id);
    void errorOccurred(const QString& error);

private:
    QJsonObject sendRequest(const QJsonObject& request);
    QString photosDir() const;

    QVariantList m_talents;
    bool         m_connected = false;
    void*        m_zmqContext = nullptr;
    void*        m_zmqSocket  = nullptr;
};

} // namespace prestige
