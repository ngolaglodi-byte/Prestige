// ============================================================
// Prestige AI — Compositor (Animated Broadcast Rendering)
// Each overlay animates in/out with style-specific transitions.
// ============================================================

#include "Compositor.h"
#include <QLinearGradient>
#include <QRadialGradient>
#include <QPainterPath>
#include <QDebug>
#include <QTime>
#include <cmath>

namespace prestige {

Compositor::Compositor(QObject* parent) : QObject(parent) {
    registerStyles();
    // Initialize GPU effects engine
    if (m_gpu.initialize()) {
        m_virtualStudio.setGpuEffects(&m_gpu);
        qInfo() << "[Compositor] GPU effects engine ACTIVE";
    } else {
        qInfo() << "[Compositor] GPU not available — using CPU fallback";
    }
    // Initialize Design Templates
    DesignRegistry::instance();
    qInfo() << "[Compositor] Design Templates loaded";
}

void Compositor::registerStyles()
{
    m_styles["bfm"]          = [this](auto& p, auto& t, auto& s, double pr) { drawBFM(p, t, s, pr); };
    m_styles["lci"]          = [this](auto& p, auto& t, auto& s, double pr) { drawLCI(p, t, s, pr); };
    m_styles["france2"]      = [this](auto& p, auto& t, auto& s, double pr) { drawFrance2(p, t, s, pr); };
    m_styles["france24"]     = [this](auto& p, auto& t, auto& s, double pr) { drawFrance24(p, t, s, pr); };
    m_styles["cnn"]          = [this](auto& p, auto& t, auto& s, double pr) { drawCNN(p, t, s, pr); };
    m_styles["bbc"]          = [this](auto& p, auto& t, auto& s, double pr) { drawBBC(p, t, s, pr); };
    m_styles["skynews"]      = [this](auto& p, auto& t, auto& s, double pr) { drawSkyNews(p, t, s, pr); };
    m_styles["aljazeera"]    = [this](auto& p, auto& t, auto& s, double pr) { drawAlJazeera(p, t, s, pr); };
    m_styles["sports"]       = [this](auto& p, auto& t, auto& s, double pr) { drawSports(p, t, s, pr); };
    m_styles["football"]     = [this](auto& p, auto& t, auto& s, double pr) { drawFootball(p, t, s, pr); };
    m_styles["olympics"]     = [this](auto& p, auto& t, auto& s, double pr) { drawOlympics(p, t, s, pr); };
    m_styles["cinema"]       = [this](auto& p, auto& t, auto& s, double pr) { drawCinema(p, t, s, pr); };
    m_styles["luxury"]       = [this](auto& p, auto& t, auto& s, double pr) { drawLuxury(p, t, s, pr); };
    m_styles["tech"]         = [this](auto& p, auto& t, auto& s, double pr) { drawTech(p, t, s, pr); };
    m_styles["minimal"]      = [this](auto& p, auto& t, auto& s, double pr) { drawMinimal(p, t, s, pr); };
    m_styles["politique"]    = [this](auto& p, auto& t, auto& s, double pr) { drawPolitique(p, t, s, pr); };
    m_styles["gouvernement"] = [this](auto& p, auto& t, auto& s, double pr) { drawGouvernement(p, t, s, pr); };
    m_styles["dual"]         = [this](auto& p, auto& t, auto& s, double pr) { drawDualPerson(p, t, s, pr); };
    m_styles["fullscreen"]   = [this](auto& p, auto& t, auto& s, double pr) { drawFullscreen(p, t, s, pr); };
    m_styles["breaking"]     = [this](auto& p, auto& t, auto& s, double pr) { drawBreaking(p, t, s, pr); };
}

// ══════════════════════════════════════════════════════════════
// Animation system
// ══════════════════════════════════════════════════════════════

double Compositor::easeOutCubic(double t) { return 1.0 - std::pow(1.0 - t, 3.0); }
double Compositor::easeInCubic(double t)  { return t * t * t; }

void Compositor::setAnimationType(const QString& type) { m_animTypeStr = type; }
void Compositor::setAnimEnterFrames(int frames) { m_enterFrames = qBound(3, frames, 60); }
void Compositor::setAnimExitFrames(int frames) { m_exitFrames = qBound(2, frames, 30); }

static AnimType animTypeFromString(const QString& s)
{
    if (s == "slide_left")  return AnimType::SlideLeft;
    if (s == "slide_right") return AnimType::SlideRight;
    if (s == "slide_up")    return AnimType::SlideUp;
    if (s == "fade_scale")  return AnimType::FadeScale;
    if (s == "wipe")        return AnimType::WipeHorizontal;
    if (s == "fade")        return AnimType::FadePure;
    if (s == "iris")        return AnimType::IrisScale;
    if (s == "glitch")      return AnimType::GlitchIn;
    return AnimType::SlideLeft;
}

AnimType Compositor::animTypeForStyle(const QString& /*id*/)
{
    return animTypeFromString(m_animTypeStr);
}

// ── Broadcast overlay setters ──────────────────────────────

void Compositor::setLowerThirdText(const QString& title, const QString& subtitle) { m_lowerTitle = title; m_lowerSubtitle = subtitle; }
void Compositor::setLowerThirdVisible(bool v) { m_lowerVisible = v; }
void Compositor::setTickerText(const QString& text) { m_tickerText = text; }
void Compositor::setTickerVisible(bool v) { m_tickerVisible = v; }
void Compositor::setLogoImage(const QImage& logo) { m_logoFrames = {logo}; m_logoFrameIndex = 0; }
void Compositor::setLogoFrames(const QList<QImage>& frames) { m_logoFrames = frames; m_logoFrameIndex = 0; }
void Compositor::setLogoVisible(bool v) { m_logoVisible = v; }
void Compositor::setClockVisible(bool v) { m_clockVisible = v; }
void Compositor::setCountdownText(const QString& text) { m_countdownText = text; }
void Compositor::setCountdownVisible(bool visible) { m_countdownVisible = visible; }
void Compositor::setQrCodeUrl(const QString& url) { m_qrUrl = url; }
void Compositor::setQrCodeVisible(bool visible) { m_qrVisible = visible; }
void Compositor::setQrCodePosition(const QString& position) { m_qrPosition = position; }

// Social chat setters
void Compositor::setSocialChatMessages(const QStringList& messages) { m_chatMessages = messages; }
void Compositor::setSocialChatVisible(bool visible) { m_chatVisible = visible; }
void Compositor::setLayoutRtl(bool rtl) { m_layoutRtl = rtl; }
void Compositor::setChannelName(const QString& n) { m_channelName = n; }

// Channel branding animation setters
void Compositor::setChannelNameShape(const QString& shape) { m_nameShape = shape; }
void Compositor::setChannelNameColors(const QColor& bg, const QColor& text, const QColor& border) { m_nameBgColor = bg; m_nameTextColor = text; m_nameBorderColor = border; }
void Compositor::setChannelNameFontSize(int size) { m_nameFontSize = qBound(8, size, 36); }
void Compositor::setLogoEntryAnim(const QString& anim) { m_logoEntryAnimType = anim; }
void Compositor::setNameEntryAnim(const QString& anim) { m_nameEntryAnimType = anim; }
void Compositor::setLogoLoopAnim(const QString& anim) { m_logoLoopAnimType = anim; }
void Compositor::setNameLoopAnim(const QString& anim) { m_nameLoopAnimType = anim; }
void Compositor::resetBrandingAnimations() { m_logoEntryProgress = 0.0; m_nameEntryProgress = 0.0; m_showTitleEntryProgress = 0.0; m_loopFrame = 0; }

// Show title design setters
void Compositor::setShowTitleShape(const QString& shape) { m_showTitleShape = shape; }
void Compositor::setShowTitleColors(const QColor& bg, const QColor& text, const QColor& border) { m_showTitleBgColor = bg; m_showTitleTextColor = text; m_showTitleBorderColor = border; }
void Compositor::setShowTitleFontSize(int size) { m_showTitleFontSize = qBound(10, size, 28); }
void Compositor::setShowTitleEntryAnim(const QString& anim) { m_showTitleEntryAnimType = anim; }
void Compositor::setShowTitleLoopAnim(const QString& anim) { m_showTitleLoopAnimType = anim; }

// Broadcast overlay layer setters
void Compositor::setChannelLogoPosition(const QString& pos) { m_logoPosition = pos; }
void Compositor::setChannelLogoSize(int heightPx) { m_logoSizeH = qBound(20, heightPx, 300); }
void Compositor::setShowChannelNameText(bool show) { m_showNameText = show; }
void Compositor::setKeepLogoDuringAds(bool keep) { m_keepLogoDuringAds = keep; }
void Compositor::setShowTitle(const QString& title, const QString& subtitle) { m_showTitleText = title; m_showSubtitleText = subtitle; }
void Compositor::setShowTitleVisible(bool visible) { m_showTitleVisible = visible; }
void Compositor::setShowTitlePosition(const QString& pos) { m_showTitlePosition = pos; }
void Compositor::setTalentNameplateVisible(bool visible) { m_talentNameplateVisible = visible; }
void Compositor::setBypassActive(bool active) { m_bypassActive = active; }

// Subtitle setters
void Compositor::setSubtitleText(const QString& text)
{
    if (m_subtitleText != text) {
        m_prevSubtitleText = m_subtitleText;
        m_subtitleText = text;
        m_subtitleFadeProgress = 0.0;
    }
}
void Compositor::setSubtitleVisible(bool v) { m_subtitleVisible = v; }
void Compositor::setSubtitleFontSize(int s) { m_subtitleFontSize = qBound(10, s, 60); }

void Compositor::setTeamLogoA(const QString& path) {
    if (m_teamLogoPathA == path) return;
    m_teamLogoPathA = path;
    m_teamLogoImgA = path.isEmpty() ? QImage() : QImage(path);
    if (!m_teamLogoImgA.isNull()) qInfo() << "[Compositor] Team A logo loaded:" << path;
}
void Compositor::setTeamLogoB(const QString& path) {
    if (m_teamLogoPathB == path) return;
    m_teamLogoPathB = path;
    m_teamLogoImgB = path.isEmpty() ? QImage() : QImage(path);
    if (!m_teamLogoImgB.isNull()) qInfo() << "[Compositor] Team B logo loaded:" << path;
}
void Compositor::setGoalAnim(bool active, const QString& text, const QString& team,
                              const QString& player, const QString& effect, int duration) {
    if (active && !m_goalAnimActive) m_goalAnimFrame = 0; // Reset on new goal
    m_goalAnimActive = active;
    m_goalAnimText = text;
    m_goalAnimTeam = team;
    m_goalAnimPlayer = player;
    m_goalAnimEffect = effect;
    m_goalAnimDuration = duration;
}
void Compositor::setSportEvent(const QString& event) {
    if (!event.isEmpty()) m_sportEventFrame = 0; // Reset on new event
    m_sportEvent = event;
}
void Compositor::setSubtitlePosition(const QString& p) { m_subtitlePosition = p; }
void Compositor::setSubtitleBgOpacity(double o) { m_subtitleBgOpacity = qBound(0.0, o, 1.0); }
void Compositor::setSubtitleTextColor(const QColor& c) { m_subtitleTextColor = c; }

void Compositor::updateAnimations(const QList<TalentOverlay>& talents)
{
    // Mark all current talents
    std::map<QString, bool> seen;
    for (const auto& t : talents) {
        if (!t.showOverlay) continue;
        seen[t.id] = true;

        auto& state = m_animStates[t.id];
        state.id = t.id;
        state.targetProg = 1.0;

        if (!state.wasVisible) {
            state.wasVisible = true;
            state.progress = 0.0; // Start animation from zero
        }
    }

    // Mark unseen talents for exit
    for (auto& [id, state] : m_animStates) {
        if (seen.find(id) == seen.end()) {
            state.targetProg = 0.0;
        }
    }

    // Advance all animations — director-configurable duration
    double enterStep = (1.0 / m_enterFrames) * m_animSpeed;
    double exitStep  = (1.0 / m_exitFrames)  * m_animSpeed;

    for (auto it = m_animStates.begin(); it != m_animStates.end(); ) {
        auto& state = it->second;

        if (state.progress < state.targetProg) {
            state.progress = std::min(1.0, state.progress + enterStep);
        } else if (state.progress > state.targetProg) {
            state.progress = std::max(0.0, state.progress - exitStep);
        }

        // Remove fully exited overlays
        if (state.progress <= 0.0 && state.targetProg <= 0.0) {
            state.wasVisible = false;
            it = m_animStates.erase(it);
        } else {
            ++it;
        }
    }
}

QImage Compositor::composite(const QImage& videoFrame, const QList<TalentOverlay>& talents)
{
    if (videoFrame.isNull()) return {};
    m_perfTimer.start();

    // Advance animation states
    updateAnimations(talents);

    // ── GPU PIPELINE ─────────────────────────────────────────
    // Step 1: Virtual Studio (chroma key + studio background) — GPU
    QImage processedFrame = m_virtualStudio.process(videoFrame);
    QImage output = processedFrame.copy();

    // Step 2: All overlays rendered on overlay layer (transparent)
    // QPainter rasterizes text/shapes → GPU composites + applies effects
    // This is the same pipeline as After Effects:
    //   Rasterize layers (CPU) → Composite (GPU) → Effects (GPU)
    QImage overlayLayer(output.size(), QImage::Format_ARGB32_Premultiplied);
    overlayLayer.fill(Qt::transparent);
    QPainter painter(&overlayLayer);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    // Draw currently visible + animating-out overlays (gated by talent nameplate visibility)
    if (m_talentNameplateVisible && !m_bypassActive) {
        for (auto& [id, state] : m_animStates) {
            if (state.progress <= 0.001) continue;

            // Find the talent data
            const TalentOverlay* talent = nullptr;
            for (const auto& t : talents) {
                if (t.id == id) { talent = &t; break; }
            }
            if (!talent) continue;

            // Apply easing: OutCubic for enter (fast start, slow end)
            //                InOutCubic for exit (smooth both ways)
            double easedProg;
            if (state.targetProg > 0.5) {
                easedProg = easeOutCubic(state.progress);  // Entering
            } else {
                easedProg = state.progress * state.progress;  // Exiting: quadratic falloff
            }

            // Draw with animation progress
            QString s = (talent->overlayStyle.isEmpty() || talent->overlayStyle == "default") ? m_styleId : talent->overlayStyle;
            auto it = m_styles.find(s);
            if (it != m_styles.end()) {
                painter.save();
                painter.setOpacity(easedProg);  // Global opacity animation
                it->second(painter, *talent, output.size(), easedProg);

                // Broadcast lower third effects (drawn around the nameplate)
                double scale = output.width() / 1920.0;
                QRectF plateRect = calcPlate(*talent, output.size(),
                    output.width() * 0.198, 64);
                if (m_animTypeStr == "line_draw")
                    fx::lineDraw(painter, plateRect, easedProg, m_accentColor, scale);
                else if (m_animTypeStr == "bar_slide")
                    fx::barSlide(painter, plateRect, easedProg, m_accentColor, scale);
                else if (m_animTypeStr == "shape_morph")
                    fx::shapeMorph(painter, plateRect, easedProg, m_accentColor, scale);
                else if (m_animTypeStr == "split_reveal")
                    fx::splitReveal(painter, plateRect, easedProg, m_accentColor, scale);
                else if (m_animTypeStr == "bracket_expand")
                    fx::bracketExpand(painter, plateRect, easedProg, m_accentColor, scale);
                else if (m_animTypeStr == "underline_grow")
                    fx::underlineGrow(painter, plateRect, easedProg, m_accentColor, scale);
                else if (m_animTypeStr == "box_wipe")
                    fx::boxWipe(painter, plateRect, easedProg, m_accentColor, scale);
                else if (m_animTypeStr == "corner_build")
                    fx::cornerBuild(painter, plateRect, easedProg, m_accentColor, scale);

                painter.restore();
            }
        }
    }

    // ── Persistent broadcast overlays ─────────────────────

    int fw = output.width();
    int fh = output.height();
    double scale = fw / 1920.0;

    // ── Sport: Team logos in scoreboard ─────────────────────
    // (rendered inside the scoreboard section below)

    // ── Sport: GOAL Animation (full-screen overlay) ──────
    if (m_goalAnimActive && m_goalAnimFrame < m_goalAnimDuration * 25) {
        m_goalAnimFrame++;
        double totalFrames = m_goalAnimDuration * 25.0;
        double prog = m_goalAnimFrame / totalFrames;
        double entryProg = std::min(1.0, m_goalAnimFrame / 20.0); // 20 frames entry
        double exitProg = std::max(0.0, (m_goalAnimFrame - totalFrames + 15) / 15.0); // 15 frames exit

        // Semi-transparent overlay
        painter.save();
        double overlayAlpha = (entryProg - exitProg) * 0.7;
        painter.fillRect(0, 0, fw, fh, QColor(0, 0, 0, static_cast<int>(overlayAlpha * 255)));

        // Team color burst from center
        QColor teamColor = (m_goalAnimTeam == "a") ? m_sbColorA : m_sbColorB;
        QRadialGradient burst(fw / 2, fh / 2, fw * 0.6 * easeOutCubic(entryProg));
        burst.setColorAt(0, QColor(teamColor.red(), teamColor.green(), teamColor.blue(), static_cast<int>(100 * (1.0 - exitProg))));
        burst.setColorAt(1, QColor(0, 0, 0, 0));
        painter.setPen(Qt::NoPen);
        painter.setBrush(burst);
        painter.drawRect(0, 0, fw, fh);

        // GOAL text (centered, large)
        double textScale = easeOutCubic(entryProg) * (1.0 - exitProg * 0.3);
        int goalFontSize = qMax(40, static_cast<int>(100 * scale * textScale));
        QFont goalFont("Helvetica Neue", goalFontSize, QFont::Black);
        painter.setFont(goalFont);

        // Apply text effect
        QRectF textRect(0, fh * 0.3, fw, fh * 0.2);
        if (m_goalAnimEffect == "kinetic_pop") {
            fx::kineticPop(painter, m_goalAnimText, textRect, entryProg, goalFont, Qt::white);
        } else if (m_goalAnimEffect == "bounce_in") {
            fx::bounceIn(painter, m_goalAnimText, textRect, entryProg, goalFont, Qt::white);
        } else if (m_goalAnimEffect == "wave_text") {
            fx::waveText(painter, m_goalAnimText, textRect, entryProg, goalFont, Qt::white);
        } else if (m_goalAnimEffect == "scale_up_letter") {
            fx::scaleUpPerLetter(painter, m_goalAnimText, textRect, entryProg, goalFont, Qt::white);
        } else {
            // Default: simple scale + fade
            painter.setPen(Qt::white);
            painter.setOpacity(entryProg * (1.0 - exitProg));
            painter.drawText(textRect, Qt::AlignHCenter | Qt::AlignVCenter, m_goalAnimText);
        }

        // Player name (below goal text)
        if (!m_goalAnimPlayer.isEmpty()) {
            int playerFontSize = qMax(18, static_cast<int>(36 * scale * textScale));
            QFont playerFont("Helvetica Neue", playerFontSize, QFont::Bold);
            painter.setFont(playerFont);
            painter.setPen(QColor(teamColor.red(), teamColor.green(), teamColor.blue(), static_cast<int>(255 * entryProg * (1.0 - exitProg))));
            painter.setOpacity(entryProg * (1.0 - exitProg));
            painter.drawText(QRectF(0, fh * 0.5, fw, fh * 0.1), Qt::AlignHCenter | Qt::AlignVCenter, m_goalAnimPlayer);
        }

        // Team logo (if available)
        const QImage& goalLogo = (m_goalAnimTeam == "a") ? m_teamLogoImgA : m_teamLogoImgB;
        if (!goalLogo.isNull()) {
            int logoH = qMax(50, static_cast<int>(120 * scale * textScale));
            QImage scaledLogo = goalLogo.scaledToHeight(logoH, Qt::SmoothTransformation);
            int logoX = (fw - scaledLogo.width()) / 2;
            int logoY = static_cast<int>(fh * 0.15);
            painter.setOpacity(entryProg * (1.0 - exitProg));
            painter.drawImage(logoX, logoY, scaledLogo);
        }

        // Particles
        fx::sparkles(painter, QRectF(0, 0, fw, fh), 40, m_goalAnimFrame * 0.06, teamColor);

        painter.restore();
    } else if (m_goalAnimActive && m_goalAnimFrame >= m_goalAnimDuration * 25) {
        m_goalAnimActive = false;
        m_goalAnimFrame = 0;
    }

    // ── Sport: Event overlay (card, halftime, etc.) ──────
    if (!m_sportEvent.isEmpty() && m_sportEventFrame < 100) { // ~4 sec at 25fps
        m_sportEventFrame++;
        double prog = std::min(1.0, m_sportEventFrame / 15.0);
        double fade = (m_sportEventFrame > 85) ? (100 - m_sportEventFrame) / 15.0 : 1.0;

        QColor eventColor;
        QString eventIcon;
        if (m_sportEvent == "yellow_card") { eventColor = QColor(255, 200, 0); eventIcon = "\xF0\x9F\x9F\xA8"; }
        else if (m_sportEvent == "red_card") { eventColor = QColor(220, 0, 0); eventIcon = "\xF0\x9F\x9F\xA5"; }
        else if (m_sportEvent == "halftime") { eventColor = QColor(100, 100, 120); eventIcon = "HT"; }
        else if (m_sportEvent == "substitution") { eventColor = QColor(0, 150, 200); eventIcon = "\xE2\x86\x94"; }
        else if (m_sportEvent == "penalty") { eventColor = QColor(200, 0, 0); eventIcon = "PEN"; }
        else if (m_sportEvent == "corner") { eventColor = QColor(100, 100, 100); eventIcon = "CK"; }
        else if (m_sportEvent == "full_time") { eventColor = QColor(200, 200, 200); eventIcon = "FT"; }
        else if (m_sportEvent == "var") { eventColor = QColor(0, 100, 200); eventIcon = "VAR"; }
        else { eventColor = m_accentColor; eventIcon = m_sportEvent; }

        int evW = static_cast<int>(280 * scale * easeOutCubic(prog));
        int evH = static_cast<int>(56 * scale);
        int evX = (fw - evW) / 2;
        int evY = static_cast<int>(fh * 0.12);

        painter.save();
        painter.setOpacity(fade);
        // Background pill
        drawGlassRect(painter, QRectF(evX, evY, evW, evH), evH / 2.0, eventColor, 0.75);
        // Icon + text
        QFont evFont("Helvetica Neue", qMax(14, static_cast<int>(20 * scale)), QFont::Bold);
        painter.setFont(evFont);
        painter.setPen(Qt::white);
        painter.drawText(QRectF(evX, evY, evW, evH), Qt::AlignCenter, eventIcon);
        painter.restore();
    }
    if (!m_sportEvent.isEmpty() && m_sportEventFrame >= 100) {
        m_sportEvent.clear();
        m_sportEventFrame = 0;
    }

    // Legacy Lower Third — REMOVED
    // Replaced by Show Title system (setShowTitle/setShowTitleVisible)
    // and Talent Nameplate system (20 styles with IA detection)

    // Ticker (scrolling text at very bottom)
    if (m_tickerVisible && !m_tickerText.isEmpty() && !m_bypassActive) {
        int tkH = static_cast<int>(36 * scale);
        int tkOffY = static_cast<int>(m_tickerOffY * scale);
        int tkY = fh - tkH - tkOffY;
        QRectF tkRect(0, tkY, fw, tkH);

        // Design Template: render ticker background layers
        double timeSec = m_loopFrame / 25.0;
        DesignRegistry::instance().render(painter, "ticker", m_tkDesign, tkRect, timeSec, scale, m_accentColor);

        painter.setPen(Qt::NoPen);
        painter.setBrush(m_tickerBgColor);
        painter.drawRect(tkRect);

        QFont tkF("Helvetica Neue", static_cast<int>(m_tickerFontSize * scale), QFont::Bold);
        painter.setFont(tkF);
        painter.setPen(m_tickerTextColor);

        // Scrolling: offset advances each frame (speed configurable)
        int scrollPx = qMax(1, m_tickerSpeed);
        int textW = QFontMetrics(tkF).horizontalAdvance(m_tickerText);
        if (m_layoutRtl || m_tickerText.isRightToLeft()) {
            m_tickerOffset += scrollPx;
            if (m_tickerOffset > fw) m_tickerOffset = -textW;
        } else {
            m_tickerOffset -= scrollPx;
            if (m_tickerOffset < -textW) m_tickerOffset = fw;
        }
        painter.drawText(m_tickerOffset, tkY + static_cast<int>(24 * scale), m_tickerText);
    }

    // Subtitles (above ticker or at position)
    if (m_subtitleVisible && !m_subtitleText.isEmpty() && !m_bypassActive) {
        // Fade in new text
        if (m_subtitleFadeProgress < 1.0)
            m_subtitleFadeProgress = qMin(1.0, m_subtitleFadeProgress + 0.1);

        QFont subFont("Helvetica Neue", static_cast<int>(m_subtitleFontSize * scale), QFont::DemiBold);
        painter.setFont(subFont);
        QFontMetrics subFm(subFont);

        int margin = static_cast<int>(40 * scale);
        int maxTextW = fw - margin * 2;
        QRect textBounds = subFm.boundingRect(QRect(0, 0, maxTextW, 0),
                                               Qt::AlignHCenter | Qt::TextWordWrap,
                                               m_subtitleText);

        int padX = static_cast<int>(20 * scale);
        int padY = static_cast<int>(10 * scale);
        int subW = textBounds.width() + padX * 2;
        int subH = textBounds.height() + padY * 2;
        int subX = (fw - subW) / 2;
        int subY;

        int subOffXPx = static_cast<int>(m_subtitleOffX * scale);
        int subOffYPx = static_cast<int>(m_subtitleOffY * scale);
        subX += subOffXPx;
        if (m_subtitlePosition == "top") {
            subY = static_cast<int>(60 * scale) + subOffYPx;
        } else {
            // Bottom — above ticker if ticker is visible
            int tickerH = (m_tickerVisible && !m_tickerText.isEmpty()) ? static_cast<int>(36 * scale) : 0;
            subY = fh - tickerH - subH - static_cast<int>(16 * scale) + subOffYPx;
        }

        QRectF subRect(subX, subY, subW, subH);

        // Semi-transparent background with rounded corners
        painter.setOpacity(m_subtitleFadeProgress);
        drawGlassRect(painter, subRect, static_cast<int>(8 * scale),
                      QColor(0, 0, 0), m_subtitleBgOpacity);

        // Text (RTL uses right alignment)
        painter.setPen(m_subtitleTextColor);
        int subAlign = (m_layoutRtl || m_subtitleText.isRightToLeft())
                           ? (Qt::AlignRight | Qt::TextWordWrap)
                           : (Qt::AlignHCenter | Qt::TextWordWrap);
        painter.drawText(QRectF(subX + padX, subY + padY, textBounds.width(), textBounds.height()),
                         subAlign, m_subtitleText);
        painter.setOpacity(1.0);
    }

    // ── Branding animation state machine ────────────────────
    m_loopFrame++;

    // Advance entry animations
    if (m_logoEntryProgress < 1.0)
        m_logoEntryProgress = qMin(1.0, m_logoEntryProgress + 0.03);
    if (m_nameEntryProgress < 1.0)
        m_nameEntryProgress = qMin(1.0, m_nameEntryProgress + 0.03);

    double loopPhase = std::sin(m_loopFrame * 0.05);

    // Logo — supports animated frames, configurable position, bypass logic, branding anims
    if (m_logoVisible && !m_logoFrames.isEmpty() && !(m_bypassActive && !m_keepLogoDuringAds)) {
        const QImage& currentFrame = m_logoFrames[m_logoFrameIndex];
        if (!currentFrame.isNull()) {
            int logoH = static_cast<int>(m_logoSizeH * scale);
            QImage scaledLogo = currentFrame.scaledToHeight(logoH, Qt::SmoothTransformation);
            int pad = static_cast<int>(16 * scale);
            int logoX, logoY;

            if (m_logoPosition == "top_left") {
                logoX = pad; logoY = pad;
            } else if (m_logoPosition == "bottom_left") {
                logoX = pad; logoY = fh - scaledLogo.height() - pad;
            } else if (m_logoPosition == "bottom_right") {
                logoX = fw - scaledLogo.width() - pad; logoY = fh - scaledLogo.height() - pad;
            } else { // top_right (default)
                logoX = fw - scaledLogo.width() - pad; logoY = pad;
            }

            // Apply logo entry animation
            double logoOpacity = 0.9;
            int logoOffX = 0, logoOffY = 0;
            double logoScale = 1.0;
            if (m_logoEntryProgress < 1.0) {
                double p = m_logoEntryProgress;
                if (m_logoEntryAnimType == "fade") {
                    logoOpacity = p * 0.9;
                } else if (m_logoEntryAnimType == "slide_left") {
                    logoOffX = static_cast<int>((1.0 - p) * -200 * scale);
                } else if (m_logoEntryAnimType == "slide_right") {
                    logoOffX = static_cast<int>((1.0 - p) * 200 * scale);
                } else if (m_logoEntryAnimType == "slide_up") {
                    logoOffY = static_cast<int>((1.0 - p) * 100 * scale);
                } else if (m_logoEntryAnimType == "slide_down") {
                    logoOffY = static_cast<int>((1.0 - p) * -100 * scale);
                } else if (m_logoEntryAnimType == "scale") {
                    logoScale = 0.5 + p * 0.5;
                    logoOpacity = p * 0.9;
                } else if (m_logoEntryAnimType == "wipe") {
                    // Handled via clip rect below
                }
                // Broadcast effects (logo reveals)
                else if (m_logoEntryAnimType == "fade_glow" || m_logoEntryAnimType == "light_streak" ||
                         m_logoEntryAnimType == "particle_form" || m_logoEntryAnimType == "pulse_reveal") {
                    logoOpacity = p * 0.9;
                    logoScale = 0.9 + p * 0.1;
                } else if (m_logoEntryAnimType == "scale_bounce") {
                    logoScale = easeOutCubic(p) * 1.0;
                    logoOpacity = p;
                } else if (m_logoEntryAnimType == "shatter_in") {
                    logoOpacity = p;
                } else if (m_logoEntryAnimType == "blur_zoom") {
                    logoScale = 1.5 - 0.5 * easeOutCubic(p);
                    logoOpacity = p;
                } else if (m_logoEntryAnimType == "rotate_3d") {
                    logoOpacity = p;
                }
            }

            // Apply logo loop animation (only after entry complete)
            if (m_logoEntryProgress >= 1.0 && m_logoLoopAnimType != "none") {
                if (m_logoLoopAnimType == "pulse") {
                    logoScale = 1.0 + loopPhase * 0.03;
                } else if (m_logoLoopAnimType == "bounce") {
                    logoOffY = static_cast<int>(std::abs(loopPhase) * 3.0 * scale);
                } else if (m_logoLoopAnimType == "rotate") {
                    // Handled via painter rotation below
                }
                // Broadcast loop effects
                else if (m_logoLoopAnimType == "neon_glow" || m_logoLoopAnimType == "shimmer" ||
                         m_logoLoopAnimType == "edge_glow" || m_logoLoopAnimType == "bloom") {
                    // These are drawn as post-effects after the logo is rendered
                }
                else if (m_logoLoopAnimType == "sparkles" || m_logoLoopAnimType == "bokeh" ||
                         m_logoLoopAnimType == "rising_particles") {
                    // Particle effects drawn after logo
                }
            }

            painter.save();
            painter.setOpacity(logoOpacity);
            int drawX = logoX + logoOffX + static_cast<int>(m_logoOffX * scale);
            int drawY = logoY + logoOffY + static_cast<int>(m_logoOffY * scale);

            if (logoScale != 1.0) {
                double cx = drawX + scaledLogo.width() / 2.0;
                double cy = drawY + scaledLogo.height() / 2.0;
                painter.translate(cx, cy);
                painter.scale(logoScale, logoScale);
                painter.translate(-cx, -cy);
            }

            if (m_logoEntryProgress >= 1.0 && m_logoLoopAnimType == "rotate") {
                double cx = drawX + scaledLogo.width() / 2.0;
                double cy = drawY + scaledLogo.height() / 2.0;
                painter.translate(cx, cy);
                painter.rotate(loopPhase * 2.0);
                painter.translate(-cx, -cy);
            }

            // Glow loop effect: draw glow layer behind logo
            if (m_logoEntryProgress >= 1.0 && m_logoLoopAnimType == "glow") {
                double glowOp = 0.3 + loopPhase * 0.15;
                painter.setOpacity(glowOp);
                QRectF glowRect(drawX - 4 * scale, drawY - 4 * scale,
                                scaledLogo.width() + 8 * scale, scaledLogo.height() + 8 * scale);
                painter.setPen(Qt::NoPen);
                painter.setBrush(QColor(255, 255, 255, 80));
                painter.drawRoundedRect(glowRect, 6 * scale, 6 * scale);
                painter.setOpacity(logoOpacity);
            }

            // Wipe entry: clip rect
            if (m_logoEntryProgress < 1.0 && m_logoEntryAnimType == "wipe") {
                painter.setClipRect(QRectF(drawX, drawY,
                    scaledLogo.width() * m_logoEntryProgress, scaledLogo.height()));
            }

            painter.drawImage(drawX, drawY, scaledLogo);

            // Shimmer loop effect: diagonal highlight sweeping across
            if (m_logoEntryProgress >= 1.0 && m_logoLoopAnimType == "shimmer") {
                double shimmerPos = (m_loopFrame % 120) / 120.0;
                double sx = drawX + (scaledLogo.width() + 20 * scale) * shimmerPos - 10 * scale;
                QLinearGradient shimGrad(sx, drawY, sx + 10 * scale, drawY + scaledLogo.height());
                shimGrad.setColorAt(0.0, QColor(255, 255, 255, 0));
                shimGrad.setColorAt(0.5, QColor(255, 255, 255, 60));
                shimGrad.setColorAt(1.0, QColor(255, 255, 255, 0));
                painter.setPen(Qt::NoPen);
                painter.setBrush(shimGrad);
                painter.setClipRect(QRectF(drawX, drawY, scaledLogo.width(), scaledLogo.height()));
                painter.drawRect(QRectF(drawX, drawY, scaledLogo.width(), scaledLogo.height()));
            }

            // Broadcast entry effects (drawn over logo during entry)
            if (m_logoEntryProgress < 1.0) {
                QRectF logoRect(drawX, drawY, scaledLogo.width(), scaledLogo.height());
                if (m_logoEntryAnimType == "fade_glow")
                    fx::fadeGlow(painter, logoRect, m_logoEntryProgress, m_accentColor);
                else if (m_logoEntryAnimType == "light_streak")
                    fx::lightStreak(painter, logoRect, m_logoEntryProgress, m_accentColor);
                else if (m_logoEntryAnimType == "particle_form")
                    fx::particleForm(painter, logoRect, m_logoEntryProgress, m_accentColor, m_loopFrame);
                else if (m_logoEntryAnimType == "pulse_reveal")
                    fx::pulseReveal(painter, logoRect, m_logoEntryProgress, m_accentColor);
                else if (m_logoEntryAnimType == "shatter_in")
                    fx::shatterIn(painter, logoRect, m_logoEntryProgress);
            }

            // Broadcast loop effects (drawn over logo continuously)
            if (m_logoEntryProgress >= 1.0 && m_logoLoopAnimType != "none") {
                QRectF logoRect(drawX, drawY, scaledLogo.width(), scaledLogo.height());
                if (m_logoLoopAnimType == "neon_glow")
                    fx::neonGlow(painter, logoRect, m_accentColor, 0.6 + loopPhase * 0.3);
                else if (m_logoLoopAnimType == "edge_glow")
                    fx::edgeGlow(painter, logoRect, m_accentColor, 4 + loopPhase * 2);
                else if (m_logoLoopAnimType == "bloom")
                    fx::bloom(painter, logoRect, m_accentColor, 0.5 + loopPhase * 0.3);
                else if (m_logoLoopAnimType == "sparkles")
                    fx::sparkles(painter, logoRect, 15, m_loopFrame * 0.05, m_accentColor);
                else if (m_logoLoopAnimType == "bokeh")
                    fx::bokeh(painter, logoRect.adjusted(-20, -20, 20, 20), 8, m_loopFrame * 0.03, m_accentColor);
                else if (m_logoLoopAnimType == "rising_particles")
                    fx::risingParticles(painter, logoRect.adjusted(-10, -20, 10, 0), 12, m_loopFrame * 0.04, m_accentColor);
                else if (m_logoLoopAnimType == "light_rays")
                    fx::lightRays(painter, logoRect, m_loopFrame * 0.5, m_accentColor, 0.4 + loopPhase * 0.2);
            }

            painter.restore();

            // Channel name text next to logo — with entry/loop animations and shape
            if (m_showNameText && !m_channelName.isEmpty()) {
                QFont nameF("Helvetica Neue", static_cast<int>(m_nameFontSize * scale), QFont::Bold);
                nameF.setLetterSpacing(QFont::AbsoluteSpacing, 1);
                painter.setFont(nameF);
                QFontMetrics nameFm(nameF);
                int textW = nameFm.horizontalAdvance(m_channelName);
                int textH = nameFm.height();
                int padH = static_cast<int>(8 * scale);
                int padV = static_cast<int>(4 * scale);
                int nameBaseX = logoX;
                int nameBaseY = logoY + scaledLogo.height() + static_cast<int>(4 * scale);

                // Name entry animation
                double nameOpacity = 1.0;
                int nameOffX = 0, nameOffY = 0;
                double nameScale = 1.0;
                if (m_nameEntryProgress < 1.0) {
                    double p = m_nameEntryProgress;
                    if (m_nameEntryAnimType == "fade") {
                        nameOpacity = p;
                    } else if (m_nameEntryAnimType == "slide_left") {
                        nameOffX = static_cast<int>((1.0 - p) * -200 * scale);
                    } else if (m_nameEntryAnimType == "slide_right") {
                        nameOffX = static_cast<int>((1.0 - p) * 200 * scale);
                    } else if (m_nameEntryAnimType == "slide_up") {
                        nameOffY = static_cast<int>((1.0 - p) * 100 * scale);
                    } else if (m_nameEntryAnimType == "slide_down") {
                        nameOffY = static_cast<int>((1.0 - p) * -100 * scale);
                    } else if (m_nameEntryAnimType == "scale") {
                        nameScale = 0.5 + p * 0.5;
                        nameOpacity = p;
                    } else if (m_nameEntryAnimType == "wipe") {
                        // Handled via clip rect
                    }
                    // Broadcast text entry effects
                    else if (m_nameEntryAnimType == "typewriter" || m_nameEntryAnimType == "bounce_in" ||
                             m_nameEntryAnimType == "wave_text" || m_nameEntryAnimType == "kinetic_pop" ||
                             m_nameEntryAnimType == "tracking_expand" || m_nameEntryAnimType == "fade_up_letter" ||
                             m_nameEntryAnimType == "scale_up_letter" || m_nameEntryAnimType == "blur_in" ||
                             m_nameEntryAnimType == "slide_per_letter" || m_nameEntryAnimType == "rotate_in_letter") {
                        nameOpacity = 1.0; // Text effects handle their own opacity per-letter
                    }
                }

                // Name loop animation (after entry complete)
                if (m_nameEntryProgress >= 1.0 && m_nameLoopAnimType != "none") {
                    if (m_nameLoopAnimType == "pulse") {
                        nameScale = 1.0 + loopPhase * 0.03;
                    } else if (m_nameLoopAnimType == "bounce") {
                        nameOffY = static_cast<int>(std::abs(loopPhase) * 3.0 * scale);
                    }
                }

                int drawNX = nameBaseX + nameOffX + static_cast<int>(m_nameOffX * scale);
                int drawNY = nameBaseY + nameOffY + static_cast<int>(m_nameOffY * scale);

                painter.save();
                painter.setOpacity(nameOpacity);

                if (nameScale != 1.0) {
                    double cx = drawNX + textW / 2.0;
                    double cy = drawNY + textH / 2.0;
                    painter.translate(cx, cy);
                    painter.scale(nameScale, nameScale);
                    painter.translate(-cx, -cy);
                }

                // Wipe entry clip
                if (m_nameEntryProgress < 1.0 && m_nameEntryAnimType == "wipe") {
                    painter.setClipRect(QRectF(drawNX, drawNY,
                        (textW + padH * 2) * m_nameEntryProgress, textH + padV * 2));
                }

                // Draw name shape background
                QRectF nameRect(drawNX, drawNY, textW + padH * 2, textH + padV * 2);

                if (m_nameShape == "rectangle") {
                    drawGlassRect(painter, nameRect, 2 * scale, m_nameBgColor, 0.85);
                    painter.setPen(QPen(m_nameBorderColor, 1));
                    painter.setBrush(Qt::NoBrush);
                    painter.drawRoundedRect(nameRect, 2 * scale, 2 * scale);
                } else if (m_nameShape == "square") {
                    double side = qMax(nameRect.width(), nameRect.height());
                    QRectF sqRect(drawNX, drawNY, side, side);
                    drawGlassRect(painter, sqRect, 2 * scale, m_nameBgColor, 0.85);
                    painter.setPen(QPen(m_nameBorderColor, 1));
                    painter.setBrush(Qt::NoBrush);
                    painter.drawRoundedRect(sqRect, 2 * scale, 2 * scale);
                    nameRect = sqRect;
                } else if (m_nameShape == "pill") {
                    double pillR = nameRect.height() / 2.0;
                    drawGlassRect(painter, nameRect, pillR, m_nameBgColor, 0.85);
                    painter.setPen(QPen(m_nameBorderColor, 1));
                    painter.setBrush(Qt::NoBrush);
                    painter.drawRoundedRect(nameRect, pillR, pillR);
                } else if (m_nameShape == "angled") {
                    double skew = 8 * scale;
                    QPainterPath angledPath;
                    angledPath.moveTo(drawNX + skew, drawNY);
                    angledPath.lineTo(drawNX + nameRect.width(), drawNY);
                    angledPath.lineTo(drawNX + nameRect.width() - skew, drawNY + nameRect.height());
                    angledPath.lineTo(drawNX, drawNY + nameRect.height());
                    angledPath.closeSubpath();
                    painter.setPen(QPen(m_nameBorderColor, 1));
                    painter.setBrush(m_nameBgColor);
                    painter.drawPath(angledPath);
                } else { // "frameless"
                    // Drop shadow only
                    painter.setPen(Qt::NoPen);
                    painter.setBrush(QColor(0, 0, 0, 120));
                    painter.drawRoundedRect(nameRect.adjusted(2 * scale, 2 * scale, 2 * scale, 2 * scale), 2 * scale, 2 * scale);
                }

                // Glow loop for name
                if (m_nameEntryProgress >= 1.0 && m_nameLoopAnimType == "glow") {
                    double glowOp = 0.3 + loopPhase * 0.15;
                    painter.setOpacity(glowOp);
                    QRectF glowRect = nameRect.adjusted(-3 * scale, -3 * scale, 3 * scale, 3 * scale);
                    painter.setPen(Qt::NoPen);
                    painter.setBrush(QColor(255, 255, 255, 60));
                    painter.drawRoundedRect(glowRect, 4 * scale, 4 * scale);
                    painter.setOpacity(nameOpacity);
                }

                // Shimmer loop for name
                if (m_nameEntryProgress >= 1.0 && m_nameLoopAnimType == "shimmer") {
                    double shimmerPos = (m_loopFrame % 120) / 120.0;
                    double sx = nameRect.x() + (nameRect.width() + 20 * scale) * shimmerPos - 10 * scale;
                    QLinearGradient shimGrad(sx, nameRect.y(), sx + 10 * scale, nameRect.y() + nameRect.height());
                    shimGrad.setColorAt(0.0, QColor(255, 255, 255, 0));
                    shimGrad.setColorAt(0.5, QColor(255, 255, 255, 50));
                    shimGrad.setColorAt(1.0, QColor(255, 255, 255, 0));
                    painter.setPen(Qt::NoPen);
                    painter.setBrush(shimGrad);
                    painter.setClipRect(nameRect);
                    painter.drawRect(nameRect);
                    painter.setClipping(false);
                }

                // Draw text — use broadcast text effect if selected, otherwise standard
                if (m_nameEntryProgress < 1.0 && m_nameEntryAnimType == "typewriter") {
                    fx::typewriter(painter, m_channelName, nameRect, m_nameEntryProgress, nameF, m_nameTextColor);
                } else if (m_nameEntryProgress < 1.0 && m_nameEntryAnimType == "bounce_in") {
                    fx::bounceIn(painter, m_channelName, nameRect, m_nameEntryProgress, nameF, m_nameTextColor);
                } else if (m_nameEntryProgress < 1.0 && m_nameEntryAnimType == "wave_text") {
                    fx::waveText(painter, m_channelName, nameRect, m_nameEntryProgress, nameF, m_nameTextColor);
                } else if (m_nameEntryProgress < 1.0 && m_nameEntryAnimType == "kinetic_pop") {
                    fx::kineticPop(painter, m_channelName, nameRect, m_nameEntryProgress, nameF, m_nameTextColor);
                } else if (m_nameEntryProgress < 1.0 && m_nameEntryAnimType == "tracking_expand") {
                    fx::trackingExpand(painter, m_channelName, nameRect, m_nameEntryProgress, nameF, m_nameTextColor);
                } else if (m_nameEntryProgress < 1.0 && m_nameEntryAnimType == "fade_up_letter") {
                    fx::fadeUpPerLetter(painter, m_channelName, nameRect, m_nameEntryProgress, nameF, m_nameTextColor);
                } else if (m_nameEntryProgress < 1.0 && m_nameEntryAnimType == "scale_up_letter") {
                    fx::scaleUpPerLetter(painter, m_channelName, nameRect, m_nameEntryProgress, nameF, m_nameTextColor);
                } else if (m_nameEntryProgress < 1.0 && m_nameEntryAnimType == "blur_in") {
                    fx::blurIn(painter, m_channelName, nameRect, m_nameEntryProgress, nameF, m_nameTextColor);
                } else if (m_nameEntryProgress < 1.0 && m_nameEntryAnimType == "slide_per_letter") {
                    fx::slidePerLetter(painter, m_channelName, nameRect, m_nameEntryProgress, nameF, m_nameTextColor);
                } else if (m_nameEntryProgress < 1.0 && m_nameEntryAnimType == "rotate_in_letter") {
                    fx::rotateInPerLetter(painter, m_channelName, nameRect, m_nameEntryProgress, nameF, m_nameTextColor);
                } else {
                    painter.setPen(m_nameTextColor);
                    painter.drawText(nameRect, Qt::AlignCenter, m_channelName);
                }

                // Broadcast loop effects on channel name
                if (m_nameEntryProgress >= 1.0 && m_nameLoopAnimType != "none") {
                    if (m_nameLoopAnimType == "neon_glow")
                        fx::neonGlow(painter, nameRect, m_accentColor, 0.5 + loopPhase * 0.3);
                    else if (m_nameLoopAnimType == "shimmer")
                        fx::shimmer(painter, nameRect, m_loopFrame * 0.04, m_accentColor);
                    else if (m_nameLoopAnimType == "edge_glow")
                        fx::edgeGlow(painter, nameRect, m_accentColor, 3 + loopPhase * 2);
                    else if (m_nameLoopAnimType == "glitch_rgb")
                        fx::glitchRGB(painter, nameRect, 0.3 + loopPhase * 0.2, m_loopFrame);
                }

                painter.restore();
            }
        }
        // Advance frame for animation (only if multiple frames)
        if (m_logoFrames.size() > 1) {
            m_logoFrameIndex = (m_logoFrameIndex + 1) % m_logoFrames.size();
        }
    }

    // Show Title (Layer 2) — permanent banner that yields to talent nameplate
    // Advance entry animation when visible
    if (m_showTitleVisible && !m_showTitleText.isEmpty() && !m_bypassActive) {
        if (m_showTitleEntryProgress < 1.0)
            m_showTitleEntryProgress = qMin(1.0, m_showTitleEntryProgress + 0.03);
        if (m_showTitleProgress < 1.0)
            m_showTitleProgress = qMin(1.0, m_showTitleProgress + 0.08);
    } else {
        if (m_showTitleProgress > 0.0)
            m_showTitleProgress = qMax(0.0, m_showTitleProgress - 0.08);
    }
    if (m_showTitleProgress > 0.01) {
        double prog = easeOutCubic(m_showTitleProgress);

        QFont titleF("Helvetica Neue", static_cast<int>(m_showTitleFontSize * scale), QFont::Bold);
        int subSize = qMax(10, m_showTitleFontSize - 4);
        QFont subF("Helvetica Neue", static_cast<int>(subSize * scale));
        QFontMetrics titleFm(titleF);
        QFontMetrics subFm(subF);

        int textW = titleFm.horizontalAdvance(m_showTitleText);
        if (!m_showSubtitleText.isEmpty())
            textW = qMax(textW, subFm.horizontalAdvance(m_showSubtitleText));
        int padX = static_cast<int>(16 * scale);
        int padY = static_cast<int>(10 * scale);
        int boxW = textW + padX * 2;
        int boxH = static_cast<int>(m_showSubtitleText.isEmpty() ? 40 * scale : 58 * scale);
        int boxX, boxY;
        int pad = static_cast<int>(16 * scale);

        if (m_showTitlePosition == "bottom_right") {
            boxX = fw - boxW - pad; boxY = fh - boxH - static_cast<int>(fh * 0.12);
        } else if (m_showTitlePosition == "top_left") {
            boxX = pad; boxY = static_cast<int>(fh * 0.08);
        } else if (m_showTitlePosition == "top_right") {
            boxX = fw - boxW - pad; boxY = static_cast<int>(fh * 0.08);
        } else { // bottom_left (default)
            boxX = pad; boxY = fh - boxH - static_cast<int>(fh * 0.12);
        }

        // Entry animation offset/opacity
        double titleOpacity = prog;
        int titleOffX = 0, titleOffY = 0;
        double titleScale = 1.0;
        if (m_showTitleEntryProgress < 1.0) {
            double p = m_showTitleEntryProgress;
            if (m_showTitleEntryAnimType == "fade") {
                titleOpacity = p * prog;
            } else if (m_showTitleEntryAnimType == "slide_left") {
                titleOffX = static_cast<int>((1.0 - p) * -200 * scale);
            } else if (m_showTitleEntryAnimType == "slide_right") {
                titleOffX = static_cast<int>((1.0 - p) * 200 * scale);
            } else if (m_showTitleEntryAnimType == "slide_up") {
                titleOffY = static_cast<int>((1.0 - p) * 100 * scale);
            } else if (m_showTitleEntryAnimType == "slide_down") {
                titleOffY = static_cast<int>((1.0 - p) * -100 * scale);
            } else if (m_showTitleEntryAnimType == "scale") {
                titleScale = 0.5 + p * 0.5;
                titleOpacity = p * prog;
            } else if (m_showTitleEntryAnimType == "wipe") {
                // Handled via clip rect
            }
        }

        // Loop animation (after entry complete)
        if (m_showTitleEntryProgress >= 1.0 && m_showTitleLoopAnimType != "none") {
            double stLoopPhase = std::sin(m_loopFrame * 0.05);
            if (m_showTitleLoopAnimType == "pulse") {
                titleScale = 1.0 + stLoopPhase * 0.03;
            } else if (m_showTitleLoopAnimType == "bounce") {
                titleOffY = static_cast<int>(std::abs(stLoopPhase) * 3.0 * scale);
            }
        }

        int drawBX = boxX + titleOffX + static_cast<int>(m_showTitleOffX * scale);
        int drawBY = boxY + titleOffY + static_cast<int>(m_showTitleOffY * scale);

        painter.save();
        painter.setOpacity(titleOpacity);

        if (titleScale != 1.0) {
            double cx = drawBX + boxW / 2.0;
            double cy = drawBY + boxH / 2.0;
            painter.translate(cx, cy);
            painter.scale(titleScale, titleScale);
            painter.translate(-cx, -cy);
        }

        // Wipe entry clip
        if (m_showTitleEntryProgress < 1.0 && m_showTitleEntryAnimType == "wipe") {
            painter.setClipRect(QRectF(drawBX, drawBY, boxW * m_showTitleEntryProgress, boxH));
        }

        QRectF boxRect(drawBX, drawBY, boxW, boxH);

        // Draw shape background (same logic as channel name)
        if (m_showTitleShape == "rectangle") {
            drawGlassRect(painter, boxRect, 4 * scale, m_showTitleBgColor, 0.85);
            painter.setPen(QPen(m_showTitleBorderColor, 1));
            painter.setBrush(Qt::NoBrush);
            painter.drawRoundedRect(boxRect, 4 * scale, 4 * scale);
        } else if (m_showTitleShape == "square") {
            double side = qMax(boxRect.width(), boxRect.height());
            QRectF sqRect(drawBX, drawBY, side, side);
            drawGlassRect(painter, sqRect, 4 * scale, m_showTitleBgColor, 0.85);
            painter.setPen(QPen(m_showTitleBorderColor, 1));
            painter.setBrush(Qt::NoBrush);
            painter.drawRoundedRect(sqRect, 4 * scale, 4 * scale);
            boxRect = sqRect;
        } else if (m_showTitleShape == "pill") {
            double pillR = boxRect.height() / 2.0;
            drawGlassRect(painter, boxRect, pillR, m_showTitleBgColor, 0.85);
            painter.setPen(QPen(m_showTitleBorderColor, 1));
            painter.setBrush(Qt::NoBrush);
            painter.drawRoundedRect(boxRect, pillR, pillR);
        } else if (m_showTitleShape == "angled") {
            double skew = 8 * scale;
            QPainterPath angledPath;
            angledPath.moveTo(drawBX + skew, drawBY);
            angledPath.lineTo(drawBX + boxRect.width(), drawBY);
            angledPath.lineTo(drawBX + boxRect.width() - skew, drawBY + boxRect.height());
            angledPath.lineTo(drawBX, drawBY + boxRect.height());
            angledPath.closeSubpath();
            painter.setPen(QPen(m_showTitleBorderColor, 1));
            painter.setBrush(m_showTitleBgColor);
            painter.drawPath(angledPath);
        } else { // "frameless"
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(0, 0, 0, 120));
            painter.drawRoundedRect(boxRect.adjusted(2 * scale, 2 * scale, 2 * scale, 2 * scale), 4 * scale, 4 * scale);
        }

        // Glow loop
        if (m_showTitleEntryProgress >= 1.0 && m_showTitleLoopAnimType == "glow") {
            double stLoopPhase2 = std::sin(m_loopFrame * 0.05);
            double glowOp = 0.3 + stLoopPhase2 * 0.15;
            painter.setOpacity(glowOp);
            QRectF glowRect = boxRect.adjusted(-3 * scale, -3 * scale, 3 * scale, 3 * scale);
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(255, 255, 255, 60));
            painter.drawRoundedRect(glowRect, 6 * scale, 6 * scale);
            painter.setOpacity(titleOpacity);
        }

        // Shimmer loop
        if (m_showTitleEntryProgress >= 1.0 && m_showTitleLoopAnimType == "shimmer") {
            double shimmerPos = (m_loopFrame % 120) / 120.0;
            double sx = boxRect.x() + (boxRect.width() + 20 * scale) * shimmerPos - 10 * scale;
            QLinearGradient shimGrad(sx, boxRect.y(), sx + 10 * scale, boxRect.y() + boxRect.height());
            shimGrad.setColorAt(0.0, QColor(255, 255, 255, 0));
            shimGrad.setColorAt(0.5, QColor(255, 255, 255, 50));
            shimGrad.setColorAt(1.0, QColor(255, 255, 255, 0));
            painter.setPen(Qt::NoPen);
            painter.setBrush(shimGrad);
            painter.setClipRect(boxRect);
            painter.drawRect(boxRect);
            painter.setClipping(false);
        }

        // Title text
        painter.setFont(titleF);
        painter.setPen(m_showTitleTextColor);
        painter.drawText(static_cast<int>(boxRect.x()) + padX, static_cast<int>(boxRect.y()) + padY + titleFm.ascent(), m_showTitleText);

        // Subtitle text
        if (!m_showSubtitleText.isEmpty()) {
            painter.setFont(subF);
            painter.setPen(m_showTitleTextColor.lighter(130));
            painter.drawText(static_cast<int>(boxRect.x()) + padX, static_cast<int>(boxRect.y()) + padY + titleFm.height() + static_cast<int>(4 * scale) + subFm.ascent(), m_showSubtitleText);
        }

        painter.restore();
    }

    // Clock (top-right) — broadcast: text + drop shadow, no box
    if (m_clockVisible && !m_bypassActive) {
        double ckS = m_clockScale;
        QString timeStr = QTime::currentTime().toString(m_clockFormat);
        QFont clockF("Menlo", qMax(12, static_cast<int>(18 * scale * ckS)), QFont::Bold);
        painter.setFont(clockF);

        int clockW = QFontMetrics(clockF).horizontalAdvance(timeStr);
        int clockX = fw - clockW - static_cast<int>(24 * scale) + static_cast<int>(m_clockOffX * scale);
        int clockY = (m_logoVisible ? static_cast<int>(72 * scale) : static_cast<int>(16 * scale)) + static_cast<int>(m_clockOffY * scale);
        int textY = clockY + QFontMetrics(clockF).ascent();

        // Drop shadow
        painter.setPen(QColor(0, 0, 0, 180));
        painter.drawText(clockX + 1, textY + 1, timeStr);
        // Main text
        painter.setPen(Qt::white);
        painter.drawText(clockX, textY, timeStr);
    }

    // Countdown pill (top-left) — broadcast: subtle glass pill
    if (m_countdownVisible && !m_countdownText.isEmpty() && !m_bypassActive) {
        double cdS = m_countdownScale;
        QFont cdFont("Menlo", qMax(12, static_cast<int>(17 * scale * cdS)), QFont::Bold);
        painter.setFont(cdFont);
        int cdW = QFontMetrics(cdFont).horizontalAdvance(m_countdownText) + static_cast<int>(24 * scale * cdS);
        int cdH = static_cast<int>(30 * scale * cdS);
        int cdX = static_cast<int>(16 * scale) + static_cast<int>(m_countdownOffX * scale);
        int cdY = static_cast<int>(72 * scale) + static_cast<int>(m_countdownOffY * scale);
        QRectF cdRect(cdX, cdY, cdW, cdH);
        drawGlassRect(painter, cdRect, cdH / 2.0, QColor(200, 0, 0), 0.55);
        // Subtle border
        painter.setPen(QPen(QColor(255, 80, 80, 65), 1));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(cdRect, cdH / 2.0, cdH / 2.0);
        painter.setPen(Qt::white);
        painter.drawText(cdRect, Qt::AlignCenter, m_countdownText);
    }

    // QR Code overlay (styled box with URL)
    if (m_qrVisible && !m_qrUrl.isEmpty() && !m_bypassActive) {
        double qrS = m_qrCodeScale;
        int qrBoxSize = static_cast<int>(120 * scale * qrS);
        int qrPad = static_cast<int>(16 * scale);
        int qrOffXPx = static_cast<int>(m_qrOffX * scale);
        int qrOffYPx = static_cast<int>(m_qrOffY * scale);
        int qrX, qrY;

        if (m_qrPosition == "top_left") {
            qrX = qrPad + qrOffXPx; qrY = qrPad + qrOffYPx;
        } else if (m_qrPosition == "top_right") {
            qrX = fw - qrBoxSize - qrPad + qrOffXPx;
            qrY = (m_logoVisible ? static_cast<int>(72 * scale) : qrPad) + qrOffYPx;
        } else if (m_qrPosition == "bottom_left") {
            int tickerH = (m_tickerVisible && !m_tickerText.isEmpty()) ? static_cast<int>(36 * scale) : 0;
            qrX = qrPad + qrOffXPx; qrY = fh - qrBoxSize - qrPad - tickerH + qrOffYPx;
        } else { // bottom_right (default)
            int tickerH = (m_tickerVisible && !m_tickerText.isEmpty()) ? static_cast<int>(36 * scale) : 0;
            qrX = fw - qrBoxSize - qrPad + qrOffXPx;
            qrY = fh - qrBoxSize - qrPad - tickerH + qrOffYPx;
        }

        QRectF qrRect(qrX, qrY, qrBoxSize, qrBoxSize);

        // Background
        drawGlassRect(painter, qrRect, static_cast<int>(8 * scale), QColor(10, 10, 15), 0.88);

        // Border pattern (QR-style)
        painter.setPen(QPen(QColor(255, 255, 255, 180), 2 * scale));
        int inset = static_cast<int>(8 * scale);
        QRectF inner(qrX + inset, qrY + inset, qrBoxSize - 2 * inset, qrBoxSize - 2 * inset);
        painter.drawRect(inner);

        // Corner squares (QR finder patterns)
        int cornerSize = static_cast<int>(20 * scale);
        painter.setBrush(QColor(255, 255, 255, 200));
        painter.setPen(Qt::NoPen);
        // Top-left corner
        painter.drawRect(QRectF(inner.left(), inner.top(), cornerSize, cornerSize));
        // Top-right corner
        painter.drawRect(QRectF(inner.right() - cornerSize, inner.top(), cornerSize, cornerSize));
        // Bottom-left corner
        painter.drawRect(QRectF(inner.left(), inner.bottom() - cornerSize, cornerSize, cornerSize));

        // Inner squares of corners (hollow effect)
        int innerCorner = static_cast<int>(10 * scale);
        int offset2 = static_cast<int>(5 * scale);
        painter.setBrush(QColor(10, 10, 15));
        painter.drawRect(QRectF(inner.left() + offset2, inner.top() + offset2, innerCorner, innerCorner));
        painter.drawRect(QRectF(inner.right() - cornerSize + offset2, inner.top() + offset2, innerCorner, innerCorner));
        painter.drawRect(QRectF(inner.left() + offset2, inner.bottom() - cornerSize + offset2, innerCorner, innerCorner));

        // "SCAN" label
        QFont scanFont("Helvetica Neue", static_cast<int>(10 * scale), QFont::Bold);
        painter.setFont(scanFont);
        painter.setPen(m_accentColor);
        painter.drawText(QRectF(qrX, qrY + static_cast<int>(60 * scale), qrBoxSize, static_cast<int>(16 * scale)),
                         Qt::AlignHCenter, "SCAN");

        // URL text (truncated if needed)
        QFont urlFont("Menlo", static_cast<int>(7 * scale));
        painter.setFont(urlFont);
        painter.setPen(QColor(180, 180, 180));
        QString displayUrl = m_qrUrl;
        QFontMetrics urlFm(urlFont);
        if (urlFm.horizontalAdvance(displayUrl) > qrBoxSize - 2 * inset)
            displayUrl = urlFm.elidedText(displayUrl, Qt::ElideMiddle, qrBoxSize - 2 * inset);
        painter.drawText(QRectF(qrX, qrY + static_cast<int>(80 * scale), qrBoxSize, static_cast<int>(30 * scale)),
                         Qt::AlignHCenter | Qt::TextWordWrap, displayUrl);
    }

    // Social chat overlay (top-right, semi-transparent vertical list)
    if (m_chatVisible && !m_chatMessages.isEmpty() && !m_bypassActive) {
        QFont chatFont("Helvetica Neue", static_cast<int>(11 * scale));
        painter.setFont(chatFont);
        QFontMetrics chatFm(chatFont);

        int lineH = chatFm.height() + static_cast<int>(4 * scale);
        int maxLines = qMin(5, m_chatMessages.size());
        int chatW = static_cast<int>(300 * scale);
        int chatH = lineH * maxLines + static_cast<int>(12 * scale);
        int chatX = fw - chatW - static_cast<int>(16 * scale);
        int chatY = static_cast<int>(16 * scale);
        // Push below logo if visible
        if (m_logoVisible) chatY = static_cast<int>(72 * scale);

        QRectF chatBg(chatX, chatY, chatW, chatH);
        drawGlassRect(painter, chatBg, static_cast<int>(6 * scale), QColor(0, 0, 0), 0.65);

        painter.setPen(QColor(220, 220, 220));
        for (int i = 0; i < maxLines; ++i) {
            QString line = chatFm.elidedText(m_chatMessages[i], Qt::ElideRight, chatW - static_cast<int>(12 * scale));
            painter.drawText(chatX + static_cast<int>(6 * scale),
                             chatY + static_cast<int>(6 * scale) + (i + 1) * lineH - static_cast<int>(2 * scale),
                             line);
        }
    }

    // Scoreboard overlay — glass morphism (FIFA/UEFA broadcast style)
    if (m_scoreboardVisible && !m_bypassActive) {
        double timeSec = m_loopFrame / 25.0;
        double sbS = m_scoreboardScale;
        int sbW = static_cast<int>(360 * scale * sbS);
        int sbH = static_cast<int>(85 * scale * sbS);
        int sbPad = static_cast<int>(16 * scale);
        int sbOffXPx = static_cast<int>(m_scoreboardOffX * scale);
        int sbOffYPx = static_cast<int>(m_scoreboardOffY * scale);
        int sbMarginY = static_cast<int>(60 * scale);
        int sbX, sbY;

        if (m_sbPosition == "top_left" || m_sbPosition == "bottom_left")
            sbX = sbPad + sbOffXPx;
        else
            sbX = fw - sbW - sbPad + sbOffXPx;
        if (m_sbPosition == "top_left" || m_sbPosition == "top_right")
            sbY = sbMarginY + sbOffYPx;
        else
            sbY = fh - sbH - sbMarginY + sbOffYPx;

        QRectF sbRect(sbX, sbY, sbW, sbH);
        // Design Template: render scoreboard background layers
        DesignRegistry::instance().render(painter, "scoreboard", m_sbDesignId, sbRect, timeSec, scale, m_accentColor);
        // Top accent line
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 255, 255, 30));
        painter.drawRect(QRectF(sbX, sbY, sbW, 1));
        // Subtle border
        painter.setPen(QPen(QColor(255, 255, 255, 20), 1));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(sbRect, 6 * scale, 6 * scale);

        // Font sizes with minimum floors — small text scales faster than large
        int fntTeam = qMax(11, static_cast<int>(14 * scale * (0.7 + sbS * 0.3)));
        int fntScore = qMax(16, static_cast<int>(22 * scale * sbS));
        int fntTime = qMax(10, static_cast<int>(12 * scale * (0.7 + sbS * 0.3)));
        int midY = sbY + sbH / 2;

        QFont teamFont("Helvetica Neue", fntTeam, QFont::DemiBold);
        QFont scoreFont("Helvetica Neue", fntScore, QFont::Bold);
        QFont timeFont("Menlo", fntTime, QFont::Bold);

        // Team A logo (if available)
        if (!m_teamLogoImgA.isNull()) {
            int tLogoH = qMax(20, static_cast<int>(sbH * 0.50));
            QImage tLogoA = m_teamLogoImgA.scaledToHeight(tLogoH, Qt::SmoothTransformation);
            painter.drawImage(static_cast<int>(sbX + sbW * 0.25 - tLogoA.width() / 2), static_cast<int>(sbY + sbH * 0.05), tLogoA);
        }

        // Team A color accent bar
        painter.setPen(Qt::NoPen);
        painter.setBrush(m_sbColorA);
        painter.drawRect(QRectF(sbX + sbW * 0.12, sbY + sbH * 0.08, sbW * 0.18, 2 * scale));

        // Team A name
        painter.setFont(teamFont);
        painter.setPen(QColor(255, 255, 255, 180));
        painter.drawText(QRectF(sbX, sbY + sbH * 0.12, sbW / 2, sbH * 0.25), Qt::AlignHCenter, m_sbTeamA);
        // Team A score
        painter.setFont(scoreFont);
        painter.setPen(Qt::white);
        painter.drawText(QRectF(sbX, midY - fntScore * 0.4, sbW / 2, fntScore * 1.0), Qt::AlignHCenter, QString::number(m_sbScoreA));

        // Center separator line
        painter.setPen(QPen(QColor(255, 255, 255, 30), 1));
        painter.drawLine(QPointF(sbX + sbW / 2.0, sbY + sbH * 0.15), QPointF(sbX + sbW / 2.0, sbY + sbH * 0.85));

        // Team B logo (if available)
        if (!m_teamLogoImgB.isNull()) {
            int tLogoH = qMax(20, static_cast<int>(sbH * 0.50));
            QImage tLogoB = m_teamLogoImgB.scaledToHeight(tLogoH, Qt::SmoothTransformation);
            painter.drawImage(static_cast<int>(sbX + sbW * 0.75 - tLogoB.width() / 2), static_cast<int>(sbY + sbH * 0.05), tLogoB);
        }

        // Team B color accent bar
        painter.setPen(Qt::NoPen);
        painter.setBrush(m_sbColorB);
        painter.drawRect(QRectF(sbX + sbW * 0.62, sbY + sbH * 0.08, sbW * 0.18, 2 * scale));

        // Team B name
        painter.setFont(teamFont);
        painter.setPen(QColor(255, 255, 255, 180));
        painter.drawText(QRectF(sbX + sbW / 2, sbY + sbH * 0.12, sbW / 2, sbH * 0.25), Qt::AlignHCenter, m_sbTeamB);
        // Team B score
        painter.setFont(scoreFont);
        painter.setPen(Qt::white);
        painter.drawText(QRectF(sbX + sbW / 2, midY - fntScore * 0.4, sbW / 2, fntScore * 1.0), Qt::AlignHCenter, QString::number(m_sbScoreB));

        // Match time + period — subtle
        painter.setFont(timeFont);
        painter.setPen(QColor(0, 229, 255, 200));
        QString timeLabel = m_sbMatchTime + "  P" + QString::number(m_sbPeriod);
        painter.drawText(QRectF(sbX, sbY + sbH * 0.80, sbW, sbH * 0.18), Qt::AlignHCenter, timeLabel);
    }

    // Weather overlay — broadcast: text + drop shadow, no box
    if (m_weatherVisible && !m_weatherCity.isEmpty() && !m_bypassActive) {
        double wS = m_weatherScale;
        int wOffXPx = static_cast<int>(m_weatherOffX * scale);
        int wOffYPx = static_cast<int>(m_weatherOffY * scale);
        int wMarginY = static_cast<int>(80 * scale);
        int wPad = static_cast<int>(24 * scale);

        // Icon
        // Font sizes with minimum floors for readability
        int iconSize = qMax(18, static_cast<int>(28 * scale * wS));
        QFont iconFont("Helvetica Neue", iconSize);

        // City — larger base, scales well
        int citySize = qMax(13, static_cast<int>(16 * scale * (0.7 + wS * 0.3)));
        QFont cityFont("Helvetica Neue", citySize, QFont::Bold);
        QFontMetrics cityFm(cityFont);

        // Temperature
        int tempSize = qMax(11, static_cast<int>(14 * scale * (0.7 + wS * 0.3)));
        QFont tempFont("Helvetica Neue", tempSize);
        QFontMetrics tempFm(tempFont);
        QString tempStr = QString::number(qRound(m_weatherTemp)) + m_weatherUnit;

        int textBlockW = qMax(cityFm.horizontalAdvance(m_weatherCity), tempFm.horizontalAdvance(tempStr));
        int iconW = static_cast<int>(30 * scale * wS);
        int gap = static_cast<int>(6 * scale);
        int totalW = iconW + gap + textBlockW;

        int wX = fw - totalW - wPad + wOffXPx;
        int wY = fh - wMarginY + wOffYPx;

        // Drop shadow for icon
        painter.setFont(iconFont);
        painter.setPen(QColor(0, 0, 0, 150));
        painter.drawText(wX + 1, wY + iconSize + 1, m_weatherIcon);
        painter.setPen(QColor(255, 215, 0));
        painter.drawText(wX, wY + iconSize, m_weatherIcon);

        int textX = wX + iconW + gap;

        // City — shadow then white
        painter.setFont(cityFont);
        painter.setPen(QColor(0, 0, 0, 170));
        painter.drawText(textX + 1, wY + cityFm.ascent() + 1, m_weatherCity);
        painter.setPen(Qt::white);
        painter.drawText(textX, wY + cityFm.ascent(), m_weatherCity);

        // Temperature — shadow then light gray
        int tempY = wY + cityFm.height() + static_cast<int>(2 * scale);
        painter.setFont(tempFont);
        painter.setPen(QColor(0, 0, 0, 170));
        painter.drawText(textX + 1, tempY + tempFm.ascent() + 1, tempStr);
        painter.setPen(QColor(255, 255, 255, 200));
        painter.drawText(textX, tempY + tempFm.ascent(), tempStr);
    }

    // Global effects are handled by GPU postProcess AFTER composite (Step 4)
    // No CPU fx:: effects here — they were invisible on transparent overlayLayer

    painter.end(); // End overlay layer rasterization

    // ── Step 3: GPU Composite overlay layer onto video ──────
    // Alpha-blend the overlay layer (text, shapes, nameplates) onto the video+studio frame
    if (m_gpu.isAvailable()) {
        m_gpu.drawImage(output, overlayLayer, 0, 0);
    } else {
        QPainter blend(&output);
        blend.setCompositionMode(QPainter::CompositionMode_SourceOver);
        blend.drawImage(0, 0, overlayLayer);
        blend.end();
    }

    // ── Step 4: GPU Post-Processing — ALL effects via GLSL shaders ──
    if (m_gpu.isAvailable() && !m_bypassActive && !m_animTypeStr.isEmpty()) {
        output = m_gpu.postProcess(output, m_animTypeStr, m_accentColor, m_loopFrame);
    }

    m_lastCompositeMs = m_perfTimer.nsecsElapsed() / 1000000.0;
    emit frameComposited(output);
    return output;
}

// ══════════════════════════════════════════════════════════════
// Premium rendering helpers (unchanged)
// ══════════════════════════════════════════════════════════════

void Compositor::drawGlassRect(QPainter& p, const QRectF& rect, double radius, const QColor& tint, double opacity, int)
{
    p.save();
    QPainterPath clip; clip.addRoundedRect(rect, radius, radius);
    p.setClipPath(clip);
    QColor base = tint; base.setAlphaF(opacity);
    p.fillRect(rect, base);
    QLinearGradient hl(rect.topLeft(), QPointF(rect.left(), rect.top() + rect.height() * 0.45));
    hl.setColorAt(0.0, QColor(255,255,255,18)); hl.setColorAt(1.0, QColor(255,255,255,0));
    p.fillRect(rect, hl);
    QLinearGradient is(QPointF(rect.left(), rect.bottom() - rect.height()*0.3), rect.bottomLeft());
    is.setColorAt(0.0, QColor(0,0,0,0)); is.setColorAt(1.0, QColor(0,0,0,25));
    p.fillRect(rect, is);
    p.setPen(QPen(QColor(255,255,255,30), 0.5));
    p.drawLine(QPointF(rect.left()+radius, rect.top()), QPointF(rect.right()-radius, rect.top()));
    p.restore();
}

void Compositor::drawSoftShadow(QPainter& p, const QRectF& rect, double radius, int spread, const QColor& color)
{
    p.save(); p.setPen(Qt::NoPen);
    for (int i = spread; i > 0; --i) {
        double f = 1.0 - static_cast<double>(i)/spread;
        QColor c = color; c.setAlphaF(color.alphaF() * f * f);
        p.setBrush(c);
        p.drawRoundedRect(rect.adjusted(-i,-i,i,i), radius+i*0.5, radius+i*0.5);
    }
    p.restore();
}

void Compositor::drawGlowLine(QPainter& p, const QPointF& p1, const QPointF& p2, double width, const QColor& color, double glowSize)
{
    p.save();
    for (int i = static_cast<int>(glowSize); i > 0; --i) {
        QColor gc = color; gc.setAlphaF(0.08*(1.0-static_cast<double>(i)/glowSize));
        p.setPen(QPen(gc, width+i*2, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(p1, p2);
    }
    p.setPen(QPen(color, width, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(p1, p2);
    p.restore();
}

void Compositor::drawPill(QPainter& p, const QRectF& rect, const QColor& bg, const QColor& textColor, const QString& label, int fontSize)
{
    p.save();
    QPainterPath path; path.addRoundedRect(rect, rect.height()/2, rect.height()/2);
    p.fillPath(path, bg);
    QFont f("Helvetica Neue", fontSize, QFont::Bold);
    p.setFont(f); p.setPen(textColor);
    p.drawText(rect, Qt::AlignCenter, label);
    p.restore();
}

QRectF Compositor::calcPlate(const TalentOverlay& t, const QSize& fs, double w, double h)
{
    // Apply nameplate scale factor
    w *= m_nameplateScale;
    h *= m_nameplateScale;
    double x = t.bbox.x();
    double y = t.bbox.y() + t.bbox.height() + 12;
    x = qBound(8.0, x, fs.width()-w-8.0);
    y = qMin(y, fs.height()-h-8.0);
    return {x, y, w, h};
}

void Compositor::setStyleId(const QString& styleId) { m_styleId = styleId; }

// ══════════════════════════════════════════════════════════════
// 20 STYLE RENDERERS — with animation progress
// prog: 0.0 = entering, 1.0 = fully visible
// Each style applies its own animation type via slide/opacity
// ══════════════════════════════════════════════════════════════

// Helper: calculate slide offset from animation progress
static double slideOffset(double prog, double maxOffset) { return maxOffset * (1.0 - prog); }

void Compositor::drawBFM(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog)
{
    QFont nameF("Helvetica Neue", 22, QFont::Bold);
    QFont roleF("Helvetica Neue", 13);
    double pw = qMax(360.0, QFontMetrics(nameF).horizontalAdvance(t.name)+52.0);
    QRectF r = calcPlate(t, fs, pw, 64);
    r.moveLeft(r.left() - slideOffset(prog, pw + 20)); // Slide from left
    drawSoftShadow(p, r, 6, 10, QColor(0,0,0,100));
    drawGlassRect(p, r, 6, QColor(8,8,12), m_bgOpacity);
    drawGlowLine(p, QPointF(r.left()+2,r.top()+6), QPointF(r.left()+2,r.bottom()-6), 3, QColor("#E30613"), 4);
    drawGlowLine(p, QPointF(r.left()+8,r.bottom()-1.5), QPointF(r.right()-8,r.bottom()-1.5), 2, QColor("#E30613"), 3);
    p.setFont(nameF); p.setPen(Qt::white); p.drawText(r.left()+20, r.top()+30, t.name);
    p.setFont(roleF); p.setPen(QColor(200,200,200)); p.drawText(r.left()+20, r.top()+50, t.role);
}

void Compositor::drawLCI(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog)
{
    QFont nameF("Helvetica Neue", 21, QFont::Bold);
    QFont roleF("Helvetica Neue", 13);
    double pw = qMax(360.0, QFontMetrics(nameF).horizontalAdvance(t.name)+48.0);
    QRectF r = calcPlate(t, fs, pw, 64);
    // FadeScale: scale from 0.95 + opacity
    double sc = 0.95 + 0.05 * prog;
    p.save(); p.translate(r.center()); p.scale(sc, sc); p.translate(-r.center());
    drawSoftShadow(p, r, 6, 8, QColor(0,0,0,80));
    QPainterPath clip; clip.addRoundedRect(r, 6, 6);
    p.save(); p.setClipPath(clip);
    QLinearGradient grad(r.topLeft(), r.topRight());
    grad.setColorAt(0, QColor(30,28,26,int(255*m_bgOpacity))); grad.setColorAt(1, QColor(10,10,10,int(255*m_bgOpacity)));
    p.fillRect(r, grad);
    p.restore();
    drawGlowLine(p, QPointF(r.left(),r.bottom()-1.5), QPointF(r.right(),r.bottom()-1.5), 2.5, QColor("#FF6D00"), 4);
    QPainterPath tri; tri.moveTo(r.right()-24,r.top()); tri.lineTo(r.right(),r.top()); tri.lineTo(r.right(),r.top()+24); tri.closeSubpath();
    p.fillPath(tri, QColor("#FF6D00"));
    p.setFont(nameF); p.setPen(Qt::white); p.drawText(r.left()+16, r.top()+29, t.name);
    p.setFont(roleF); p.setPen(QColor("#E0E0E0")); p.drawText(r.left()+16, r.top()+49, t.role);
    p.restore();
}

void Compositor::drawFrance2(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog)
{
    QFont nameF("Helvetica Neue", 21, QFont::Bold); QFont roleF("Helvetica Neue", 13);
    double pw = qMax(360.0, QFontMetrics(nameF).horizontalAdvance(t.name)+40.0);
    QRectF r = calcPlate(t, fs, pw, 64);
    r.moveTop(r.top() + slideOffset(prog, 30)); // Slide from bottom
    drawSoftShadow(p, r, 8, 12, QColor(0,0,50,50));
    drawGlassRect(p, r, 8, QColor(250,250,255), 0.93);
    drawGlowLine(p, QPointF(r.left(),r.bottom()-2), QPointF(r.right(),r.bottom()-2), 3, QColor("#003189"), 3);
    p.setFont(nameF); p.setPen(QColor("#003189")); p.drawText(r.left()+16, r.top()+30, t.name);
    p.setFont(roleF); p.setPen(QColor("#556677")); p.drawText(r.left()+16, r.top()+50, t.role);
}

void Compositor::drawFrance24(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog)
{
    QFont nameF("Helvetica Neue", 22, QFont::Bold); QFont roleF("Helvetica Neue", 13);
    double pw = qMax(380.0, QFontMetrics(nameF).horizontalAdvance(t.name)+40.0);
    QRectF r = calcPlate(t, fs, pw, 66);
    // Wipe: clip width by progress
    p.save(); p.setClipRect(QRectF(r.left(), r.top(), r.width() * prog, r.height()));
    drawSoftShadow(p, r, 5, 8, QColor(0,0,0,90));
    drawGlassRect(p, r, 5, QColor(6,6,10), m_bgOpacity);
    drawGlowLine(p, QPointF(r.left(),r.bottom()-5), QPointF(r.right(),r.bottom()-5), 2.5, QColor("#D4001A"), 3);
    drawGlowLine(p, QPointF(r.left(),r.bottom()-1.5), QPointF(r.right(),r.bottom()-1.5), 2.5, QColor("#003580"), 3);
    p.setFont(nameF); p.setPen(Qt::white); p.drawText(r.left()+16, r.top()+31, t.name);
    p.setFont(roleF); p.setPen(QColor("#BBBBCC")); p.drawText(r.left()+16, r.top()+51, t.role);
    p.restore();
}

void Compositor::drawCNN(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog)
{
    QFont nameF("Helvetica Neue", 24, QFont::Bold); QFont roleF("Helvetica Neue", 13);
    double pw = qMax(360.0, QFontMetrics(nameF).horizontalAdvance(t.name)+40.0);
    QRectF r = calcPlate(t, fs, pw, 64);
    r.moveLeft(r.left() - slideOffset(prog, pw + 20)); // Fast slide left
    drawSoftShadow(p, r, 5, 10, QColor(180,0,0,60));
    drawGlassRect(p, r, 5, QColor(190,0,0), 0.92);
    p.setFont(nameF); p.setPen(Qt::white); p.drawText(r.left()+16, r.top()+32, t.name);
    p.setFont(roleF); p.setPen(QColor(255,200,200)); p.drawText(r.left()+16, r.top()+52, t.role);
}

void Compositor::drawBBC(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog)
{
    QFont nameF("Helvetica Neue", 21, QFont::Bold); QFont roleF("Helvetica Neue", 13);
    double pw = qMax(400.0, QFontMetrics(nameF).horizontalAdvance(t.name)+80.0);
    QRectF r = calcPlate(t, fs, pw, 64);
    // Wipe from left
    p.save(); p.setClipRect(QRectF(r.left(), r.top(), r.width() * prog, r.height()));
    drawSoftShadow(p, r, 5, 8, QColor(120,0,0,50));
    drawGlassRect(p, r, 5, QColor(170,20,20), 0.93);
    p.setPen(Qt::NoPen); p.setBrush(Qt::white);
    p.drawRoundedRect(QRectF(r.left()+1,r.top()+4,6,r.height()-8), 3, 3);
    QRectF badge(r.right()-58,r.center().y()-13,50,26);
    drawPill(p, badge, QColor(255,255,255,35), Qt::white, m_channelName.isEmpty() ? "BBC" : m_channelName.left(3).toUpper(), 11);
    p.setFont(nameF); p.setPen(Qt::white); p.drawText(r.left()+18, r.top()+29, t.name);
    p.setFont(roleF); p.setPen(QColor("#FFAAAA")); p.drawText(r.left()+18, r.top()+49, t.role);
    p.restore();
}

void Compositor::drawSkyNews(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog)
{
    QFont nameF("Helvetica Neue", 21, QFont::Bold); QFont roleF("Helvetica Neue", 13);
    double pw = qMax(360.0, QFontMetrics(nameF).horizontalAdvance(t.name)+40.0);
    QRectF r = calcPlate(t, fs, pw, 64);
    r.moveLeft(r.left() + slideOffset(prog, pw + 20)); // Slide from right
    drawSoftShadow(p, r, 6, 8, QColor(0,40,120,50));
    drawGlassRect(p, r, 6, QColor(0,70,155), 0.90);
    drawGlowLine(p, QPointF(r.left(),r.bottom()-1.5), QPointF(r.right(),r.bottom()-1.5), 2.5, QColor("#00A0DC"), 4);
    p.setFont(nameF); p.setPen(Qt::white); p.drawText(r.left()+16, r.top()+29, t.name);
    p.setFont(roleF); p.setPen(QColor("#A8D4F0")); p.drawText(r.left()+16, r.top()+49, t.role);
}

void Compositor::drawAlJazeera(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog)
{
    Q_UNUSED(prog) // Pure fade via painter opacity
    QFont nameF("Helvetica Neue", 21, QFont::Bold); QFont roleF("Helvetica Neue", 13);
    double pw = qMax(400.0, QFontMetrics(nameF).horizontalAdvance(t.name)+80.0);
    QRectF r = calcPlate(t, fs, pw, 64);
    drawSoftShadow(p, r, 5, 8, QColor(80,0,0,50));
    drawGlassRect(p, r, 5, QColor(110,0,0), 0.88);
    QRectF badge(r.left()+2,r.top(),48,r.height());
    p.save(); QPainterPath bc; bc.addRoundedRect(badge,5,5); p.setClipPath(bc);
    p.fillRect(badge, QColor(212,160,23,50)); p.restore();
    QFont bF("Helvetica Neue", 15, QFont::Bold); p.setFont(bF); p.setPen(QColor("#D4A017"));
    p.drawText(badge, Qt::AlignCenter, m_channelName.isEmpty() ? "AJ" : m_channelName.left(2).toUpper());
    p.setFont(nameF); p.setPen(QColor("#D4A017")); p.drawText(r.left()+60, r.top()+29, t.name);
    p.setFont(roleF); p.setPen(Qt::white); p.drawText(r.left()+60, r.top()+49, t.role);
}

void Compositor::drawSports(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog)
{
    QFont nameF("Helvetica Neue", 26, QFont::ExtraBold); QFont roleF("Helvetica Neue", 15);
    double pw = qMax(400.0, QFontMetrics(nameF).horizontalAdvance(t.name)+88.0);
    QRectF r = calcPlate(t, fs, pw, 72);
    double sc = 0.9 + 0.1 * prog;
    p.save(); p.translate(r.center()); p.scale(sc, sc); p.translate(-r.center());
    drawSoftShadow(p, r, 5, 10, QColor(0,0,0,90));
    drawGlassRect(p, r, 5, QColor(5,5,8), 0.88);
    QRectF badge(r.left(),r.top(),58,r.height());
    p.setPen(Qt::NoPen); p.setBrush(m_accentColor); p.drawRoundedRect(badge.adjusted(0,0,4,0), 5, 5);
    QFont numF("Helvetica Neue", 28, QFont::Bold); p.setFont(numF); p.setPen(Qt::white); p.drawText(badge, Qt::AlignCenter, "10");
    p.setFont(nameF); p.setPen(Qt::white); p.drawText(r.left()+70, r.top()+36, t.name);
    p.setFont(roleF); p.setPen(m_accentColor); p.drawText(r.left()+70, r.top()+58, t.role);
    p.restore();
}

void Compositor::drawFootball(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog)
{
    QFont nameF("Helvetica Neue", 24, QFont::Bold); QFont roleF("Helvetica Neue", 14);
    double pw = qMax(400.0, QFontMetrics(nameF).horizontalAdvance(t.name)+40.0);
    QRectF r = calcPlate(t, fs, pw, 72);
    r.moveTop(r.top() + slideOffset(prog, 40)); // Slide up
    drawSoftShadow(p, r, 5, 8, QColor(0,30,0,60));
    drawGlassRect(p, r, 5, QColor(18,60,18), 0.88);
    p.setPen(QColor(255,255,255,12));
    for (int i=0; i<4; ++i) p.drawLine(QPointF(r.left()+50+i*100,r.top()), QPointF(r.left()+50+i*100,r.bottom()));
    p.setPen(Qt::NoPen);
    QFont gF("Helvetica Neue", 64, QFont::Bold); p.setFont(gF); p.setPen(QColor(255,255,255,16));
    p.drawText(QRectF(r.right()-60,r.top(),55,r.height()), Qt::AlignCenter, "9");
    p.setFont(nameF); p.setPen(Qt::white); p.drawText(r.left()+16, r.top()+34, t.name);
    p.setFont(roleF); p.setPen(QColor("#FFFF00")); p.drawText(r.left()+16, r.top()+56, t.role);
}

void Compositor::drawOlympics(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog)
{
    Q_UNUSED(prog)
    QFont nameF("Helvetica Neue", 21, QFont::Bold); QFont roleF("Helvetica Neue", 13);
    double pw = qMax(400.0, QFontMetrics(nameF).horizontalAdvance(t.name)+110.0);
    QRectF r = calcPlate(t, fs, pw, 68);
    drawSoftShadow(p, r, 8, 10, QColor(0,0,50,40));
    drawGlassRect(p, r, 8, QColor(248,248,252), 0.92);
    QColor rings[]={QColor("#0085C7"),QColor("#222"),QColor("#DF0024"),QColor("#F4C300"),QColor("#009F3D")};
    for (int i=0; i<5; ++i) { p.setPen(QPen(rings[i],2.5)); p.setBrush(Qt::NoBrush); p.drawEllipse(QPointF(r.left()+16+i*18,r.center().y()),8,8); }
    p.setFont(nameF); p.setPen(QColor("#0033A0")); p.drawText(r.left()+108, r.top()+30, t.name);
    p.setFont(roleF); p.setPen(QColor("#667788")); p.drawText(r.left()+108, r.top()+50, t.role);
}

void Compositor::drawCinema(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog)
{
    Q_UNUSED(prog)
    QFont nameF("Helvetica Neue", 24, QFont::Bold); QFont roleF("Helvetica Neue", 14); roleF.setItalic(true);
    double pw = qMax(420.0, QFontMetrics(nameF).horizontalAdvance(t.name)+48.0);
    QRectF r = calcPlate(t, fs, pw, 68);
    drawSoftShadow(p, r, 4, 12, QColor(0,0,0,120));
    drawGlassRect(p, r, 4, QColor(0,0,0), 0.96);
    drawGlowLine(p, QPointF(r.left()+4,r.top()+0.5), QPointF(r.right()-4,r.top()+0.5), 1, QColor("#D4AF37"), 3);
    drawGlowLine(p, QPointF(r.left()+4,r.bottom()-0.5), QPointF(r.right()-4,r.bottom()-0.5), 1, QColor("#D4AF37"), 3);
    p.setFont(nameF); p.setPen(QColor("#D4AF37")); p.drawText(r.left()+20, r.top()+32, t.name);
    p.setFont(roleF); p.setPen(QColor("#C0A060")); p.drawText(r.left()+20, r.top()+54, t.role);
}

void Compositor::drawLuxury(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog)
{
    Q_UNUSED(prog)
    QFont nameF("Helvetica Neue", 26, QFont::Light); nameF.setLetterSpacing(QFont::AbsoluteSpacing, 3);
    QFont roleF("Helvetica Neue", 11, QFont::ExtraLight); roleF.setLetterSpacing(QFont::AbsoluteSpacing, 6); roleF.setCapitalization(QFont::AllUppercase);
    QRectF r = calcPlate(t, fs, 460, 74);
    drawSoftShadow(p, r, 3, 14, QColor(0,0,0,80));
    drawGlassRect(p, r, 3, QColor(10,10,12), 0.96);
    drawGlowLine(p, QPointF(r.left()+6,r.top()+0.5), QPointF(r.right()-6,r.top()+0.5), 0.5, QColor("#C0C0C0"), 2);
    drawGlowLine(p, QPointF(r.left()+6,r.bottom()-0.5), QPointF(r.right()-6,r.bottom()-0.5), 0.5, QColor("#C0C0C0"), 2);
    p.setFont(nameF); p.setPen(QColor("#F0F0F0")); p.drawText(r, Qt::AlignHCenter|Qt::AlignTop, "\n"+t.name);
    p.setFont(roleF); p.setPen(QColor("#888888")); p.drawText(r.adjusted(0,44,0,0), Qt::AlignHCenter|Qt::AlignTop, t.role.toUpper());
}

void Compositor::drawTech(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog)
{
    QFont nameF("Menlo", 20, QFont::Bold); QFont roleF("Menlo", 12);
    double pw = qMax(400.0, QFontMetrics(nameF).horizontalAdvance(t.name)+48.0);
    QRectF r = calcPlate(t, fs, pw, 64);
    // Glitch effect: flicker at low progress
    if (prog < 0.3) {
        double flicker = std::sin(prog * 50) * 0.5 + 0.5;
        p.setOpacity(p.opacity() * flicker);
    }
    drawSoftShadow(p, r, 4, 8, QColor(0,100,140,30));
    drawGlassRect(p, r, 4, QColor(8,12,22), 0.90);
    p.save(); QPainterPath cl; cl.addRoundedRect(r,4,4); p.setClipPath(cl);
    p.setPen(QColor(255,255,255,8));
    for (double y=r.top(); y<r.bottom(); y+=3) p.drawLine(QPointF(r.left(),y),QPointF(r.right(),y));
    p.restore();
    drawGlowLine(p, QPointF(r.left(),r.bottom()-1), QPointF(r.right(),r.bottom()-1), 1.5, QColor("#00E5FF"), 5);
    QRectF lp(r.right()-52,r.top()+6,44,20);
    drawPill(p, lp, QColor(0,229,255,30), QColor("#00E5FF"), "LIVE", 9);
    p.setFont(nameF); p.setPen(Qt::white); p.drawText(r.left()+14, r.top()+28, t.name);
    p.setFont(roleF); p.setPen(QColor("#00E5FF")); p.drawText(r.left()+14, r.top()+48, t.role);
}

void Compositor::drawMinimal(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog)
{
    Q_UNUSED(prog) // Pure fade via painter opacity
    QFont nameF("Helvetica Neue", 23, QFont::Bold); QFont roleF("Helvetica Neue", 14);
    double x = qBound(20.0, t.bbox.x(), fs.width()-300.0);
    double y = qMin(t.bbox.y()+t.bbox.height()+18, fs.height()-56.0);
    for (int s=3; s>0; --s) { p.setFont(nameF); p.setPen(QColor(0,0,0,60/s)); p.drawText(QPointF(x+s,y+s), t.name); }
    p.setFont(nameF); p.setPen(Qt::white); p.drawText(QPointF(x,y), t.name);
    double lw = QFontMetrics(nameF).horizontalAdvance(t.name);
    drawGlowLine(p, QPointF(x,y+5), QPointF(x+lw,y+5), 0.8, Qt::white, 2);
    p.setFont(roleF); p.setPen(QColor(255,255,255,178)); p.drawText(QPointF(x,y+22), t.role);
}

void Compositor::drawPolitique(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog)
{
    QFont nameF("Helvetica Neue", 21, QFont::Bold); QFont roleF("Helvetica Neue", 13); roleF.setItalic(true);
    double pw = qMax(400.0, QFontMetrics(nameF).horizontalAdvance(t.name)+36.0);
    QRectF r = calcPlate(t, fs, pw, 68);
    // Iris/scale from center
    double sc = 0.6 + 0.4 * prog;
    p.save(); p.translate(r.center()); p.scale(sc, sc); p.translate(-r.center());
    drawSoftShadow(p, r, 6, 10, QColor(0,0,40,60));
    drawGlassRect(p, r, 6, QColor(20,32,60), 0.88);
    double fw=8, th=r.height()/3; p.setPen(Qt::NoPen);
    p.setBrush(QColor("#002395")); p.drawRect(QRectF(r.left(),r.top(),fw,th));
    p.setBrush(Qt::white); p.drawRect(QRectF(r.left(),r.top()+th,fw,th));
    p.setBrush(QColor("#ED2939")); p.drawRect(QRectF(r.left(),r.top()+th*2,fw,r.height()-th*2));
    p.setFont(nameF); p.setPen(Qt::white); p.drawText(r.left()+20, r.top()+30, t.name);
    p.setFont(roleF); p.setPen(QColor("#A8C4E0")); p.drawText(r.left()+20, r.top()+52, t.role);
    p.restore();
}

void Compositor::drawGouvernement(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog)
{
    Q_UNUSED(prog)
    QFont nameF("Helvetica Neue", 21, QFont::Bold); QFont roleF("Helvetica Neue", 13);
    QRectF r = calcPlate(t, fs, 440, 68);
    drawSoftShadow(p, r, 8, 12, QColor(0,0,0,35));
    drawGlassRect(p, r, 8, QColor(250,250,252), 0.95);
    double fw=6, th=r.height()/3; p.setPen(Qt::NoPen);
    p.setBrush(QColor("#002395")); p.drawRect(QRectF(r.left(),r.top(),fw,th));
    p.setBrush(Qt::white); p.drawRect(QRectF(r.left(),r.top()+th,fw,th));
    p.setBrush(QColor("#ED2939")); p.drawRect(QRectF(r.left(),r.top()+th*2,fw,r.height()-th*2));
    QRectF seal(r.right()-54,r.center().y()-22,44,44);
    p.setBrush(QColor("#EEEEF0")); p.drawEllipse(seal);
    p.setPen(QPen(QColor("#002395"),1.5)); p.setBrush(Qt::NoBrush); p.drawEllipse(seal.adjusted(2,2,-2,-2));
    QFont sf("Helvetica Neue", 13, QFont::Bold); p.setFont(sf); p.setPen(QColor("#002395")); p.drawText(seal, Qt::AlignCenter, m_channelName.isEmpty() ? "RF" : m_channelName.left(2).toUpper());
    p.setFont(nameF); p.setPen(QColor("#1A1A1A")); p.drawText(r.left()+18, r.top()+30, t.name);
    p.setFont(roleF); p.setPen(QColor("#444444")); p.drawText(r.left()+18, r.top()+50, t.role);
}

void Compositor::drawDualPerson(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog)
{
    QFont nameF("Helvetica Neue", 19, QFont::Bold); QFont roleF("Helvetica Neue", 12);
    double pw=280, ph=58;
    double cx=t.bbox.x()+t.bbox.width()/2;
    double px=(cx<fs.width()/2) ? fs.width()*0.03 : fs.width()*0.97-pw;
    double py=fs.height()*0.78;
    QRectF r(px, py, pw, ph);
    r.moveTop(r.top() + slideOffset(prog, 20)); // Subtle slide up
    drawSoftShadow(p, r, 6, 8, QColor(0,0,0,80));
    drawGlassRect(p, r, 6, QColor(8,8,12), m_bgOpacity);
    drawGlowLine(p, QPointF(r.left(),r.bottom()-1.5), QPointF(r.right(),r.bottom()-1.5), 2, m_accentColor, 3);
    p.setFont(nameF); p.setPen(Qt::white); p.drawText(r.left()+12, r.top()+26, t.name);
    p.setFont(roleF); p.setPen(QColor("#CCCCCC")); p.drawText(r.left()+12, r.top()+44, t.role);
}

void Compositor::drawFullscreen(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog)
{
    Q_UNUSED(prog) // Fade via painter opacity
    QRectF gradR(0, fs.height()-150, fs.width(), 150);
    QLinearGradient grad(gradR.topLeft(), gradR.bottomLeft());
    grad.setColorAt(0.0, QColor(0,0,0,0)); grad.setColorAt(0.4, QColor(0,0,0,40)); grad.setColorAt(1.0, QColor(0,0,0,190));
    p.fillRect(gradR, grad);
    double medS=76, medX=fs.width()/2.0-210, medY=fs.height()-108;
    QRectF med(medX, medY, medS, medS);
    drawSoftShadow(p, med, medS/2, 8, QColor(0,0,0,60));
    p.setBrush(QColor("#2A2A30")); p.setPen(Qt::NoPen); p.drawEllipse(med);
    p.setPen(QPen(m_accentColor,2)); p.setBrush(Qt::NoBrush); p.drawEllipse(med.adjusted(1,1,-1,-1));
    QFont iF("Helvetica Neue", 30, QFont::Bold); p.setFont(iF); p.setPen(Qt::white); p.drawText(med, Qt::AlignCenter, t.name.left(1));
    double tx=medX+medS+16;
    QFont nF("Helvetica Neue", 30, QFont::Bold); p.setFont(nF); p.setPen(Qt::white); p.drawText(QPointF(tx,medY+34), t.name);
    QFont rF("Helvetica Neue", 14); p.setFont(rF); p.setPen(QColor("#CCCCCC")); p.drawText(QPointF(tx,medY+56), t.role);
}

void Compositor::drawBreaking(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog)
{
    // Banner top — always full (no animation, it's urgent)
    QRectF banner(0, fs.height()*0.06, fs.width(), 48);
    QLinearGradient bg(banner.topLeft(), banner.topRight());
    bg.setColorAt(0.0, QColor(200,0,0)); bg.setColorAt(0.5, QColor(220,0,0)); bg.setColorAt(1.0, QColor(180,0,0));
    p.fillRect(banner, bg);
    QFont bF("Helvetica Neue", 20, QFont::Bold); bF.setLetterSpacing(QFont::AbsoluteSpacing, 4);
    p.setFont(bF); p.setPen(Qt::white); p.drawText(banner, Qt::AlignCenter, "BREAKING NEWS");

    drawBFM(p, t, fs, prog); // Nameplate animates normally

    QRectF ticker(0, fs.height()-fs.height()*0.055, fs.width(), 34);
    p.fillRect(ticker, QColor("#CC0000"));
    QFont tF("Helvetica Neue", 13, QFont::Bold); p.setFont(tF); p.setPen(Qt::white);
    p.drawText(ticker.adjusted(20,0,0,0), Qt::AlignVCenter,
        QStringLiteral("DERNIÈRE MINUTE  \u2014  %1  \u2014  %2").arg(t.name, t.role));
}

void Compositor::drawDefault(QPainter& p, const TalentOverlay& t, const QSize& fs, double prog) { drawBFM(p, t, fs, prog); }

} // namespace prestige
