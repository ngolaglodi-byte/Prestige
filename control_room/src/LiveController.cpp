// ============================================================
// Prestige AI — Live Controller Implementation
// ============================================================

#include "LiveController.h"
#include <QDebug>

namespace prestige {

LiveController::LiveController(QObject* parent)
    : QObject(parent)
    , m_statsTimer(new QTimer(this))
    , m_overlayDelayTimer(new QTimer(this))
    , m_recordingTimer(new QTimer(this))
    , m_countdownTimer(new QTimer(this))
{
    connect(m_statsTimer, &QTimer::timeout, this, &LiveController::updateStats);

    m_overlayDelayTimer->setSingleShot(true);
    connect(m_overlayDelayTimer, &QTimer::timeout, this, [this]() {
        // Delay expired — now actually show the overlay
        emit detectionChanged();
    });

    // Recording timer: updates duration every second
    connect(m_recordingTimer, &QTimer::timeout, this, [this]() {
        m_recordingElapsed++;
        m_recordingDuration = formattedDuration(m_recordingElapsed);
        emit recordingChanged();
    });

    // Broadcast overlay cycle timers
    m_talentHideTimer = new QTimer(this);
    m_talentHideTimer->setSingleShot(true);
    connect(m_talentHideTimer, &QTimer::timeout, this, [this]() {
        m_talentNameplateVisible = false;
        emit talentVisibleChanged();
        // Start title reappear timer
        if (!m_bypassed) {
            m_titleReappearTimer->start(m_titleReappearDelaySec * 1000);
        }
    });

    m_titleReappearTimer = new QTimer(this);
    m_titleReappearTimer->setSingleShot(true);
    connect(m_titleReappearTimer, &QTimer::timeout, this, [this]() {
        // Only show title if no active talent detection and not bypassed
        if (!m_talentNameplateVisible && !m_bypassed) {
            m_showTitleVisible = true;
            emit showTitleVisibleChanged();
        }
    });

    // Countdown timer: decrements every second
    connect(m_countdownTimer, &QTimer::timeout, this, [this]() {
        if (m_countdownSeconds > 0) {
            m_countdownSeconds--;
            emit countdownChanged();
        }
        if (m_countdownSeconds <= 0) {
            m_countdownActive = false;
            m_countdownTimer->stop();
            emit countdownChanged();
            qInfo() << "[Live] Countdown finished:" << m_countdownLabel;
        }
    });
}

void LiveController::setOverlaysActive(bool active)
{
    if (m_overlaysActive == active) return;
    m_overlaysActive = active;
    emit overlaysChanged();
    qInfo() << "[Live] Overlays" << (active ? "ON" : "OFF");
}

void LiveController::toggleOverlays()
{
    setOverlaysActive(!m_overlaysActive);
}

void LiveController::startLive()
{
    if (m_isLive) return;

    m_isLive = true;
    // Don't force overlays on — Always-On Passthrough starts in passthrough mode
    m_sceneType = "empty";
    m_faceCount = 0;
    m_bypassed = false;
    m_showTitleVisible = true;
    m_talentNameplateVisible = false;
    m_statsTimer->start(1000);

    emit liveStateChanged();
    emit liveStarted();
    emit overlaysChanged();
    emit sceneTypeChanged();
    emit showTitleVisibleChanged();
    emit talentVisibleChanged();
    emit bypassChanged();
    qInfo() << "[Live] Pipeline started (always-on passthrough)";
}

void LiveController::stopLive()
{
    // In Always-On Passthrough mode, stopLive disables overlay processing
    // but detection continues in background and pipeline keeps running
    if (!m_isLive) return;

    m_overlaysActive = false;
    m_overlayDelayTimer->stop();
    m_talentHideTimer->stop();
    m_titleReappearTimer->stop();

    emit overlaysChanged();
    qInfo() << "[Live] Overlay processing disabled (passthrough)";
}

QString LiveController::formattedDuration(int seconds) const
{
    int h = seconds / 3600;
    int m = (seconds % 3600) / 60;
    int s = seconds % 60;
    return QStringLiteral("%1:%2:%3")
        .arg(h, 2, 10, QLatin1Char('0'))
        .arg(m, 2, 10, QLatin1Char('0'))
        .arg(s, 2, 10, QLatin1Char('0'));
}

void LiveController::setMultiFaceConfig(const QString& name, const QString& role)
{
    m_multiFaceName = name;
    m_multiFaceRole = role;
    emit multiFaceConfigChanged();
    qInfo() << "[Live] Multi-face config:" << name << "/" << role;
}

void LiveController::setOverlayDelays(int singleMs, int multiMs, int hideMs)
{
    m_singleFaceDelayMs = singleMs;
    m_multiFaceDelayMs = multiMs;
    m_overlayHideDelayMs = hideMs;
    qInfo() << "[Live] Delays: single=" << singleMs << "ms, multi=" << multiMs << "ms, hide=" << hideMs << "ms";
}

void LiveController::setSelectedStyle(const QString& styleId)
{
    m_selectedStyle = styleId;
    qInfo() << "[Live] Style:" << styleId;
}

void LiveController::updateRealStats(double fps, double latencyMs)
{
    m_fps = static_cast<int>(fps);
    m_latencyMs = static_cast<int>(latencyMs);
    m_realStatsAvailable = true;
    emit statsChanged();
}

void LiveController::onTalentDetected(const QString& name, const QString& role, double conf)
{
    m_talentDetected = true;
    m_detectedName = name;
    m_detectedRole = role;
    m_confidence = conf;
    emit detectionChanged();
}

void LiveController::onMultiFaceDetected(int faceCount)
{
    onSceneChanged("interview", faceCount);
}

void LiveController::onSceneChanged(const QString& sceneType, int faceCount)
{
    bool changed = (m_sceneType != sceneType) || (m_faceCount != faceCount);
    m_sceneType = sceneType;
    m_faceCount = faceCount;

    if (changed) {
        emit sceneTypeChanged();

        if (sceneType == "interview" && faceCount >= 2) {
            // Multi-face: show the configured multi-face overlay
            m_detectedName = m_multiFaceName;
            m_detectedRole = m_multiFaceRole;
            m_talentDetected = !m_multiFaceName.isEmpty();
            m_confidence = 1.0;

            // Apply multi-face delay
            if (m_multiFaceDelayMs > 0 && m_overlayDelayTimer) {
                m_overlayDelayTimer->start(m_multiFaceDelayMs);
            } else {
                emit detectionChanged();
            }

            // Broadcast overlay cycle: talent detected → hide show title, show nameplate
            if (!m_bypassed) {
                m_titleReappearTimer->stop();
                if (m_showTitleVisible) {
                    m_showTitleVisible = false;
                    emit showTitleVisibleChanged();
                }
                m_talentNameplateVisible = true;
                emit talentVisibleChanged();
                m_talentHideTimer->start(m_talentDisplayDurationSec * 1000);
            }

            qInfo() << "[Live] Multi-face:" << faceCount << "faces → showing" << m_multiFaceName;
        } else if (sceneType == "single") {
            // Single face: the individual talent overlay is handled by onTalentDetected
            // Apply single-face delay
            if (m_singleFaceDelayMs > 0 && m_overlayDelayTimer) {
                m_overlayDelayTimer->start(m_singleFaceDelayMs);
            }

            // Broadcast overlay cycle: talent detected → hide show title, show nameplate
            if (!m_bypassed) {
                m_titleReappearTimer->stop();
                if (m_showTitleVisible) {
                    m_showTitleVisible = false;
                    emit showTitleVisibleChanged();
                }
                m_talentNameplateVisible = true;
                emit talentVisibleChanged();
                m_talentHideTimer->start(m_talentDisplayDurationSec * 1000);
            }
        } else if (sceneType == "empty") {
            // No face: hide overlay after delay
            if (m_overlayHideDelayMs > 0 && m_overlayDelayTimer) {
                m_overlayDelayTimer->start(m_overlayHideDelayMs);
            }
            m_talentDetected = false;
            m_faceCount = 0;

            // If talent nameplate was visible, start hiding it
            if (m_talentNameplateVisible && !m_bypassed) {
                m_talentHideTimer->stop();
                m_talentNameplateVisible = false;
                emit talentVisibleChanged();
                m_titleReappearTimer->start(m_titleReappearDelaySec * 1000);
            }
        }
    }
}

void LiveController::onTalentLost()
{
    if (m_overlayHideDelayMs > 0) {
        // Don't hide immediately — wait for the configured delay
        m_overlayDelayTimer->start(m_overlayHideDelayMs);
    } else {
        m_talentDetected = false;
        m_detectedName.clear();
        m_detectedRole.clear();
        m_confidence = 0.0;
        emit detectionChanged();
    }
}

// ── Recording ──────────────────────────────────────────────

void LiveController::toggleRecording()
{
    m_isRecording = !m_isRecording;
    if (m_isRecording) {
        m_recordingElapsed = 0;
        m_recordingDuration = "00:00:00";
        m_recordingTimer->start(1000);
        qInfo() << "[Live] Recording started → " << m_recordingPath;
    } else {
        m_recordingTimer->stop();
        qInfo() << "[Live] Recording stopped (" << m_recordingDuration << ")";
    }
    emit recordingChanged();
}

void LiveController::setRecordingPath(const QString& path)
{
    if (m_recordingPath == path) return;
    m_recordingPath = path;
    emit recordingChanged();
    qInfo() << "[Live] Recording path:" << path;
}

// ── Countdown ─────────────────────────────────────────────

void LiveController::startCountdown(int totalSeconds, const QString& label)
{
    m_countdownSeconds = totalSeconds;
    m_countdownLabel = label;
    m_countdownActive = true;
    m_countdownTimer->start(1000);
    emit countdownChanged();
    qInfo() << "[Live] Countdown started:" << totalSeconds << "s —" << label;
}

void LiveController::stopCountdown()
{
    m_countdownActive = false;
    m_countdownSeconds = 0;
    m_countdownTimer->stop();
    emit countdownChanged();
    qInfo() << "[Live] Countdown stopped";
}

// ── Broadcast overlay cycle ──────────────────────────────

void LiveController::setBypassed(bool b)
{
    if (m_bypassed == b) return;
    m_bypassed = b;

    if (b) {
        // Bypass ON: hide show title and talent nameplate
        m_showTitleVisible = false;
        m_talentNameplateVisible = false;
        m_talentHideTimer->stop();
        m_titleReappearTimer->stop();
        emit showTitleVisibleChanged();
        emit talentVisibleChanged();
    } else {
        // Bypass OFF: restore normal state
        if (!m_talentDetected) {
            m_showTitleVisible = true;
            emit showTitleVisibleChanged();
        }
    }

    emit bypassChanged();
    qInfo() << "[Live] Bypass" << (b ? "ON" : "OFF");
}

void LiveController::setTalentTimingConfig(int displaySec, int reappearSec)
{
    m_talentDisplayDurationSec = displaySec;
    m_titleReappearDelaySec = reappearSec;
    qInfo() << "[Live] Talent timing: display=" << displaySec << "s, reappear=" << reappearSec << "s";
}

// ── QR Code ──────────────────────────────────────────────

void LiveController::setQrCode(const QString& url, bool visible, const QString& position)
{
    m_qrCodeUrl = url;
    m_qrCodeVisible = visible;
    m_qrCodePosition = position;
    emit qrCodeChanged();
    qInfo() << "[Live] QR Code:" << url << "visible:" << visible << "pos:" << position;
}

void LiveController::updateStats()
{
    // If real stats are available (from PreviewMonitor FPS), use them
    // Otherwise these remain at last known values
    if (!m_realStatsAvailable) {
        // No data yet — show zeros to indicate "waiting for pipeline"
        m_fps = 0;
        m_latencyMs = 0;
    }

    // CPU/GPU are not easily measurable without platform-specific APIs
    // Show 0 instead of fake numbers
    m_cpuPercent = 0;
    m_gpuPercent = 0;

    emit statsChanged();
    emit outputStatusChanged();
}

} // namespace prestige
