#pragma once

// ============================================================
// Prestige AI — Emission Profile
// Pre-production configuration saved per show
// ============================================================

#include <QObject>
#include <QString>
#include <QStringList>
#include <QColor>
#include <QDateTime>
#include <QJsonObject>
#include <QList>

namespace prestige {

struct EmissionProfile {
    QString     id;
    QString     name;               // "JT 20h", "Matinale", "Débat politique"
    QString     overlayStyleId;     // "bfm", "lci", "minimal"...
    QString     animationType = "slide_left"; // Animation type (independent from style)
    int         animEnterFrames = 15;  // Duration enter animation (frames)
    int         animExitFrames  = 8;   // Duration exit animation (frames)
    QColor      accentColor = QColor("#E30613");
    double      backgroundOpacity = 0.8;
    int         overlayPosition = 0;  // 0=bas-gauche, 1=bas-centre, 2=bas-droite, 3=haut-gauche
    QString     logoPath;

    // Source vidéo
    QString     inputType = "webcam";   // "ndi", "decklink", "webcam"
    QString     inputSource;            // NDI source name or device index

    // Sorties actives
    bool        outputSDI  = false;
    bool        outputNDI  = false;
    bool        outputRTMP = false;
    bool        outputSRT  = false;
    QString     rtmpUrl;
    QString     rtmpKey;
    QString     srtUrl;

    // Talents assignés
    QStringList talentIds;

    // Réseaux sociaux
    QJsonObject socialMediaConfigs;
    QString     socialOutputsJson;

    // Multi-face overlay
    QString     multiFaceName;
    QString     multiFaceRole;
    QString     multiFaceStyleId;

    // Overlay timing
    int         singleFaceDelayMs = 1000;
    int         multiFaceDelayMs  = 500;
    int         overlayHideDelayMs = 2000;

    // Output quality
    int         outputFps = 25;
    int         outputBitrate = 8;

    // Channel branding (persisted)
    QString     channelLogoPath;
    QString     channelLogoPosition = "top_right";
    int         channelLogoSize = 60;
    bool        showChannelNameText = true;
    bool        keepLogoDuringAds = true;
    QString     channelNameShape = "rectangle";
    QString     channelNameBgColor = "#CC0000";
    QString     channelNameTextColor = "#FFFFFF";
    QString     channelNameBorderColor = "#FFFFFF";
    int         channelNameFontSize = 14;
    QString     logoEntryAnim = "fade";
    QString     nameEntryAnim = "slide_left";
    QString     logoLoopAnim = "pulse";
    QString     nameLoopAnim = "none";

    // Show title (persisted)
    QString     showTitle;
    QString     showSubtitle;
    bool        showTitleEnabled = true;
    QString     showTitlePosition = "bottom_left";
    QString     showTitleShape = "rectangle";
    QString     showTitleBgColor = "#1A1A2E";
    QString     showTitleTextColor = "#FFFFFF";
    QString     showTitleBorderColor = "#5B4FDB";
    int         showTitleFontSize = 16;

    // Overlay visibility
    bool        scoreboardVisible = false;
    bool        weatherVisible = false;
    bool        clockVisible = false;
    QString     clockFormat = "HH:mm:ss";
    bool        tickerVisible = false;

    // Overlay scales
    double      nameplateScale = 1.0;
    double      scoreboardScale = 1.0;
    double      weatherScale = 1.0;
    double      clockScale = 1.0;
    double      countdownScale = 1.0;
    double      qrCodeScale = 1.0;

    // Virtual Studio
    bool        virtualStudioEnabled = false;
    int         virtualStudioId = 0;
    bool        chromaKeyEnabled = false;
    QString     chromaKeyColor = "green";
    double      chromaKeyTolerance = 0.35;
    double      chromaKeySmooth = 0.05;

    // Sport system
    QString     teamLogoA;
    QString     teamLogoB;
    QString     goalAnimText = "GOAL!";
    QString     goalAnimEffect = "kinetic_pop";
    int         goalAnimDuration = 5;

    // Design Templates
    QString     nameplateDesign = "np_broadcast_news";
    QString     titleDesign = "tt_broadcast";
    QString     channelDesign = "ch_rectangle_red";
    QString     tickerDesign = "tk_news_red";
    QString     scoreboardDesign = "sb_glass_dark";
    QString     clockDesign = "ck_shadow_only";
    QString     weatherDesign = "wt_shadow_only";

    // Scoreboard detailed data
    QString     scoreboardTeamA = "HOME";
    QString     scoreboardTeamB = "AWAY";
    QString     scoreboardColorA = "#CC0000";
    QString     scoreboardColorB = "#0066CC";
    QString     scoreboardPosition = "top_left";

    // Ticker config
    QString     tickerBgColor = "#CC0000";
    QString     tickerTextColor = "#FFFFFF";
    int         tickerFontSize = 14;
    int         tickerSpeed = 2;
    QString     tickerManualText;

    // Show title animations
    QString     showTitleEntryAnim = "slide_up";
    QString     showTitleLoopAnim = "none";

    // Talent timing
    int         talentDisplayDurationSec = 8;
    int         titleReappearDelaySec = 2;

    // Virtual Studio colors
    QString     vsPrimaryColor;
    QString     vsSecondaryColor;
    QString     vsAccentColor;
    QString     vsFloorColor;
    double      vsLightIntensity = 1.0;
    bool        vsAnimationsEnabled = true;
    QString     vsCustomBackground;

    QDateTime   createdAt;
    QDateTime   lastUsed;

    QJsonObject toJson() const;
    static EmissionProfile fromJson(const QJsonObject& obj);
};

class ProfileManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QStringList profileNames READ profileNames NOTIFY profilesChanged)
    Q_PROPERTY(QString currentProfileId READ currentProfileId NOTIFY currentProfileChanged)

public:
    explicit ProfileManager(QObject* parent = nullptr);

    QList<EmissionProfile> loadAll();
    EmissionProfile loadById(const QString& id);
    void save(const EmissionProfile& profile);
    void remove(const QString& id);

    Q_INVOKABLE EmissionProfile createNew(const QString& name);
    Q_INVOKABLE bool loadProfile(const QString& id);
    Q_INVOKABLE QStringList profileNames() const;

    QString currentProfileId() const { return m_currentId; }
    const EmissionProfile& current() const { return m_current; }
    EmissionProfile& current() { return m_current; }

signals:
    void profilesChanged();
    void currentProfileChanged();
    void profileLoaded(const QString& name);

private:
    QString profilesDir() const;

    EmissionProfile         m_current;
    QString                 m_currentId;
    QList<EmissionProfile>  m_cache;
};

} // namespace prestige
