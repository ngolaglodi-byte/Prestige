// ============================================================
// Prestige AI — Weather Fetcher Implementation
// Uses OpenWeatherMap API (free tier: 1000 calls/day)
// ============================================================

#include "WeatherFetcher.h"
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QUrlQuery>
#include <QDebug>
#include <cmath>

namespace prestige {

WeatherFetcher::WeatherFetcher(QObject* parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
    , m_timer(new QTimer(this))
{
    m_timer->setInterval(m_refreshMin * 60 * 1000);
    connect(m_timer, &QTimer::timeout, this, &WeatherFetcher::doFetch);
}

void WeatherFetcher::setAutoMode(bool on)
{
    if (m_autoMode == on) return;
    m_autoMode = on;
    emit autoModeChanged();

    if (on && !m_apiKey.isEmpty() && !m_city.isEmpty()) {
        doFetch();
        m_timer->start();
    } else {
        m_timer->stop();
    }
}

void WeatherFetcher::setApiKey(const QString& key)
{
    if (m_apiKey == key) return;
    m_apiKey = key.trimmed();
    emit apiKeyChanged();

    if (m_autoMode && !m_apiKey.isEmpty() && !m_city.isEmpty())
        doFetch();
}

void WeatherFetcher::setCity(const QString& city)
{
    if (m_city == city) return;
    m_city = city.trimmed();
    emit cityChanged();

    if (m_autoMode && !m_apiKey.isEmpty() && !m_city.isEmpty())
        doFetch();
}

void WeatherFetcher::setRefreshMinutes(int min)
{
    min = qBound(5, min, 120);
    if (m_refreshMin == min) return;
    m_refreshMin = min;
    m_timer->setInterval(min * 60 * 1000);
    emit refreshMinutesChanged();
}

void WeatherFetcher::setTemperature(double t)
{
    if (m_autoMode) return;  // In auto mode, don't allow manual override
    m_temperature = t;
    emit weatherChanged();
}

void WeatherFetcher::setUnit(const QString& u)
{
    m_unit = u;
    emit weatherChanged();
}

void WeatherFetcher::setCondition(const QString& c)
{
    if (m_autoMode) return;
    m_condition = c;
    emit weatherChanged();
}

void WeatherFetcher::fetchNow()
{
    if (!m_apiKey.isEmpty() && !m_city.isEmpty())
        doFetch();
    else {
        m_error = m_apiKey.isEmpty()
            ? QString::fromUtf8("Cl\u00e9 API manquante")
            : QString::fromUtf8("Ville manquante");
        emit errorChanged();
    }
}

void WeatherFetcher::doFetch()
{
    if (m_apiKey.isEmpty() || m_city.isEmpty()) return;

    m_fetching = true;
    m_error.clear();
    emit fetchingChanged();

    // Build OpenWeatherMap URL
    // https://api.openweathermap.org/data/2.5/weather?q=Paris&appid=KEY&units=metric&lang=fr
    QUrl url(QStringLiteral("https://api.openweathermap.org/data/2.5/weather"));
    QUrlQuery query;
    query.addQueryItem("q", m_city);
    query.addQueryItem("appid", m_apiKey);
    query.addQueryItem("units", m_unit.contains("F") ? "imperial" : "metric");
    query.addQueryItem("lang", "fr");
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "PrestigeAI/1.0");

    auto* reply = m_nam->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        m_fetching = false;
        emit fetchingChanged();

        if (reply->error() != QNetworkReply::NoError) {
            int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            if (status == 401)
                m_error = QString::fromUtf8("Cl\u00e9 API invalide");
            else if (status == 404)
                m_error = QString::fromUtf8("Ville introuvable");
            else
                m_error = reply->errorString();
            qWarning() << "[Weather] Fetch error:" << m_error;
            emit errorChanged();
            return;
        }

        parseResponse(reply->readAll());
        m_lastUpdate = QDateTime::currentDateTime();
        m_error.clear();
        emit weatherChanged();
        emit errorChanged();
        qInfo() << "[Weather]" << m_city << m_temperature << m_unit << m_condition;
    });
}

void WeatherFetcher::parseResponse(const QByteArray& data)
{
    auto doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) return;
    auto root = doc.object();

    // Main temperature data
    auto main = root["main"].toObject();
    m_temperature = std::round(main["temp"].toDouble() * 10.0) / 10.0;
    m_feelsLike = std::round(main["feels_like"].toDouble() * 10.0) / 10.0;
    m_humidity = main["humidity"].toInt();

    // Wind
    auto wind = root["wind"].toObject();
    m_windSpeed = std::round(wind["speed"].toDouble() * 10.0) / 10.0;
    double windDeg = wind["deg"].toDouble();
    // Convert degrees to compass direction
    static const char* dirs[] = {"N", "NE", "E", "SE", "S", "SO", "O", "NO"};
    int idx = static_cast<int>(std::round(windDeg / 45.0)) % 8;
    m_windDir = QString::fromLatin1(dirs[idx]);

    // Weather condition
    auto weather = root["weather"].toArray();
    if (!weather.isEmpty()) {
        auto w = weather[0].toObject();
        int code = w["id"].toInt();
        m_description = w["description"].toString();
        m_conditionIcon = mapConditionIcon(code);
        m_condition = mapConditionName(code);
    }

    // Country
    auto sys = root["sys"].toObject();
    m_country = sys["country"].toString();

    // Update city name to match API response (proper capitalization)
    QString apiName = root["name"].toString();
    if (!apiName.isEmpty())
        m_city = apiName;
}

QString WeatherFetcher::mapConditionIcon(int code) const
{
    // OpenWeatherMap condition codes → Unicode weather icons
    if (code >= 200 && code < 300) return QString::fromUtf8("\u26A1");        // Thunderstorm
    if (code >= 300 && code < 400) return QString::fromUtf8("\xF0\x9F\x8C\xA7");  // Drizzle 🌧
    if (code >= 500 && code < 600) return QString::fromUtf8("\u2602");        // Rain
    if (code >= 600 && code < 700) return QString::fromUtf8("\u2744");        // Snow
    if (code >= 700 && code < 800) return QString::fromUtf8("\xF0\x9F\x8C\xAB");  // Fog/Mist 🌫
    if (code == 800)               return QString::fromUtf8("\u2600");        // Clear
    if (code == 801)               return QString::fromUtf8("\u26C5");        // Few clouds
    if (code >= 802)               return QString::fromUtf8("\u2601");        // Cloudy
    return QString::fromUtf8("\u2600");
}

QString WeatherFetcher::mapConditionName(int code) const
{
    if (code >= 200 && code < 300) return "orage";
    if (code >= 300 && code < 400) return "bruine";
    if (code >= 500 && code < 600) return "pluie";
    if (code >= 600 && code < 700) return "neige";
    if (code >= 700 && code < 800) return "brouillard";
    if (code == 800)               return "ensoleille";
    if (code >= 801)               return "nuageux";
    return "ensoleille";
}

} // namespace prestige
