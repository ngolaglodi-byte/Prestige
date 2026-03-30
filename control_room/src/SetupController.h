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

    // Talent display timing
    int talentDisplayDurationSec() const { return m_talentDisplayDurationSec; }
    void setTalentDisplayDurationSec(int v);
    int titleReappearDelaySec() const { return m_titleReappearDelaySec; }
    void setTitleReappearDelaySec(int v);

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

    // Talent display timing
    int     m_talentDisplayDurationSec = 8;
    int     m_titleReappearDelaySec = 2;
};

} // namespace prestige
