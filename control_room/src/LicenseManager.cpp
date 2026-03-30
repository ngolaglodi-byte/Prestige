#include "LicenseManager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QUrl>
#include <QSettings>
#include <QSysInfo>
#include <QCryptographicHash>
#include <QDebug>

namespace prestige {

const QString LicenseManager::API_URL = QStringLiteral("https://qkcchctrmrpdyseplbvb.supabase.co/functions/v1/license-api");
const QString LicenseManager::API_KEY = QStringLiteral("eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InFrY2NoY3RybXJwZHlzZXBsYnZiIiwicm9sZSI6ImFub24iLCJpYXQiOjE3NzExMjExNDQsImV4cCI6MjA4NjY5NzE0NH0.rdCOcwqBd4DYEEbI9rU8kqotJ6VdWOLRYMEM5w5aD2s");

LicenseManager::LicenseManager(QObject* parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{
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
        m_error = "Veuillez entrer une cle de licence";
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
            m_error = "Erreur reseau: " + reply->errorString();
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
            saveLicense(m_key);
            emit statusChanged();
            emit activationSuccess();
            qInfo() << "[License] Activated:" << m_type << "expires" << m_expiration;
        } else {
            m_error = obj["error"].toString("Activation echouee");
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
            emit statusChanged();
            qInfo() << "[License] Valid:" << m_type << "expires" << m_expiration;
        } else {
            m_activated = false;
            m_error = obj["error"].toString("Licence invalide ou expiree");
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
