#pragma once

// ============================================================
// Prestige AI — Main Window (Control Room)
// ============================================================

#include <QObject>
#include <QQmlApplicationEngine>
#include <QThread>
#include <QString>
#include <QProcess>

#ifdef PRESTIGE_HAVE_ZMQ
#include <zmq.h>
#endif

namespace prestige {

class ConfigManager;
class PreviewMonitor;
class OverlayController;
class ProfileManager;
class SetupController;
class LiveController;
class RssFetcher;
class SubtitleController;
class WeatherFetcher;
class MacroEngine;
class AudioMeter;
class SocialChatController;
class WebRemoteServer;
class LicenseManager;
class AnalyticsEngine;
class GraphicsQueue;

class MainWindow : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isLiveMode READ isLiveMode NOTIFY modeChanged)
    Q_PROPERTY(bool overlaysActive READ overlaysActive NOTIFY overlaysActiveChanged)
    Q_PROPERTY(int activeProgram READ activeProgram NOTIFY activeProgramChanged)
    Q_PROPERTY(QStringList programList READ programList NOTIFY programListChanged)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)

public:
    explicit MainWindow(QObject* parent = nullptr);
    ~MainWindow() override;

    bool initialize(QQmlApplicationEngine* engine);
    bool isLiveMode() const { return m_liveMode; }

    // Always-On Passthrough: overlay state
    bool overlaysActive() const { return m_overlaysActive; }
    int activeProgram() const { return m_activeProgram; }
    QStringList programList() const;

    Q_INVOKABLE void switchProgram(int index);
    Q_INVOKABLE void activateOverlays();
    Q_INVOKABLE void deactivateOverlays();
    Q_INVOKABLE void toggleOverlays();

    // i18n
    QString language() const { return m_language; }
    void setLanguage(const QString& lang);
    Q_INVOKABLE QStringList availableLanguages() const;

    // Backward compat — now just activate/deactivate overlays
    Q_INVOKABLE void launchEmission();
    Q_INVOKABLE void stopEmission();

signals:
    void modeChanged();
    void emissionLaunched();
    void emissionStopped();
    void overlaysActiveChanged();
    void activeProgramChanged();
    void programListChanged();
    void languageChanged();

private:
    void connectDetectionPipeline();
    void publishConfig();  // Send style/anim config to Vision Engine

    ConfigManager*     m_config         = nullptr;
    PreviewMonitor*    m_preview        = nullptr;
    OverlayController* m_overlay        = nullptr;
    ProfileManager*    m_profileManager = nullptr;
    SetupController*   m_setupController = nullptr;
    LiveController*    m_liveController  = nullptr;
    bool               m_liveMode       = false;
    bool               m_emissionStarting = false;

    // Always-On Passthrough state
    int                m_activeProgram   = -1;
    bool               m_overlaysActive  = false;

    // Detection subscriber thread
    QThread*           m_zmqDetThread   = nullptr;
    bool               m_detRunning     = false;

    // RSS & Subtitles
    RssFetcher*        m_rssFetcher     = nullptr;
    SubtitleController* m_subtitleCtrl  = nullptr;
    WeatherFetcher*    m_weatherFetcher = nullptr;

    // Macro engine
    MacroEngine*       m_macroEngine    = nullptr;
    AudioMeter*        m_audioMeter     = nullptr;

    // Social chat
    SocialChatController* m_socialChatCtrl = nullptr;

    // Web remote
    WebRemoteServer*   m_webRemote      = nullptr;

    // License
    LicenseManager*    m_license        = nullptr;

    // Analytics
    AnalyticsEngine*   m_analytics      = nullptr;

    // Graphics Queue
    GraphicsQueue*     m_graphicsQueue  = nullptr;

    // i18n
    QString            m_language        = "fr";

    // Sub-processes (auto-launched)
    QProcess*          m_aiProcess      = nullptr;
    QProcess*          m_visionProcess  = nullptr;
    void startSubProcesses();
    void stopSubProcesses();

    // Config publisher to Vision Engine (:5559)
    void*              m_configZmqCtx   = nullptr;
    void*              m_configZmqSock  = nullptr;
};

} // namespace prestige
