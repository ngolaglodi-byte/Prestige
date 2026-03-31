#pragma once

// ============================================================
// Prestige AI — Setup Controller
// Manages the pre-production wizard state for QML
// ============================================================

#include <QObject>
#include <QString>
#include <QColor>
#include <QVariantList>

#include "EmissionProfile.h"

namespace prestige {

class SetupController : public QObject {
    Q_OBJECT

    // Current profile
    Q_PROPERTY(QString currentProfileName READ currentProfileName NOTIFY profileChanged)
    Q_PROPERTY(QString emissionName READ emissionName WRITE setEmissionName NOTIFY profileChanged)

    // Style
    Q_PROPERTY(QString selectedStyle READ selectedStyle WRITE setSelectedStyle NOTIFY styleChanged)
    Q_PROPERTY(QColor  accentColor READ accentColor WRITE setAccentColor NOTIFY styleChanged)
    Q_PROPERTY(double  backgroundOpacity READ backgroundOpacity WRITE setBackgroundOpacity NOTIFY styleChanged)
    Q_PROPERTY(int     overlayPosition READ overlayPosition WRITE setOverlayPosition NOTIFY styleChanged)
    Q_PROPERTY(QString logoPath READ logoPath WRITE setLogoPath NOTIFY styleChanged)

    // Animation (independent from style)
    Q_PROPERTY(QString animationType READ animationType WRITE setAnimationType NOTIFY animationChanged)
    Q_PROPERTY(int animEnterFrames READ animEnterFrames WRITE setAnimEnterFrames NOTIFY animationChanged)
    Q_PROPERTY(int animExitFrames READ animExitFrames WRITE setAnimExitFrames NOTIFY animationChanged)

    // Source
    Q_PROPERTY(QString inputType READ inputType WRITE setInputType NOTIFY sourceChanged)
    Q_PROPERTY(QString inputSource READ inputSource WRITE setInputSource NOTIFY sourceChanged)

    // Multi-face overlay
    Q_PROPERTY(QString multiFaceName READ multiFaceName WRITE setMultiFaceName NOTIFY overlayTimingChanged)
    Q_PROPERTY(QString multiFaceRole READ multiFaceRole WRITE setMultiFaceRole NOTIFY overlayTimingChanged)
    Q_PROPERTY(QString multiFaceStyleId READ multiFaceStyleId WRITE setMultiFaceStyleId NOTIFY overlayTimingChanged)

    // Overlay timing
    Q_PROPERTY(int singleFaceDelayMs READ singleFaceDelayMs WRITE setSingleFaceDelayMs NOTIFY overlayTimingChanged)
    Q_PROPERTY(int multiFaceDelayMs READ multiFaceDelayMs WRITE setMultiFaceDelayMs NOTIFY overlayTimingChanged)
    Q_PROPERTY(int overlayHideDelayMs READ overlayHideDelayMs WRITE setOverlayHideDelayMs NOTIFY overlayTimingChanged)

    // Outputs
    Q_PROPERTY(bool outputSDI READ outputSDI WRITE setOutputSDI NOTIFY outputsChanged)
    Q_PROPERTY(bool outputNDI READ outputNDI WRITE setOutputNDI NOTIFY outputsChanged)
    Q_PROPERTY(bool outputRTMP READ outputRTMP WRITE setOutputRTMP NOTIFY outputsChanged)
    Q_PROPERTY(bool outputSRT READ outputSRT WRITE setOutputSRT NOTIFY outputsChanged)
    Q_PROPERTY(QString rtmpUrl READ rtmpUrl WRITE setRtmpUrl NOTIFY outputsChanged)
    Q_PROPERTY(QString rtmpKey READ rtmpKey WRITE setRtmpKey NOTIFY outputsChanged)

    // Channel branding
    Q_PROPERTY(QString channelLogoPath READ channelLogoPath WRITE setChannelLogoPath NOTIFY brandingChanged)
    Q_PROPERTY(QString channelLogoPosition READ channelLogoPosition WRITE setChannelLogoPosition NOTIFY brandingChanged)
    Q_PROPERTY(int channelLogoSize READ channelLogoSize WRITE setChannelLogoSize NOTIFY brandingChanged)
    Q_PROPERTY(bool showChannelNameText READ showChannelNameText WRITE setShowChannelNameText NOTIFY brandingChanged)
    Q_PROPERTY(bool keepLogoDuringAds READ keepLogoDuringAds WRITE setKeepLogoDuringAds NOTIFY brandingChanged)

    // Channel logo/name offsets
    Q_PROPERTY(int channelLogoOffsetX READ channelLogoOffsetX WRITE setChannelLogoOffsetX NOTIFY brandingChanged)
    Q_PROPERTY(int channelLogoOffsetY READ channelLogoOffsetY WRITE setChannelLogoOffsetY NOTIFY brandingChanged)
    Q_PROPERTY(int channelNameOffsetX READ channelNameOffsetX WRITE setChannelNameOffsetX NOTIFY brandingChanged)
    Q_PROPERTY(int channelNameOffsetY READ channelNameOffsetY WRITE setChannelNameOffsetY NOTIFY brandingChanged)

    // Channel name design
    Q_PROPERTY(QString channelNameShape READ channelNameShape WRITE setChannelNameShape NOTIFY brandingChanged)
    Q_PROPERTY(QString channelNameBgColor READ channelNameBgColor WRITE setChannelNameBgColor NOTIFY brandingChanged)
    Q_PROPERTY(QString channelNameTextColor READ channelNameTextColor WRITE setChannelNameTextColor NOTIFY brandingChanged)
    Q_PROPERTY(QString channelNameBorderColor READ channelNameBorderColor WRITE setChannelNameBorderColor NOTIFY brandingChanged)
    Q_PROPERTY(int channelNameFontSize READ channelNameFontSize WRITE setChannelNameFontSize NOTIFY brandingChanged)

    // Entry animations (play once at emission start)
    Q_PROPERTY(QString logoEntryAnim READ logoEntryAnim WRITE setLogoEntryAnim NOTIFY brandingChanged)
    Q_PROPERTY(QString nameEntryAnim READ nameEntryAnim WRITE setNameEntryAnim NOTIFY brandingChanged)

    // Permanent loop animations (continuous subtle effect)
    Q_PROPERTY(QString logoLoopAnim READ logoLoopAnim WRITE setLogoLoopAnim NOTIFY brandingChanged)
    Q_PROPERTY(QString nameLoopAnim READ nameLoopAnim WRITE setNameLoopAnim NOTIFY brandingChanged)

    // Show title (static banner)
    Q_PROPERTY(QString showTitle READ showTitle WRITE setShowTitle NOTIFY showTitleChanged)
    Q_PROPERTY(QString showSubtitle READ showSubtitle WRITE setShowSubtitle NOTIFY showTitleChanged)
    Q_PROPERTY(bool showTitleEnabled READ showTitleEnabled WRITE setShowTitleEnabled NOTIFY showTitleChanged)
    Q_PROPERTY(QString showTitlePosition READ showTitlePosition WRITE setShowTitlePosition NOTIFY showTitleChanged)

    // Show title offsets
    Q_PROPERTY(int showTitleOffsetX READ showTitleOffsetX WRITE setShowTitleOffsetX NOTIFY showTitleChanged)
    Q_PROPERTY(int showTitleOffsetY READ showTitleOffsetY WRITE setShowTitleOffsetY NOTIFY showTitleChanged)

    // Ticker offset (Y only — horizontal position is full width)
    Q_PROPERTY(int tickerOffsetY READ tickerOffsetY WRITE setTickerOffsetY NOTIFY brandingChanged)

    // Subtitle offsets
    Q_PROPERTY(int subtitleOffsetX READ subtitleOffsetX WRITE setSubtitleOffsetX NOTIFY brandingChanged)
    Q_PROPERTY(int subtitleOffsetY READ subtitleOffsetY WRITE setSubtitleOffsetY NOTIFY brandingChanged)

    // Countdown offsets
    Q_PROPERTY(int countdownOffsetX READ countdownOffsetX WRITE setCountdownOffsetX NOTIFY brandingChanged)
    Q_PROPERTY(int countdownOffsetY READ countdownOffsetY WRITE setCountdownOffsetY NOTIFY brandingChanged)

    // Clock offsets
    Q_PROPERTY(int clockOffsetX READ clockOffsetX WRITE setClockOffsetX NOTIFY brandingChanged)
    Q_PROPERTY(int clockOffsetY READ clockOffsetY WRITE setClockOffsetY NOTIFY brandingChanged)

    // QR Code offsets
    Q_PROPERTY(int qrCodeOffsetX READ qrCodeOffsetX WRITE setQrCodeOffsetX NOTIFY brandingChanged)
    Q_PROPERTY(int qrCodeOffsetY READ qrCodeOffsetY WRITE setQrCodeOffsetY NOTIFY brandingChanged)

    // Scoreboard offsets
    Q_PROPERTY(int scoreboardOffsetX READ scoreboardOffsetX WRITE setScoreboardOffsetX NOTIFY brandingChanged)
    Q_PROPERTY(int scoreboardOffsetY READ scoreboardOffsetY WRITE setScoreboardOffsetY NOTIFY brandingChanged)

    // Weather offsets
    Q_PROPERTY(int weatherOffsetX READ weatherOffsetX WRITE setWeatherOffsetX NOTIFY brandingChanged)
    Q_PROPERTY(int weatherOffsetY READ weatherOffsetY WRITE setWeatherOffsetY NOTIFY brandingChanged)

    // Overlay scale factors (0.5 – 2.0, default 1.0 = 100%)
    Q_PROPERTY(double nameplateScale READ nameplateScale WRITE setNameplateScale NOTIFY brandingChanged)
    Q_PROPERTY(double scoreboardScale READ scoreboardScale WRITE setScoreboardScale NOTIFY brandingChanged)
    Q_PROPERTY(double weatherScale READ weatherScale WRITE setWeatherScale NOTIFY brandingChanged)
    Q_PROPERTY(double clockScale READ clockScale WRITE setClockScale NOTIFY brandingChanged)
    Q_PROPERTY(double countdownScale READ countdownScale WRITE setCountdownScale NOTIFY brandingChanged)
    Q_PROPERTY(double qrCodeScale READ qrCodeScale WRITE setQrCodeScale NOTIFY brandingChanged)

    // Show title design & animation
    Q_PROPERTY(QString showTitleShape READ showTitleShape WRITE setShowTitleShape NOTIFY showTitleChanged)
    Q_PROPERTY(QString showTitleBgColor READ showTitleBgColor WRITE setShowTitleBgColor NOTIFY showTitleChanged)
    Q_PROPERTY(QString showTitleTextColor READ showTitleTextColor WRITE setShowTitleTextColor NOTIFY showTitleChanged)
    Q_PROPERTY(QString showTitleBorderColor READ showTitleBorderColor WRITE setShowTitleBorderColor NOTIFY showTitleChanged)
    Q_PROPERTY(int showTitleFontSize READ showTitleFontSize WRITE setShowTitleFontSize NOTIFY showTitleChanged)
    Q_PROPERTY(QString showTitleEntryAnim READ showTitleEntryAnim WRITE setShowTitleEntryAnim NOTIFY showTitleChanged)
    Q_PROPERTY(QString showTitleLoopAnim READ showTitleLoopAnim WRITE setShowTitleLoopAnim NOTIFY showTitleChanged)

    // Talent display timing
    Q_PROPERTY(int talentDisplayDurationSec READ talentDisplayDurationSec WRITE setTalentDisplayDurationSec NOTIFY timingChanged)
    Q_PROPERTY(int titleReappearDelaySec READ titleReappearDelaySec WRITE setTitleReappearDelaySec NOTIFY timingChanged)

    // Overlay visibility
    Q_PROPERTY(bool scoreboardVisible READ scoreboardVisible WRITE setScoreboardVisible NOTIFY brandingChanged)
    Q_PROPERTY(bool weatherVisible READ weatherVisible WRITE setWeatherVisible NOTIFY brandingChanged)
    Q_PROPERTY(bool clockVisible READ clockVisible WRITE setClockVisible NOTIFY brandingChanged)
    Q_PROPERTY(QString clockFormat READ clockFormat WRITE setClockFormat NOTIFY brandingChanged)
    Q_PROPERTY(bool tickerVisible READ tickerVisible WRITE setTickerVisible NOTIFY brandingChanged)

    // Scoreboard data
    Q_PROPERTY(QString scoreboardTeamA READ scoreboardTeamA WRITE setScoreboardTeamA NOTIFY brandingChanged)
    Q_PROPERTY(QString scoreboardTeamB READ scoreboardTeamB WRITE setScoreboardTeamB NOTIFY brandingChanged)
    Q_PROPERTY(int scoreboardScoreA READ scoreboardScoreA WRITE setScoreboardScoreA NOTIFY brandingChanged)
    Q_PROPERTY(int scoreboardScoreB READ scoreboardScoreB WRITE setScoreboardScoreB NOTIFY brandingChanged)

    // Ticker appearance
    Q_PROPERTY(QString tickerBgColor READ tickerBgColor WRITE setTickerBgColor NOTIFY brandingChanged)
    Q_PROPERTY(QString tickerTextColor READ tickerTextColor WRITE setTickerTextColor NOTIFY brandingChanged)
    Q_PROPERTY(int tickerFontSize READ tickerFontSize WRITE setTickerFontSize NOTIFY brandingChanged)
    Q_PROPERTY(int tickerSpeed READ tickerSpeed WRITE setTickerSpeed NOTIFY brandingChanged)

    // Scoreboard colors
    Q_PROPERTY(QString scoreboardColorA READ scoreboardColorA WRITE setScoreboardColorA NOTIFY brandingChanged)
    Q_PROPERTY(QString scoreboardColorB READ scoreboardColorB WRITE setScoreboardColorB NOTIFY brandingChanged)
    Q_PROPERTY(QString scoreboardPosition READ scoreboardPosition WRITE setScoreboardPosition NOTIFY brandingChanged)

    // Ticker manual text
    Q_PROPERTY(QString tickerManualText READ tickerManualText WRITE setTickerManualText NOTIFY brandingChanged)

    // Scoreboard timer/period/cards
    Q_PROPERTY(QString scoreboardMatchTime READ scoreboardMatchTime WRITE setScoreboardMatchTime NOTIFY brandingChanged)
    Q_PROPERTY(int scoreboardPeriod READ scoreboardPeriod WRITE setScoreboardPeriod NOTIFY brandingChanged)
    Q_PROPERTY(int scoreboardYellowA READ scoreboardYellowA WRITE setScoreboardYellowA NOTIFY brandingChanged)
    Q_PROPERTY(int scoreboardYellowB READ scoreboardYellowB WRITE setScoreboardYellowB NOTIFY brandingChanged)
    Q_PROPERTY(int scoreboardRedA READ scoreboardRedA WRITE setScoreboardRedA NOTIFY brandingChanged)
    Q_PROPERTY(int scoreboardRedB READ scoreboardRedB WRITE setScoreboardRedB NOTIFY brandingChanged)

public:
    explicit SetupController(ProfileManager* profiles, QObject* parent = nullptr);

    // Profile
    QString currentProfileName() const;
    QString emissionName() const;
    void setEmissionName(const QString& name);

    // Style
    QString selectedStyle() const { return m_profile.overlayStyleId; }
    void setSelectedStyle(const QString& id);
    QColor accentColor() const { return m_profile.accentColor; }
    void setAccentColor(const QColor& c);
    double backgroundOpacity() const { return m_profile.backgroundOpacity; }
    void setBackgroundOpacity(double v);
    int overlayPosition() const { return m_profile.overlayPosition; }
    void setOverlayPosition(int pos);

    // Animation
    QString animationType() const { return m_profile.animationType; }
    void setAnimationType(const QString& type);
    int animEnterFrames() const { return m_profile.animEnterFrames; }
    void setAnimEnterFrames(int frames);
    int animExitFrames() const { return m_profile.animExitFrames; }
    void setAnimExitFrames(int frames);
    QString logoPath() const { return m_profile.logoPath; }
    void setLogoPath(const QString& path);

    // Multi-face overlay
    QString multiFaceName() const { return m_profile.multiFaceName; }
    void setMultiFaceName(const QString& v);
    QString multiFaceRole() const { return m_profile.multiFaceRole; }
    void setMultiFaceRole(const QString& v);
    QString multiFaceStyleId() const { return m_profile.multiFaceStyleId; }
    void setMultiFaceStyleId(const QString& v);

    // Overlay timing
    int singleFaceDelayMs() const { return m_profile.singleFaceDelayMs; }
    void setSingleFaceDelayMs(int v);
    int multiFaceDelayMs() const { return m_profile.multiFaceDelayMs; }
    void setMultiFaceDelayMs(int v);
    int overlayHideDelayMs() const { return m_profile.overlayHideDelayMs; }
    void setOverlayHideDelayMs(int v);

    // Source
    QString inputType() const { return m_profile.inputType; }
    void setInputType(const QString& type);
    QString inputSource() const { return m_profile.inputSource; }
    void setInputSource(const QString& src);

    // Outputs
    bool outputSDI() const { return m_profile.outputSDI; }
    void setOutputSDI(bool v);
    bool outputNDI() const { return m_profile.outputNDI; }
    void setOutputNDI(bool v);
    bool outputRTMP() const { return m_profile.outputRTMP; }
    void setOutputRTMP(bool v);
    bool outputSRT() const { return m_profile.outputSRT; }
    void setOutputSRT(bool v);
    QString rtmpUrl() const { return m_profile.rtmpUrl; }
    void setRtmpUrl(const QString& url);
    QString rtmpKey() const { return m_profile.rtmpKey; }
    void setRtmpKey(const QString& key);

    // Channel branding
    QString channelLogoPath() const { return m_channelLogoPath; }
    void setChannelLogoPath(const QString& v);
    QString channelLogoPosition() const { return m_channelLogoPosition; }
    void setChannelLogoPosition(const QString& v);
    int channelLogoSize() const { return m_channelLogoSize; }
    void setChannelLogoSize(int v);
    bool showChannelNameText() const { return m_showChannelNameText; }
    void setShowChannelNameText(bool v);
    bool keepLogoDuringAds() const { return m_keepLogoDuringAds; }
    void setKeepLogoDuringAds(bool v);

    // Channel logo/name offsets
    int channelLogoOffsetX() const { return m_channelLogoOffsetX; }
    void setChannelLogoOffsetX(int v);
    int channelLogoOffsetY() const { return m_channelLogoOffsetY; }
    void setChannelLogoOffsetY(int v);
    int channelNameOffsetX() const { return m_channelNameOffsetX; }
    void setChannelNameOffsetX(int v);
    int channelNameOffsetY() const { return m_channelNameOffsetY; }
    void setChannelNameOffsetY(int v);

    // Channel name design
    QString channelNameShape() const { return m_channelNameShape; }
    void setChannelNameShape(const QString& v);
    QString channelNameBgColor() const { return m_channelNameBgColor; }
    void setChannelNameBgColor(const QString& v);
    QString channelNameTextColor() const { return m_channelNameTextColor; }
    void setChannelNameTextColor(const QString& v);
    QString channelNameBorderColor() const { return m_channelNameBorderColor; }
    void setChannelNameBorderColor(const QString& v);
    int channelNameFontSize() const { return m_channelNameFontSize; }
    void setChannelNameFontSize(int v);

    // Entry animations
    QString logoEntryAnim() const { return m_logoEntryAnim; }
    void setLogoEntryAnim(const QString& v);
    QString nameEntryAnim() const { return m_nameEntryAnim; }
    void setNameEntryAnim(const QString& v);

    // Loop animations
    QString logoLoopAnim() const { return m_logoLoopAnim; }
    void setLogoLoopAnim(const QString& v);
    QString nameLoopAnim() const { return m_nameLoopAnim; }
    void setNameLoopAnim(const QString& v);

    // Show title
    QString showTitle() const { return m_showTitle; }
    void setShowTitle(const QString& v);
    QString showSubtitle() const { return m_showSubtitle; }
    void setShowSubtitle(const QString& v);
    bool showTitleEnabled() const { return m_showTitleEnabled; }
    void setShowTitleEnabled(bool v);
    QString showTitlePosition() const { return m_showTitlePosition; }
    void setShowTitlePosition(const QString& v);

    // Show title design & animation
    QString showTitleShape() const { return m_showTitleShape; }
    void setShowTitleShape(const QString& v);
    QString showTitleBgColor() const { return m_showTitleBgColor; }
    void setShowTitleBgColor(const QString& v);
    QString showTitleTextColor() const { return m_showTitleTextColor; }
    void setShowTitleTextColor(const QString& v);
    QString showTitleBorderColor() const { return m_showTitleBorderColor; }
    void setShowTitleBorderColor(const QString& v);
    int showTitleFontSize() const { return m_showTitleFontSize; }
    void setShowTitleFontSize(int v);
    QString showTitleEntryAnim() const { return m_showTitleEntryAnim; }
    void setShowTitleEntryAnim(const QString& v);
    QString showTitleLoopAnim() const { return m_showTitleLoopAnim; }
    void setShowTitleLoopAnim(const QString& v);

    // Show title offsets
    int showTitleOffsetX() const { return m_showTitleOffsetX; }
    void setShowTitleOffsetX(int v);
    int showTitleOffsetY() const { return m_showTitleOffsetY; }
    void setShowTitleOffsetY(int v);

    // Ticker offset
    int tickerOffsetY() const { return m_tickerOffsetY; }
    void setTickerOffsetY(int v);

    // Subtitle offsets
    int subtitleOffsetX() const { return m_subtitleOffsetX; }
    void setSubtitleOffsetX(int v);
    int subtitleOffsetY() const { return m_subtitleOffsetY; }
    void setSubtitleOffsetY(int v);

    // Countdown offsets
    int countdownOffsetX() const { return m_countdownOffsetX; }
    void setCountdownOffsetX(int v);
    int countdownOffsetY() const { return m_countdownOffsetY; }
    void setCountdownOffsetY(int v);

    // Clock offsets
    int clockOffsetX() const { return m_clockOffsetX; }
    void setClockOffsetX(int v);
    int clockOffsetY() const { return m_clockOffsetY; }
    void setClockOffsetY(int v);

    // QR Code offsets
    int qrCodeOffsetX() const { return m_qrCodeOffsetX; }
    void setQrCodeOffsetX(int v);
    int qrCodeOffsetY() const { return m_qrCodeOffsetY; }
    void setQrCodeOffsetY(int v);

    // Scoreboard offsets
    int scoreboardOffsetX() const { return m_scoreboardOffsetX; }
    void setScoreboardOffsetX(int v);
    int scoreboardOffsetY() const { return m_scoreboardOffsetY; }
    void setScoreboardOffsetY(int v);

    // Weather offsets
    int weatherOffsetX() const { return m_weatherOffsetX; }
    void setWeatherOffsetX(int v);
    int weatherOffsetY() const { return m_weatherOffsetY; }
    void setWeatherOffsetY(int v);

    // Overlay scale factors
    double nameplateScale() const { return m_nameplateScale; }
    void setNameplateScale(double v);
    double scoreboardScale() const { return m_scoreboardScale; }
    void setScoreboardScale(double v);
    double weatherScale() const { return m_weatherScale; }
    void setWeatherScale(double v);
    double clockScale() const { return m_clockScale; }
    void setClockScale(double v);
    double countdownScale() const { return m_countdownScale; }
    void setCountdownScale(double v);
    double qrCodeScale() const { return m_qrCodeScale; }
    void setQrCodeScale(double v);

    // Talent display timing
    int talentDisplayDurationSec() const { return m_talentDisplayDurationSec; }
    void setTalentDisplayDurationSec(int v);
    int titleReappearDelaySec() const { return m_titleReappearDelaySec; }
    void setTitleReappearDelaySec(int v);

    // Overlay visibility
    bool scoreboardVisible() const { return m_scoreboardVisible; }
    void setScoreboardVisible(bool v);
    bool weatherVisible() const { return m_weatherVisible; }
    void setWeatherVisible(bool v);
    bool clockVisible() const { return m_clockVisible; }
    void setClockVisible(bool v);
    QString clockFormat() const { return m_clockFormat; }
    void setClockFormat(const QString& v);
    bool tickerVisible() const { return m_tickerVisible; }
    void setTickerVisible(bool v);

    // Scoreboard data
    QString scoreboardTeamA() const { return m_scoreboardTeamA; }
    void setScoreboardTeamA(const QString& v);
    QString scoreboardTeamB() const { return m_scoreboardTeamB; }
    void setScoreboardTeamB(const QString& v);
    int scoreboardScoreA() const { return m_scoreboardScoreA; }
    void setScoreboardScoreA(int v);
    int scoreboardScoreB() const { return m_scoreboardScoreB; }
    void setScoreboardScoreB(int v);

    // Ticker appearance
    QString tickerBgColor() const { return m_tickerBgColor; }
    void setTickerBgColor(const QString& v);
    QString tickerTextColor() const { return m_tickerTextColor; }
    void setTickerTextColor(const QString& v);
    int tickerFontSize() const { return m_tickerFontSize; }
    void setTickerFontSize(int v);
    int tickerSpeed() const { return m_tickerSpeed; }
    void setTickerSpeed(int v);

    // Scoreboard colors & position
    QString scoreboardColorA() const { return m_scoreboardColorA; }
    void setScoreboardColorA(const QString& v);
    QString scoreboardColorB() const { return m_scoreboardColorB; }
    void setScoreboardColorB(const QString& v);
    QString scoreboardPosition() const { return m_scoreboardPosition; }
    void setScoreboardPosition(const QString& v);

    // Ticker manual text
    QString tickerManualText() const { return m_tickerManualText; }
    void setTickerManualText(const QString& v);

    // Scoreboard timer/period/cards
    QString scoreboardMatchTime() const { return m_scoreboardMatchTime; }
    void setScoreboardMatchTime(const QString& v);
    int scoreboardPeriod() const { return m_scoreboardPeriod; }
    void setScoreboardPeriod(int v);
    int scoreboardYellowA() const { return m_scoreboardYellowA; }
    void setScoreboardYellowA(int v);
    int scoreboardYellowB() const { return m_scoreboardYellowB; }
    void setScoreboardYellowB(int v);
    int scoreboardRedA() const { return m_scoreboardRedA; }
    void setScoreboardRedA(int v);
    int scoreboardRedB() const { return m_scoreboardRedB; }
    void setScoreboardRedB(int v);

    Q_INVOKABLE void loadAnimatedLogo(const QString& path);
    Q_INVOKABLE bool isReadyToGo() const;
    Q_INVOKABLE void saveProfile();
    Q_INVOKABLE void newProfile(const QString& name);
    Q_INVOKABLE void loadProfile(const QString& id);
    Q_INVOKABLE QVariantList availableProfiles();
    Q_INVOKABLE void exportProfile(const QString& filePath);
    Q_INVOKABLE void importProfile(const QString& filePath);

signals:
    void profileChanged();
    void styleChanged();
    void animationChanged();
    void sourceChanged();
    void outputsChanged();
    void overlayTimingChanged();
    void readyToLaunch();
    void brandingChanged();
    void showTitleChanged();
    void timingChanged();

private:
    ProfileManager* m_profiles;
    EmissionProfile m_profile;

    // Channel branding
    QString m_channelLogoPath;
    QString m_channelLogoPosition = "top_right";
    int     m_channelLogoSize = 60;
    bool    m_showChannelNameText = true;
    bool    m_keepLogoDuringAds = true;
    int     m_channelLogoOffsetX = 0;
    int     m_channelLogoOffsetY = 0;
    int     m_channelNameOffsetX = 0;
    int     m_channelNameOffsetY = 0;

    // Channel name design
    QString m_channelNameShape = "rectangle";
    QString m_channelNameBgColor = "#CC0000";
    QString m_channelNameTextColor = "#FFFFFF";
    QString m_channelNameBorderColor = "#FFFFFF";
    int     m_channelNameFontSize = 14;

    // Entry animations
    QString m_logoEntryAnim = "fade";
    QString m_nameEntryAnim = "slide_left";

    // Loop animations
    QString m_logoLoopAnim = "pulse";
    QString m_nameLoopAnim = "none";

    // Show title
    QString m_showTitle;
    QString m_showSubtitle;
    bool    m_showTitleEnabled = false;
    QString m_showTitlePosition = "bottom_left";

    // Show title design & animation
    QString m_showTitleShape = "rectangle";
    QString m_showTitleBgColor = "#1A1A2E";
    QString m_showTitleTextColor = "#FFFFFF";
    QString m_showTitleBorderColor = "#5B4FDB";
    int     m_showTitleFontSize = 16;
    QString m_showTitleEntryAnim = "slide_up";
    QString m_showTitleLoopAnim = "none";

    // Show title offsets
    int     m_showTitleOffsetX = 0;
    int     m_showTitleOffsetY = 0;

    // Ticker offset
    int     m_tickerOffsetY = 0;

    // Subtitle offsets
    int     m_subtitleOffsetX = 0;
    int     m_subtitleOffsetY = 0;

    // Countdown offsets
    int     m_countdownOffsetX = 0;
    int     m_countdownOffsetY = 0;

    // Clock offsets
    int     m_clockOffsetX = 0;
    int     m_clockOffsetY = 0;

    // QR Code offsets
    int     m_qrCodeOffsetX = 0;
    int     m_qrCodeOffsetY = 0;

    // Scoreboard offsets
    int     m_scoreboardOffsetX = 0;
    int     m_scoreboardOffsetY = 0;

    // Weather offsets
    int     m_weatherOffsetX = 0;
    int     m_weatherOffsetY = 0;

    // Overlay scale factors
    double  m_nameplateScale = 1.0;
    double  m_scoreboardScale = 1.0;
    double  m_weatherScale = 1.0;
    double  m_clockScale = 1.0;
    double  m_countdownScale = 1.0;
    double  m_qrCodeScale = 1.0;

    // Talent display timing
    int     m_talentDisplayDurationSec = 8;
    int     m_titleReappearDelaySec = 2;

    // Overlay visibility
    bool    m_scoreboardVisible = false;
    bool    m_weatherVisible = false;
    bool    m_clockVisible = true;
    QString m_clockFormat = "HH:mm:ss";
    bool    m_tickerVisible = true;

    // Scoreboard data
    QString m_scoreboardTeamA = "TEAM A";
    QString m_scoreboardTeamB = "TEAM B";
    int     m_scoreboardScoreA = 0;
    int     m_scoreboardScoreB = 0;

    // Ticker appearance
    QString m_tickerBgColor = "#CC0000";
    QString m_tickerTextColor = "#FFFFFF";
    int     m_tickerFontSize = 14;
    int     m_tickerSpeed = 2;

    // Scoreboard colors & position
    QString m_scoreboardColorA = "#CC0000";
    QString m_scoreboardColorB = "#0066CC";
    QString m_scoreboardPosition = "top_left";

    // Ticker manual text
    QString m_tickerManualText;

    // Scoreboard timer/period/cards
    QString m_scoreboardMatchTime = "00:00";
    int     m_scoreboardPeriod = 1;
    int     m_scoreboardYellowA = 0;
    int     m_scoreboardYellowB = 0;
    int     m_scoreboardRedA = 0;
    int     m_scoreboardRedB = 0;
};

} // namespace prestige
