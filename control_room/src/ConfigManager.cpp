// ============================================================
// Prestige AI — Configuration Manager Implementation
// ============================================================

#include "ConfigManager.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDebug>

namespace prestige {

ConfigManager::ConfigManager(QObject* parent)
    : QObject(parent)
{
    // Default config
    m_config = QJsonObject{
        {"input", QJsonObject{
            {"source", "webcam"},
            {"resolution", QJsonObject{{"width", 1920}, {"height", 1080}}},
            {"fps", 25}
        }},
        {"channel_name", ""},
        {"talents_db", "talents.json"},
        {"outputs", QJsonObject{
            {"sdi",  QJsonObject{{"enabled", false}, {"device", ""}}},
            {"ndi",  QJsonObject{{"enabled", false}, {"name", "Prestige AI"}}},
            {"rtmp", QJsonObject{{"enabled", false}, {"url", ""}}},
            {"srt",  QJsonObject{{"enabled", false}, {"url", ""}}}
        }},
        {"overlay_styles", QJsonObject{
            {"default", QJsonObject{
                {"accent_color", "#E30613"},
                {"bg_color", "rgba(0,0,0,0.75)"},
                {"font_family", "Noto Sans"},
                {"font_size", 18}
            }}
        }}
    };
}

void ConfigManager::load()
{
    QString path = configPath();
    QFile file(path);

    if (!file.exists()) {
        qInfo() << "[ConfigManager] No config file found — using defaults";
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "[ConfigManager] Cannot open config:" << path;
        return;
    }

    auto doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isObject()) {
        m_config = doc.object();
        qInfo() << "[ConfigManager] Loaded config from" << path;
    } else {
        qWarning() << "[ConfigManager] Invalid config format — using defaults";
    }
}

void ConfigManager::save()
{
    QString path = configPath();

    // Ensure directory exists
    QDir().mkpath(QFileInfo(path).absolutePath());

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "[ConfigManager] Cannot write config:" << path;
        return;
    }

    file.write(QJsonDocument(m_config).toJson(QJsonDocument::Indented));
    qInfo() << "[ConfigManager] Saved config to" << path;
    emit configChanged();
}

QString ConfigManager::inputSource() const
{
    return m_config["input"].toObject()["source"].toString("webcam");
}

void ConfigManager::setInputSource(const QString& source)
{
    auto input = m_config["input"].toObject();
    input["source"] = source;
    m_config["input"] = input;
    emit inputSourceChanged();
    save();
}

QSize ConfigManager::inputResolution() const
{
    auto res = m_config["input"].toObject()["resolution"].toObject();
    return {res["width"].toInt(1920), res["height"].toInt(1080)};
}

QString ConfigManager::channelName() const
{
    return m_config["channel_name"].toString("");
}

void ConfigManager::setChannelName(const QString& name)
{
    if (channelName() == name) return;
    m_config["channel_name"] = name;
    emit channelNameChanged();
    save();
}

QString ConfigManager::talentsDbPath() const
{
    return m_config["talents_db"].toString("talents.json");
}

void ConfigManager::setTalentsDbPath(const QString& path)
{
    m_config["talents_db"] = path;
    emit talentsDbPathChanged();
    save();
}

QStringList ConfigManager::activeOutputTypes() const
{
    QStringList result;
    auto outputs = m_config["outputs"].toObject();
    for (auto it = outputs.begin(); it != outputs.end(); ++it) {
        if (it.value().toObject()["enabled"].toBool())
            result.append(it.key());
    }
    return result;
}

void ConfigManager::setOutputEnabled(const QString& type, bool enabled)
{
    auto outputs = m_config["outputs"].toObject();
    auto entry = outputs[type].toObject();
    entry["enabled"] = enabled;
    outputs[type] = entry;
    m_config["outputs"] = outputs;
    save();
}

QString ConfigManager::outputUrl(const QString& type) const
{
    return m_config["outputs"].toObject()[type].toObject()["url"].toString();
}

void ConfigManager::setOutputUrl(const QString& type, const QString& url)
{
    auto outputs = m_config["outputs"].toObject();
    auto entry = outputs[type].toObject();
    entry["url"] = url;
    outputs[type] = entry;
    m_config["outputs"] = outputs;
    save();
}

QJsonObject ConfigManager::overlayStyle(const QString& name) const
{
    return m_config["overlay_styles"].toObject()[name].toObject();
}

void ConfigManager::setOverlayStyle(const QString& name, const QJsonObject& style)
{
    auto styles = m_config["overlay_styles"].toObject();
    styles[name] = style;
    m_config["overlay_styles"] = styles;
    save();
}

QString ConfigManager::configPath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
           + "/.prestige-ai/config.json";
}

} // namespace prestige
