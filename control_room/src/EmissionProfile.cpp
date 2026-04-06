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

    // Overlay offsets
    obj["ch_logo_off_x"]         = channelLogoOffsetX;
    obj["ch_logo_off_y"]         = channelLogoOffsetY;
    obj["ch_name_off_x"]         = channelNameOffsetX;
    obj["ch_name_off_y"]         = channelNameOffsetY;
    obj["st_off_x"]              = showTitleOffsetX;
    obj["st_off_y"]              = showTitleOffsetY;
    obj["ticker_off_y"]          = tickerOffsetY;
    obj["sub_off_x"]             = subtitleOffsetX;
    obj["sub_off_y"]             = subtitleOffsetY;
    obj["cd_off_x"]              = countdownOffsetX;
    obj["cd_off_y"]              = countdownOffsetY;
    obj["clk_off_x"]             = clockOffsetX;
    obj["clk_off_y"]             = clockOffsetY;
    obj["qr_off_x"]              = qrCodeOffsetX;
    obj["qr_off_y"]              = qrCodeOffsetY;
    obj["sb_off_x"]              = scoreboardOffsetX;
    obj["sb_off_y"]              = scoreboardOffsetY;
    obj["wth_off_x"]             = weatherOffsetX;
    obj["wth_off_y"]             = weatherOffsetY;

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

    // Design Templates
    obj["d_nameplate"]   = nameplateDesign;
    obj["d_title"]       = titleDesign;
    obj["d_channel"]     = channelDesign;
    obj["d_ticker"]      = tickerDesign;
    obj["d_scoreboard"]  = scoreboardDesign;
    obj["d_clock"]       = clockDesign;
    obj["d_weather"]     = weatherDesign;

    // Scoreboard data
    obj["p_sb_team_a"]           = scoreboardTeamA;
    obj["p_sb_team_b"]           = scoreboardTeamB;
    obj["p_sb_color_a"]          = scoreboardColorA;
    obj["p_sb_color_b"]          = scoreboardColorB;
    obj["p_sb_position"]         = scoreboardPosition;

    // Ticker config
    obj["p_ticker_bg"]           = tickerBgColor;
    obj["p_ticker_text"]         = tickerTextColor;
    obj["p_ticker_font"]         = tickerFontSize;
    obj["p_ticker_speed"]        = tickerSpeed;
    obj["p_ticker_manual"]       = tickerManualText;

    // Show title animations
    obj["st_entry_anim"]         = showTitleEntryAnim;
    obj["st_loop_anim"]          = showTitleLoopAnim;

    // Talent timing
    obj["talent_display_sec"]    = talentDisplayDurationSec;
    obj["title_reappear_sec"]    = titleReappearDelaySec;

    // VS colors
    obj["vs_primary_color"]      = vsPrimaryColor;
    obj["vs_secondary_color"]    = vsSecondaryColor;
    obj["vs_accent_color"]       = vsAccentColor;
    obj["vs_floor_color"]        = vsFloorColor;
    obj["vs_light_intensity"]    = vsLightIntensity;
    obj["vs_animations_enabled"] = vsAnimationsEnabled;
    obj["vs_custom_bg"]          = vsCustomBackground;

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

    // Overlay offsets
    p.channelLogoOffsetX  = obj["ch_logo_off_x"].toInt(0);
    p.channelLogoOffsetY  = obj["ch_logo_off_y"].toInt(0);
    p.channelNameOffsetX  = obj["ch_name_off_x"].toInt(0);
    p.channelNameOffsetY  = obj["ch_name_off_y"].toInt(0);
    p.showTitleOffsetX    = obj["st_off_x"].toInt(0);
    p.showTitleOffsetY    = obj["st_off_y"].toInt(0);
    p.tickerOffsetY       = obj["ticker_off_y"].toInt(0);
    p.subtitleOffsetX     = obj["sub_off_x"].toInt(0);
    p.subtitleOffsetY     = obj["sub_off_y"].toInt(0);
    p.countdownOffsetX    = obj["cd_off_x"].toInt(0);
    p.countdownOffsetY    = obj["cd_off_y"].toInt(0);
    p.clockOffsetX        = obj["clk_off_x"].toInt(0);
    p.clockOffsetY        = obj["clk_off_y"].toInt(0);
    p.qrCodeOffsetX       = obj["qr_off_x"].toInt(0);
    p.qrCodeOffsetY       = obj["qr_off_y"].toInt(0);
    p.scoreboardOffsetX   = obj["sb_off_x"].toInt(0);
    p.scoreboardOffsetY   = obj["sb_off_y"].toInt(0);
    p.weatherOffsetX      = obj["wth_off_x"].toInt(0);
    p.weatherOffsetY      = obj["wth_off_y"].toInt(0);

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

    // Design Templates
    p.nameplateDesign     = obj["d_nameplate"].toString("np_broadcast_news");
    p.titleDesign         = obj["d_title"].toString("tt_broadcast");
    p.channelDesign       = obj["d_channel"].toString("ch_rectangle_red");
    p.tickerDesign        = obj["d_ticker"].toString("tk_news_red");
    p.scoreboardDesign    = obj["d_scoreboard"].toString("sb_glass_dark");
    p.clockDesign         = obj["d_clock"].toString("ck_shadow_only");
    p.weatherDesign       = obj["d_weather"].toString("wt_shadow_only");

    // Scoreboard data
    p.scoreboardTeamA     = obj["p_sb_team_a"].toString("HOME");
    p.scoreboardTeamB     = obj["p_sb_team_b"].toString("AWAY");
    p.scoreboardColorA    = obj["p_sb_color_a"].toString("#CC0000");
    p.scoreboardColorB    = obj["p_sb_color_b"].toString("#0066CC");
    p.scoreboardPosition  = obj["p_sb_position"].toString("top_left");

    // Ticker config
    p.tickerBgColor       = obj["p_ticker_bg"].toString("#CC0000");
    p.tickerTextColor     = obj["p_ticker_text"].toString("#FFFFFF");
    p.tickerFontSize      = obj["p_ticker_font"].toInt(14);
    p.tickerSpeed         = obj["p_ticker_speed"].toInt(2);
    p.tickerManualText    = obj["p_ticker_manual"].toString();

    // Show title animations
    p.showTitleEntryAnim  = obj["st_entry_anim"].toString("slide_up");
    p.showTitleLoopAnim   = obj["st_loop_anim"].toString("none");

    // Talent timing
    p.talentDisplayDurationSec = obj["talent_display_sec"].toInt(8);
    p.titleReappearDelaySec    = obj["title_reappear_sec"].toInt(2);

    // VS colors
    p.vsPrimaryColor      = obj["vs_primary_color"].toString();
    p.vsSecondaryColor    = obj["vs_secondary_color"].toString();
    p.vsAccentColor       = obj["vs_accent_color"].toString();
    p.vsFloorColor        = obj["vs_floor_color"].toString();
    p.vsLightIntensity    = obj["vs_light_intensity"].toDouble(1.0);
    p.vsAnimationsEnabled = obj["vs_animations_enabled"].toBool(true);
    p.vsCustomBackground  = obj["vs_custom_bg"].toString();

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
