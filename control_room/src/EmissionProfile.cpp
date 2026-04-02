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
    obj["social_outputs_json"]   = socialOutputsJson;
    obj["output_fps"]            = outputFps;
    obj["output_bitrate"]        = outputBitrate;

    // Channel branding
    obj["channel_logo_path"]     = channelLogoPath;
    obj["channel_logo_position"] = channelLogoPosition;
    obj["channel_logo_size"]     = channelLogoSize;
    obj["show_channel_name"]     = showChannelNameText;
    obj["keep_logo_ads"]         = keepLogoDuringAds;
    obj["name_shape"]            = channelNameShape;
    obj["name_bg_color"]         = channelNameBgColor;
    obj["name_text_color"]       = channelNameTextColor;
    obj["name_border_color"]     = channelNameBorderColor;
    obj["name_font_size"]        = channelNameFontSize;
    obj["logo_entry_anim"]       = logoEntryAnim;
    obj["name_entry_anim"]       = nameEntryAnim;
    obj["logo_loop_anim"]        = logoLoopAnim;
    obj["name_loop_anim"]        = nameLoopAnim;

    // Show title
    obj["p_show_title"]          = showTitle;
    obj["p_show_subtitle"]       = showSubtitle;
    obj["show_title_enabled"]    = showTitleEnabled;
    obj["show_title_position"]   = showTitlePosition;
    obj["show_title_shape"]      = showTitleShape;
    obj["show_title_bg_color"]   = showTitleBgColor;
    obj["show_title_text_color"] = showTitleTextColor;
    obj["show_title_border_color"]= showTitleBorderColor;
    obj["show_title_font_size"]  = showTitleFontSize;

    // Visibility + scales
    obj["scoreboard_visible"]    = scoreboardVisible;
    obj["weather_visible"]       = weatherVisible;
    obj["clock_visible"]         = clockVisible;
    obj["clock_format"]          = clockFormat;
    obj["ticker_visible"]        = tickerVisible;
    obj["nameplate_scale"]       = nameplateScale;
    obj["scoreboard_scale"]      = scoreboardScale;
    obj["weather_scale"]         = weatherScale;
    obj["clock_scale"]           = clockScale;
    obj["countdown_scale"]       = countdownScale;
    obj["qr_code_scale"]         = qrCodeScale;

    // Virtual Studio
    obj["vs_enabled"]            = virtualStudioEnabled;
    obj["vs_studio_id"]          = virtualStudioId;
    obj["chroma_key_enabled"]    = chromaKeyEnabled;
    obj["chroma_key_color"]      = chromaKeyColor;
    obj["chroma_key_tolerance"]  = chromaKeyTolerance;
    obj["chroma_key_smooth"]     = chromaKeySmooth;

    // Sport
    obj["team_logo_a"]           = teamLogoA;
    obj["team_logo_b"]           = teamLogoB;
    obj["goal_anim_text"]        = goalAnimText;
    obj["goal_anim_effect"]      = goalAnimEffect;
    obj["goal_anim_duration"]    = goalAnimDuration;

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
    p.socialOutputsJson   = obj["social_outputs_json"].toString();
    p.outputFps           = obj["output_fps"].toInt(25);
    p.outputBitrate       = obj["output_bitrate"].toInt(8);

    // Channel branding
    p.channelLogoPath     = obj["channel_logo_path"].toString();
    p.channelLogoPosition = obj["channel_logo_position"].toString("top_right");
    p.channelLogoSize     = obj["channel_logo_size"].toInt(60);
    p.showChannelNameText = obj["show_channel_name"].toBool(true);
    p.keepLogoDuringAds   = obj["keep_logo_ads"].toBool(true);
    p.channelNameShape    = obj["name_shape"].toString("rectangle");
    p.channelNameBgColor  = obj["name_bg_color"].toString("#CC0000");
    p.channelNameTextColor= obj["name_text_color"].toString("#FFFFFF");
    p.channelNameBorderColor = obj["name_border_color"].toString("#FFFFFF");
    p.channelNameFontSize = obj["name_font_size"].toInt(14);
    p.logoEntryAnim       = obj["logo_entry_anim"].toString("fade");
    p.nameEntryAnim       = obj["name_entry_anim"].toString("slide_left");
    p.logoLoopAnim        = obj["logo_loop_anim"].toString("pulse");
    p.nameLoopAnim        = obj["name_loop_anim"].toString("none");

    // Show title
    p.showTitle           = obj["p_show_title"].toString();
    p.showSubtitle        = obj["p_show_subtitle"].toString();
    p.showTitleEnabled    = obj["show_title_enabled"].toBool(true);
    p.showTitlePosition   = obj["show_title_position"].toString("bottom_left");
    p.showTitleShape      = obj["show_title_shape"].toString("rectangle");
    p.showTitleBgColor    = obj["show_title_bg_color"].toString("#1A1A2E");
    p.showTitleTextColor  = obj["show_title_text_color"].toString("#FFFFFF");
    p.showTitleBorderColor= obj["show_title_border_color"].toString("#5B4FDB");
    p.showTitleFontSize   = obj["show_title_font_size"].toInt(16);

    // Visibility + scales
    p.scoreboardVisible   = obj["scoreboard_visible"].toBool(false);
    p.weatherVisible      = obj["weather_visible"].toBool(false);
    p.clockVisible        = obj["clock_visible"].toBool(false);
    p.clockFormat         = obj["clock_format"].toString("HH:mm:ss");
    p.tickerVisible       = obj["ticker_visible"].toBool(false);
    p.nameplateScale      = obj["nameplate_scale"].toDouble(1.0);
    p.scoreboardScale     = obj["scoreboard_scale"].toDouble(1.0);
    p.weatherScale        = obj["weather_scale"].toDouble(1.0);
    p.clockScale          = obj["clock_scale"].toDouble(1.0);
    p.countdownScale      = obj["countdown_scale"].toDouble(1.0);
    p.qrCodeScale         = obj["qr_code_scale"].toDouble(1.0);

    // Virtual Studio
    p.virtualStudioEnabled = obj["vs_enabled"].toBool(false);
    p.virtualStudioId     = obj["vs_studio_id"].toInt(0);
    p.chromaKeyEnabled    = obj["chroma_key_enabled"].toBool(false);
    p.chromaKeyColor      = obj["chroma_key_color"].toString("green");
    p.chromaKeyTolerance  = obj["chroma_key_tolerance"].toDouble(0.35);
    p.chromaKeySmooth     = obj["chroma_key_smooth"].toDouble(0.05);

    // Sport
    p.teamLogoA           = obj["team_logo_a"].toString();
    p.teamLogoB           = obj["team_logo_b"].toString();
    p.goalAnimText        = obj["goal_anim_text"].toString("GOAL!");
    p.goalAnimEffect      = obj["goal_anim_effect"].toString("kinetic_pop");
    p.goalAnimDuration    = obj["goal_anim_duration"].toInt(5);

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
