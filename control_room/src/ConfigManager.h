#pragma once

// ============================================================
// Prestige AI — Configuration Manager
// Persistent JSON configuration for the Control Room
// ============================================================

#include <QObject>
#include <QJsonObject>
#include <QString>
#include <QStringList>
#include <QSize>

namespace prestige {

class ConfigManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString inputSource READ inputSource WRITE setInputSource NOTIFY inputSourceChanged)
    Q_PROPERTY(QString talentsDbPath READ talentsDbPath WRITE setTalentsDbPath NOTIFY talentsDbPathChanged)
    Q_PROPERTY(QString channelName READ channelName WRITE setChannelName NOTIFY channelNameChanged)

public:
    explicit ConfigManager(QObject* parent = nullptr);

    void load();
    void save();

    // Input
    QString inputSource() const;
    void setInputSource(const QString& source);
    QSize inputResolution() const;

    // Channel name
    QString channelName() const;
    void setChannelName(const QString& name);

    // Talents
    QString talentsDbPath() const;
    void setTalentsDbPath(const QString& path);

    // Outputs
    QStringList activeOutputTypes() const;
    Q_INVOKABLE void setOutputEnabled(const QString& type, bool enabled);
    Q_INVOKABLE QString outputUrl(const QString& type) const;
    Q_INVOKABLE void setOutputUrl(const QString& type, const QString& url);

    // Overlay styles
    Q_INVOKABLE QJsonObject overlayStyle(const QString& name) const;
    Q_INVOKABLE void setOverlayStyle(const QString& name, const QJsonObject& style);

signals:
    void inputSourceChanged();
    void talentsDbPathChanged();
    void channelNameChanged();
    void configChanged();

private:
    QString configPath() const;

    QJsonObject m_config;
};

} // namespace prestige
