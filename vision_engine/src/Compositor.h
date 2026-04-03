#pragma once

// ============================================================
// Prestige AI — Compositor
// Premium broadcast overlay rendering WITH ANIMATIONS.
// Each overlay has an animation state (enter/visible/exit).
// Easing curves, slide, fade, scale — frame-by-frame.
// ============================================================

#include <QObject>
#include <QImage>
#include <QPainter>
#include <QFont>
#include <QColor>
#include <QRectF>
#include <QList>
#include <QElapsedTimer>
#include <QString>
#include <functional>
#include <map>

#include "TalentData.h"
#include "BroadcastEffects.h"
#include "DesignTemplates.h"
#include "VirtualStudio.h"
#include "GpuEffects.h"
#include "AEEasing.h"
#include "LottieEngine.h"

namespace prestige {

// ── Per-overlay animation state ────────────────────────────
struct OverlayAnimState {
    QString id;
    double  progress  = 0.0;    // 0.0 = invisible, 1.0 = fully visible
    double  targetProg = 0.0;   // Where we're animating toward
    bool    wasVisible = false;
    int     enterFrames  = 15;  // ~500ms at 30fps
    int     exitFrames   = 8;   // ~260ms at 30fps
};

// ── Animation types ────────────────────────────────────────
enum class AnimType {
    SlideLeft,      // BFM, CNN — slide from left
    SlideRight,     // Sky News — slide from right
    SlideUp,        // France 2, Football — slide from bottom
    FadeScale,      // LCI, Sports — fade + scale from 0.95
    WipeHorizontal, // France 24, BBC — horizontal wipe/clip
    FadePure,       // Cinema, Luxury, Minimal, Olympics — pure opacity
    IrisScale,      // Politique — scale from center
    GlitchIn,       // Tech — flicker then appear
};

class Compositor : public QObject {
    Q_OBJECT

public:
    explicit Compositor(QObject* parent = nullptr);

    QImage composite(const QImage& videoFrame, const QList<TalentOverlay>& talents);

    void setStyleId(const QString& styleId);
    QString styleId() const { return m_styleId; }
    void setAccentColor(const QColor& color) { m_accentColor = color; }
    void setBgOpacity(double opacity) { m_bgOpacity = opacity; }
    void setAnimationSpeed(double factor) { m_animSpeed = factor; }
    void setAnimationType(const QString& type);
    void setAnimEnterFrames(int frames);
    void setAnimExitFrames(int frames);
    double lastCompositeMs() const { return m_lastCompositeMs; }

    // ── Broadcast overlays (logo, clock, ticker) ─────────
    // Note: Lower Third was replaced by Show Title + Talent Nameplate system
    void setTickerText(const QString& text);
    void setTickerVisible(bool visible);
    void setLogoImage(const QImage& logo);
    void setLogoVisible(bool visible);
    void setClockVisible(bool visible);

    // ── Subtitle overlays ─────────────────────────────────
    void setSubtitleText(const QString& text);
    void setSubtitleVisible(bool visible);
    void setSubtitleFontSize(int size);
    void setSubtitlePosition(const QString& pos);
    void setSubtitleBgOpacity(double opacity);
    void setSubtitleTextColor(const QColor& color);

    // ── Countdown overlay ─────────────────────────────────
    void setCountdownText(const QString& text);
    void setCountdownVisible(bool visible);

    // ── QR Code overlay ──────────────────────────────────
    void setQrCodeUrl(const QString& url);
    void setQrCodeVisible(bool visible);
    void setQrCodePosition(const QString& position);

    // ── Social Chat overlay ──────────────────────────────
    void setSocialChatMessages(const QStringList& messages);
    void setSocialChatVisible(bool visible);
    void setSocialChatPosition(const QString& pos) { m_chatPosition = pos; }
    void setSocialChatOffset(int x, int y) { m_chatOffX = x; m_chatOffY = y; }

    // ── Channel Name ─────────────────────────────────────
    void setChannelName(const QString& name);

    // ── Channel branding animations ─────────────────────
    void setChannelNameShape(const QString& shape);
    void setChannelNameColors(const QColor& bg, const QColor& text, const QColor& border);
    void setChannelNameFontSize(int size);
    void setLogoEntryAnim(const QString& anim);
    void setNameEntryAnim(const QString& anim);
    void setLogoLoopAnim(const QString& anim);
    void setNameLoopAnim(const QString& anim);
    void resetBrandingAnimations();

    // ── Broadcast overlay layers ─────────────────────────
    void setChannelLogoPosition(const QString& pos);
    void setChannelLogoSize(int heightPx);
    void setShowChannelNameText(bool show);
    void setKeepLogoDuringAds(bool keep);
    void setShowTitle(const QString& title, const QString& subtitle);
    void setShowTitleVisible(bool visible);
    void setShowTitlePosition(const QString& pos);
    void setShowTitleShape(const QString& shape);
    void setShowTitleColors(const QColor& bg, const QColor& text, const QColor& border);
    void setShowTitleFontSize(int size);
    void setShowTitleEntryAnim(const QString& anim);
    void setShowTitleLoopAnim(const QString& anim);
    void setTalentNameplateVisible(bool visible);
    void setBypassActive(bool active);

    // ── Virtual Studio ────────────────────────────────────
    void setVirtualStudioEnabled(bool v) { m_virtualStudio.setEnabled(v); }
    void setVirtualStudioId(int id) { m_virtualStudio.setStudioId(id); }
    void setChromaKeyEnabled(bool v) { m_virtualStudio.setChromaKeyEnabled(v); }
    void setChromaKeyColor(const QString& c) { m_virtualStudio.setChromaKeyColor(c); }
    void setChromaKeyTolerance(double v) { m_virtualStudio.setChromaKeyTolerance(v); }
    void setChromaKeySmooth(double v) { m_virtualStudio.setChromaKeySmooth(v); }
    void setVsPrimaryColor(const QColor& c) { m_virtualStudio.setPrimaryColor(c); }
    void setVsSecondaryColor(const QColor& c) { m_virtualStudio.setSecondaryColor(c); }
    void setVsAccentColor(const QColor& c) { m_virtualStudio.setAccentColor(c); }
    void setVsFloorColor(const QColor& c) { m_virtualStudio.setFloorColor(c); }
    void setVsLightColor(const QColor& c) { m_virtualStudio.setLightColor(c); }
    void setVsLightIntensity(double v) { m_virtualStudio.setLightIntensity(v); }
    void setVsAnimationsEnabled(bool v) { m_virtualStudio.setAnimationsEnabled(v); }
    void setVsCustomBackground(const QString& path) { m_virtualStudio.setCustomBackgroundPath(path); }

    // ══════════════════════════════════════════════════════
    // AE EFFECTS — Applied to overlay layer in real-time
    // ══════════════════════════════════════════════════════

    // ── Overlay Blend Mode (AE 27 modes) ────────────────
    void setOverlayBlendMode(const QString& mode) { m_overlayBlendMode = ae::blendModeByName(mode); m_overlayBlendModeName = mode; }
    QString overlayBlendMode() const { return m_overlayBlendModeName; }

    // ── Easing Curve (AE Graph Editor) ──────────────────
    void setEasingCurve(const QString& curve) { m_easingCurve = curve; m_easingFunc = ae::easingByName(curve); }
    QString easingCurve() const { return m_easingCurve; }

    // ── AE Post-Effects on Overlay Layer ─────────────────
    // These are applied to the composited overlay BEFORE blending onto video
    void setAeEffectId(const QString& id) { m_aeEffectId = id; }
    QString aeEffectId() const { return m_aeEffectId; }
    void setAeEffectIntensity(double v) { m_aeEffectIntensity = v; }
    void setAeEffectParam1(double v) { m_aeEffectParam1 = v; }
    void setAeEffectParam2(double v) { m_aeEffectParam2 = v; }
    void setAeEffectColor1(const QColor& c) { m_aeEffectColor1 = c; }
    void setAeEffectColor2(const QColor& c) { m_aeEffectColor2 = c; }

    // ── Lottie Animation Preset (from AE exports) ────────
    void setLottiePreset(const QString& id) { m_lottiePresetId = id; m_lottie.setActivePreset(id); }
    QString lottiePreset() const { return m_lottiePresetId; }
    void loadLottiePresets(const QString& dir) { m_lottie.loadPresets(dir); }
    QStringList lottiePresetIds() const { return m_lottie.presetIds(); }
    QStringList lottiePresetNames() const { return m_lottie.presetNames(); }

    // ── AE Text Animator Selection ───────────────────────
    void setTextAnimatorId(const QString& id) { m_textAnimatorId = id; }
    QString textAnimatorId() const { return m_textAnimatorId; }

    // ── AE Transition Selection ──────────────────────────
    void setTransitionId(const QString& id) { m_transitionId = id; }
    QString transitionId() const { return m_transitionId; }

    // ── AE Expression Controls ──────────────────────────
    void setWiggleEnabled(bool v) { m_wiggleEnabled = v; }
    void setWiggleFreq(double f) { m_wiggleFreq = f; }
    void setWiggleAmp(double a) { m_wiggleAmp = a; }

    // ── AE Shape Layer Effect ───────────────────────────
    void setShapeEffectId(const QString& id) { m_shapeEffectId = id; }

    // ── All AE effect/easing lists for UI ───────────────
    static QStringList allAeEffectIds();
    static QStringList allAeTextAnimatorIds();
    static QStringList allAeTransitionIds();
    static QStringList allAeShapeEffectIds();

    // ── Design Templates ────────────────────────────────
    void setNameplateDesign(const QString& d) { m_npDesign = d; }
    void setTitleDesign(const QString& d) { m_titleDesignId = d; }
    void setChannelDesign(const QString& d) { m_chDesign = d; }
    void setTickerDesign(const QString& d) { m_tkDesign = d; }
    void setScoreboardDesign(const QString& d) { m_sbDesignId = d; }
    void setClockDesign(const QString& d) { m_ckDesign = d; }
    void setWeatherDesign(const QString& d) { m_wtDesign = d; }

    // ── Sport system ─────────────────────────────────────
    void setTeamLogoA(const QString& path);
    void setTeamLogoB(const QString& path);
    void setGoalAnim(bool active, const QString& text, const QString& team,
                     const QString& player, const QString& effect, int duration);
    void setSportEvent(const QString& event);

    // ── Overlay scale factors ─────────────────────────────
    void setNameplateScale(double s) { m_nameplateScale = s; }
    void setScoreboardScale(double s) { m_scoreboardScale = s; }
    void setWeatherScale(double s) { m_weatherScale = s; }
    void setClockScale(double s) { m_clockScale = s; }
    void setCountdownScale(double s) { m_countdownScale = s; }
    void setQrCodeScale(double s) { m_qrCodeScale = s; }

    // ── Overlay offsets (from drag handles) ──────────────
    void setShowTitleOffset(int x, int y) { m_showTitleOffX = x; m_showTitleOffY = y; }
    void setTickerOffsetY(int y) { m_tickerOffY = y; }
    void setSubtitleOffset(int x, int y) { m_subtitleOffX = x; m_subtitleOffY = y; }
    void setCountdownOffset(int x, int y) { m_countdownOffX = x; m_countdownOffY = y; }
    void setClockOffset(int x, int y) { m_clockOffX = x; m_clockOffY = y; }
    void setQrCodeOffset(int x, int y) { m_qrOffX = x; m_qrOffY = y; }
    void setScoreboardOffset(int x, int y) { m_scoreboardOffX = x; m_scoreboardOffY = y; }
    void setWeatherOffset(int x, int y) { m_weatherOffX = x; m_weatherOffY = y; }
    void setLogoOffset(int x, int y) { m_logoOffX = x; m_logoOffY = y; }
    void setNameOffset(int x, int y) { m_nameOffX = x; m_nameOffY = y; }

    // ── Ticker appearance ─────────────────────────────────
    void setTickerFontSize(int s) { m_tickerFontSize = s; }
    void setTickerBgColor(const QColor& c) { m_tickerBgColor = c; }
    void setTickerTextColor(const QColor& c) { m_tickerTextColor = c; }
    void setTickerSpeed(int s) { m_tickerSpeed = s; }

    // ── Clock config ─────────────────────────────────────
    void setClockFormat(const QString& f) { m_clockFormat = f; }

    // ── Scoreboard overlay ────────────────────────────────
    void setScoreboardVisible(bool v) { m_scoreboardVisible = v; }
    void setScoreboardData(const QString& teamA, const QString& teamB,
                           int scoreA, int scoreB,
                           const QColor& colorA, const QColor& colorB,
                           const QString& position, const QString& matchTime, int period) {
        m_sbTeamA = teamA; m_sbTeamB = teamB;
        m_sbScoreA = scoreA; m_sbScoreB = scoreB;
        m_sbColorA = colorA; m_sbColorB = colorB;
        m_sbPosition = position; m_sbMatchTime = matchTime; m_sbPeriod = period;
    }
    void setScoreboardCards(int yellowA, int yellowB, int redA, int redB) {
        m_sbYellowA = yellowA; m_sbYellowB = yellowB; m_sbRedA = redA; m_sbRedB = redB;
    }

    // ── Weather overlay ──────────────────────────────────
    void setWeatherVisible(bool v) { m_weatherVisible = v; }
    void setWeatherData(const QString& city, double temp, const QString& unit, const QString& icon) {
        m_weatherCity = city; m_weatherTemp = temp; m_weatherUnit = unit; m_weatherIcon = icon;
    }

    // ── RTL support ───────────────────────────────────────
    void setLayoutRtl(bool rtl);

    // ── Animated Logo ────────────────────────────────────
    void setLogoFrames(const QList<QImage>& frames);

signals:
    void frameComposited(const QImage& frame);

private:
    using DrawFunc = std::function<void(QPainter&, const TalentOverlay&, const QSize&, double progress)>;
    void registerStyles();

    // Animation
    void updateAnimations(const QList<TalentOverlay>& talents);
    double easeOutCubic(double t);
    double easeInCubic(double t);
    AnimType animTypeForStyle(const QString& styleId);

    // Premium rendering helpers
    void drawGlassRect(QPainter& p, const QRectF& rect, double radius,
                       const QColor& tint, double opacity, int blur = 0);
    void drawSoftShadow(QPainter& p, const QRectF& rect, double radius,
                        int spread, const QColor& color);
    void drawGlowLine(QPainter& p, const QPointF& p1, const QPointF& p2,
                      double width, const QColor& color, double glowSize);
    void drawPill(QPainter& p, const QRectF& rect, const QColor& bg, const QColor& text,
                  const QString& label, int fontSize);
    QRectF calcPlate(const TalentOverlay& t, const QSize& fs, double w, double h);

    // 20 style renderers — now take progress (0→1) for animation
    void drawBFM(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog);
    void drawLCI(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog);
    void drawFrance2(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog);
    void drawFrance24(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog);
    void drawCNN(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog);
    void drawBBC(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog);
    void drawSkyNews(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog);
    void drawAlJazeera(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog);
    void drawSports(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog);
    void drawFootball(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog);
    void drawOlympics(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog);
    void drawCinema(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog);
    void drawLuxury(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog);
    void drawTech(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog);
    void drawMinimal(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog);
    void drawPolitique(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog);
    void drawGouvernement(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog);
    void drawDualPerson(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog);
    void drawFullscreen(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog);
    void drawBreaking(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog);
    void drawDefault(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog);

    std::map<QString, DrawFunc> m_styles;
    std::map<QString, OverlayAnimState> m_animStates;  // per-talent animation
    QString       m_styleId = "bfm";
    QString       m_animTypeStr = "slide_left";
    int           m_enterFrames = 15;
    int           m_exitFrames  = 8;
    QColor        m_accentColor = QColor("#E30613");
    double        m_bgOpacity = 0.82;
    double        m_animSpeed = 1.0;
    double        m_lastCompositeMs = 0.0;
    QElapsedTimer m_perfTimer;
    QElapsedTimer m_wallClock;       // Wall clock for frame-rate independent timing
    double        m_wallTimeSec = 0; // Accumulated wall time in seconds
    double        m_deltaTime = 0;   // Delta time since last frame (seconds)
    bool          m_wallClockStarted = false;

    // Broadcast overlays
    QString m_tickerText;
    bool    m_tickerVisible = false;
    int     m_tickerOffset = 0;
    QList<QImage> m_logoFrames;
    int     m_logoFrameIndex = 0;
    bool    m_logoVisible = false;
    bool    m_clockVisible = false;

    // QR Code
    QString m_qrUrl;
    bool    m_qrVisible = false;
    QString m_qrPosition = "bottom_right";

    // Subtitles
    QString m_subtitleText;
    bool    m_subtitleVisible  = false;
    int     m_subtitleFontSize = 18;
    QString m_subtitlePosition = "bottom";
    double  m_subtitleBgOpacity = 0.6;
    QColor  m_subtitleTextColor = Qt::white;
    double  m_subtitleFadeProgress = 0.0;
    QString m_prevSubtitleText;

    // Countdown
    QString m_countdownText;
    bool    m_countdownVisible = false;

    // Social chat overlay
    QStringList m_chatMessages;
    bool        m_chatVisible = false;
    QString     m_chatPosition = "top_right";
    int         m_chatOffX = 0, m_chatOffY = 0;

    // Channel name
    QString     m_channelName;

    // Channel name design
    QString m_nameShape = "rectangle";
    QColor  m_nameBgColor = QColor("#CC0000");
    QColor  m_nameTextColor = Qt::white;
    QColor  m_nameBorderColor = Qt::white;
    int     m_nameFontSize = 14;

    // Entry animation state (0.0 -> 1.0, plays once)
    QString m_logoEntryAnimType = "fade";
    QString m_nameEntryAnimType = "slide_left";
    double  m_logoEntryProgress = 0.0;
    double  m_nameEntryProgress = 0.0;

    // Loop animation state (continuous)
    QString m_logoLoopAnimType = "pulse";
    QString m_nameLoopAnimType = "none";
    int     m_loopFrame = 0;

    // RTL
    bool        m_layoutRtl = false;

    // Broadcast overlay layers
    QString m_logoPosition = "top_right";
    int     m_logoSizeH = 60;
    bool    m_showNameText = true;
    bool    m_keepLogoDuringAds = true;
    QString m_showTitleText;
    QString m_showSubtitleText;
    bool    m_showTitleVisible = false;
    QString m_showTitlePosition = "bottom_left";
    double  m_showTitleProgress = 0.0;

    // Show title design
    QString m_showTitleShape = "rectangle";
    QColor  m_showTitleBgColor = QColor("#1A1A2E");
    QColor  m_showTitleTextColor = Qt::white;
    QColor  m_showTitleBorderColor = QColor("#5B4FDB");
    int     m_showTitleFontSize = 16;
    QString m_showTitleEntryAnimType = "slide_up";
    QString m_showTitleLoopAnimType = "none";
    double  m_showTitleEntryProgress = 0.0;
    bool    m_talentNameplateVisible = true;
    bool    m_bypassActive = false;

    // Design Templates
    QString m_npDesign = "np_broadcast_news";
    QString m_titleDesignId = "tt_broadcast";
    QString m_chDesign = "ch_rectangle_red";
    QString m_tkDesign = "tk_news_red";
    QString m_sbDesignId = "sb_glass_dark";
    QString m_ckDesign = "ck_shadow_only";
    QString m_wtDesign = "wt_shadow_only";

    // ── AE Effects State ────────────────────────────────
    ae::BlendMode m_overlayBlendMode = ae::BlendMode::Normal;
    QString       m_overlayBlendModeName = "normal";
    QString       m_easingCurve = "ease_out_cubic";
    ae::EaseFunc  m_easingFunc = ae::easeOutCubic;
    QString       m_aeEffectId;          // Active post-effect on overlay
    double        m_aeEffectIntensity = 0.5;
    double        m_aeEffectParam1 = 0.5;
    double        m_aeEffectParam2 = 0.5;
    QColor        m_aeEffectColor1 = QColor("#E30613");
    QColor        m_aeEffectColor2 = QColor("#FFFFFF");
    QString       m_textAnimatorId;       // Active text animator
    QString       m_transitionId;         // Active transition
    bool          m_wiggleEnabled = false;
    double        m_wiggleFreq = 3.0;
    double        m_wiggleAmp = 5.0;
    QString       m_shapeEffectId;        // Active shape layer effect
    int           m_aeFrameCount = 0;     // Frame counter for time-based effects
    QImage        m_prevOverlayLayer;     // Previous frame for time effects

    // Lottie Animation Engine
    LottieEngine m_lottie;
    QString      m_lottiePresetId;

    // GPU Effects Engine
    GpuEffects m_gpu;

    // Virtual Studio
    VirtualStudio m_virtualStudio;

    // Sport system
    QImage  m_teamLogoImgA;
    QImage  m_teamLogoImgB;
    QString m_teamLogoPathA;
    QString m_teamLogoPathB;
    bool    m_goalAnimActive = false;
    QString m_goalAnimText = "GOAL!";
    QString m_goalAnimTeam;
    QString m_goalAnimPlayer;
    QString m_goalAnimEffect = "kinetic_pop";
    int     m_goalAnimDuration = 5;     // Duration in seconds
    double  m_goalAnimTime = 0;         // Accumulated time (seconds, wall-clock)
    QString m_sportEvent;
    int     m_sportEventFrame = 0;

    // Overlay scale factors
    double m_nameplateScale = 1.0;
    double m_scoreboardScale = 1.0;
    double m_weatherScale = 1.0;
    double m_clockScale = 1.0;
    double m_countdownScale = 1.0;
    double m_qrCodeScale = 1.0;

    // Overlay offsets (pixels at 1920x1080 reference)
    int m_showTitleOffX = 0, m_showTitleOffY = 0;
    int m_tickerOffY = 0;
    int m_subtitleOffX = 0, m_subtitleOffY = 0;
    int m_countdownOffX = 0, m_countdownOffY = 0;
    int m_clockOffX = 0, m_clockOffY = 0;
    int m_qrOffX = 0, m_qrOffY = 0;
    int m_scoreboardOffX = 0, m_scoreboardOffY = 0;
    int m_weatherOffX = 0, m_weatherOffY = 0;
    int m_logoOffX = 0, m_logoOffY = 0;
    int m_nameOffX = 0, m_nameOffY = 0;

    // Ticker appearance
    int    m_tickerFontSize = 14;
    QColor m_tickerBgColor = QColor("#CC0000");
    QColor m_tickerTextColor = Qt::white;
    int    m_tickerSpeed = 2;

    // Clock format
    QString m_clockFormat = "HH:mm:ss";

    // Scoreboard
    bool    m_scoreboardVisible = false;
    QString m_sbTeamA = "HOME", m_sbTeamB = "AWAY";
    int     m_sbScoreA = 0, m_sbScoreB = 0;
    QColor  m_sbColorA = QColor("#CC0000"), m_sbColorB = QColor("#0066CC");
    QString m_sbPosition = "top_left";
    QString m_sbMatchTime = "00:00";
    int     m_sbPeriod = 1;
    int     m_sbYellowA = 0, m_sbYellowB = 0;
    int     m_sbRedA = 0, m_sbRedB = 0;

    // Weather
    bool    m_weatherVisible = false;
    QString m_weatherCity;
    double  m_weatherTemp = 0;
    QString m_weatherUnit = "\u00B0C";
    QString m_weatherIcon;
};

} // namespace prestige
