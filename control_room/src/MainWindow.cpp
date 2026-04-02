// ============================================================
// Prestige AI — Main Window Implementation
// All signal/slot connections are wired here.
// ============================================================

#include "MainWindow.h"
#include "ConfigManager.h"
#include "PreviewMonitor.h"
#include "OverlayController.h"
#include "EmissionProfile.h"
#include "SetupController.h"
#include "LiveController.h"
#include "StyleRegistry.h"
#include "TalentManager.h"
#include "NdiCapture.h"
#include "HardwareScanner.h"
#include "RssFetcher.h"
#include "SubtitleController.h"
#include "WeatherFetcher.h"
#include "MacroEngine.h"
#include "AudioMeter.h"
#include "SocialChatController.h"
#include "WebRemoteServer.h"
#include "LicenseManager.h"
#include "AnalyticsEngine.h"
#include "GraphicsQueue.h"

#include <QQmlContext>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QTimer>



namespace prestige {

MainWindow::MainWindow(QObject* parent)
    : QObject(parent)
    , m_config(new ConfigManager(this))
    , m_preview(new PreviewMonitor(this))
    , m_overlay(new OverlayController(this))
    , m_profileManager(new ProfileManager(this))
    , m_liveController(new LiveController(this))
{
    m_setupController = new SetupController(m_profileManager, this);
    m_macroEngine = new MacroEngine(m_liveController, m_overlay, this);
    m_audioMeter = new AudioMeter(this);
    m_rssFetcher = new RssFetcher(this);
    m_subtitleCtrl = new SubtitleController(this);
    m_weatherFetcher = new WeatherFetcher(this);
    m_socialChatCtrl = new SocialChatController(this);
    m_analytics = new AnalyticsEngine(this);
    m_webRemote = new WebRemoteServer(m_liveController, m_overlay, m_macroEngine,
                                      m_setupController, m_analytics, m_subtitleCtrl,
                                      m_rssFetcher, this);
    m_graphicsQueue = new GraphicsQueue(this);
    m_webRemote->setGraphicsQueue(m_graphicsQueue);
    m_license = new LicenseManager(this);

    // Connect LiveController signals to AnalyticsEngine
    connect(m_liveController, &LiveController::detectionChanged, this, [this]() {
        if (m_liveController->talentDetected()) {
            m_analytics->recordDetection(
                m_liveController->detectedName(),
                m_liveController->detectedRole(),
                m_liveController->confidence()
            );
        }
    });
    connect(m_liveController, &LiveController::statsChanged, this, [this]() {
        m_analytics->recordFps(m_liveController->fps());
        m_analytics->recordLatency(m_liveController->latencyMs());
    });

    // Connect PreviewMonitor FPS → LiveController real stats
    connect(m_preview, &PreviewMonitor::fpsChanged, this, [this]() {
        m_liveController->updateRealStats(m_preview->fps(), 0);
    });

    // Debounced config publisher — coalesces rapid signal bursts into single ZMQ publish
    auto* configDebounce = new QTimer(this);
    configDebounce->setSingleShot(true);
    configDebounce->setInterval(5); // 5ms debounce — fast enough for real-time, prevents duplicate sends
    connect(configDebounce, &QTimer::timeout, this, [this]() { publishConfig(); });
    auto schedulePublish = [configDebounce]() { configDebounce->start(); };

    // Publish config to Vision Engine when style/animation changes
    connect(m_setupController, &SetupController::styleChanged, this, schedulePublish);
    connect(m_setupController, &SetupController::animationChanged, this, schedulePublish);

    // Publish config when ticker/subtitle settings change
    connect(m_rssFetcher, &RssFetcher::headlinesChanged, this, schedulePublish);
    connect(m_subtitleCtrl, &SubtitleController::configChanged, this, schedulePublish);

    // Publish config when recording or countdown state changes
    connect(m_liveController, &LiveController::recordingChanged, this, schedulePublish);
    connect(m_liveController, &LiveController::countdownChanged, this, schedulePublish);
    connect(m_liveController, &LiveController::qrCodeChanged, this, schedulePublish);

    // Publish config when social chat config changes
    connect(m_socialChatCtrl, &SocialChatController::configChanged, this, schedulePublish);

    // Publish config when graphics queue item is taken
    connect(m_graphicsQueue, &GraphicsQueue::itemTaken, this, schedulePublish);
    connect(m_graphicsQueue, &GraphicsQueue::queueChanged, this, schedulePublish);

    // Publish config when channel name changes
    connect(m_config, &ConfigManager::channelNameChanged, this, schedulePublish);

    // Publish config when branding/show title/timing changes
    connect(m_setupController, &SetupController::brandingChanged, this, schedulePublish);
    connect(m_setupController, &SetupController::showTitleChanged, this, schedulePublish);
    connect(m_setupController, &SetupController::timingChanged, this, schedulePublish);
    connect(m_setupController, &SetupController::sourceChanged, this, schedulePublish);
    connect(m_setupController, &SetupController::outputsChanged, this, schedulePublish);
    connect(m_setupController, &SetupController::overlayTimingChanged, this, schedulePublish);
    connect(m_setupController, &SetupController::virtualStudioChanged, this, schedulePublish);
    connect(m_weatherFetcher, &WeatherFetcher::weatherChanged, this, schedulePublish);

    // Auto-save profile when branding/settings change (debounced via timer)
    auto* autoSaveTimer = new QTimer(this);
    autoSaveTimer->setSingleShot(true);
    autoSaveTimer->setInterval(2000); // Save 2s after last change
    connect(autoSaveTimer, &QTimer::timeout, this, [this]() { m_setupController->saveProfile(); });
    auto triggerAutoSave = [autoSaveTimer]() { autoSaveTimer->start(); };
    connect(m_setupController, &SetupController::brandingChanged, this, triggerAutoSave);
    connect(m_setupController, &SetupController::styleChanged, this, triggerAutoSave);
    connect(m_setupController, &SetupController::showTitleChanged, this, triggerAutoSave);
    connect(m_setupController, &SetupController::outputsChanged, this, triggerAutoSave);
    connect(m_setupController, &SetupController::virtualStudioChanged, this, triggerAutoSave);

    // Publish config when broadcast overlay cycle state changes
    connect(m_liveController, &LiveController::showTitleVisibleChanged, this, schedulePublish);
    connect(m_liveController, &LiveController::talentVisibleChanged, this, schedulePublish);
    connect(m_liveController, &LiveController::bypassChanged, this, schedulePublish);

    // Init config publisher ZMQ
#ifdef PRESTIGE_HAVE_ZMQ
    m_configZmqCtx = zmq_ctx_new();
    m_configZmqSock = zmq_socket(m_configZmqCtx, ZMQ_PUB);
    int linger = 0;
    zmq_setsockopt(m_configZmqSock, ZMQ_LINGER, &linger, sizeof(linger));
    zmq_bind(m_configZmqSock, "tcp://127.0.0.1:5559");
    qInfo() << "[ControlRoom] Config publisher bound (:5559)";
#endif

    // Connect detection data from Python → LiveController
    connectDetectionPipeline();

    // Always-On Passthrough: pipeline is always running from startup
    // The m_liveMode starts true — signal always passes through
    m_liveMode = true;
    m_liveController->startLive();
    m_audioMeter->start();
    qInfo() << "[ControlRoom] Always-On Passthrough: pipeline running from startup";

    // Publish initial config after short delay to ensure Vision Engine is ready
    QTimer::singleShot(1500, this, schedulePublish);
    // Also re-publish periodically during first 10s for reliability
    QTimer::singleShot(3000, this, schedulePublish);
    QTimer::singleShot(6000, this, schedulePublish);
}

MainWindow::~MainWindow()
{
    stopSubProcesses();
}

void MainWindow::startSubProcesses()
{
    QString appDir = QCoreApplication::applicationDirPath();

    // AI Engine is now integrated into the Vision Engine (C++ ONNX Runtime)
    // No separate Python process needed
    qInfo() << "[Prestige AI] AI Engine: integrated (C++ ONNX Runtime)";

    // ── Start Vision Engine (hidden, no terminal) ────────
    QString visionExe;
#ifdef Q_OS_WIN
    visionExe = appDir + "/prestige_vision.exe";
#else
    visionExe = appDir + "/prestige_vision";
    // Dev layout: control_room/ and vision_engine/ are siblings
    if (!QFileInfo(visionExe).exists())
        visionExe = appDir + "/../vision_engine/prestige_vision";
#endif

    if (QFileInfo(visionExe).exists()) {
        m_visionProcess = new QProcess(this);
        m_visionProcess->setStandardOutputFile(QProcess::nullDevice());
        m_visionProcess->setStandardErrorFile(QProcess::nullDevice());

        m_visionProcess->start(visionExe);
        if (m_visionProcess->waitForStarted(5000)) {
            qInfo() << "[Prestige AI] Vision Engine started (PID:" << m_visionProcess->processId() << ")";
        } else {
            qWarning() << "[Prestige AI] Failed to start Vision Engine:" << m_visionProcess->errorString();
        }
    } else {
        qInfo() << "[Prestige AI] Vision Engine not found at:" << visionExe << "(standalone mode)";
    }
}

void MainWindow::stopSubProcesses()
{
    // AI Engine is integrated (C++ ONNX Runtime) — no process to stop
    if (m_visionProcess) {
        m_visionProcess->terminate();
        if (!m_visionProcess->waitForFinished(3000))
            m_visionProcess->kill();
        qInfo() << "[Prestige AI] Vision Engine stopped";
    }
}

bool MainWindow::initialize(QQmlApplicationEngine* engine)
{
    if (!engine)
        return false;

    m_config->load();

    auto* talentMgr = new TalentManager(this);
    auto* ndi = new NdiCapture(this);
    auto* styleReg = new StyleRegistry(this);
    auto* hwScanner = new HardwareScanner(this);

    auto* ctx = engine->rootContext();
    ctx->setContextProperty("mainWindow", this);
    ctx->setContextProperty("configManager", m_config);
    ctx->setContextProperty("previewMonitor", m_preview);
    ctx->setContextProperty("overlayController", m_overlay);
    ctx->setContextProperty("profileManager", m_profileManager);
    ctx->setContextProperty("setupController", m_setupController);
    ctx->setContextProperty("liveController", m_liveController);
    ctx->setContextProperty("styleRegistry", styleReg);
    ctx->setContextProperty("talentManager", talentMgr);
    ctx->setContextProperty("ndiCapture", ndi);
    ctx->setContextProperty("hardwareScanner", hwScanner);
    ctx->setContextProperty("rssFetcher", m_rssFetcher);
    ctx->setContextProperty("subtitleController", m_subtitleCtrl);
    ctx->setContextProperty("weatherFetcher", m_weatherFetcher);
    ctx->setContextProperty("macroEngine", m_macroEngine);
    ctx->setContextProperty("audioMeter", m_audioMeter);
    ctx->setContextProperty("socialChatController", m_socialChatCtrl);
    ctx->setContextProperty("webRemote", m_webRemote);
    ctx->setContextProperty("licenseManager", m_license);
    ctx->setContextProperty("analyticsEngine", m_analytics);
    ctx->setContextProperty("graphicsQueue", m_graphicsQueue);

    // Validate stored license on startup
    m_license->validateStoredKey();

    // Auto-scan hardware at startup
    hwScanner->scan();

    // Auto-start sub-processes (AI Engine + Vision Engine)
    startSubProcesses();

    qInfo() << "[ControlRoom] MainWindow initialized (Always-On Passthrough)";
    return true;
}

void MainWindow::launchEmission()
{
    // Backward compat: now just activates overlays
    activateOverlays();
}

void MainWindow::stopEmission()
{
    // Backward compat: now just deactivates overlays
    deactivateOverlays();
}

void MainWindow::activateOverlays()
{
    if (m_overlaysActive)
        return;

    m_setupController->saveProfile();

    // Transfer config to live controller
    m_liveController->setMultiFaceConfig(
        m_setupController->multiFaceName(),
        m_setupController->multiFaceRole()
    );
    m_liveController->setOverlayDelays(
        m_setupController->singleFaceDelayMs(),
        m_setupController->multiFaceDelayMs(),
        m_setupController->overlayHideDelayMs()
    );
    m_liveController->setSelectedStyle(m_setupController->selectedStyle());

    // Transfer talent timing config
    m_liveController->setTalentTimingConfig(
        m_setupController->talentDisplayDurationSec(),
        m_setupController->titleReappearDelaySec()
    );

    m_overlaysActive = true;
    m_liveController->setOverlaysActive(true);

    // Send config to Vision Engine Compositor (with emission_starting flag)
    m_emissionStarting = true;
    publishConfig();
    m_emissionStarting = false;

    m_analytics->start();

    emit overlaysActiveChanged();
    emit emissionLaunched();
    qInfo() << "[ControlRoom] === OVERLAYS ACTIVATED ===";
}

void MainWindow::deactivateOverlays()
{
    if (!m_overlaysActive)
        return;

    m_overlaysActive = false;
    m_liveController->setOverlaysActive(false);
    m_liveController->setBypassed(false);
    m_analytics->stop();

    publishConfig();

    emit overlaysActiveChanged();
    emit emissionStopped();
    qInfo() << "[ControlRoom] === OVERLAYS DEACTIVATED (passthrough) ===";
}

void MainWindow::toggleOverlays()
{
    if (m_overlaysActive)
        deactivateOverlays();
    else
        activateOverlays();
}

void MainWindow::switchProgram(int index)
{
    QStringList programs = programList();
    if (index < 0 || index >= programs.size())
        return;

    m_activeProgram = index;

    // Last entry is "Passthrough" — no overlays
    if (index == programs.size() - 1) {
        deactivateOverlays();
    } else {
        // Load profile at this index
        QVariantList profiles = m_setupController->availableProfiles();
        if (index < profiles.size()) {
            QVariantMap prof = profiles.at(index).toMap();
            QString profileId = prof.value("id").toString();
            if (!profileId.isEmpty()) {
                m_setupController->loadProfile(profileId);

                // Transfer config to live controller
                m_liveController->setMultiFaceConfig(
                    m_setupController->multiFaceName(),
                    m_setupController->multiFaceRole()
                );
                m_liveController->setOverlayDelays(
                    m_setupController->singleFaceDelayMs(),
                    m_setupController->multiFaceDelayMs(),
                    m_setupController->overlayHideDelayMs()
                );
                m_liveController->setSelectedStyle(m_setupController->selectedStyle());
                m_liveController->setTalentTimingConfig(
                    m_setupController->talentDisplayDurationSec(),
                    m_setupController->titleReappearDelaySec()
                );
            }
        }
        activateOverlays();
    }

    publishConfig();
    emit activeProgramChanged();
    qInfo() << "[ControlRoom] Switched to program:" << programs.at(index);
}

QStringList MainWindow::programList() const
{
    QStringList list = m_profileManager->profileNames();
    list.append(QStringLiteral("Passthrough (aucun overlay)"));
    return list;
}

void MainWindow::setLanguage(const QString& lang)
{
    if (m_language == lang) return;
    m_language = lang;
    emit languageChanged();
    publishConfig();
    qInfo() << "[ControlRoom] Language set to:" << lang;
}

QStringList MainWindow::availableLanguages() const
{
    return QStringList{
        QStringLiteral("Fran\u00E7ais"),
        QStringLiteral("English"),
        QStringLiteral("Espa\u00F1ol"),
        QStringLiteral("\u0627\u0644\u0639\u0631\u0628\u064A\u0629"),
        QStringLiteral("\u4E2D\u6587")
    };
}

void MainWindow::publishConfig()
{
#ifdef PRESTIGE_HAVE_ZMQ
    if (!m_configZmqSock) return;

    QJsonObject obj;
    obj["style_id"] = m_setupController->selectedStyle();
    obj["animation_type"] = m_setupController->animationType();
    obj["anim_enter_frames"] = m_setupController->animEnterFrames();
    obj["anim_exit_frames"] = m_setupController->animExitFrames();
    obj["accent_color"] = m_setupController->accentColor().name();
    obj["bg_opacity"] = m_setupController->backgroundOpacity();

    // Input source config
    obj["input_type"] = m_setupController->inputType();
    obj["input_source"] = m_setupController->inputSource();

    // Ticker config
    QString tickerContent = m_rssFetcher->headlines();
    if (tickerContent.isEmpty())
        tickerContent = m_setupController->tickerManualText();
    obj["ticker_visible"] = m_setupController->tickerVisible() && !tickerContent.isEmpty();
    obj["ticker_text"] = tickerContent;

    // Subtitle config
    obj["subtitle_visible"] = m_subtitleCtrl->isEnabled();
    obj["subtitle_font_size"] = m_subtitleCtrl->fontSize();
    obj["subtitle_position"] = m_subtitleCtrl->position();
    obj["subtitle_bg_opacity"] = m_subtitleCtrl->bgOpacity();
    obj["subtitle_text_color"] = m_subtitleCtrl->textColor();

    // Recording config
    obj["recording_active"] = m_liveController->isRecording();
    obj["recording_path"] = m_liveController->recordingPath();

    // Countdown config
    obj["countdown_active"] = m_liveController->countdownActive();
    obj["countdown_seconds"] = m_liveController->countdownSeconds();
    obj["countdown_label"] = m_liveController->countdownLabel();

    // QR Code config
    obj["qr_visible"] = m_liveController->qrCodeVisible();
    obj["qr_url"] = m_liveController->qrCodeUrl();
    obj["qr_position"] = m_liveController->qrCodePosition();

    // Social chat config
    obj["social_chat_visible"] = m_socialChatCtrl->showOnOutput();
    QJsonArray chatArr;
    for (const auto& line : m_socialChatCtrl->chatLinesForOverlay())
        chatArr.append(line);
    obj["social_chat_messages"] = chatArr;

    // Channel name
    obj["channel_name"] = m_config->channelName();

    // Always-On Passthrough: overlay activation state
    obj["overlays_active"] = m_overlaysActive;

    // RTL layout
    obj["layout_rtl"] = (m_language == "ar");

    // Channel branding
    obj["channel_logo_path"] = m_setupController->channelLogoPath();
    obj["channel_logo_position"] = m_setupController->channelLogoPosition();
    obj["channel_logo_size"] = m_setupController->channelLogoSize();
    obj["channel_logo_offset_x"] = m_setupController->channelLogoOffsetX();
    obj["channel_logo_offset_y"] = m_setupController->channelLogoOffsetY();
    obj["show_channel_name_text"] = m_setupController->showChannelNameText();
    obj["keep_logo_during_ads"] = m_setupController->keepLogoDuringAds();
    obj["channel_name_offset_x"] = m_setupController->channelNameOffsetX();
    obj["channel_name_offset_y"] = m_setupController->channelNameOffsetY();

    // Channel name design
    obj["channel_name_shape"] = m_setupController->channelNameShape();
    obj["channel_name_bg_color"] = m_setupController->channelNameBgColor();
    obj["channel_name_text_color"] = m_setupController->channelNameTextColor();
    obj["channel_name_border_color"] = m_setupController->channelNameBorderColor();
    obj["channel_name_font_size"] = m_setupController->channelNameFontSize();

    // Branding animations
    obj["logo_entry_anim"] = m_setupController->logoEntryAnim();
    obj["name_entry_anim"] = m_setupController->nameEntryAnim();
    obj["logo_loop_anim"] = m_setupController->logoLoopAnim();
    obj["name_loop_anim"] = m_setupController->nameLoopAnim();

    // Emission start flag (resets branding entry animations)
    if (m_emissionStarting)
        obj["emission_starting"] = true;

    // Show title
    obj["show_title"] = m_setupController->showTitle();
    obj["show_subtitle"] = m_setupController->showSubtitle();
    obj["show_title_enabled"] = m_setupController->showTitleEnabled();
    obj["show_title_position"] = m_setupController->showTitlePosition();

    // Show title design & animation
    obj["show_title_shape"] = m_setupController->showTitleShape();
    obj["show_title_bg_color"] = m_setupController->showTitleBgColor();
    obj["show_title_text_color"] = m_setupController->showTitleTextColor();
    obj["show_title_border_color"] = m_setupController->showTitleBorderColor();
    obj["show_title_font_size"] = m_setupController->showTitleFontSize();
    obj["show_title_entry_anim"] = m_setupController->showTitleEntryAnim();
    obj["show_title_loop_anim"] = m_setupController->showTitleLoopAnim();

    // Overlay element offsets
    obj["show_title_offset_x"] = m_setupController->showTitleOffsetX();
    obj["show_title_offset_y"] = m_setupController->showTitleOffsetY();
    obj["ticker_offset_y"] = m_setupController->tickerOffsetY();
    obj["subtitle_offset_x"] = m_setupController->subtitleOffsetX();
    obj["subtitle_offset_y"] = m_setupController->subtitleOffsetY();
    obj["countdown_offset_x"] = m_setupController->countdownOffsetX();
    obj["countdown_offset_y"] = m_setupController->countdownOffsetY();
    obj["clock_offset_x"] = m_setupController->clockOffsetX();
    obj["clock_offset_y"] = m_setupController->clockOffsetY();
    obj["qr_offset_x"] = m_setupController->qrCodeOffsetX();
    obj["qr_offset_y"] = m_setupController->qrCodeOffsetY();
    obj["scoreboard_offset_x"] = m_setupController->scoreboardOffsetX();
    obj["scoreboard_offset_y"] = m_setupController->scoreboardOffsetY();
    obj["weather_offset_x"] = m_setupController->weatherOffsetX();
    obj["weather_offset_y"] = m_setupController->weatherOffsetY();

    // Overlay scale factors (0.5 – 2.0)
    obj["nameplate_scale"] = m_setupController->nameplateScale();
    obj["scoreboard_scale"] = m_setupController->scoreboardScale();
    obj["weather_scale"] = m_setupController->weatherScale();
    obj["clock_scale"] = m_setupController->clockScale();
    obj["countdown_scale"] = m_setupController->countdownScale();
    obj["qr_code_scale"] = m_setupController->qrCodeScale();

    // Ticker appearance (font, color, speed)
    obj["ticker_font_size"] = m_setupController->tickerFontSize();
    obj["ticker_bg_color"] = m_setupController->tickerBgColor();
    obj["ticker_text_color"] = m_setupController->tickerTextColor();
    obj["ticker_speed"] = m_setupController->tickerSpeed();
    obj["ticker_manual_text"] = m_setupController->tickerManualText();

    // Clock config
    obj["clock_visible"] = m_setupController->clockVisible();
    obj["clock_format"] = m_setupController->clockFormat();

    // Scoreboard data
    obj["scoreboard_visible"] = m_setupController->scoreboardVisible();
    obj["scoreboard_team_a"] = m_setupController->scoreboardTeamA();
    obj["scoreboard_team_b"] = m_setupController->scoreboardTeamB();
    obj["scoreboard_score_a"] = m_setupController->scoreboardScoreA();
    obj["scoreboard_score_b"] = m_setupController->scoreboardScoreB();
    obj["scoreboard_color_a"] = m_setupController->scoreboardColorA();
    obj["scoreboard_color_b"] = m_setupController->scoreboardColorB();
    obj["scoreboard_position"] = m_setupController->scoreboardPosition();
    obj["scoreboard_match_time"] = m_setupController->scoreboardMatchTime();
    obj["scoreboard_period"] = m_setupController->scoreboardPeriod();

    // Sport system
    obj["team_logo_a"] = m_setupController->teamLogoA();
    obj["team_logo_b"] = m_setupController->teamLogoB();
    obj["goal_anim_active"] = m_setupController->goalAnimActive();
    obj["goal_anim_text"] = m_setupController->goalAnimText();
    obj["goal_anim_team"] = m_setupController->goalAnimTeam();
    obj["goal_anim_player"] = m_setupController->goalAnimPlayer();
    obj["goal_anim_effect"] = m_setupController->goalAnimEffect();
    obj["goal_anim_duration"] = m_setupController->goalAnimDuration();
    obj["sport_event"] = m_setupController->sportEvent();

    // Weather data
    obj["weather_visible"] = m_setupController->weatherVisible();
    obj["weather_city"] = m_weatherFetcher->city();
    obj["weather_temperature"] = m_weatherFetcher->temperature();
    obj["weather_unit"] = m_weatherFetcher->unit();
    obj["weather_condition_icon"] = m_weatherFetcher->conditionIcon();

    // Output destinations
    obj["output_sdi"] = m_setupController->outputSDI();
    obj["output_ndi"] = m_setupController->outputNDI();
    obj["output_fps"] = m_setupController->outputFps();
    obj["output_bitrate"] = m_setupController->outputBitrate();
    obj["output_rtmp"] = m_setupController->outputRTMP();
    obj["output_srt"] = m_setupController->outputSRT();
    obj["rtmp_url"] = m_setupController->rtmpUrl();
    obj["rtmp_key"] = m_setupController->rtmpKey();
    obj["srt_url"] = m_setupController->srtUrl();

    // Social media RTMP outputs (JSON array of {platform, url})
    QString socialJson = m_setupController->socialOutputsJson();
    if (!socialJson.isEmpty()) {
        auto socialArr = QJsonDocument::fromJson(socialJson.toUtf8()).array();
        obj["social_outputs"] = socialArr;
    }

    // Virtual Studio
    obj["virtual_studio_enabled"] = m_setupController->virtualStudioEnabled();
    obj["virtual_studio_id"] = m_setupController->virtualStudioId();
    obj["chroma_key_enabled"] = m_setupController->chromaKeyEnabled();
    obj["chroma_key_color"] = m_setupController->chromaKeyColor();
    obj["chroma_key_tolerance"] = m_setupController->chromaKeyTolerance();
    obj["chroma_key_smooth"] = m_setupController->chromaKeySmooth();
    obj["vs_primary_color"] = m_setupController->vsPrimaryColor().isValid() ? m_setupController->vsPrimaryColor().name() : "";
    obj["vs_secondary_color"] = m_setupController->vsSecondaryColor().isValid() ? m_setupController->vsSecondaryColor().name() : "";
    obj["vs_accent_color"] = m_setupController->vsAccentColor().isValid() ? m_setupController->vsAccentColor().name() : "";
    obj["vs_floor_color"] = m_setupController->vsFloorColor().isValid() ? m_setupController->vsFloorColor().name() : "";
    obj["vs_light_intensity"] = m_setupController->vsLightIntensity();
    obj["vs_animations_enabled"] = m_setupController->vsAnimationsEnabled();
    obj["vs_custom_background"] = m_setupController->vsCustomBackground();

    // Visibility states — gated behind overlays_active
    // Channel branding (logo, name) is sent unconditionally above
    // Only talent nameplates and show title depend on overlay activation
    obj["show_title_visible"] = m_overlaysActive && m_liveController->isShowTitleVisible();
    obj["talent_nameplate_visible"] = m_overlaysActive && m_liveController->isTalentNameplateVisible();
    obj["bypass_active"] = m_liveController->isBypassed();

    // Graphics queue current item
    obj["queue_current"] = QJsonObject::fromVariantMap(m_graphicsQueue->currentItem());

    QByteArray payload = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    zmq_send(m_configZmqSock, payload.constData(), payload.size(), ZMQ_NOBLOCK);

    qInfo() << "[ControlRoom] Config published: style=" << m_setupController->selectedStyle()
            << "anim=" << m_setupController->animationType();
#endif
}

void MainWindow::connectDetectionPipeline()
{
    // The Control Room subscribes to the same ZMQ :5555 that the Vision Engine uses.
    // This gives us real-time detection data for the Live UI.
#ifdef PRESTIGE_HAVE_ZMQ
    m_zmqDetThread = new QThread(this);
    auto* worker = new QObject;
    worker->moveToThread(m_zmqDetThread);

    connect(m_zmqDetThread, &QThread::started, worker, [this, worker]() {
        void* ctx = zmq_ctx_new();
        void* socket = zmq_socket(ctx, ZMQ_SUB);
        zmq_setsockopt(socket, ZMQ_SUBSCRIBE, "", 0);
        int conflate = 1;
        zmq_setsockopt(socket, ZMQ_CONFLATE, &conflate, sizeof(conflate));
        int timeout = 200;
        zmq_setsockopt(socket, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
        zmq_connect(socket, "tcp://127.0.0.1:5555");

        char buf[65536];
        while (m_detRunning) {
            // Try multipart first (subtitle messages use topic)
            zmq_msg_t topicMsg, dataMsg;
            zmq_msg_init(&topicMsg);

            int n = zmq_msg_recv(&topicMsg, socket, 0);
            if (n <= 0) {
                zmq_msg_close(&topicMsg);
                continue;
            }

            QByteArray topicData(static_cast<char*>(zmq_msg_data(&topicMsg)), zmq_msg_size(&topicMsg));
            zmq_msg_close(&topicMsg);

            // Check if this is a multipart message (has MORE flag)
            int more = 0;
            size_t moreSize = sizeof(more);
            zmq_getsockopt(socket, ZMQ_RCVMORE, &more, &moreSize);

            QByteArray raw;
            if (more) {
                // Multipart: topic + payload (subtitle messages)
                zmq_msg_init(&dataMsg);
                zmq_msg_recv(&dataMsg, socket, 0);
                raw = QByteArray(static_cast<char*>(zmq_msg_data(&dataMsg)), zmq_msg_size(&dataMsg));
                zmq_msg_close(&dataMsg);

                if (topicData == "subtitle") {
                    auto doc = QJsonDocument::fromJson(raw);
                    auto obj = doc.object();
                    QString text = obj["text"].toString();
                    QString lang = obj["language"].toString();
                    double conf = obj["confidence"].toDouble(0.9);

                    QMetaObject::invokeMethod(m_subtitleCtrl, [this, text, lang, conf]() {
                        m_subtitleCtrl->onSubtitleReceived(text, lang, conf);
                    }, Qt::QueuedConnection);
                    continue;
                }

                if (topicData == "social_chat") {
                    auto doc = QJsonDocument::fromJson(raw);
                    auto obj = doc.object();
                    QString platform = obj["platform"].toString();
                    QString author = obj["author"].toString();
                    QString message = obj["message"].toString();
                    QString color = obj["color"].toString("#FFFFFF");

                    QMetaObject::invokeMethod(m_socialChatCtrl, [this, platform, author, message, color]() {
                        m_socialChatCtrl->onChatMessageReceived(platform, author, message, color);
                    }, Qt::QueuedConnection);
                    continue;
                }
            } else {
                // Single part: detection message (backward compat)
                raw = topicData;
            }

            auto doc = QJsonDocument::fromJson(raw);
            auto obj = doc.object();

            QString sceneType = obj["scene_type"].toString("empty");
            int faceCount = obj["face_count"].toInt(0);

            auto talents = obj["talents"].toArray();
            if (!talents.isEmpty()) {
                auto first = talents[0].toObject();
                QString name = first["name"].toString();
                QString role = first["role"].toString();
                double conf = first["confidence"].toDouble();

                if (name != "Inconnu" && name != "Détecté" && !name.isEmpty()) {
                    QMetaObject::invokeMethod(m_liveController, [this, name, role, conf]() {
                        m_liveController->onTalentDetected(name, role, conf);
                    }, Qt::QueuedConnection);
                }
            }

            QMetaObject::invokeMethod(m_liveController, [this, sceneType, faceCount]() {
                m_liveController->onSceneChanged(sceneType, faceCount);
            }, Qt::QueuedConnection);
        }

        zmq_close(socket);
        zmq_ctx_destroy(ctx);
        worker->deleteLater();
    });
    connect(m_zmqDetThread, &QThread::finished, worker, &QObject::deleteLater);

    m_detRunning = true;
    m_zmqDetThread->start();
    qInfo() << "[ControlRoom] Detection pipeline connected (:5555)";
#else
    qWarning() << "[ControlRoom] No ZMQ — detection pipeline disabled";
#endif
}

} // namespace prestige
