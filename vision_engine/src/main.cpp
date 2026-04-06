// ============================================================
// Prestige AI — Vision Engine
// Copyright (c) 2024-2026 Prestige Technologie Company
// All rights reserved.
//
//   VideoCapture ──→ FrameSender ──→ Python AI
//       │                               │
//       │ frameCaptured()          ZMQ :5555
//       │                               │
//       ▼                               ▼
//   Compositor ←──── TalentStore ←── ZmqSubscriber
//       │
//       │ composited frame (video + overlay fused)
//       │
//       ├──→ OutputRouter ──→ RTMP / SRT / File / NDI
//       │
//       └──→ Live output display (single screen)
// ============================================================

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QQuickImageProvider>
#include <QTimer>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <atomic>

#ifdef PRESTIGE_HAVE_ZMQ
#include <zmq.h>
#endif

#include "TalentData.h"
#include "ZmqSubscriber.h"
#include "OverlayRenderer.h"
#include "VideoCapture.h"
#include "FrameSender.h"
#include "Compositor.h"
#include "VideoEncoder.h"
#include "OutputRouter.h"
#include "PreviewSender.h"
#include "AiPipeline.h"
#include "WhisperEngine.h"
#include "TwitchChat.h"
#include <QDir>

// ── Live output image provider (QML displays composited frame) ──

class LiveOutputProvider : public QQuickImageProvider {
public:
    LiveOutputProvider() : QQuickImageProvider(QQuickImageProvider::Image) {}

    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override {
        Q_UNUSED(id) Q_UNUSED(requestedSize)
        QMutexLocker lock(&m_mutex);
        if (size) *size = m_frame.size();
        return m_frame;
    }

    void updateFrame(const QImage& frame) {
        QMutexLocker lock(&m_mutex);
        m_frame = frame;
    }

private:
    QImage m_frame;
    QMutex m_mutex;
};

// ── Main ────────────────────────────────────────────────────

int main(int argc, char* argv[])
{
    QQuickStyle::setStyle("Basic");

    QGuiApplication app(argc, argv);
    app.setOrganizationName("Prestige Technologie Company");
    app.setOrganizationDomain("prestigetech.com");
    app.setApplicationName("Prestige AI");
    app.setApplicationVersion("1.0.0");

    // ── 1. Video Capture ─────────────────────────────────────
    prestige::VideoCapture capture;

    // ── 2. AI Pipeline (C++ ONNX Runtime — replaces Python) ──
    prestige::TalentStore talentStore;
    prestige::ai::AiPipeline aiPipeline;

    // Find models directory
    QString modelsDir = QCoreApplication::applicationDirPath() + "/models/buffalo_l";
    if (!QDir(modelsDir).exists())
        modelsDir = QCoreApplication::applicationDirPath() + "/../Resources/models/buffalo_l";
    if (!QDir(modelsDir).exists())
        modelsDir = QCoreApplication::applicationDirPath() + "/../../ai_engine/models/buffalo_l"; // dev layout

    QString talentsDb = QDir::homePath() + "/.prestige-ai/talents.json";

    if (aiPipeline.initialize(modelsDir, talentsDb)) {
        qInfo() << "[VisionEngine] AI Pipeline initialized — detection ACTIVE";
    } else {
        qInfo() << "[VisionEngine] AI Pipeline not available — overlay-only mode";
    }

    // Connect: capture → AI pipeline → talent store
    QObject::connect(&capture, &prestige::VideoCapture::frameCaptured,
        &aiPipeline, &prestige::ai::AiPipeline::processFrame);

    QObject::connect(&aiPipeline, &prestige::ai::AiPipeline::detectionsUpdated,
        [&talentStore](const QList<prestige::ai::TrackedFace>& faces, const QString&, int) {
            QList<prestige::TalentOverlay> overlays;
            for (const auto& f : faces) {
                prestige::TalentOverlay ov;
                ov.id = f.id;
                ov.name = f.name;
                ov.role = f.role;
                ov.confidence = f.confidence;
                ov.showOverlay = f.showOverlay;
                ov.overlayStyle = f.overlayStyle;
                ov.bbox = f.smoothedBbox.isValid() ? f.smoothedBbox : f.bbox;
                overlays.append(ov);
            }
            talentStore.update(overlays);
        });

    // Keep ZMQ subscriber for backward compat (Control Room still subscribes :5555)
    prestige::ZmqSubscriber zmqSub(talentStore);
    // Don't start it — AI is now in-process
    // zmqSub.start();

    // Also keep FrameSender disabled
    // prestige::FrameSender frameSender;

    // ── 4. Compositor (video + overlay → output frame) ───────
    prestige::Compositor compositor;

    // ── Whisper Subtitles (C++) ──────────────────────────────
    prestige::ai::WhisperEngine whisperEngine;
    QString whisperModel = modelsDir + "/../whisper/ggml-base.bin";
    whisperEngine.initialize(whisperModel);

    QObject::connect(&whisperEngine, &prestige::ai::WhisperEngine::subtitleReady,
        [&compositor](const QString& text, const QString&, double) {
            compositor.setSubtitleText(text);
        });

    // ── Social Chat (C++) ─────────────────────────────────────
    prestige::ai::TwitchChat twitchChat;
    // Twitch/YouTube chat → Compositor social chat overlay
    QStringList chatBuffer;
    QObject::connect(&twitchChat, &prestige::ai::TwitchChat::messageReceived,
        [&compositor, &chatBuffer](const QString& author, const QString& msg, const QString&) {
            chatBuffer.append(author + ": " + msg);
            if (chatBuffer.size() > 5) chatBuffer.removeFirst(); // Keep last 5 messages
            compositor.setSocialChatMessages(chatBuffer);
            qInfo() << "[Chat]" << author << ":" << msg;
        });

    // ── 5. Output Router (RTMP/SRT/File) ─────────────────────
    prestige::OutputRouter outputRouter;

    // ── 5b. Audio passthrough: Whisper audio capture → OutputRouter ──
    // The WhisperEngine captures audio at 16kHz mono. For broadcast output,
    // we need a separate 48kHz stereo audio capture routed to the encoder.
    // For now, connect the whisper audio buffer as a source.

    // ── 4b. Config receiver from Control Room (:5559) ────────
    // Listens for style/animation changes from the director
#ifdef PRESTIGE_HAVE_ZMQ
    QThread configThread;
    bool configRunning = true;
    QObject* configWorker = new QObject;
    configWorker->moveToThread(&configThread);

    QObject::connect(&configThread, &QThread::started, configWorker, [&compositor, &outputRouter, &capture, &configRunning, configWorker]() {
        void* ctx = zmq_ctx_new();
        void* sock = zmq_socket(ctx, ZMQ_SUB);
        zmq_setsockopt(sock, ZMQ_SUBSCRIBE, "", 0);
        int conflate = 1;
        zmq_setsockopt(sock, ZMQ_CONFLATE, &conflate, sizeof(conflate));
        int timeout = 200;
        zmq_setsockopt(sock, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
        zmq_connect(sock, "tcp://127.0.0.1:5559");

        char buf[8192];
        while (configRunning) {
            int n = zmq_recv(sock, buf, sizeof(buf) - 1, 0);
            if (n <= 0) continue;

            buf[n] = '\0';
            auto doc = QJsonDocument::fromJson(QByteArray(buf, n));
            auto obj = doc.object();

            QString styleId = obj["style_id"].toString();
            QString animType = obj["animation_type"].toString();
            int enterFrames = obj["anim_enter_frames"].toInt(15);
            int exitFrames = obj["anim_exit_frames"].toInt(8);

            if (!styleId.isEmpty()) {
                QMetaObject::invokeMethod(&compositor, [&compositor, styleId]() {
                    compositor.setStyleId(styleId);
                }, Qt::QueuedConnection);
            }
            if (!animType.isEmpty()) {
                QMetaObject::invokeMethod(&compositor, [&compositor, animType, enterFrames, exitFrames]() {
                    compositor.setAnimationType(animType);
                    compositor.setAnimEnterFrames(enterFrames);
                    compositor.setAnimExitFrames(exitFrames);
                }, Qt::QueuedConnection);
            }

            // Accent color + background opacity
            QString accentColor = obj["accent_color"].toString("#E30613");
            double bgOpacity = obj["bg_opacity"].toDouble(0.82);
            QMetaObject::invokeMethod(&compositor, [&compositor, accentColor, bgOpacity]() {
                compositor.setAccentColor(QColor(accentColor));
                compositor.setBgOpacity(bgOpacity);
            }, Qt::QueuedConnection);

            // ── AE Effects ─────────────────────────────────
            QString lottiePreset = obj["lottie_preset"].toString();
            if (lottiePreset.isEmpty()) lottiePreset = "title_01";
            QString easingCurve = obj["easing_curve"].toString();
            QString blendMode = obj["overlay_blend_mode"].toString();
            QString aeEffectId = obj["ae_effect_id"].toString();
            double aeIntensity = obj["ae_effect_intensity"].toDouble(0.5);
            double aeParam1 = obj["ae_effect_param1"].toDouble(0.5);
            double aeParam2 = obj["ae_effect_param2"].toDouble(0.5);
            QString aeColor1 = obj["ae_effect_color1"].toString("#E30613");
            QString aeColor2 = obj["ae_effect_color2"].toString("#FFFFFF");
            bool wiggleOn = obj["wiggle_enabled"].toBool(false);
            double wiggleFreq = obj["wiggle_freq"].toDouble(3.0);
            double wiggleAmp = obj["wiggle_amp"].toDouble(5.0);

            QMetaObject::invokeMethod(&compositor, [&compositor, lottiePreset, easingCurve, blendMode, aeEffectId,
                                                      aeIntensity, aeParam1, aeParam2, aeColor1, aeColor2,
                                                      wiggleOn, wiggleFreq, wiggleAmp]() {
                if (!lottiePreset.isEmpty()) compositor.setLottiePreset(lottiePreset);
                if (!easingCurve.isEmpty()) compositor.setEasingCurve(easingCurve);
                if (!blendMode.isEmpty())   compositor.setOverlayBlendMode(blendMode);
                compositor.setAeEffectId(aeEffectId);
                compositor.setAeEffectIntensity(aeIntensity);
                compositor.setAeEffectParam1(aeParam1);
                compositor.setAeEffectParam2(aeParam2);
                compositor.setAeEffectColor1(QColor(aeColor1));
                compositor.setAeEffectColor2(QColor(aeColor2));
                compositor.setWiggleEnabled(wiggleOn);
                compositor.setWiggleFreq(wiggleFreq);
                compositor.setWiggleAmp(wiggleAmp);
            }, Qt::QueuedConnection);

            // Input source — open/switch video source dynamically
            QString inputType = obj["input_type"].toString();
            QString inputSource = obj["input_source"].toString();
            if (!inputType.isEmpty()) {
                QMetaObject::invokeMethod(&capture, [&capture, inputType, inputSource]() {
                    capture.openSource(inputType, inputSource);
                }, Qt::QueuedConnection);
            }

            // Ticker config
            bool tickerVisible = obj["ticker_visible"].toBool(false);
            QString tickerText = obj["ticker_text"].toString();
            QMetaObject::invokeMethod(&compositor, [&compositor, tickerVisible, tickerText]() {
                compositor.setTickerVisible(tickerVisible);
                if (!tickerText.isEmpty())
                    compositor.setTickerText(tickerText);
            }, Qt::QueuedConnection);

            // Subtitle config
            bool subVisible = obj["subtitle_visible"].toBool(false);
            int subFontSize = obj["subtitle_font_size"].toInt(18);
            QString subPosition = obj["subtitle_position"].toString("bottom");
            double subBgOpacity = obj["subtitle_bg_opacity"].toDouble(0.6);
            QString subTextColor = obj["subtitle_text_color"].toString("#FFFFFF");
            QMetaObject::invokeMethod(&compositor, [&compositor, subVisible, subFontSize, subPosition, subBgOpacity, subTextColor]() {
                compositor.setSubtitleVisible(subVisible);
                compositor.setSubtitleFontSize(subFontSize);
                compositor.setSubtitlePosition(subPosition);
                compositor.setSubtitleBgOpacity(subBgOpacity);
                compositor.setSubtitleTextColor(QColor(subTextColor));
            }, Qt::QueuedConnection);

            // Recording config
            bool recActive = obj["recording_active"].toBool(false);
            QString recPath = obj["recording_path"].toString();

            // Output destinations
            bool outRtmp = obj["output_rtmp"].toBool(false);
            QString rtmpUrl = obj["rtmp_url"].toString();
            QString rtmpKey = obj["rtmp_key"].toString();
            bool outSrt = obj["output_srt"].toBool(false);
            QString srtUrl = obj["srt_url"].toString();
            bool outNdi = obj["output_ndi"].toBool(false);
            bool outSdi = obj["output_sdi"].toBool(false);
            int outFps = obj["output_fps"].toInt(25);
            int outBitrate = obj["output_bitrate"].toInt(8);

            // Social media RTMP outputs
            QStringList socialUrls;
            auto socialArr = obj["social_outputs"].toArray();
            if (!socialArr.isEmpty())
                qInfo() << "[VisionEngine] Social outputs:" << socialArr.size() << "destinations";
            for (const auto& v : socialArr) {
                QString url = v.toObject()["url"].toString();
                if (!url.isEmpty())
                    socialUrls.append(url);
            }

            QMetaObject::invokeMethod(&outputRouter, [&outputRouter, recActive, recPath,
                                       outRtmp, rtmpUrl, rtmpKey, outSrt, srtUrl,
                                       outNdi, outSdi, outFps, outBitrate, socialUrls]() {
                // File recording (OutputType::File = 6)
                if (recActive && !recPath.isEmpty())
                    outputRouter.addOutput(6, recPath, outBitrate, outFps);
                else
                    outputRouter.removeOutput(6);

                // Primary RTMP output (OutputType::RTMP = 3)
                if (outRtmp && !rtmpUrl.isEmpty() && !rtmpKey.isEmpty())
                    outputRouter.addOutput(3, rtmpUrl + rtmpKey, outBitrate, outFps);
                else
                    outputRouter.removeOutput(3);

                // SRT output (OutputType::SRT = 4)
                if (outSrt && !srtUrl.isEmpty())
                    outputRouter.addOutput(4, srtUrl, outBitrate, outFps);
                else
                    outputRouter.removeOutput(4);

                // NDI output (OutputType::NDI = 2)
                if (outNdi)
                    outputRouter.addOutput(2, "Prestige AI", 0, outFps);
                else
                    outputRouter.removeOutput(2);

                // SDI output (OutputType::SDI = 0)
                if (outSdi)
                    outputRouter.addOutput(0, "DeckLink", 0, outFps);
                else
                    outputRouter.removeOutput(0);

                // Social media RTMP outputs (100-106) — only reconnect if URL changed
                int socialBitrate = qMax(4, outBitrate - 2);
                for (int i = 0; i < 7; ++i) {
                    int outId = 100 + i;
                    if (i < socialUrls.size() && !socialUrls[i].isEmpty()) {
                        // addOutput skips if same URL already active
                        outputRouter.addOutput(outId, socialUrls[i], socialBitrate, outFps);
                    } else {
                        outputRouter.removeOutput(outId);
                    }
                }
            }, Qt::QueuedConnection);

            // Countdown config
            bool cdActive = obj["countdown_active"].toBool(false);
            int cdSeconds = obj["countdown_seconds"].toInt(0);
            QString cdLabel = obj["countdown_label"].toString();
            QString cdText;
            if (cdActive && cdSeconds > 0) {
                int mm = cdSeconds / 60;
                int ss = cdSeconds % 60;
                cdText = QStringLiteral("%1:%2")
                    .arg(mm, 2, 10, QLatin1Char('0'))
                    .arg(ss, 2, 10, QLatin1Char('0'));
                if (!cdLabel.isEmpty())
                    cdText = cdLabel + " " + cdText;
            }
            QMetaObject::invokeMethod(&compositor, [&compositor, cdActive, cdText]() {
                compositor.setCountdownVisible(cdActive);
                compositor.setCountdownText(cdText);
            }, Qt::QueuedConnection);

            // QR Code config
            bool qrVisible = obj["qr_visible"].toBool(false);
            QString qrUrl = obj["qr_url"].toString();
            QString qrPosition = obj["qr_position"].toString("bottom_right");
            QMetaObject::invokeMethod(&compositor, [&compositor, qrVisible, qrUrl, qrPosition]() {
                compositor.setQrCodeVisible(qrVisible);
                compositor.setQrCodeUrl(qrUrl);
                compositor.setQrCodePosition(qrPosition);
            }, Qt::QueuedConnection);

            // Social chat config
            bool chatVisible = obj["social_chat_visible"].toBool(false);
            QStringList chatMsgs;
            auto chatArr = obj["social_chat_messages"].toArray();
            for (const auto& v : chatArr)
                chatMsgs.append(v.toString());
            QMetaObject::invokeMethod(&compositor, [&compositor, chatVisible, chatMsgs]() {
                compositor.setSocialChatVisible(chatVisible);
                compositor.setSocialChatMessages(chatMsgs);
            }, Qt::QueuedConnection);

            // Channel name
            QString channelName = obj["channel_name"].toString();
            QMetaObject::invokeMethod(&compositor, [&compositor, channelName]() {
                compositor.setChannelName(channelName);
            }, Qt::QueuedConnection);

            // Channel logo — load image from path and set on compositor
            QString logoPath = obj["channel_logo_path"].toString();
            static QString lastLogoPath; // track to avoid reloading same image
            if (!logoPath.isEmpty() && logoPath != lastLogoPath) {
                lastLogoPath = logoPath;
                QMetaObject::invokeMethod(&compositor, [&compositor, logoPath]() {
                    QImage logoImg(logoPath);
                    if (!logoImg.isNull()) {
                        compositor.setLogoFrames({logoImg});
                        compositor.setLogoVisible(true);
                        qInfo() << "[VisionEngine] Channel logo loaded:" << logoPath;
                    } else {
                        compositor.setLogoVisible(false);
                        qWarning() << "[VisionEngine] Failed to load logo:" << logoPath;
                    }
                }, Qt::QueuedConnection);
            } else if (logoPath.isEmpty()) {
                QMetaObject::invokeMethod(&compositor, [&compositor]() {
                    compositor.setLogoVisible(false);
                }, Qt::QueuedConnection);
            }

            // Design Templates
            QString npDesign = obj["nameplate_design"].toString("np_broadcast_news");
            QString ttDesign = obj["title_design"].toString("tt_broadcast");
            QString chDesign = obj["channel_design"].toString("ch_rectangle_red");
            QString tkDesign = obj["ticker_design"].toString("tk_news_red");
            QString sbDesign = obj["scoreboard_design"].toString("sb_glass_dark");
            QString ckDesign = obj["clock_design"].toString("ck_shadow_only");
            QString wtDesign = obj["weather_design"].toString("wt_shadow_only");
            QMetaObject::invokeMethod(&compositor, [&compositor, npDesign, ttDesign, chDesign, tkDesign, sbDesign, ckDesign, wtDesign]() {
                compositor.setNameplateDesign(npDesign);
                compositor.setTitleDesign(ttDesign);
                compositor.setChannelDesign(chDesign);
                compositor.setTickerDesign(tkDesign);
                compositor.setScoreboardDesign(sbDesign);
                compositor.setClockDesign(ckDesign);
                compositor.setWeatherDesign(wtDesign);
            }, Qt::QueuedConnection);

            // Sport system
            QString teamLogoA = obj["team_logo_a"].toString();
            QString teamLogoB = obj["team_logo_b"].toString();
            bool goalActive = obj["goal_anim_active"].toBool(false);
            QString goalText = obj["goal_anim_text"].toString("GOAL!");
            QString goalTeam = obj["goal_anim_team"].toString();
            QString goalPlayer = obj["goal_anim_player"].toString();
            QString goalEffect = obj["goal_anim_effect"].toString("kinetic_pop");
            int goalDuration = obj["goal_anim_duration"].toInt(5);
            QString sportEvt = obj["sport_event"].toString();
            QMetaObject::invokeMethod(&compositor, [&compositor, teamLogoA, teamLogoB, goalActive, goalText, goalTeam, goalPlayer, goalEffect, goalDuration, sportEvt]() {
                compositor.setTeamLogoA(teamLogoA);
                compositor.setTeamLogoB(teamLogoB);
                compositor.setGoalAnim(goalActive, goalText, goalTeam, goalPlayer, goalEffect, goalDuration);
                if (!sportEvt.isEmpty()) compositor.setSportEvent(sportEvt);
            }, Qt::QueuedConnection);

            // Virtual Studio config
            bool vsEnabled = obj["virtual_studio_enabled"].toBool(false);
            int vsStudioId = obj["virtual_studio_id"].toInt(0);
            bool vsChromaKey = obj["chroma_key_enabled"].toBool(false);
            QString vsChromaColor = obj["chroma_key_color"].toString("green");
            double vsChromaTol = obj["chroma_key_tolerance"].toDouble(0.35);
            double vsChromaSmooth = obj["chroma_key_smooth"].toDouble(0.05);
            // Virtual Studio customization
            QString vsPrimary = obj["vs_primary_color"].toString();
            QString vsSecondary = obj["vs_secondary_color"].toString();
            QString vsAccent = obj["vs_accent_color"].toString();
            QString vsFloor = obj["vs_floor_color"].toString();
            double vsLightInt = obj["vs_light_intensity"].toDouble(1.0);
            bool vsAnimEnabled = obj["vs_animations_enabled"].toBool(true);
            QString vsCustomBg = obj["vs_custom_background"].toString();

            QMetaObject::invokeMethod(&compositor, [&compositor, vsEnabled, vsStudioId, vsChromaKey, vsChromaColor, vsChromaTol, vsChromaSmooth,
                                       vsPrimary, vsSecondary, vsAccent, vsFloor, vsLightInt, vsAnimEnabled, vsCustomBg]() {
                compositor.setVirtualStudioEnabled(vsEnabled);
                compositor.setVirtualStudioId(vsStudioId);
                compositor.setChromaKeyEnabled(vsChromaKey);
                compositor.setChromaKeyColor(vsChromaColor);
                compositor.setChromaKeyTolerance(vsChromaTol);
                compositor.setChromaKeySmooth(vsChromaSmooth);
                // Customization
                compositor.setVsPrimaryColor(vsPrimary.isEmpty() ? QColor() : QColor(vsPrimary));
                compositor.setVsSecondaryColor(vsSecondary.isEmpty() ? QColor() : QColor(vsSecondary));
                compositor.setVsAccentColor(vsAccent.isEmpty() ? QColor() : QColor(vsAccent));
                compositor.setVsFloorColor(vsFloor.isEmpty() ? QColor() : QColor(vsFloor));
                compositor.setVsLightIntensity(vsLightInt);
                compositor.setVsAnimationsEnabled(vsAnimEnabled);
                compositor.setVsCustomBackground(vsCustomBg);
            }, Qt::QueuedConnection);

            // Overlay scale factors + offsets
            double npScale = obj["nameplate_scale"].toDouble(1.0);
            int npOffX = obj["nameplate_offset_x"].toInt(0);
            int npOffY = obj["nameplate_offset_y"].toInt(0);
            int npFontSize = obj["nameplate_font_size"].toInt(28);
            QString npTextColor = obj["nameplate_text_color"].toString("#FFFFFF");
            double sbScale = obj["scoreboard_scale"].toDouble(1.0);
            double wScale = obj["weather_scale"].toDouble(1.0);
            double ckScale = obj["clock_scale"].toDouble(1.0);
            double cdScale = obj["countdown_scale"].toDouble(1.0);
            double qrScale = obj["qr_code_scale"].toDouble(1.0);

            // Ticker appearance
            int tkFontSize = obj["ticker_font_size"].toInt(14);
            QString tkBgColor = obj["ticker_bg_color"].toString("#CC0000");
            QString tkTextColor = obj["ticker_text_color"].toString("#FFFFFF");
            int tkSpeed = obj["ticker_speed"].toInt(2);

            // Clock config
            bool clockVis = obj["clock_visible"].toBool(false);
            QString clockFmt = obj["clock_format"].toString("HH:mm:ss");

            // Overlay offsets
            int stOffX = obj["show_title_offset_x"].toInt(0);
            int stOffY = obj["show_title_offset_y"].toInt(0);
            int tkOffY = obj["ticker_offset_y"].toInt(0);
            QString tkPosition = obj["ticker_position"].toString("bottom");
            int subOffX = obj["subtitle_offset_x"].toInt(0);
            int subOffY = obj["subtitle_offset_y"].toInt(0);
            int cdOffX = obj["countdown_offset_x"].toInt(0);
            int cdOffY = obj["countdown_offset_y"].toInt(0);
            int ckOffX = obj["clock_offset_x"].toInt(0);
            int ckOffY = obj["clock_offset_y"].toInt(0);
            int qrOffX = obj["qr_offset_x"].toInt(0);
            int qrOffY = obj["qr_offset_y"].toInt(0);
            int sbOffX = obj["scoreboard_offset_x"].toInt(0);
            int sbOffY = obj["scoreboard_offset_y"].toInt(0);
            int wOffX = obj["weather_offset_x"].toInt(0);
            int wOffY = obj["weather_offset_y"].toInt(0);
            int logoOffX = obj["channel_logo_offset_x"].toInt(0);
            int logoOffY = obj["channel_logo_offset_y"].toInt(0);
            int nameOffX = obj["channel_name_offset_x"].toInt(0);
            int nameOffY = obj["channel_name_offset_y"].toInt(0);

            // Scoreboard data
            bool scoreVis = obj["scoreboard_visible"].toBool(false);
            QString sbTeamA = obj["scoreboard_team_a"].toString("HOME");
            QString sbTeamB = obj["scoreboard_team_b"].toString("AWAY");
            int sbScoreA = obj["scoreboard_score_a"].toInt(0);
            int sbScoreB = obj["scoreboard_score_b"].toInt(0);
            QString sbColA = obj["scoreboard_color_a"].toString("#CC0000");
            QString sbColB = obj["scoreboard_color_b"].toString("#0066CC");
            QString sbPos = obj["scoreboard_position"].toString("top_left");
            QString sbTime = obj["scoreboard_match_time"].toString("00:00");
            int sbPeriod = obj["scoreboard_period"].toInt(1);
            int sbYellowA = obj["scoreboard_yellow_a"].toInt(0);
            int sbYellowB = obj["scoreboard_yellow_b"].toInt(0);
            int sbRedA = obj["scoreboard_red_a"].toInt(0);
            int sbRedB = obj["scoreboard_red_b"].toInt(0);

            // Weather data
            bool weatherVis = obj["weather_visible"].toBool(false);
            QString wCity = obj["weather_city"].toString();
            double wTemp = obj["weather_temperature"].toDouble(0);
            QString wUnit = obj["weather_unit"].toString("\u00B0C");
            QString wIcon = obj["weather_condition_icon"].toString();

            QMetaObject::invokeMethod(&compositor, [&compositor,
                npScale, npOffX, npOffY, npFontSize, npTextColor, sbScale, wScale, ckScale, cdScale, qrScale,
                tkFontSize, tkBgColor, tkTextColor, tkSpeed,
                clockVis, clockFmt,
                stOffX, stOffY, tkOffY, tkPosition, subOffX, subOffY,
                cdOffX, cdOffY, ckOffX, ckOffY, qrOffX, qrOffY,
                sbOffX, sbOffY, wOffX, wOffY, logoOffX, logoOffY, nameOffX, nameOffY,
                scoreVis, sbTeamA, sbTeamB, sbScoreA, sbScoreB, sbColA, sbColB, sbPos, sbTime, sbPeriod,
                sbYellowA, sbYellowB, sbRedA, sbRedB,
                weatherVis, wCity, wTemp, wUnit, wIcon]() {
                // Scales
                compositor.setNameplateScale(npScale);
                compositor.setNameplateOffset(npOffX, npOffY);
                compositor.setNameplateFontSize(npFontSize);
                compositor.setNameplateTextColor(QColor(npTextColor));
                compositor.setScoreboardScale(sbScale);
                compositor.setWeatherScale(wScale);
                compositor.setClockScale(ckScale);
                compositor.setCountdownScale(cdScale);
                compositor.setQrCodeScale(qrScale);
                // Ticker
                compositor.setTickerFontSize(tkFontSize);
                compositor.setTickerBgColor(QColor(tkBgColor));
                compositor.setTickerTextColor(QColor(tkTextColor));
                compositor.setTickerSpeed(tkSpeed);
                // Clock
                compositor.setClockVisible(clockVis);
                compositor.setClockFormat(clockFmt);
                // Offsets
                compositor.setShowTitleOffset(stOffX, stOffY);
                compositor.setTickerOffsetY(tkOffY);
                compositor.setTickerPosition(tkPosition);
                compositor.setSubtitleOffset(subOffX, subOffY);
                compositor.setCountdownOffset(cdOffX, cdOffY);
                compositor.setClockOffset(ckOffX, ckOffY);
                compositor.setQrCodeOffset(qrOffX, qrOffY);
                compositor.setScoreboardOffset(sbOffX, sbOffY);
                compositor.setWeatherOffset(wOffX, wOffY);
                compositor.setLogoOffset(logoOffX, logoOffY);
                compositor.setNameOffset(nameOffX, nameOffY);
                // Scoreboard
                compositor.setScoreboardVisible(scoreVis);
                compositor.setScoreboardData(sbTeamA, sbTeamB, sbScoreA, sbScoreB,
                                             QColor(sbColA), QColor(sbColB), sbPos, sbTime, sbPeriod);
                compositor.setScoreboardCards(sbYellowA, sbYellowB, sbRedA, sbRedB);
                // Weather
                compositor.setWeatherVisible(weatherVis);
                compositor.setWeatherData(wCity, wTemp, wUnit, wIcon);
            }, Qt::QueuedConnection);

            // RTL layout
            bool layoutRtl = obj["layout_rtl"].toBool(false);
            QMetaObject::invokeMethod(&compositor, [&compositor, layoutRtl]() {
                compositor.setLayoutRtl(layoutRtl);
            }, Qt::QueuedConnection);

            // Channel branding
            QString logoPos = obj["channel_logo_position"].toString("top_right");
            int logoSize = obj["channel_logo_size"].toInt(60);
            bool showNameText = obj["show_channel_name_text"].toBool(true);
            bool keepLogoDuringAds = obj["keep_logo_during_ads"].toBool(true);

            // Show title
            QString showTitleStr = obj["show_title"].toString();
            QString showSubtitle = obj["show_subtitle"].toString();
            bool showTitleVisible = obj["show_title_visible"].toBool(false);
            QString showTitlePos = obj["show_title_position"].toString("bottom_left");

            // Show title design & animation
            QString stShape = obj["show_title_shape"].toString("rectangle");
            QString stBgColor = obj["show_title_bg_color"].toString("#1A1A2E");
            QString stTextColor = obj["show_title_text_color"].toString("#FFFFFF");
            QString stBorderColor = obj["show_title_border_color"].toString("#5B4FDB");
            int stFontSize = obj["show_title_font_size"].toInt(16);
            QString stEntryAnim = obj["show_title_entry_anim"].toString("slide_up");
            QString stLoopAnim = obj["show_title_loop_anim"].toString("none");

            // Visibility
            bool talentVisible = obj["talent_nameplate_visible"].toBool(true);
            bool bypassActive = obj["bypass_active"].toBool(false);

            // Channel name design
            QString nameShape = obj["channel_name_shape"].toString("rectangle");
            QString nameBgColor = obj["channel_name_bg_color"].toString("#CC0000");
            QString nameTextColor = obj["channel_name_text_color"].toString("#FFFFFF");
            QString nameBorderColor = obj["channel_name_border_color"].toString("#FFFFFF");
            int nameFontSize = obj["channel_name_font_size"].toInt(14);

            // Branding animations
            QString logoEntryAnim = obj["logo_entry_anim"].toString("fade");
            QString nameEntryAnim = obj["name_entry_anim"].toString("slide_left");
            QString logoLoopAnim = obj["logo_loop_anim"].toString("pulse");
            QString nameLoopAnim = obj["name_loop_anim"].toString("none");
            bool emissionStarting = obj["emission_starting"].toBool(false);

            QMetaObject::invokeMethod(&compositor, [&compositor, logoPos, logoSize, showNameText, keepLogoDuringAds,
                                                     showTitleStr, showSubtitle, showTitleVisible, showTitlePos,
                                                     stShape, stBgColor, stTextColor, stBorderColor, stFontSize, stEntryAnim, stLoopAnim,
                                                     talentVisible, bypassActive,
                                                     nameShape, nameBgColor, nameTextColor, nameBorderColor, nameFontSize,
                                                     logoEntryAnim, nameEntryAnim, logoLoopAnim, nameLoopAnim,
                                                     emissionStarting]() {
                compositor.setChannelLogoPosition(logoPos);
                compositor.setChannelLogoSize(logoSize);
                compositor.setShowChannelNameText(showNameText);
                compositor.setKeepLogoDuringAds(keepLogoDuringAds);
                compositor.setShowTitle(showTitleStr, showSubtitle);
                compositor.setShowTitleVisible(showTitleVisible);
                compositor.setShowTitlePosition(showTitlePos);
                compositor.setShowTitleShape(stShape);
                compositor.setShowTitleColors(QColor(stBgColor), QColor(stTextColor), QColor(stBorderColor));
                compositor.setShowTitleFontSize(stFontSize);
                compositor.setShowTitleEntryAnim(stEntryAnim);
                compositor.setShowTitleLoopAnim(stLoopAnim);
                compositor.setTalentNameplateVisible(talentVisible);
                compositor.setBypassActive(bypassActive);

                // Channel name design
                compositor.setChannelNameShape(nameShape);
                compositor.setChannelNameColors(QColor(nameBgColor), QColor(nameTextColor), QColor(nameBorderColor));
                compositor.setChannelNameFontSize(nameFontSize);

                // Branding animations
                compositor.setLogoEntryAnim(logoEntryAnim);
                compositor.setNameEntryAnim(nameEntryAnim);
                compositor.setLogoLoopAnim(logoLoopAnim);
                compositor.setNameLoopAnim(nameLoopAnim);

                // Reset entry animations at emission start
                if (emissionStarting)
                    compositor.resetBrandingAnimations();
            }, Qt::QueuedConnection);

            qInfo() << "[VisionEngine] Config received: style=" << styleId << "anim=" << animType << "lottie=" << lottiePreset;
        }

        zmq_close(sock);
        zmq_ctx_destroy(ctx);
        configWorker->deleteLater();
    });
    QObject::connect(&configThread, &QThread::finished, configWorker, &QObject::deleteLater);
    configThread.start();
    qInfo() << "[VisionEngine] Config listener started (:5559)";
#endif

    // ── 5b. Preview Sender → Control Room (:5558) ────────────
    prestige::PreviewSender previewSender;
    previewSender.start();

    // ── 6. QML Engine ────────────────────────────────────────
    QQmlApplicationEngine engine;

    auto* liveProvider = new LiveOutputProvider;
    engine.addImageProvider("liveoutput", liveProvider);

    prestige::OverlayRenderer overlayRenderer;
    engine.rootContext()->setContextProperty("overlayRenderer", &overlayRenderer);
    engine.rootContext()->setContextProperty("videoCapture", &capture);
    engine.rootContext()->setContextProperty("outputRouter", &outputRouter);
    engine.rootContext()->setContextProperty("zmqSubscriber", &zmqSub);

    // ── 7. Connect ZMQ → OverlayRenderer ─────────────────────
    QObject::connect(&zmqSub, &prestige::ZmqSubscriber::messageReceived,
        [&overlayRenderer, &talentStore](const prestige::DetectionMessage&) {
            overlayRenderer.updateTalents(talentStore.snapshot());
        });

    // ── 8. THE PIPELINE: Capture → Composite → Output ────────
    // Frame-dropping pipeline: process only the latest frame, skip backlog.
    // This prevents slow-motion when compositor can't keep up with capture rate.
    QImage latestFrame;
    QMutex frameMutex;
    std::atomic<bool> compositing{false};
    std::atomic<int64_t> capturedCount{0};
    std::atomic<int64_t> processedCount{0};

    // Step 1: Capture stores latest frame (never blocks)
    QObject::connect(&capture, &prestige::VideoCapture::frameCaptured,
        [&latestFrame, &frameMutex, &capturedCount](
            const QImage& frame, qint64 /*frameId*/, qint64 /*ts*/)
        {
            QMutexLocker lock(&frameMutex);
            latestFrame = frame; // Always keep only the latest
            capturedCount++;
        });

    // Step 2: Timer drives compositing at steady rate (not tied to capture rate)
    QTimer compositeTimer;
    QObject::connect(&compositeTimer, &QTimer::timeout,
        [&compositor, &talentStore, &aiPipeline, &outputRouter, &previewSender, liveProvider,
         &latestFrame, &frameMutex, &compositing, &processedCount]()
        {
            static int timerTick = 0;
            if (timerTick++ == 0) qInfo() << "[Pipeline] Timer first tick";
            if (compositing.load()) { if (timerTick % 300 == 0) qInfo() << "[Pipeline] BLOCKED — compositing stuck"; return; }

            QImage frame;
            {
                QMutexLocker lock(&frameMutex);
                frame = latestFrame; // Grab latest (may be null — Compositor handles it)
            }

            compositing.store(true);

            try {
            // Get interpolated overlay positions (called every render frame for smooth motion)
            // predictNow() uses velocity extrapolation between 5fps detections
            auto predicted = aiPipeline.predictNow();
            QList<prestige::TalentOverlay> talents;
            for (const auto& f : predicted) {
                prestige::TalentOverlay ov;
                ov.id = f.id; ov.name = f.name; ov.role = f.role;
                ov.confidence = f.confidence; ov.showOverlay = f.showOverlay;
                ov.overlayStyle = f.overlayStyle;
                ov.bbox = f.smoothedBbox.isValid() ? f.smoothedBbox : f.bbox;
                talents.append(ov);
            }

            // Composite: video frame + overlay nameplates → single frame
            QImage composited = compositor.composite(frame, talents);
            static int logCount = 0;
            if (logCount++ % 150 == 0) { // Log every ~5 seconds
                qInfo() << "[Pipeline]" << (frame.isNull() ? "no-source" : "source-ok") << composited.size() << "frames:" << processedCount.load();
            }

            // Send to all outputs (RTMP/SRT/File)
            outputRouter.sendFrame(composited);

            // Update live display in QML (local)
            liveProvider->updateFrame(composited);

            // Send to Control Room for live preview (:5558)
            previewSender.sendFrame(composited);

            processedCount++;
            } catch (const std::exception& e) {
                qWarning() << "[Pipeline] CRASH in composite:" << e.what();
            } catch (...) {
                qWarning() << "[Pipeline] CRASH in composite (unknown)";
            }
            compositing.store(false);
        });
    compositeTimer.start(16); // ~60fps max, adjusts dynamically when config received

    // ── 9. Load QML ──────────────────────────────────────────
    const QUrl mainQml(QStringLiteral("qrc:/PrestigeVision/qml/OverlayNameplate.qml"));
    engine.load(mainQml);

    // ── 10. Start capture ────────────────────────────────────
    // Don't auto-open webcam — wait for config from Control Room
    // The Control Room will send the input source via ZMQ :5559
    // For now, only open if explicitly requested via command line
    bool autoCapture = false;
    for (int i = 1; i < argc; ++i) {
        if (QString(argv[i]) == "--webcam") {
            autoCapture = true;
        }
    }
    if (autoCapture) {
        capture.openWebcam(0);
        qInfo() << "[VisionEngine] Webcam auto-opened (--webcam flag)";
    } else {
        qInfo() << "[VisionEngine] Waiting for input source from Control Room...";
    }

    // ── 11. Stats ────────────────────────────────────────────
    QTimer statsTimer;
    QObject::connect(&statsTimer, &QTimer::timeout,
        [&capture, &aiPipeline, &compositor, &outputRouter]() {
            qInfo().noquote() << QStringLiteral(
                "[Pipeline] Cap:%1fps | AI:%2 | Composite:%3ms | Outputs:%4")
                .arg(capture.currentFps(), 0, 'f', 1)
                .arg(aiPipeline.isDetecting() ? "ACTIVE" : "IDLE")
                .arg(compositor.lastCompositeMs(), 0, 'f', 1)
                .arg(outputRouter.activeCount());
        });
    statsTimer.start(5000);

    qInfo() << "[VisionEngine] Complete pipeline started:";
    qInfo() << "  Capture → AI Pipeline (C++ ONNX) → Compositor → Output";

    return app.exec();
}
