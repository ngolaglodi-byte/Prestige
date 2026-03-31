#include "LicenseManager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QUrl>
#include <QSettings>
#include <QSysInfo>
#include <QCryptographicHash>
#include <QDate>
#include <QTimer>
#include <QDebug>

namespace prestige {

const QString LicenseManager::API_URL = QStringLiteral("https://qkcchctrmrpdyseplbvb.supabase.co/functions/v1/license-api");
const QString LicenseManager::API_KEY = QStringLiteral("eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InFrY2NoY3RybXJwZHlzZXBsYnZiIiwicm9sZSI6ImFub24iLCJpYXQiOjE3NzExMjExNDQsImV4cCI6MjA4NjY5NzE0NH0.rdCOcwqBd4DYEEbI9rU8kqotJ6VdWOLRYMEM5w5aD2s");

LicenseManager::LicenseManager(QObject* parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{
    m_expirationTimer = new QTimer(this);
    connect(m_expirationTimer, &QTimer::timeout, this, [this]() {
        if (m_activated && isExpired()) {
            m_activated = false;
            m_error = "Votre licence a expiré. Veuillez la renouveler.";
            clearStoredLicense();
            emit statusChanged();
            emit errorChanged();
            qWarning() << "[License] EXPIRED — license deactivated";
        }
        emit statusChanged(); // Refresh daysRemaining
    });
    m_expirationTimer->start(60000); // Check every minute
}

int LicenseManager::daysRemaining() const
{
    if (m_expiration.isEmpty()) return -1;
    QDate exp = QDate::fromString(m_expiration, "yyyy-MM-dd");
    if (!exp.isValid()) return -1;
    return QDate::currentDate().daysTo(exp);
}

bool LicenseManager::isExpired() const
{
    return daysRemaining() >= 0 && daysRemaining() <= 0;
}

bool LicenseManager::isExpiringSoon() const
{
    int days = daysRemaining();
    return days > 0 && days <= 30;
}

QString LicenseManager::statusMessage() const
{
    if (!m_activated) return QString();
    int days = daysRemaining();
    if (days < 0) return "Licence active";
    if (days <= 0) return "Licence expirée";
    if (days <= 7) return QString("Expire dans %1 jour(s) !").arg(days);
    if (days <= 30) return QString("Expire dans %1 jours").arg(days);
    return QString("Valide — expire le %1").arg(m_expiration);
}

QString LicenseManager::machineId() const
{
    // Generate unique machine ID from hardware info
    QString raw = QSysInfo::machineUniqueId().isEmpty()
        ? (QSysInfo::machineHostName() + QSysInfo::productType() + QSysInfo::currentCpuArchitecture())
        : QString::fromUtf8(QSysInfo::machineUniqueId());
    return QString::fromUtf8(QCryptographicHash::hash(raw.toUtf8(), QCryptographicHash::Sha256).toHex().left(32));
}

void LicenseManager::saveLicense(const QString& key)
{
    QSettings settings("PrestigeAI", "PrestigeControl");
    settings.setValue("license/key", key);
    settings.setValue("license/machine_id", machineId());
}

QString LicenseManager::loadStoredKey() const
{
    QSettings settings("PrestigeAI", "PrestigeControl");
    return settings.value("license/key").toString();
}

void LicenseManager::clearStoredLicense()
{
    QSettings settings("PrestigeAI", "PrestigeControl");
    settings.remove("license/key");
    settings.remove("license/machine_id");
}

bool LicenseManager::hasStoredLicense() const
{
    return !loadStoredKey().isEmpty();
}

void LicenseManager::activateKey(const QString& key)
{
    if (key.trimmed().isEmpty()) {
        m_error = "Veuillez entrer une clé de licence";
        emit errorChanged();
        return;
    }

    m_error.clear();
    emit errorChanged();

    QJsonObject params;
    params["action"] = QStringLiteral("activate_key");
    params["key"] = key.trimmed();
    params["machine_id"] = machineId();

    m_checking = true;
    emit checkingChanged();

    QUrl url(API_URL);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + API_KEY).toUtf8());

    auto* reply = m_nam->post(request, QJsonDocument(params).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply, key]() {
        reply->deleteLater();
        m_checking = false;
        emit checkingChanged();

        if (reply->error() != QNetworkReply::NoError) {
            m_error = "Impossible de contacter le serveur de licence. Vérifiez votre connexion internet.";
            emit errorChanged();
            emit activationFailed(m_error);
            return;
        }

        auto doc = QJsonDocument::fromJson(reply->readAll());
        auto obj = doc.object();

        if (obj["success"].toBool()) {
            m_activated = true;
            m_key = key.trimmed();
            m_type = obj["license_type"].toString();
            m_expiration = obj["expiration_date"].toString().left(10); // YYYY-MM-DD
            m_copyright = obj["copyright"].toString();
            m_product = obj["product"].toString();

            if (isExpired()) {
                m_activated = false;
                m_error = QString("Votre licence a expiré le %1. Veuillez la renouveler.").arg(m_expiration);
                clearStoredLicense();
                emit statusChanged();
                emit errorChanged();
                emit activationFailed(m_error);
                return;
            }

            saveLicense(m_key);
            emit statusChanged();
            emit activationSuccess();
            qInfo() << "[License] Activated:" << m_type << "expires" << m_expiration;
        } else {
            QString serverError = obj["error"].toString();
            if (serverError.contains("already activated", Qt::CaseInsensitive) || serverError.contains("another machine", Qt::CaseInsensitive) || serverError.contains("another device", Qt::CaseInsensitive)) {
                m_error = "Cette licence est déjà activée sur un autre appareil.";
            } else if (serverError.contains("invalid", Qt::CaseInsensitive) || serverError.contains("not found", Qt::CaseInsensitive)) {
                m_error = "Clé de licence invalide. Vérifiez votre clé et réessayez.";
            } else if (serverError.contains("expired", Qt::CaseInsensitive) || serverError.contains("expir", Qt::CaseInsensitive)) {
                m_error = "Votre licence a expiré. Veuillez la renouveler.";
            } else {
                m_error = serverError.isEmpty() ? "Activation échouée" : serverError;
            }
            emit errorChanged();
            emit activationFailed(m_error);
            qWarning() << "[License] Activation failed:" << m_error;
        }
    });
}

void LicenseManager::validateStoredKey()
{
    QString stored = loadStoredKey();
    if (stored.isEmpty()) {
        m_activated = false;
        emit statusChanged();
        return;
    }

    m_checking = true;
    emit checkingChanged();

    QJsonObject params;
    params["action"] = QStringLiteral("validate_key");
    params["key"] = stored;

    QUrl url(API_URL);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + API_KEY).toUtf8());

    auto* reply = m_nam->post(request, QJsonDocument(params).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply, stored]() {
        reply->deleteLater();
        m_checking = false;
        emit checkingChanged();

        if (reply->error() != QNetworkReply::NoError) {
            // Network error - allow offline use if previously activated
            m_activated = true;
            m_key = stored;
            m_type = "Offline";
            qInfo() << "[License] Offline mode - using stored license";
            emit statusChanged();
            return;
        }

        auto doc = QJsonDocument::fromJson(reply->readAll());
        auto obj = doc.object();

        if (obj["valid"].toBool()) {
            m_activated = true;
            m_key = stored;
            m_type = obj["license_type"].toString();
            m_expiration = obj["expiration_date"].toString().left(10);
            m_copyright = obj["copyright"].toString();
            m_product = obj["product"].toString();

            if (isExpired()) {
                m_activated = false;
                m_error = QString("Votre licence a expiré le %1. Veuillez la renouveler.").arg(m_expiration);
                clearStoredLicense();
                emit statusChanged();
                emit errorChanged();
                qWarning() << "[License] EXPIRED at validation:" << m_expiration;
                return;
            }

            emit statusChanged();
            qInfo() << "[License] Valid:" << m_type << "expires" << m_expiration;
        } else {
            m_activated = false;
            m_error = obj["error"].toString("Licence invalide ou expirée");
            clearStoredLicense();
            emit statusChanged();
            emit errorChanged();
            qWarning() << "[License] Invalid:" << m_error;
        }
    });
}

void LicenseManager::deactivate()
{
    if (m_key.isEmpty()) return;

    QJsonObject params;
    params["action"] = QStringLiteral("deactivate_key");
    params["key"] = m_key;
    params["machine_id"] = machineId();

    QUrl url(API_URL);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + API_KEY).toUtf8());

    auto* reply = m_nam->post(request, QJsonDocument(params).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        m_activated = false;
        m_key.clear();
        m_type.clear();
        m_expiration.clear();
        clearStoredLicense();
        emit statusChanged();
        qInfo() << "[License] Deactivated";
    });
}

} // namespace prestige
