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

    // ── Broadcast overlays (lower third, logo, clock) ────
    void setLowerThirdText(const QString& title, const QString& subtitle);
    void setLowerThirdVisible(bool visible);
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
    void applyAnimation(QPainter& p, const QRectF& plateRect, const QSize& frameSize,
                        double progress, AnimType type);

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

    // Broadcast overlays
    QString m_lowerTitle;
    QString m_lowerSubtitle;
    bool    m_lowerVisible = false;
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
};

} // namespace prestige
