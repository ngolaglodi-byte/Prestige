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
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

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
    QObject::connect(&twitchChat, &prestige::ai::TwitchChat::messageReceived,
        [](const QString& author, const QString& msg, const QString&) {
            qInfo() << "[Chat]" << author << ":" << msg;
        });

    // ── 5. Output Router (RTMP/SRT/File) ─────────────────────
    // (Moved before config receiver so it can be captured by the lambda)
    prestige::OutputRouter outputRouter;

    // ── 4b. Config receiver from Control Room (:5559) ────────
    // Listens for style/animation changes from the director
#ifdef PRESTIGE_HAVE_ZMQ
    QThread configThread;
    bool configRunning = true;
    QObject* configWorker = new QObject;
    configWorker->moveToThread(&configThread);

    QObject::connect(&configThread, &QThread::started, configWorker, [&compositor, &outputRouter, &configRunning, configWorker]() {
        void* ctx = zmq_ctx_new();
        void* sock = zmq_socket(ctx, ZMQ_SUB);
        zmq_setsockopt(sock, ZMQ_SUBSCRIBE, "", 0);
        int conflate = 1;
        zmq_setsockopt(sock, ZMQ_CONFLATE, &conflate, sizeof(conflate));
        int timeout = 200;
        zmq_setsockopt(sock, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
        zmq_connect(sock, "tcp://127.0.0.1:5559");

        char buf[4096];
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
            QMetaObject::invokeMethod(&outputRouter, [&outputRouter, recActive, recPath]() {
                if (recActive && !recPath.isEmpty()) {
                    // OutputType::File = 6
                    outputRouter.addOutput(6, recPath, 8, 25);
                } else {
                    outputRouter.removeOutput(6);
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

            qInfo() << "[VisionEngine] Config received: style=" << styleId << "anim=" << animType;
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
    // Every frame: get video + overlay → fuse → encode → send
    QObject::connect(&capture, &prestige::VideoCapture::frameCaptured,
        [&compositor, &talentStore, &outputRouter, &previewSender, liveProvider](
            const QImage& frame, qint64 /*frameId*/, qint64 /*ts*/)
        {
            // Get current overlay data
            auto talents = talentStore.snapshot();

            // Composite: video frame + overlay nameplates → single frame
            QImage composited = compositor.composite(frame, talents);

            // Send to all outputs (RTMP/SRT/File)
            outputRouter.sendFrame(composited);

            // Update live display in QML (local)
            liveProvider->updateFrame(composited);

            // Send to Control Room for live preview (:5558)
            previewSender.sendFrame(composited);
        });

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
