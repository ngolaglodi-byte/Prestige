#pragma once

// ============================================================
// Prestige AI — Live Controller
// Manages state during a live broadcast
// ============================================================

#include <QObject>
#include <QString>
#include <QTimer>

namespace prestige {

class LiveController : public QObject {
    Q_OBJECT

    // Detected talent
    Q_PROPERTY(bool    talentDetected READ talentDetected NOTIFY detectionChanged)
    Q_PROPERTY(QString detectedName READ detectedName NOTIFY detectionChanged)
    Q_PROPERTY(QString detectedRole READ detectedRole NOTIFY detectionChanged)
    Q_PROPERTY(double  confidence READ confidence NOTIFY detectionChanged)

    // Scene mode (single vs multi-face)
    Q_PROPERTY(QString sceneType READ sceneType NOTIFY sceneTypeChanged)
    Q_PROPERTY(int     faceCount READ faceCount NOTIFY sceneTypeChanged)
    Q_PROPERTY(bool    isMultiFace READ isMultiFace NOTIFY sceneTypeChanged)
    Q_PROPERTY(QString multiFaceDisplayName READ multiFaceDisplayName NOTIFY multiFaceConfigChanged)
    Q_PROPERTY(QString multiFaceDisplayRole READ multiFaceDisplayRole NOTIFY multiFaceConfigChanged)

    // Overlays
    Q_PROPERTY(bool overlaysActive READ overlaysActive WRITE setOverlaysActive NOTIFY overlaysChanged)

    // Broadcast overlay cycle
    Q_PROPERTY(bool showTitleVisible READ isShowTitleVisible NOTIFY showTitleVisibleChanged)
    Q_PROPERTY(bool talentNameplateVisible READ isTalentNameplateVisible NOTIFY talentVisibleChanged)
    Q_PROPERTY(bool isBypassed READ isBypassed WRITE setBypassed NOTIFY bypassChanged)

    // Technical stats
    Q_PROPERTY(int fps READ fps NOTIFY statsChanged)
    Q_PROPERTY(int latencyMs READ latencyMs NOTIFY statsChanged)
    Q_PROPERTY(int cpuPercent READ cpuPercent NOTIFY statsChanged)
    Q_PROPERTY(int gpuPercent READ gpuPercent NOTIFY statsChanged)

    // Output status
    Q_PROPERTY(bool sdiActive READ sdiActive NOTIFY outputStatusChanged)
    Q_PROPERTY(bool ndiActive READ ndiActive NOTIFY outputStatusChanged)
    Q_PROPERTY(bool rtmpActive READ rtmpActive NOTIFY outputStatusChanged)
    Q_PROPERTY(bool srtActive READ srtActive NOTIFY outputStatusChanged)

    // Emission state
    Q_PROPERTY(bool isLive READ isLive NOTIFY liveStateChanged)

    // Recording
    Q_PROPERTY(bool isRecording READ isRecording NOTIFY recordingChanged)
    Q_PROPERTY(QString recordingDuration READ recordingDuration NOTIFY recordingChanged)
    Q_PROPERTY(QString recordingPath READ recordingPath NOTIFY recordingChanged)

    // Countdown
    Q_PROPERTY(bool countdownActive READ countdownActive NOTIFY countdownChanged)
    Q_PROPERTY(int countdownSeconds READ countdownSeconds NOTIFY countdownChanged)
    Q_PROPERTY(QString countdownLabel READ countdownLabel NOTIFY countdownChanged)

    // QR Code
    Q_PROPERTY(QString qrCodeUrl READ qrCodeUrl NOTIFY qrCodeChanged)
    Q_PROPERTY(bool qrCodeVisible READ qrCodeVisible NOTIFY qrCodeChanged)
    Q_PROPERTY(QString qrCodePosition READ qrCodePosition NOTIFY qrCodeChanged)

public:
    explicit LiveController(QObject* parent = nullptr);

    // Detection
    bool talentDetected() const { return m_talentDetected; }
    QString detectedName() const { return m_detectedName; }
    QString detectedRole() const { return m_detectedRole; }
    double confidence() const { return m_confidence; }

    // Scene type
    QString sceneType() const { return m_sceneType; }
    int faceCount() const { return m_faceCount; }
    bool isMultiFace() const { return m_faceCount >= 2; }
    QString multiFaceDisplayName() const { return m_multiFaceName; }
    QString multiFaceDisplayRole() const { return m_multiFaceRole; }

    Q_INVOKABLE void setMultiFaceConfig(const QString& name, const QString& role);
    Q_INVOKABLE void setOverlayDelays(int singleMs, int multiMs, int hideMs);
    Q_INVOKABLE void setSelectedStyle(const QString& styleId);
    Q_INVOKABLE void updateRealStats(double fps, double latencyMs);

    // Overlays
    bool overlaysActive() const { return m_overlaysActive; }
    void setOverlaysActive(bool active);
    Q_INVOKABLE void toggleOverlays();

    // Broadcast overlay cycle
    bool isShowTitleVisible() const { return m_showTitleVisible; }
    bool isTalentNameplateVisible() const { return m_talentNameplateVisible; }
    bool isBypassed() const { return m_bypassed; }
    void setBypassed(bool b);
    void setTalentTimingConfig(int displaySec, int reappearSec);

    // Stats
    int fps() const { return m_fps; }
    int latencyMs() const { return m_latencyMs; }
    int cpuPercent() const { return m_cpuPercent; }
    int gpuPercent() const { return m_gpuPercent; }

    // Outputs
    bool sdiActive() const { return m_sdiActive; }
    bool ndiActive() const { return m_ndiActive; }
    bool rtmpActive() const { return m_rtmpActive; }
    bool srtActive() const { return m_srtActive; }

    // Live control
    bool isLive() const { return m_isLive; }
    Q_INVOKABLE void startLive();
    Q_INVOKABLE void stopLive();
    Q_INVOKABLE QString formattedDuration(int seconds) const;

    // Recording
    bool isRecording() const { return m_isRecording; }
    QString recordingDuration() const { return m_recordingDuration; }
    QString recordingPath() const { return m_recordingPath; }
    Q_INVOKABLE void toggleRecording();
    Q_INVOKABLE void setRecordingPath(const QString& path);

    // Countdown
    bool countdownActive() const { return m_countdownActive; }
    int countdownSeconds() const { return m_countdownSeconds; }
    QString countdownLabel() const { return m_countdownLabel; }
    Q_INVOKABLE void startCountdown(int totalSeconds, const QString& label);
    Q_INVOKABLE void stopCountdown();

    // QR Code
    QString qrCodeUrl() const { return m_qrCodeUrl; }
    bool qrCodeVisible() const { return m_qrCodeVisible; }
    QString qrCodePosition() const { return m_qrCodePosition; }
    Q_INVOKABLE void setQrCode(const QString& url, bool visible, const QString& position);

public slots:
    void onTalentDetected(const QString& name, const QString& role, double conf);
    void onMultiFaceDetected(int faceCount);
    void onSceneChanged(const QString& sceneType, int faceCount);
    void onTalentLost();

signals:
    void detectionChanged();
    void sceneTypeChanged();
    void multiFaceConfigChanged();
    void overlaysChanged();
    void statsChanged();
    void outputStatusChanged();
    void liveStateChanged();
    void liveStarted();
    void liveStopped();
    void recordingChanged();
    void countdownChanged();
    void qrCodeChanged();
    void showTitleVisibleChanged();
    void talentVisibleChanged();
    void bypassChanged();

private:
    void updateStats();

    // Detection
    bool    m_talentDetected = false;
    QString m_detectedName;
    QString m_detectedRole;
    double  m_confidence = 0.0;

    // Scene type
    QString m_sceneType = "empty";
    int     m_faceCount = 0;
    QString m_multiFaceName;
    QString m_multiFaceRole;

    // Overlay timing
    int     m_singleFaceDelayMs  = 1000;
    int     m_multiFaceDelayMs   = 500;
    int     m_overlayHideDelayMs = 2000;
    QTimer* m_overlayDelayTimer  = nullptr;

    // Overlays
    bool m_overlaysActive = true;

    // Stats
    int m_fps        = 0;
    int m_latencyMs  = 0;
    int m_cpuPercent = 0;
    int m_gpuPercent = 0;

    // Outputs
    bool m_sdiActive  = false;
    bool m_ndiActive  = false;
    bool m_rtmpActive = false;
    bool m_srtActive  = false;

    // State
    bool    m_isLive = false;
    QTimer* m_statsTimer = nullptr;
    QString m_selectedStyle;
    bool    m_realStatsAvailable = false;

    // Recording
    bool    m_isRecording = false;
    QString m_recordingDuration = "00:00:00";
    QString m_recordingPath;
    int     m_recordingElapsed = 0;
    QTimer* m_recordingTimer = nullptr;

    // Countdown
    bool    m_countdownActive = false;
    int     m_countdownSeconds = 0;
    QString m_countdownLabel;
    QTimer* m_countdownTimer = nullptr;

    // QR Code
    QString m_qrCodeUrl;
    bool    m_qrCodeVisible = false;
    QString m_qrCodePosition = "bottom_right";

    // Broadcast overlay cycle
    bool    m_showTitleVisible = true;
    bool    m_talentNameplateVisible = false;
    bool    m_bypassed = false;
    int     m_talentDisplayDurationSec = 8;
    int     m_titleReappearDelaySec = 2;
    QTimer* m_talentHideTimer = nullptr;
    QTimer* m_titleReappearTimer = nullptr;
};

} // namespace prestige
