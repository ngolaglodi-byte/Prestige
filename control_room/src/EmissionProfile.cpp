// ============================================================
// Prestige AI — Emission Profile Implementation
// ============================================================

#include "EmissionProfile.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QStandardPaths>
#include <QUuid>
#include <QDebug>

namespace prestige {

// ── EmissionProfile serialization ──────────────────────────

QJsonObject EmissionProfile::toJson() const
{
    QJsonObject obj;
    obj["id"]                = id;
    obj["name"]              = name;
    obj["overlay_style_id"]  = overlayStyleId;
    obj["animation_type"]    = animationType;
    obj["anim_enter_frames"] = animEnterFrames;
    obj["anim_exit_frames"]  = animExitFrames;
    obj["accent_color"]      = accentColor.name();
    obj["background_opacity"]= backgroundOpacity;
    obj["overlay_position"]  = overlayPosition;
    obj["logo_path"]         = logoPath;
    obj["input_type"]        = inputType;
    obj["input_source"]      = inputSource;
    obj["output_sdi"]        = outputSDI;
    obj["output_ndi"]        = outputNDI;
    obj["output_rtmp"]       = outputRTMP;
    obj["output_srt"]        = outputSRT;
    obj["rtmp_url"]          = rtmpUrl;
    obj["rtmp_key"]          = rtmpKey;
    obj["srt_url"]           = srtUrl;
    obj["talent_ids"]          = QJsonArray::fromStringList(talentIds);
    obj["social_media"]        = socialMediaConfigs;
    obj["multi_face_name"]     = multiFaceName;
    obj["multi_face_role"]     = multiFaceRole;
    obj["multi_face_style_id"] = multiFaceStyleId;
    obj["single_face_delay_ms"]  = singleFaceDelayMs;
    obj["multi_face_delay_ms"]   = multiFaceDelayMs;
    obj["overlay_hide_delay_ms"] = overlayHideDelayMs;
    obj["created_at"]          = createdAt.toString(Qt::ISODate);
    obj["last_used"]           = lastUsed.toString(Qt::ISODate);
    return obj;
}

EmissionProfile EmissionProfile::fromJson(const QJsonObject& obj)
{
    EmissionProfile p;
    p.id                = obj["id"].toString();
    p.name              = obj["name"].toString();
    p.overlayStyleId    = obj["overlay_style_id"].toString();
    p.animationType     = obj["animation_type"].toString("slide_left");
    p.animEnterFrames   = obj["anim_enter_frames"].toInt(15);
    p.animExitFrames    = obj["anim_exit_frames"].toInt(8);
    p.accentColor       = QColor(obj["accent_color"].toString("#E30613"));
    p.backgroundOpacity = obj["background_opacity"].toDouble(0.8);
    p.overlayPosition   = obj["overlay_position"].toInt(0);
    p.logoPath          = obj["logo_path"].toString();
    p.inputType         = obj["input_type"].toString("webcam");
    p.inputSource       = obj["input_source"].toString();
    p.outputSDI         = obj["output_sdi"].toBool(false);
    p.outputNDI         = obj["output_ndi"].toBool(false);
    p.outputRTMP        = obj["output_rtmp"].toBool(false);
    p.outputSRT         = obj["output_srt"].toBool(false);
    p.rtmpUrl           = obj["rtmp_url"].toString();
    p.rtmpKey           = obj["rtmp_key"].toString();
    p.srtUrl            = obj["srt_url"].toString();
    p.socialMediaConfigs  = obj["social_media"].toObject();
    p.multiFaceName       = obj["multi_face_name"].toString();
    p.multiFaceRole       = obj["multi_face_role"].toString();
    p.multiFaceStyleId    = obj["multi_face_style_id"].toString();
    p.singleFaceDelayMs   = obj["single_face_delay_ms"].toInt(1000);
    p.multiFaceDelayMs    = obj["multi_face_delay_ms"].toInt(500);
    p.overlayHideDelayMs  = obj["overlay_hide_delay_ms"].toInt(2000);
    p.createdAt           = QDateTime::fromString(obj["created_at"].toString(), Qt::ISODate);
    p.lastUsed            = QDateTime::fromString(obj["last_used"].toString(), Qt::ISODate);

    for (const auto& v : obj["talent_ids"].toArray())
        p.talentIds.append(v.toString());

    return p;
}

// ── ProfileManager ─────────────────────────────────────────

ProfileManager::ProfileManager(QObject* parent)
    : QObject(parent)
{
    QDir().mkpath(profilesDir());
}

QString ProfileManager::profilesDir() const
{
    return QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
           + "/.prestige-ai/profiles";
}

QList<EmissionProfile> ProfileManager::loadAll()
{
    m_cache.clear();
    QDir dir(profilesDir());

    for (const auto& entry : dir.entryInfoList({"*.json"}, QDir::Files)) {
        QFile file(entry.absoluteFilePath());
        if (!file.open(QIODevice::ReadOnly))
            continue;

        auto doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isObject()) {
            m_cache.append(EmissionProfile::fromJson(doc.object()));
        }
    }

    qInfo() << "[ProfileManager] Loaded" << m_cache.size() << "profiles";
    return m_cache;
}

EmissionProfile ProfileManager::loadById(const QString& id)
{
    if (m_cache.isEmpty())
        loadAll();

    for (const auto& p : m_cache) {
        if (p.id == id)
            return p;
    }

    qWarning() << "[ProfileManager] Profile not found:" << id;
    return {};
}

void ProfileManager::save(const EmissionProfile& profile)
{
    QString path = profilesDir() + "/" + profile.id + ".json";
    QFile file(path);

    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "[ProfileManager] Cannot save profile:" << path;
        return;
    }

    file.write(QJsonDocument(profile.toJson()).toJson(QJsonDocument::Indented));
    qInfo() << "[ProfileManager] Saved profile:" << profile.name;

    // Update cache
    for (int i = 0; i < m_cache.size(); ++i) {
        if (m_cache[i].id == profile.id) {
            m_cache[i] = profile;
            emit profilesChanged();
            return;
        }
    }
    m_cache.append(profile);
    emit profilesChanged();
}

void ProfileManager::remove(const QString& id)
{
    QString path = profilesDir() + "/" + id + ".json";
    QFile::remove(path);

    m_cache.erase(
        std::remove_if(m_cache.begin(), m_cache.end(),
            [&id](const EmissionProfile& p) { return p.id == id; }),
        m_cache.end());

    emit profilesChanged();
}

EmissionProfile ProfileManager::createNew(const QString& name)
{
    EmissionProfile p;
    p.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    p.name = name;
    p.overlayStyleId = "bfm";
    p.createdAt = QDateTime::currentDateTime();
    p.lastUsed = p.createdAt;

    save(p);
    return p;
}

bool ProfileManager::loadProfile(const QString& id)
{
    m_current = loadById(id);
    if (m_current.id.isEmpty())
        return false;

    m_currentId = id;
    m_current.lastUsed = QDateTime::currentDateTime();
    save(m_current);

    emit currentProfileChanged();
    emit profileLoaded(m_current.name);
    qInfo() << "[ProfileManager] Active profile:" << m_current.name;
    return true;
}

QStringList ProfileManager::profileNames() const
{
    QStringList names;
    for (const auto& p : m_cache)
        names.append(p.name);
    return names;
}

} // namespace prestige
