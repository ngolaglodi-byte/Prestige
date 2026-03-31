// ============================================================
// Prestige AI — Setup Controller Implementation
// ============================================================

#include "SetupController.h"
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMovie>
#include <QImage>
#include <QBuffer>

namespace prestige {

SetupController::SetupController(ProfileManager* profiles, QObject* parent)
    : QObject(parent)
    , m_profiles(profiles)
{
    // Load or create default profile
    auto all = m_profiles->loadAll();
    if (all.isEmpty()) {
        m_profile = m_profiles->createNew("Nouvelle émission");
    } else {
        m_profile = all.first();
    }
    m_profiles->loadProfile(m_profile.id);
}

QString SetupController::currentProfileName() const
{
    return m_profile.name;
}

QString SetupController::emissionName() const
{
    return m_profile.name;
}

void SetupController::setEmissionName(const QString& name)
{
    if (m_profile.name == name) return;
    m_profile.name = name;
    emit profileChanged();
}

void SetupController::setSelectedStyle(const QString& id)
{
    if (m_profile.overlayStyleId == id) return;
    m_profile.overlayStyleId = id;
    emit styleChanged();
}

void SetupController::setAccentColor(const QColor& c)
{
    if (m_profile.accentColor == c) return;
    m_profile.accentColor = c;
    emit styleChanged();
}

void SetupController::setBackgroundOpacity(double v)
{
    if (qFuzzyCompare(m_profile.backgroundOpacity, v)) return;
    m_profile.backgroundOpacity = v;
    emit styleChanged();
}

void SetupController::setOverlayPosition(int pos)
{
    if (m_profile.overlayPosition == pos) return;
    m_profile.overlayPosition = pos;
    emit styleChanged();
}

void SetupController::setLogoPath(const QString& path)
{
    if (m_profile.logoPath == path) return;
    m_profile.logoPath = path;
    emit styleChanged();
}

// ── Animation ───────────────────────────────────────────────

void SetupController::setAnimationType(const QString& type)
{
    if (m_profile.animationType == type) return;
    m_profile.animationType = type;
    emit animationChanged();
}

void SetupController::setAnimEnterFrames(int frames)
{
    if (m_profile.animEnterFrames == frames) return;
    m_profile.animEnterFrames = qBound(3, frames, 60);
    emit animationChanged();
}

void SetupController::setAnimExitFrames(int frames)
{
    if (m_profile.animExitFrames == frames) return;
    m_profile.animExitFrames = qBound(2, frames, 30);
    emit animationChanged();
}

// ── Multi-face overlay ──────────────────────────────────────

void SetupController::setMultiFaceName(const QString& v)
{
    if (m_profile.multiFaceName == v) return;
    m_profile.multiFaceName = v;
    emit overlayTimingChanged();
}

void SetupController::setMultiFaceRole(const QString& v)
{
    if (m_profile.multiFaceRole == v) return;
    m_profile.multiFaceRole = v;
    emit overlayTimingChanged();
}

void SetupController::setMultiFaceStyleId(const QString& v)
{
    if (m_profile.multiFaceStyleId == v) return;
    m_profile.multiFaceStyleId = v;
    emit overlayTimingChanged();
}

void SetupController::setSingleFaceDelayMs(int v)
{
    if (m_profile.singleFaceDelayMs == v) return;
    m_profile.singleFaceDelayMs = v;
    emit overlayTimingChanged();
}

void SetupController::setMultiFaceDelayMs(int v)
{
    if (m_profile.multiFaceDelayMs == v) return;
    m_profile.multiFaceDelayMs = v;
    emit overlayTimingChanged();
}

void SetupController::setOverlayHideDelayMs(int v)
{
    if (m_profile.overlayHideDelayMs == v) return;
    m_profile.overlayHideDelayMs = v;
    emit overlayTimingChanged();
}

// ── Source ──────────────────────────────────────────────────

void SetupController::setInputType(const QString& type)
{
    if (m_profile.inputType == type) return;
    m_profile.inputType = type;
    emit sourceChanged();
}

void SetupController::setInputSource(const QString& src)
{
    if (m_profile.inputSource == src) return;
    m_profile.inputSource = src;
    emit sourceChanged();
}

void SetupController::setOutputSDI(bool v)  { if (m_profile.outputSDI != v)  { m_profile.outputSDI  = v; emit outputsChanged(); } }
void SetupController::setOutputNDI(bool v)  { if (m_profile.outputNDI != v)  { m_profile.outputNDI  = v; emit outputsChanged(); } }
void SetupController::setOutputRTMP(bool v) { if (m_profile.outputRTMP != v) { m_profile.outputRTMP = v; emit outputsChanged(); } }
void SetupController::setOutputSRT(bool v)  { if (m_profile.outputSRT != v)  { m_profile.outputSRT  = v; emit outputsChanged(); } }

void SetupController::setRtmpUrl(const QString& url)
{
    if (m_profile.rtmpUrl == url) return;
    m_profile.rtmpUrl = url;
    emit outputsChanged();
}

void SetupController::setRtmpKey(const QString& key)
{
    if (m_profile.rtmpKey == key) return;
    m_profile.rtmpKey = key;
    emit outputsChanged();
}

// ── Channel branding ────────────────────────────────────────

void SetupController::setChannelLogoPath(const QString& v) { if (m_channelLogoPath == v) return; m_channelLogoPath = v; emit brandingChanged(); }
void SetupController::setChannelLogoPosition(const QString& v) { if (m_channelLogoPosition == v) return; m_channelLogoPosition = v; emit brandingChanged(); }
void SetupController::setChannelLogoSize(int v) { if (m_channelLogoSize == v) return; m_channelLogoSize = qBound(30, v, 120); emit brandingChanged(); }
void SetupController::setShowChannelNameText(bool v) { if (m_showChannelNameText == v) return; m_showChannelNameText = v; emit brandingChanged(); }
void SetupController::setKeepLogoDuringAds(bool v) { if (m_keepLogoDuringAds == v) return; m_keepLogoDuringAds = v; emit brandingChanged(); }

// ── Channel logo/name offsets ────────────────────────────────

void SetupController::setChannelLogoOffsetX(int v) { if (m_channelLogoOffsetX == v) return; m_channelLogoOffsetX = v; emit brandingChanged(); }
void SetupController::setChannelLogoOffsetY(int v) { if (m_channelLogoOffsetY == v) return; m_channelLogoOffsetY = v; emit brandingChanged(); }
void SetupController::setChannelNameOffsetX(int v) { if (m_channelNameOffsetX == v) return; m_channelNameOffsetX = v; emit brandingChanged(); }
void SetupController::setChannelNameOffsetY(int v) { if (m_channelNameOffsetY == v) return; m_channelNameOffsetY = v; emit brandingChanged(); }

// ── Channel name design ─────────────────────────────────────

void SetupController::setChannelNameShape(const QString& v) { if (m_channelNameShape == v) return; m_channelNameShape = v; emit brandingChanged(); }
void SetupController::setChannelNameBgColor(const QString& v) { if (m_channelNameBgColor == v) return; m_channelNameBgColor = v; emit brandingChanged(); }
void SetupController::setChannelNameTextColor(const QString& v) { if (m_channelNameTextColor == v) return; m_channelNameTextColor = v; emit brandingChanged(); }
void SetupController::setChannelNameBorderColor(const QString& v) { if (m_channelNameBorderColor == v) return; m_channelNameBorderColor = v; emit brandingChanged(); }
void SetupController::setChannelNameFontSize(int v) { if (m_channelNameFontSize == v) return; m_channelNameFontSize = qBound(8, v, 36); emit brandingChanged(); }

// ── Entry animations ────────────────────────────────────────

void SetupController::setLogoEntryAnim(const QString& v) { if (m_logoEntryAnim == v) return; m_logoEntryAnim = v; emit brandingChanged(); }
void SetupController::setNameEntryAnim(const QString& v) { if (m_nameEntryAnim == v) return; m_nameEntryAnim = v; emit brandingChanged(); }

// ── Loop animations ─────────────────────────────────────────

void SetupController::setLogoLoopAnim(const QString& v) { if (m_logoLoopAnim == v) return; m_logoLoopAnim = v; emit brandingChanged(); }
void SetupController::setNameLoopAnim(const QString& v) { if (m_nameLoopAnim == v) return; m_nameLoopAnim = v; emit brandingChanged(); }

// ── Show title ──────────────────────────────────────────────

void SetupController::setShowTitle(const QString& v) { if (m_showTitle == v) return; m_showTitle = v; emit showTitleChanged(); }
void SetupController::setShowSubtitle(const QString& v) { if (m_showSubtitle == v) return; m_showSubtitle = v; emit showTitleChanged(); }
void SetupController::setShowTitleEnabled(bool v) { if (m_showTitleEnabled == v) return; m_showTitleEnabled = v; emit showTitleChanged(); }
void SetupController::setShowTitlePosition(const QString& v) { if (m_showTitlePosition == v) return; m_showTitlePosition = v; emit showTitleChanged(); }

// ── Show title design & animation ───────────────────────

void SetupController::setShowTitleShape(const QString& v) { if (m_showTitleShape == v) return; m_showTitleShape = v; emit showTitleChanged(); }
void SetupController::setShowTitleBgColor(const QString& v) { if (m_showTitleBgColor == v) return; m_showTitleBgColor = v; emit showTitleChanged(); }
void SetupController::setShowTitleTextColor(const QString& v) { if (m_showTitleTextColor == v) return; m_showTitleTextColor = v; emit showTitleChanged(); }
void SetupController::setShowTitleBorderColor(const QString& v) { if (m_showTitleBorderColor == v) return; m_showTitleBorderColor = v; emit showTitleChanged(); }
void SetupController::setShowTitleFontSize(int v) { if (m_showTitleFontSize == v) return; m_showTitleFontSize = qBound(10, v, 28); emit showTitleChanged(); }
void SetupController::setShowTitleEntryAnim(const QString& v) { if (m_showTitleEntryAnim == v) return; m_showTitleEntryAnim = v; emit showTitleChanged(); }
void SetupController::setShowTitleLoopAnim(const QString& v) { if (m_showTitleLoopAnim == v) return; m_showTitleLoopAnim = v; emit showTitleChanged(); }

// ── Show title offsets ───────────────────────────────────────

void SetupController::setShowTitleOffsetX(int v) { if (m_showTitleOffsetX == v) return; m_showTitleOffsetX = v; emit showTitleChanged(); }
void SetupController::setShowTitleOffsetY(int v) { if (m_showTitleOffsetY == v) return; m_showTitleOffsetY = v; emit showTitleChanged(); }

// ── Ticker offset ───────────────────────────────────────────

void SetupController::setTickerOffsetY(int v) { if (m_tickerOffsetY == v) return; m_tickerOffsetY = v; emit brandingChanged(); }

// ── Subtitle offsets ────────────────────────────────────────

void SetupController::setSubtitleOffsetX(int v) { if (m_subtitleOffsetX == v) return; m_subtitleOffsetX = v; emit brandingChanged(); }
void SetupController::setSubtitleOffsetY(int v) { if (m_subtitleOffsetY == v) return; m_subtitleOffsetY = v; emit brandingChanged(); }

// ── Countdown offsets ───────────────────────────────────────

void SetupController::setCountdownOffsetX(int v) { if (m_countdownOffsetX == v) return; m_countdownOffsetX = v; emit brandingChanged(); }
void SetupController::setCountdownOffsetY(int v) { if (m_countdownOffsetY == v) return; m_countdownOffsetY = v; emit brandingChanged(); }

// ── Clock offsets ───────────────────────────────────────────

void SetupController::setClockOffsetX(int v) { if (m_clockOffsetX == v) return; m_clockOffsetX = v; emit brandingChanged(); }
void SetupController::setClockOffsetY(int v) { if (m_clockOffsetY == v) return; m_clockOffsetY = v; emit brandingChanged(); }

// ── QR Code offsets ─────────────────────────────────────────

void SetupController::setQrCodeOffsetX(int v) { if (m_qrCodeOffsetX == v) return; m_qrCodeOffsetX = v; emit brandingChanged(); }
void SetupController::setQrCodeOffsetY(int v) { if (m_qrCodeOffsetY == v) return; m_qrCodeOffsetY = v; emit brandingChanged(); }

// ── Scoreboard offsets ──────────────────────────────────────

void SetupController::setScoreboardOffsetX(int v) { if (m_scoreboardOffsetX == v) return; m_scoreboardOffsetX = v; emit brandingChanged(); }
void SetupController::setScoreboardOffsetY(int v) { if (m_scoreboardOffsetY == v) return; m_scoreboardOffsetY = v; emit brandingChanged(); }

// ── Weather offsets ─────────────────────────────────────────

void SetupController::setWeatherOffsetX(int v) { if (m_weatherOffsetX == v) return; m_weatherOffsetX = v; emit brandingChanged(); }
void SetupController::setWeatherOffsetY(int v) { if (m_weatherOffsetY == v) return; m_weatherOffsetY = v; emit brandingChanged(); }

// ── Talent display timing ───────────────────────────────────

void SetupController::setTalentDisplayDurationSec(int v) { if (m_talentDisplayDurationSec == v) return; m_talentDisplayDurationSec = qBound(3, v, 30); emit timingChanged(); }
void SetupController::setTitleReappearDelaySec(int v) { if (m_titleReappearDelaySec == v) return; m_titleReappearDelaySec = qBound(1, v, 10); emit timingChanged(); }

void SetupController::loadAnimatedLogo(const QString& path)
{
    QString localPath = path;
    if (localPath.startsWith("file://"))
        localPath = QUrl(localPath).toLocalFile();

    QFileInfo info(localPath);
    if (!info.exists()) {
        qWarning() << "[SetupController] Logo path not found:" << localPath;
        return;
    }

    if (info.isDir()) {
        // Load all .png files sorted by name from directory
        QDir dir(localPath);
        QStringList filters;
        filters << "*.png" << "*.PNG";
        QStringList files = dir.entryList(filters, QDir::Files, QDir::Name);
        qInfo() << "[SetupController] Loading animated logo from directory:" << localPath
                << "(" << files.size() << "frames)";
        // Store path — frames will be loaded by Vision Engine
        m_profile.logoPath = localPath;
        emit styleChanged();
    } else if (localPath.endsWith(".gif", Qt::CaseInsensitive)) {
        // GIF: extract frames using QMovie
        QMovie movie(localPath);
        if (!movie.isValid()) {
            qWarning() << "[SetupController] Invalid GIF:" << localPath;
            return;
        }
        int frameCount = movie.frameCount();
        qInfo() << "[SetupController] Loading animated GIF logo:" << localPath
                << "(" << frameCount << "frames)";
        m_profile.logoPath = localPath;
        emit styleChanged();
    } else {
        // Single image — static logo
        QImage img(localPath);
        if (img.isNull()) {
            qWarning() << "[SetupController] Cannot load logo image:" << localPath;
            return;
        }
        qInfo() << "[SetupController] Loaded static logo:" << localPath;
        m_profile.logoPath = localPath;
        emit styleChanged();
    }
}

bool SetupController::isReadyToGo() const
{
    // Must have a style selected and at least one output
    return !m_profile.overlayStyleId.isEmpty()
        && (m_profile.outputSDI || m_profile.outputNDI
            || m_profile.outputRTMP || m_profile.outputSRT
            || m_profile.inputType == "webcam"); // dev mode always OK
}

void SetupController::saveProfile()
{
    m_profiles->save(m_profile);
    qInfo() << "[SetupController] Profile saved:" << m_profile.name;
}

void SetupController::newProfile(const QString& name)
{
    m_profile = m_profiles->createNew(name);
    m_profiles->loadProfile(m_profile.id);
    emit profileChanged();
    emit styleChanged();
    emit sourceChanged();
    emit outputsChanged();
}

void SetupController::loadProfile(const QString& id)
{
    m_profiles->loadProfile(id);
    m_profile = m_profiles->current();
    emit profileChanged();
    emit styleChanged();
    emit sourceChanged();
    emit outputsChanged();
}

QVariantList SetupController::availableProfiles()
{
    QVariantList result;
    for (const auto& p : m_profiles->loadAll()) {
        QVariantMap map;
        map["id"] = p.id;
        map["name"] = p.name;
        map["lastUsed"] = p.lastUsed.toString("dd/MM/yyyy HH:mm");
        map["style"] = p.overlayStyleId;
        result.append(map);
    }
    return result;
}

void SetupController::exportProfile(const QString& filePath)
{
    QString path = filePath;
    if (path.startsWith("file://"))
        path = QUrl(path).toLocalFile();

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "[SetupController] Cannot write to:" << path;
        return;
    }
    QJsonDocument doc(m_profile.toJson());
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    qInfo() << "[SetupController] Profile exported to:" << path;
}

void SetupController::importProfile(const QString& filePath)
{
    QString path = filePath;
    if (path.startsWith("file://"))
        path = QUrl(path).toLocalFile();

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "[SetupController] Cannot read from:" << path;
        return;
    }
    QByteArray data = file.readAll();
    file.close();

    auto doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        qWarning() << "[SetupController] Invalid profile JSON:" << path;
        return;
    }

    EmissionProfile imported = EmissionProfile::fromJson(doc.object());
    // Create as new profile (new ID + name with "Importé" suffix)
    m_profile = m_profiles->createNew(imported.name + " (import\u00E9)");
    // Copy all settings from imported profile
    m_profile.overlayStyleId = imported.overlayStyleId;
    m_profile.animationType = imported.animationType;
    m_profile.animEnterFrames = imported.animEnterFrames;
    m_profile.animExitFrames = imported.animExitFrames;
    m_profile.accentColor = imported.accentColor;
    m_profile.backgroundOpacity = imported.backgroundOpacity;
    m_profile.overlayPosition = imported.overlayPosition;
    m_profile.logoPath = imported.logoPath;
    m_profile.inputType = imported.inputType;
    m_profile.inputSource = imported.inputSource;
    m_profile.outputSDI = imported.outputSDI;
    m_profile.outputNDI = imported.outputNDI;
    m_profile.outputRTMP = imported.outputRTMP;
    m_profile.outputSRT = imported.outputSRT;
    m_profile.rtmpUrl = imported.rtmpUrl;
    m_profile.rtmpKey = imported.rtmpKey;
    m_profile.multiFaceName = imported.multiFaceName;
    m_profile.multiFaceRole = imported.multiFaceRole;
    m_profile.multiFaceStyleId = imported.multiFaceStyleId;
    m_profile.singleFaceDelayMs = imported.singleFaceDelayMs;
    m_profile.multiFaceDelayMs = imported.multiFaceDelayMs;
    m_profile.overlayHideDelayMs = imported.overlayHideDelayMs;

    m_profiles->save(m_profile);
    m_profiles->loadProfile(m_profile.id);

    emit profileChanged();
    emit styleChanged();
    emit sourceChanged();
    emit outputsChanged();
    emit animationChanged();
    emit overlayTimingChanged();

    qInfo() << "[SetupController] Profile imported from:" << path << "as" << m_profile.name;
}

} // namespace prestige
