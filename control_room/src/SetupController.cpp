// ============================================================
// Prestige AI — Setup Controller Implementation
// ============================================================

#include "SetupController.h"
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMovie>
#include <QImage>
#include <QBuffer>

namespace prestige {

SetupController::SetupController(ProfileManager* profiles, QObject* parent)
    : QObject(parent)
    , m_profiles(profiles)
{
    // Load or create default profile
    auto all = m_profiles->loadAll();
    if (all.isEmpty()) {
        m_profile = m_profiles->createNew("Nouvelle émission");
    } else {
        m_profile = all.first();
    }
    m_profiles->loadProfile(m_profile.id);

    // Sync profile → local members on startup
    m_channelLogoPath = m_profile.channelLogoPath;
    m_channelLogoPosition = m_profile.channelLogoPosition;
    m_channelLogoSize = m_profile.channelLogoSize;
    m_showChannelNameText = m_profile.showChannelNameText;
    m_keepLogoDuringAds = m_profile.keepLogoDuringAds;
    m_channelNameShape = m_profile.channelNameShape;
    m_channelNameBgColor = m_profile.channelNameBgColor;
    m_channelNameTextColor = m_profile.channelNameTextColor;
    m_channelNameBorderColor = m_profile.channelNameBorderColor;
    m_channelNameFontSize = m_profile.channelNameFontSize;
    m_logoEntryAnim = m_profile.logoEntryAnim;
    m_nameEntryAnim = m_profile.nameEntryAnim;
    m_logoLoopAnim = m_profile.logoLoopAnim;
    m_nameLoopAnim = m_profile.nameLoopAnim;
    m_showTitle = m_profile.showTitle;
    m_showSubtitle = m_profile.showSubtitle;
    m_showTitleEnabled = m_profile.showTitleEnabled;
    m_showTitlePosition = m_profile.showTitlePosition;
    m_showTitleShape = m_profile.showTitleShape;
    m_showTitleBgColor = m_profile.showTitleBgColor;
    m_showTitleTextColor = m_profile.showTitleTextColor;
    m_showTitleBorderColor = m_profile.showTitleBorderColor;
    m_showTitleFontSize = m_profile.showTitleFontSize;
    m_scoreboardVisible = m_profile.scoreboardVisible;
    m_weatherVisible = m_profile.weatherVisible;
    m_clockVisible = m_profile.clockVisible;
    m_clockFormat = m_profile.clockFormat;
    m_tickerVisible = m_profile.tickerVisible;
    m_nameplateScale = m_profile.nameplateScale;
    m_scoreboardScale = m_profile.scoreboardScale;
    m_weatherScale = m_profile.weatherScale;
    m_clockScale = m_profile.clockScale;
    m_countdownScale = m_profile.countdownScale;
    m_qrCodeScale = m_profile.qrCodeScale;
    m_vsEnabled = m_profile.virtualStudioEnabled;
    m_vsStudioId = m_profile.virtualStudioId;
    m_vsChromaKey = m_profile.chromaKeyEnabled;
    m_vsChromaColor = m_profile.chromaKeyColor;
    m_vsChromaTol = m_profile.chromaKeyTolerance;
    m_vsChromaSmooth = m_profile.chromaKeySmooth;
    m_teamLogoA = m_profile.teamLogoA;
    m_teamLogoB = m_profile.teamLogoB;
    m_goalAnimText = m_profile.goalAnimText;
    m_goalAnimEffect = m_profile.goalAnimEffect;
    m_goalAnimDuration = m_profile.goalAnimDuration;
    m_socialOutputsJson = m_profile.socialOutputsJson;
    m_outputFps = m_profile.outputFps;
    m_outputBitrate = m_profile.outputBitrate;
    // Scoreboard data
    m_scoreboardTeamA = m_profile.scoreboardTeamA;
    m_scoreboardTeamB = m_profile.scoreboardTeamB;
    m_scoreboardColorA = m_profile.scoreboardColorA;
    m_scoreboardColorB = m_profile.scoreboardColorB;
    m_scoreboardPosition = m_profile.scoreboardPosition;
    // Ticker config
    m_tickerBgColor = m_profile.tickerBgColor;
    m_tickerTextColor = m_profile.tickerTextColor;
    m_tickerFontSize = m_profile.tickerFontSize;
    m_tickerSpeed = m_profile.tickerSpeed;
    m_tickerManualText = m_profile.tickerManualText;
    // Show title animations
    m_showTitleEntryAnim = m_profile.showTitleEntryAnim;
    m_showTitleLoopAnim = m_profile.showTitleLoopAnim;
    // Talent timing
    m_talentDisplayDurationSec = m_profile.talentDisplayDurationSec;
    m_titleReappearDelaySec = m_profile.titleReappearDelaySec;
    // VS colors
    m_vsPrimary = m_profile.vsPrimaryColor.isEmpty() ? QColor() : QColor(m_profile.vsPrimaryColor);
    m_vsSecondary = m_profile.vsSecondaryColor.isEmpty() ? QColor() : QColor(m_profile.vsSecondaryColor);
    m_vsAccent = m_profile.vsAccentColor.isEmpty() ? QColor() : QColor(m_profile.vsAccentColor);
    m_vsFloor = m_profile.vsFloorColor.isEmpty() ? QColor() : QColor(m_profile.vsFloorColor);
    m_vsLightIntensity = m_profile.vsLightIntensity;
    m_vsAnimEnabled = m_profile.vsAnimationsEnabled;
    m_vsCustomBg = m_profile.vsCustomBackground;
    m_srtUrl = m_profile.srtUrl;
    // Design Templates
    m_nameplateDesign = m_profile.nameplateDesign;
    m_titleDesign = m_profile.titleDesign;
    m_channelDesign = m_profile.channelDesign;
    m_tickerDesign = m_profile.tickerDesign;
    m_scoreboardDesign = m_profile.scoreboardDesign;
    m_clockDesign = m_profile.clockDesign;
    m_weatherDesign = m_profile.weatherDesign;

    // Overlay offsets
    m_channelLogoOffsetX = m_profile.channelLogoOffsetX;
    m_channelLogoOffsetY = m_profile.channelLogoOffsetY;
    m_channelNameOffsetX = m_profile.channelNameOffsetX;
    m_channelNameOffsetY = m_profile.channelNameOffsetY;
    m_showTitleOffsetX = m_profile.showTitleOffsetX;
    m_showTitleOffsetY = m_profile.showTitleOffsetY;
    m_tickerOffsetY = m_profile.tickerOffsetY;
    m_subtitleOffsetX = m_profile.subtitleOffsetX;
    m_subtitleOffsetY = m_profile.subtitleOffsetY;
    m_countdownOffsetX = m_profile.countdownOffsetX;
    m_countdownOffsetY = m_profile.countdownOffsetY;
    m_clockOffsetX = m_profile.clockOffsetX;
    m_clockOffsetY = m_profile.clockOffsetY;
    m_qrCodeOffsetX = m_profile.qrCodeOffsetX;
    m_qrCodeOffsetY = m_profile.qrCodeOffsetY;
    m_scoreboardOffsetX = m_profile.scoreboardOffsetX;
    m_scoreboardOffsetY = m_profile.scoreboardOffsetY;
    m_weatherOffsetX = m_profile.weatherOffsetX;
    m_weatherOffsetY = m_profile.weatherOffsetY;
}

QString SetupController::currentProfileName() const
{
    return m_profile.name;
}

QString SetupController::emissionName() const
{
    return m_profile.name;
}

void SetupController::setEmissionName(const QString& name)
{
    if (m_profile.name == name) return;
    m_profile.name = name;
    emit profileChanged();
}

void SetupController::setSelectedStyle(const QString& id)
{
    if (m_profile.overlayStyleId == id) return;
    m_profile.overlayStyleId = id;
    emit styleChanged();
}

void SetupController::setAccentColor(const QColor& c)
{
    if (m_profile.accentColor == c) return;
    m_profile.accentColor = c;
    emit styleChanged();
}

void SetupController::setBackgroundOpacity(double v)
{
    if (qFuzzyCompare(m_profile.backgroundOpacity, v)) return;
    m_profile.backgroundOpacity = v;
    emit styleChanged();
}

void SetupController::setOverlayPosition(int pos)
{
    if (m_profile.overlayPosition == pos) return;
    m_profile.overlayPosition = pos;
    emit styleChanged();
}

void SetupController::setLogoPath(const QString& path)
{
    if (m_profile.logoPath == path) return;
    m_profile.logoPath = path;
    emit styleChanged();
}

// ── Animation ───────────────────────────────────────────────

void SetupController::setAnimationType(const QString& type)
{
    if (m_profile.animationType == type) return;
    m_profile.animationType = type;
    emit animationChanged();
}

void SetupController::setAnimEnterFrames(int frames)
{
    if (m_profile.animEnterFrames == frames) return;
    m_profile.animEnterFrames = qBound(3, frames, 60);
    emit animationChanged();
}

void SetupController::setAnimExitFrames(int frames)
{
    if (m_profile.animExitFrames == frames) return;
    m_profile.animExitFrames = qBound(2, frames, 30);
    emit animationChanged();
}

// ── Multi-face overlay ──────────────────────────────────────

void SetupController::setMultiFaceName(const QString& v)
{
    if (m_profile.multiFaceName == v) return;
    m_profile.multiFaceName = v;
    emit overlayTimingChanged();
}

void SetupController::setMultiFaceRole(const QString& v)
{
    if (m_profile.multiFaceRole == v) return;
    m_profile.multiFaceRole = v;
    emit overlayTimingChanged();
}

void SetupController::setMultiFaceStyleId(const QString& v)
{
    if (m_profile.multiFaceStyleId == v) return;
    m_profile.multiFaceStyleId = v;
    emit overlayTimingChanged();
}

void SetupController::setSingleFaceDelayMs(int v)
{
    if (m_profile.singleFaceDelayMs == v) return;
    m_profile.singleFaceDelayMs = v;
    emit overlayTimingChanged();
}

void SetupController::setMultiFaceDelayMs(int v)
{
    if (m_profile.multiFaceDelayMs == v) return;
    m_profile.multiFaceDelayMs = v;
    emit overlayTimingChanged();
}

void SetupController::setOverlayHideDelayMs(int v)
{
    if (m_profile.overlayHideDelayMs == v) return;
    m_profile.overlayHideDelayMs = v;
    emit overlayTimingChanged();
}

// ── Source ──────────────────────────────────────────────────

void SetupController::setInputType(const QString& type)
{
    if (m_profile.inputType == type) return;
    m_profile.inputType = type;
    emit sourceChanged();
}

void SetupController::setInputSource(const QString& src)
{
    if (m_profile.inputSource == src) return;
    m_profile.inputSource = src;
    emit sourceChanged();
}

void SetupController::setOutputSDI(bool v)  { if (m_profile.outputSDI != v)  { m_profile.outputSDI  = v; emit outputsChanged(); } }
void SetupController::setOutputNDI(bool v)  { if (m_profile.outputNDI != v)  { m_profile.outputNDI  = v; emit outputsChanged(); } }
void SetupController::setOutputRTMP(bool v) { if (m_profile.outputRTMP != v) { m_profile.outputRTMP = v; emit outputsChanged(); } }
void SetupController::setOutputSRT(bool v)  { if (m_profile.outputSRT != v)  { m_profile.outputSRT  = v; emit outputsChanged(); } }

void SetupController::setRtmpUrl(const QString& url)
{
    if (m_profile.rtmpUrl == url) return;
    m_profile.rtmpUrl = url;
    emit outputsChanged();
}

void SetupController::setRtmpKey(const QString& key)
{
    if (m_profile.rtmpKey == key) return;
    m_profile.rtmpKey = key;
    emit outputsChanged();
}

// ── Channel branding ────────────────────────────────────────

void SetupController::setChannelLogoPath(const QString& v) { if (m_channelLogoPath == v) return; m_channelLogoPath = v; emit brandingChanged(); }
void SetupController::setChannelLogoPosition(const QString& v) { if (m_channelLogoPosition == v) return; m_channelLogoPosition = v; emit brandingChanged(); }
void SetupController::setChannelLogoSize(int v) { if (m_channelLogoSize == v) return; m_channelLogoSize = qBound(20, v, 300); emit brandingChanged(); }
void SetupController::setShowChannelNameText(bool v) { if (m_showChannelNameText == v) return; m_showChannelNameText = v; emit brandingChanged(); }
void SetupController::setKeepLogoDuringAds(bool v) { if (m_keepLogoDuringAds == v) return; m_keepLogoDuringAds = v; emit brandingChanged(); }

// ── Channel logo/name offsets ────────────────────────────────

void SetupController::setChannelLogoOffsetX(int v) { if (m_channelLogoOffsetX == v) return; m_channelLogoOffsetX = v; emit brandingChanged(); }
void SetupController::setChannelLogoOffsetY(int v) { if (m_channelLogoOffsetY == v) return; m_channelLogoOffsetY = v; emit brandingChanged(); }
void SetupController::setChannelNameOffsetX(int v) { if (m_channelNameOffsetX == v) return; m_channelNameOffsetX = v; emit brandingChanged(); }
void SetupController::setChannelNameOffsetY(int v) { if (m_channelNameOffsetY == v) return; m_channelNameOffsetY = v; emit brandingChanged(); }

// ── Channel name design ─────────────────────────────────────

void SetupController::setChannelNameShape(const QString& v) { if (m_channelNameShape == v) return; m_channelNameShape = v; emit brandingChanged(); }
void SetupController::setChannelNameBgColor(const QString& v) { if (m_channelNameBgColor == v) return; m_channelNameBgColor = v; emit brandingChanged(); }
void SetupController::setChannelNameTextColor(const QString& v) { if (m_channelNameTextColor == v) return; m_channelNameTextColor = v; emit brandingChanged(); }
void SetupController::setChannelNameBorderColor(const QString& v) { if (m_channelNameBorderColor == v) return; m_channelNameBorderColor = v; emit brandingChanged(); }
void SetupController::setChannelNameFontSize(int v) { if (m_channelNameFontSize == v) return; m_channelNameFontSize = qBound(6, v, 72); emit brandingChanged(); }

// ── Entry animations ────────────────────────────────────────

void SetupController::setLogoEntryAnim(const QString& v) { if (m_logoEntryAnim == v) return; m_logoEntryAnim = v; emit brandingChanged(); }
void SetupController::setNameEntryAnim(const QString& v) { if (m_nameEntryAnim == v) return; m_nameEntryAnim = v; emit brandingChanged(); }

// ── Loop animations ─────────────────────────────────────────

void SetupController::setLogoLoopAnim(const QString& v) { if (m_logoLoopAnim == v) return; m_logoLoopAnim = v; emit brandingChanged(); }
void SetupController::setNameLoopAnim(const QString& v) { if (m_nameLoopAnim == v) return; m_nameLoopAnim = v; emit brandingChanged(); }

// ── Show title ──────────────────────────────────────────────

void SetupController::setShowTitle(const QString& v) { if (m_showTitle == v) return; m_showTitle = v; emit showTitleChanged(); }
void SetupController::setShowSubtitle(const QString& v) { if (m_showSubtitle == v) return; m_showSubtitle = v; emit showTitleChanged(); }
void SetupController::setShowTitleEnabled(bool v) { if (m_showTitleEnabled == v) return; m_showTitleEnabled = v; emit showTitleChanged(); }
void SetupController::setShowTitlePosition(const QString& v) { if (m_showTitlePosition == v) return; m_showTitlePosition = v; emit showTitleChanged(); }

// ── Show title design & animation ───────────────────────

void SetupController::setShowTitleShape(const QString& v) { if (m_showTitleShape == v) return; m_showTitleShape = v; emit showTitleChanged(); }
void SetupController::setShowTitleBgColor(const QString& v) { if (m_showTitleBgColor == v) return; m_showTitleBgColor = v; emit showTitleChanged(); }
void SetupController::setShowTitleTextColor(const QString& v) { if (m_showTitleTextColor == v) return; m_showTitleTextColor = v; emit showTitleChanged(); }
void SetupController::setShowTitleBorderColor(const QString& v) { if (m_showTitleBorderColor == v) return; m_showTitleBorderColor = v; emit showTitleChanged(); }
void SetupController::setShowTitleFontSize(int v) { if (m_showTitleFontSize == v) return; m_showTitleFontSize = qBound(8, v, 60); emit showTitleChanged(); }
void SetupController::setShowTitleEntryAnim(const QString& v) { if (m_showTitleEntryAnim == v) return; m_showTitleEntryAnim = v; emit showTitleChanged(); }
void SetupController::setShowTitleLoopAnim(const QString& v) { if (m_showTitleLoopAnim == v) return; m_showTitleLoopAnim = v; emit showTitleChanged(); }

// ── Show title offsets ───────────────────────────────────────

void SetupController::setShowTitleOffsetX(int v) { if (m_showTitleOffsetX == v) return; m_showTitleOffsetX = v; emit showTitleChanged(); }
void SetupController::setShowTitleOffsetY(int v) { if (m_showTitleOffsetY == v) return; m_showTitleOffsetY = v; emit showTitleChanged(); }

// ── Ticker offset ───────────────────────────────────────────

void SetupController::setTickerOffsetY(int v) { if (m_tickerOffsetY == v) return; m_tickerOffsetY = v; emit brandingChanged(); }

// ── Subtitle offsets ────────────────────────────────────────

void SetupController::setSubtitleOffsetX(int v) { if (m_subtitleOffsetX == v) return; m_subtitleOffsetX = v; emit brandingChanged(); }
void SetupController::setSubtitleOffsetY(int v) { if (m_subtitleOffsetY == v) return; m_subtitleOffsetY = v; emit brandingChanged(); }

// ── Countdown offsets ───────────────────────────────────────

void SetupController::setCountdownOffsetX(int v) { if (m_countdownOffsetX == v) return; m_countdownOffsetX = v; emit brandingChanged(); }
void SetupController::setCountdownOffsetY(int v) { if (m_countdownOffsetY == v) return; m_countdownOffsetY = v; emit brandingChanged(); }

// ── Clock offsets ───────────────────────────────────────────

void SetupController::setClockOffsetX(int v) { if (m_clockOffsetX == v) return; m_clockOffsetX = v; emit brandingChanged(); }
void SetupController::setClockOffsetY(int v) { if (m_clockOffsetY == v) return; m_clockOffsetY = v; emit brandingChanged(); }

// ── QR Code offsets ─────────────────────────────────────────

void SetupController::setQrCodeOffsetX(int v) { if (m_qrCodeOffsetX == v) return; m_qrCodeOffsetX = v; emit brandingChanged(); }
void SetupController::setQrCodeOffsetY(int v) { if (m_qrCodeOffsetY == v) return; m_qrCodeOffsetY = v; emit brandingChanged(); }

// ── Scoreboard offsets ──────────────────────────────────────

void SetupController::setScoreboardOffsetX(int v) { if (m_scoreboardOffsetX == v) return; m_scoreboardOffsetX = v; emit brandingChanged(); }
void SetupController::setScoreboardOffsetY(int v) { if (m_scoreboardOffsetY == v) return; m_scoreboardOffsetY = v; emit brandingChanged(); }

// ── Weather offsets ─────────────────────────────────────────

void SetupController::setWeatherOffsetX(int v) { if (m_weatherOffsetX == v) return; m_weatherOffsetX = v; emit brandingChanged(); }
void SetupController::setWeatherOffsetY(int v) { if (m_weatherOffsetY == v) return; m_weatherOffsetY = v; emit brandingChanged(); }

// ── Overlay scale factors ───────────────────────────────────

void SetupController::setNameplateScale(double v) { v = qBound(0.3, v, 4.0); if (qFuzzyCompare(m_nameplateScale, v)) return; m_nameplateScale = v; emit brandingChanged(); }
void SetupController::setScoreboardScale(double v) { v = qBound(0.3, v, 4.0); if (qFuzzyCompare(m_scoreboardScale, v)) return; m_scoreboardScale = v; emit brandingChanged(); }
void SetupController::setWeatherScale(double v) { v = qBound(0.3, v, 4.0); if (qFuzzyCompare(m_weatherScale, v)) return; m_weatherScale = v; emit brandingChanged(); }
void SetupController::setClockScale(double v) { v = qBound(0.3, v, 4.0); if (qFuzzyCompare(m_clockScale, v)) return; m_clockScale = v; emit brandingChanged(); }
void SetupController::setCountdownScale(double v) { v = qBound(0.3, v, 4.0); if (qFuzzyCompare(m_countdownScale, v)) return; m_countdownScale = v; emit brandingChanged(); }
void SetupController::setQrCodeScale(double v) { v = qBound(0.3, v, 4.0); if (qFuzzyCompare(m_qrCodeScale, v)) return; m_qrCodeScale = v; emit brandingChanged(); }

// ── Talent display timing ───────────────────────────────────

void SetupController::setTalentDisplayDurationSec(int v) { if (m_talentDisplayDurationSec == v) return; m_talentDisplayDurationSec = qBound(3, v, 30); emit timingChanged(); }
void SetupController::setTitleReappearDelaySec(int v) { if (m_titleReappearDelaySec == v) return; m_titleReappearDelaySec = qBound(1, v, 10); emit timingChanged(); }

// ── Overlay visibility ───────────────────────────────────────

void SetupController::setScoreboardVisible(bool v) { if (m_scoreboardVisible == v) return; m_scoreboardVisible = v; emit brandingChanged(); }
void SetupController::setWeatherVisible(bool v) { if (m_weatherVisible == v) return; m_weatherVisible = v; emit brandingChanged(); }
void SetupController::setClockVisible(bool v) { if (m_clockVisible == v) return; m_clockVisible = v; emit brandingChanged(); }
void SetupController::setClockFormat(const QString& v) { if (m_clockFormat == v) return; m_clockFormat = v; emit brandingChanged(); }
void SetupController::setTickerVisible(bool v) { if (m_tickerVisible == v) return; m_tickerVisible = v; emit brandingChanged(); }

// ── Scoreboard data ─────────────────────────────────────────

void SetupController::setScoreboardTeamA(const QString& v) { if (m_scoreboardTeamA == v) return; m_scoreboardTeamA = v; emit brandingChanged(); }
void SetupController::setScoreboardTeamB(const QString& v) { if (m_scoreboardTeamB == v) return; m_scoreboardTeamB = v; emit brandingChanged(); }
void SetupController::setScoreboardScoreA(int v) { if (m_scoreboardScoreA == v) return; m_scoreboardScoreA = v; emit brandingChanged(); }
void SetupController::setScoreboardScoreB(int v) { if (m_scoreboardScoreB == v) return; m_scoreboardScoreB = v; emit brandingChanged(); }

// ── Ticker appearance ────────────────────────────────────────

void SetupController::setTickerBgColor(const QString& v) { if (m_tickerBgColor == v) return; m_tickerBgColor = v; emit brandingChanged(); }
void SetupController::setTickerTextColor(const QString& v) { if (m_tickerTextColor == v) return; m_tickerTextColor = v; emit brandingChanged(); }
void SetupController::setTickerFontSize(int v) { if (m_tickerFontSize == v) return; m_tickerFontSize = qBound(8, v, 48); emit brandingChanged(); }
void SetupController::setTickerSpeed(int v) { if (m_tickerSpeed == v) return; m_tickerSpeed = qBound(1, v, 5); emit brandingChanged(); }

// ── Scoreboard colors & position ────────────────────────────

void SetupController::setScoreboardColorA(const QString& v) { if (m_scoreboardColorA == v) return; m_scoreboardColorA = v; emit brandingChanged(); }
void SetupController::setScoreboardColorB(const QString& v) { if (m_scoreboardColorB == v) return; m_scoreboardColorB = v; emit brandingChanged(); }
void SetupController::setScoreboardPosition(const QString& v) { if (m_scoreboardPosition == v) return; m_scoreboardPosition = v; emit brandingChanged(); }

// ── Ticker manual text ───────────────────────────────────────

void SetupController::setTickerManualText(const QString& v) { if (m_tickerManualText == v) return; m_tickerManualText = v; emit brandingChanged(); }

// ── Scoreboard timer/period/cards ────────────────────────────

void SetupController::setScoreboardMatchTime(const QString& v) { if (m_scoreboardMatchTime == v) return; m_scoreboardMatchTime = v; emit brandingChanged(); }
void SetupController::setScoreboardPeriod(int v) { if (m_scoreboardPeriod == v) return; m_scoreboardPeriod = v; emit brandingChanged(); }
void SetupController::setScoreboardYellowA(int v) { if (m_scoreboardYellowA == v) return; m_scoreboardYellowA = v; emit brandingChanged(); }
void SetupController::setScoreboardYellowB(int v) { if (m_scoreboardYellowB == v) return; m_scoreboardYellowB = v; emit brandingChanged(); }
void SetupController::setScoreboardRedA(int v) { if (m_scoreboardRedA == v) return; m_scoreboardRedA = v; emit brandingChanged(); }
void SetupController::setScoreboardRedB(int v) { if (m_scoreboardRedB == v) return; m_scoreboardRedB = v; emit brandingChanged(); }

void SetupController::loadAnimatedLogo(const QString& path)
{
    QString localPath = path;
    if (localPath.startsWith("file://"))
        localPath = QUrl(localPath).toLocalFile();

    QFileInfo info(localPath);
    if (!info.exists()) {
        qWarning() << "[SetupController] Logo path not found:" << localPath;
        return;
    }

    if (info.isDir()) {
        // Load all .png files sorted by name from directory
        QDir dir(localPath);
        QStringList filters;
        filters << "*.png" << "*.PNG";
        QStringList files = dir.entryList(filters, QDir::Files, QDir::Name);
        qInfo() << "[SetupController] Loading animated logo from directory:" << localPath
                << "(" << files.size() << "frames)";
        // Store path — frames will be loaded by Vision Engine
        m_profile.logoPath = localPath;
        emit styleChanged();
    } else if (localPath.endsWith(".gif", Qt::CaseInsensitive)) {
        // GIF: extract frames using QMovie
        QMovie movie(localPath);
        if (!movie.isValid()) {
            qWarning() << "[SetupController] Invalid GIF:" << localPath;
            return;
        }
        int frameCount = movie.frameCount();
        qInfo() << "[SetupController] Loading animated GIF logo:" << localPath
                << "(" << frameCount << "frames)";
        m_profile.logoPath = localPath;
        emit styleChanged();
    } else {
        // Single image — static logo
        QImage img(localPath);
        if (img.isNull()) {
            qWarning() << "[SetupController] Cannot load logo image:" << localPath;
            return;
        }
        qInfo() << "[SetupController] Loaded static logo:" << localPath;
        m_profile.logoPath = localPath;
        emit styleChanged();
    }
}

bool SetupController::isReadyToGo() const
{
    // Must have a style selected and at least one output
    return !m_profile.overlayStyleId.isEmpty()
        && (m_profile.outputSDI || m_profile.outputNDI
            || m_profile.outputRTMP || m_profile.outputSRT
            || m_profile.inputType == "webcam"); // dev mode always OK
}

void SetupController::saveProfile()
{
    // Sync ALL local m_* members → m_profile before saving
    m_profile.channelLogoPath = m_channelLogoPath;
    m_profile.channelLogoPosition = m_channelLogoPosition;
    m_profile.channelLogoSize = m_channelLogoSize;
    m_profile.showChannelNameText = m_showChannelNameText;
    m_profile.keepLogoDuringAds = m_keepLogoDuringAds;
    m_profile.channelNameShape = m_channelNameShape;
    m_profile.channelNameBgColor = m_channelNameBgColor;
    m_profile.channelNameTextColor = m_channelNameTextColor;
    m_profile.channelNameBorderColor = m_channelNameBorderColor;
    m_profile.channelNameFontSize = m_channelNameFontSize;
    m_profile.logoEntryAnim = m_logoEntryAnim;
    m_profile.nameEntryAnim = m_nameEntryAnim;
    m_profile.logoLoopAnim = m_logoLoopAnim;
    m_profile.nameLoopAnim = m_nameLoopAnim;
    m_profile.showTitle = m_showTitle;
    m_profile.showSubtitle = m_showSubtitle;
    m_profile.showTitleEnabled = m_showTitleEnabled;
    m_profile.showTitlePosition = m_showTitlePosition;
    m_profile.showTitleShape = m_showTitleShape;
    m_profile.showTitleBgColor = m_showTitleBgColor;
    m_profile.showTitleTextColor = m_showTitleTextColor;
    m_profile.showTitleBorderColor = m_showTitleBorderColor;
    m_profile.showTitleFontSize = m_showTitleFontSize;
    m_profile.scoreboardVisible = m_scoreboardVisible;
    m_profile.weatherVisible = m_weatherVisible;
    m_profile.clockVisible = m_clockVisible;
    m_profile.clockFormat = m_clockFormat;
    m_profile.tickerVisible = m_tickerVisible;
    m_profile.nameplateScale = m_nameplateScale;
    m_profile.scoreboardScale = m_scoreboardScale;
    m_profile.weatherScale = m_weatherScale;
    m_profile.clockScale = m_clockScale;
    m_profile.countdownScale = m_countdownScale;
    m_profile.qrCodeScale = m_qrCodeScale;
    m_profile.virtualStudioEnabled = m_vsEnabled;
    m_profile.virtualStudioId = m_vsStudioId;
    m_profile.chromaKeyEnabled = m_vsChromaKey;
    m_profile.chromaKeyColor = m_vsChromaColor;
    m_profile.chromaKeyTolerance = m_vsChromaTol;
    m_profile.chromaKeySmooth = m_vsChromaSmooth;
    m_profile.teamLogoA = m_teamLogoA;
    m_profile.teamLogoB = m_teamLogoB;
    m_profile.goalAnimText = m_goalAnimText;
    m_profile.goalAnimEffect = m_goalAnimEffect;
    m_profile.goalAnimDuration = m_goalAnimDuration;
    m_profile.socialOutputsJson = m_socialOutputsJson;
    m_profile.outputFps = m_outputFps;
    m_profile.outputBitrate = m_outputBitrate;
    // Scoreboard data
    m_profile.scoreboardTeamA = m_scoreboardTeamA;
    m_profile.scoreboardTeamB = m_scoreboardTeamB;
    m_profile.scoreboardColorA = m_scoreboardColorA;
    m_profile.scoreboardColorB = m_scoreboardColorB;
    m_profile.scoreboardPosition = m_scoreboardPosition;
    // Ticker config
    m_profile.tickerBgColor = m_tickerBgColor;
    m_profile.tickerTextColor = m_tickerTextColor;
    m_profile.tickerFontSize = m_tickerFontSize;
    m_profile.tickerSpeed = m_tickerSpeed;
    m_profile.tickerManualText = m_tickerManualText;
    // Show title animations
    m_profile.showTitleEntryAnim = m_showTitleEntryAnim;
    m_profile.showTitleLoopAnim = m_showTitleLoopAnim;
    // Talent timing
    m_profile.talentDisplayDurationSec = m_talentDisplayDurationSec;
    m_profile.titleReappearDelaySec = m_titleReappearDelaySec;
    // VS colors
    m_profile.vsPrimaryColor = m_vsPrimary.isValid() ? m_vsPrimary.name() : "";
    m_profile.vsSecondaryColor = m_vsSecondary.isValid() ? m_vsSecondary.name() : "";
    m_profile.vsAccentColor = m_vsAccent.isValid() ? m_vsAccent.name() : "";
    m_profile.vsFloorColor = m_vsFloor.isValid() ? m_vsFloor.name() : "";
    m_profile.vsLightIntensity = m_vsLightIntensity;
    m_profile.vsAnimationsEnabled = m_vsAnimEnabled;
    m_profile.vsCustomBackground = m_vsCustomBg;
    m_profile.srtUrl = m_srtUrl;
    // Design Templates
    m_profile.nameplateDesign = m_nameplateDesign;
    m_profile.titleDesign = m_titleDesign;
    m_profile.channelDesign = m_channelDesign;
    m_profile.tickerDesign = m_tickerDesign;
    m_profile.scoreboardDesign = m_scoreboardDesign;
    m_profile.clockDesign = m_clockDesign;
    m_profile.weatherDesign = m_weatherDesign;

    // Overlay offsets
    m_profile.channelLogoOffsetX = m_channelLogoOffsetX;
    m_profile.channelLogoOffsetY = m_channelLogoOffsetY;
    m_profile.channelNameOffsetX = m_channelNameOffsetX;
    m_profile.channelNameOffsetY = m_channelNameOffsetY;
    m_profile.showTitleOffsetX = m_showTitleOffsetX;
    m_profile.showTitleOffsetY = m_showTitleOffsetY;
    m_profile.tickerOffsetY = m_tickerOffsetY;
    m_profile.subtitleOffsetX = m_subtitleOffsetX;
    m_profile.subtitleOffsetY = m_subtitleOffsetY;
    m_profile.countdownOffsetX = m_countdownOffsetX;
    m_profile.countdownOffsetY = m_countdownOffsetY;
    m_profile.clockOffsetX = m_clockOffsetX;
    m_profile.clockOffsetY = m_clockOffsetY;
    m_profile.qrCodeOffsetX = m_qrCodeOffsetX;
    m_profile.qrCodeOffsetY = m_qrCodeOffsetY;
    m_profile.scoreboardOffsetX = m_scoreboardOffsetX;
    m_profile.scoreboardOffsetY = m_scoreboardOffsetY;
    m_profile.weatherOffsetX = m_weatherOffsetX;
    m_profile.weatherOffsetY = m_weatherOffsetY;

    m_profiles->save(m_profile);
    qInfo() << "[SetupController] Profile saved:" << m_profile.name;
}

void SetupController::newProfile(const QString& name)
{
    m_profile = m_profiles->createNew(name);
    m_profiles->loadProfile(m_profile.id);
    emit profileChanged();
    emit styleChanged();
    emit sourceChanged();
    emit outputsChanged();
}

void SetupController::loadProfile(const QString& id)
{
    m_profiles->loadProfile(id);
    m_profile = m_profiles->current();

    // Sync ALL m_profile fields → local m_* members
    m_channelLogoPath = m_profile.channelLogoPath;
    m_channelLogoPosition = m_profile.channelLogoPosition;
    m_channelLogoSize = m_profile.channelLogoSize;
    m_showChannelNameText = m_profile.showChannelNameText;
    m_keepLogoDuringAds = m_profile.keepLogoDuringAds;
    m_channelNameShape = m_profile.channelNameShape;
    m_channelNameBgColor = m_profile.channelNameBgColor;
    m_channelNameTextColor = m_profile.channelNameTextColor;
    m_channelNameBorderColor = m_profile.channelNameBorderColor;
    m_channelNameFontSize = m_profile.channelNameFontSize;
    m_logoEntryAnim = m_profile.logoEntryAnim;
    m_nameEntryAnim = m_profile.nameEntryAnim;
    m_logoLoopAnim = m_profile.logoLoopAnim;
    m_nameLoopAnim = m_profile.nameLoopAnim;
    m_showTitle = m_profile.showTitle;
    m_showSubtitle = m_profile.showSubtitle;
    m_showTitleEnabled = m_profile.showTitleEnabled;
    m_showTitlePosition = m_profile.showTitlePosition;
    m_showTitleShape = m_profile.showTitleShape;
    m_showTitleBgColor = m_profile.showTitleBgColor;
    m_showTitleTextColor = m_profile.showTitleTextColor;
    m_showTitleBorderColor = m_profile.showTitleBorderColor;
    m_showTitleFontSize = m_profile.showTitleFontSize;
    m_scoreboardVisible = m_profile.scoreboardVisible;
    m_weatherVisible = m_profile.weatherVisible;
    m_clockVisible = m_profile.clockVisible;
    m_clockFormat = m_profile.clockFormat;
    m_tickerVisible = m_profile.tickerVisible;
    m_nameplateScale = m_profile.nameplateScale;
    m_scoreboardScale = m_profile.scoreboardScale;
    m_weatherScale = m_profile.weatherScale;
    m_clockScale = m_profile.clockScale;
    m_countdownScale = m_profile.countdownScale;
    m_qrCodeScale = m_profile.qrCodeScale;
    m_vsEnabled = m_profile.virtualStudioEnabled;
    m_vsStudioId = m_profile.virtualStudioId;
    m_vsChromaKey = m_profile.chromaKeyEnabled;
    m_vsChromaColor = m_profile.chromaKeyColor;
    m_vsChromaTol = m_profile.chromaKeyTolerance;
    m_vsChromaSmooth = m_profile.chromaKeySmooth;
    m_teamLogoA = m_profile.teamLogoA;
    m_teamLogoB = m_profile.teamLogoB;
    m_goalAnimText = m_profile.goalAnimText;
    m_goalAnimEffect = m_profile.goalAnimEffect;
    m_goalAnimDuration = m_profile.goalAnimDuration;
    m_socialOutputsJson = m_profile.socialOutputsJson;
    m_outputFps = m_profile.outputFps;
    m_outputBitrate = m_profile.outputBitrate;

    // Overlay offsets
    m_channelLogoOffsetX = m_profile.channelLogoOffsetX;
    m_channelLogoOffsetY = m_profile.channelLogoOffsetY;
    m_channelNameOffsetX = m_profile.channelNameOffsetX;
    m_channelNameOffsetY = m_profile.channelNameOffsetY;
    m_showTitleOffsetX = m_profile.showTitleOffsetX;
    m_showTitleOffsetY = m_profile.showTitleOffsetY;
    m_tickerOffsetY = m_profile.tickerOffsetY;
    m_subtitleOffsetX = m_profile.subtitleOffsetX;
    m_subtitleOffsetY = m_profile.subtitleOffsetY;
    m_countdownOffsetX = m_profile.countdownOffsetX;
    m_countdownOffsetY = m_profile.countdownOffsetY;
    m_clockOffsetX = m_profile.clockOffsetX;
    m_clockOffsetY = m_profile.clockOffsetY;
    m_qrCodeOffsetX = m_profile.qrCodeOffsetX;
    m_qrCodeOffsetY = m_profile.qrCodeOffsetY;
    m_scoreboardOffsetX = m_profile.scoreboardOffsetX;
    m_scoreboardOffsetY = m_profile.scoreboardOffsetY;
    m_weatherOffsetX = m_profile.weatherOffsetX;
    m_weatherOffsetY = m_profile.weatherOffsetY;

    emit profileChanged();
    emit styleChanged();
    emit sourceChanged();
    emit outputsChanged();
    emit brandingChanged();
    emit showTitleChanged();
    emit virtualStudioChanged();
}

QVariantList SetupController::availableProfiles()
{
    QVariantList result;
    for (const auto& p : m_profiles->loadAll()) {
        QVariantMap map;
        map["id"] = p.id;
        map["name"] = p.name;
        map["lastUsed"] = p.lastUsed.toString("dd/MM/yyyy HH:mm");
        map["style"] = p.overlayStyleId;
        result.append(map);
    }
    return result;
}

void SetupController::exportProfile(const QString& filePath)
{
    QString path = filePath;
    if (path.startsWith("file://"))
        path = QUrl(path).toLocalFile();

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "[SetupController] Cannot write to:" << path;
        return;
    }
    QJsonDocument doc(m_profile.toJson());
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    qInfo() << "[SetupController] Profile exported to:" << path;
}

void SetupController::importProfile(const QString& filePath)
{
    QString path = filePath;
    if (path.startsWith("file://"))
        path = QUrl(path).toLocalFile();

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "[SetupController] Cannot read from:" << path;
        return;
    }
    QByteArray data = file.readAll();
    file.close();

    auto doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        qWarning() << "[SetupController] Invalid profile JSON:" << path;
        return;
    }

    EmissionProfile imported = EmissionProfile::fromJson(doc.object());
    // Create as new profile (new ID + name with "Importé" suffix)
    m_profile = m_profiles->createNew(imported.name + " (import\u00E9)");
    // Copy all settings from imported profile
    m_profile.overlayStyleId = imported.overlayStyleId;
    m_profile.animationType = imported.animationType;
    m_profile.animEnterFrames = imported.animEnterFrames;
    m_profile.animExitFrames = imported.animExitFrames;
    m_profile.accentColor = imported.accentColor;
    m_profile.backgroundOpacity = imported.backgroundOpacity;
    m_profile.overlayPosition = imported.overlayPosition;
    m_profile.logoPath = imported.logoPath;
    m_profile.inputType = imported.inputType;
    m_profile.inputSource = imported.inputSource;
    m_profile.outputSDI = imported.outputSDI;
    m_profile.outputNDI = imported.outputNDI;
    m_profile.outputRTMP = imported.outputRTMP;
    m_profile.outputSRT = imported.outputSRT;
    m_profile.rtmpUrl = imported.rtmpUrl;
    m_profile.rtmpKey = imported.rtmpKey;
    m_profile.multiFaceName = imported.multiFaceName;
    m_profile.multiFaceRole = imported.multiFaceRole;
    m_profile.multiFaceStyleId = imported.multiFaceStyleId;
    m_profile.singleFaceDelayMs = imported.singleFaceDelayMs;
    m_profile.multiFaceDelayMs = imported.multiFaceDelayMs;
    m_profile.overlayHideDelayMs = imported.overlayHideDelayMs;

    m_profiles->save(m_profile);
    m_profiles->loadProfile(m_profile.id);

    emit profileChanged();
    emit styleChanged();
    emit sourceChanged();
    emit outputsChanged();
    emit animationChanged();
    emit overlayTimingChanged();

    qInfo() << "[SetupController] Profile imported from:" << path << "as" << m_profile.name;
}

} // namespace prestige
