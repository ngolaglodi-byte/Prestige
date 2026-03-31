#pragma once
#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QDate>
#include <QTimer>

namespace prestige {

class LicenseManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isActivated READ isActivated NOTIFY statusChanged)
    Q_PROPERTY(bool isChecking READ isChecking NOTIFY checkingChanged)
    Q_PROPERTY(QString licenseKey READ licenseKey NOTIFY statusChanged)
    Q_PROPERTY(QString licenseType READ licenseType NOTIFY statusChanged)
    Q_PROPERTY(QString expirationDate READ expirationDate NOTIFY statusChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorChanged)
    Q_PROPERTY(QString copyright READ copyright NOTIFY statusChanged)
    Q_PROPERTY(QString product READ product NOTIFY statusChanged)
    Q_PROPERTY(int daysRemaining READ daysRemaining NOTIFY statusChanged)
    Q_PROPERTY(bool isExpired READ isExpired NOTIFY statusChanged)
    Q_PROPERTY(bool isExpiringSoon READ isExpiringSoon NOTIFY statusChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusChanged)

public:
    explicit LicenseManager(QObject* parent = nullptr);

    bool isActivated() const { return m_activated; }
    bool isChecking() const { return m_checking; }
    QString licenseKey() const { return m_key; }
    QString licenseType() const { return m_type; }
    QString expirationDate() const { return m_expiration; }
    QString errorString() const { return m_error; }
    QString copyright() const { return m_copyright; }
    QString product() const { return m_product; }
    int daysRemaining() const;
    bool isExpired() const;
    bool isExpiringSoon() const;
    QString statusMessage() const;

    Q_INVOKABLE void activateKey(const QString& key);
    Q_INVOKABLE void validateStoredKey();
    Q_INVOKABLE void deactivate();
    Q_INVOKABLE bool hasStoredLicense() const;

signals:
    void statusChanged();
    void checkingChanged();
    void errorChanged();
    void activationSuccess();
    void activationFailed(const QString& reason);

private:
    void sendRequest(const QString& action, const QJsonObject& params);
    QString machineId() const;
    void saveLicense(const QString& key);
    QString loadStoredKey() const;
    void clearStoredLicense();

    QNetworkAccessManager* m_nam = nullptr;
    bool m_activated = false;
    bool m_checking = false;
    QString m_key;
    QString m_type;
    QString m_expiration;
    QString m_error;
    QString m_copyright;
    QString m_product;

    QTimer* m_expirationTimer = nullptr;

    static const QString API_URL;
    static const QString API_KEY;
};

} // namespace prestige
