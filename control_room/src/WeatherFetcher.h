#pragma once

// ============================================================
// Prestige AI — Weather Fetcher
// Fetches real-time weather from OpenWeatherMap API.
// Fallback: manual mode if no API key or no internet.
// ============================================================

#include <QObject>
#include <QString>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QDateTime>

namespace prestige {

class WeatherFetcher : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool autoMode READ isAutoMode WRITE setAutoMode NOTIFY autoModeChanged)
    Q_PROPERTY(QString apiKey READ apiKey WRITE setApiKey NOTIFY apiKeyChanged)
    Q_PROPERTY(QString city READ city WRITE setCity NOTIFY cityChanged)
    Q_PROPERTY(int refreshMinutes READ refreshMinutes WRITE setRefreshMinutes NOTIFY refreshMinutesChanged)

    // Weather data (auto-filled or manual)
    Q_PROPERTY(double temperature READ temperature WRITE setTemperature NOTIFY weatherChanged)
    Q_PROPERTY(QString unit READ unit WRITE setUnit NOTIFY weatherChanged)
    Q_PROPERTY(QString condition READ condition WRITE setCondition NOTIFY weatherChanged)
    Q_PROPERTY(QString conditionIcon READ conditionIcon NOTIFY weatherChanged)
    Q_PROPERTY(int humidity READ humidity NOTIFY weatherChanged)
    Q_PROPERTY(double windSpeed READ windSpeed NOTIFY weatherChanged)
    Q_PROPERTY(QString windDir READ windDir NOTIFY weatherChanged)
    Q_PROPERTY(double feelsLike READ feelsLike NOTIFY weatherChanged)
    Q_PROPERTY(QString description READ description NOTIFY weatherChanged)
    Q_PROPERTY(QString country READ country NOTIFY weatherChanged)

    // Status
    Q_PROPERTY(bool fetching READ isFetching NOTIFY fetchingChanged)
    Q_PROPERTY(QString lastUpdate READ lastUpdate NOTIFY weatherChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorChanged)

public:
    explicit WeatherFetcher(QObject* parent = nullptr);

    bool isAutoMode() const { return m_autoMode; }
    void setAutoMode(bool on);

    QString apiKey() const { return m_apiKey; }
    void setApiKey(const QString& key);

    QString city() const { return m_city; }
    void setCity(const QString& city);

    int refreshMinutes() const { return m_refreshMin; }
    void setRefreshMinutes(int min);

    double temperature() const { return m_temperature; }
    void setTemperature(double t);

    QString unit() const { return m_unit; }
    void setUnit(const QString& u);

    QString condition() const { return m_condition; }
    void setCondition(const QString& c);

    QString conditionIcon() const { return m_conditionIcon; }
    int humidity() const { return m_humidity; }
    double windSpeed() const { return m_windSpeed; }
    QString windDir() const { return m_windDir; }
    double feelsLike() const { return m_feelsLike; }
    QString description() const { return m_description; }
    QString country() const { return m_country; }

    bool isFetching() const { return m_fetching; }
    QString lastUpdate() const { return m_lastUpdate.isValid() ? m_lastUpdate.toString("HH:mm") : QString::fromUtf8("\u2014"); }
    QString errorString() const { return m_error; }

    Q_INVOKABLE void fetchNow();

signals:
    void autoModeChanged();
    void apiKeyChanged();
    void cityChanged();
    void refreshMinutesChanged();
    void weatherChanged();
    void fetchingChanged();
    void errorChanged();

private slots:
    void doFetch();

private:
    void parseResponse(const QByteArray& data);
    QString mapConditionIcon(int owmCode) const;
    QString mapConditionName(int owmCode) const;
    double degToCompass(double deg) const;

    QNetworkAccessManager* m_nam = nullptr;
    QTimer*     m_timer        = nullptr;

    bool        m_autoMode     = false;
    QString     m_apiKey;
    QString     m_city         = "Paris";
    int         m_refreshMin   = 15;

    double      m_temperature  = 18.0;
    QString     m_unit         = QString::fromUtf8("\u00B0C");
    QString     m_condition    = "ensoleille";
    QString     m_conditionIcon = QString::fromUtf8("\u2600");
    int         m_humidity     = 0;
    double      m_windSpeed    = 0.0;
    QString     m_windDir;
    double      m_feelsLike    = 0.0;
    QString     m_description;
    QString     m_country;

    bool        m_fetching     = false;
    QDateTime   m_lastUpdate;
    QString     m_error;
};

} // namespace prestige
