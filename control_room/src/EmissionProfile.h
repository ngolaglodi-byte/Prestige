#pragma once

// ============================================================
// Prestige AI — Emission Profile
// Pre-production configuration saved per show
// ============================================================

#include <QObject>
#include <QString>
#include <QStringList>
#include <QColor>
#include <QDateTime>
#include <QJsonObject>
#include <QList>

namespace prestige {

struct EmissionProfile {
    QString     id;
    QString     name;               // "JT 20h", "Matinale", "Débat politique"
    QString     overlayStyleId;     // "bfm", "lci", "minimal"...
    QString     animationType = "slide_left"; // Animation type (independent from style)
    int         animEnterFrames = 15;  // Duration enter animation (frames)
    int         animExitFrames  = 8;   // Duration exit animation (frames)
    QColor      accentColor = QColor("#E30613");
    double      backgroundOpacity = 0.8;
    int         overlayPosition = 0;  // 0=bas-gauche, 1=bas-centre, 2=bas-droite, 3=haut-gauche
    QString     logoPath;

    // Source vidéo
    QString     inputType = "webcam";   // "ndi", "decklink", "webcam"
    QString     inputSource;            // NDI source name or device index

    // Sorties actives
    bool        outputSDI  = false;
    bool        outputNDI  = false;
    bool        outputRTMP = false;
    bool        outputSRT  = false;
    QString     rtmpUrl;
    QString     rtmpKey;
    QString     srtUrl;

    // Talents assignés
    QStringList talentIds;

    // Réseaux sociaux (clés de stream RTMP)
    QJsonObject socialMediaConfigs;  // {"youtube": {"enabled":true,"key":"xxx"}, ...}

    // Multi-face overlay (when 2+ faces detected)
    QString     multiFaceName;          // e.g. "JT 20h — Le Débat"
    QString     multiFaceRole;          // e.g. "En direct"
    QString     multiFaceStyleId;       // Style for multi-face (can differ from single)

    // Overlay timing (milliseconds delay before overlay appears)
    int         singleFaceDelayMs = 1000;  // Delay for single face overlay
    int         multiFaceDelayMs  = 500;   // Delay for multi-face overlay
    int         overlayHideDelayMs = 2000; // Delay before hiding after face lost

    QDateTime   createdAt;
    QDateTime   lastUsed;

    QJsonObject toJson() const;
    static EmissionProfile fromJson(const QJsonObject& obj);
};

class ProfileManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QStringList profileNames READ profileNames NOTIFY profilesChanged)
    Q_PROPERTY(QString currentProfileId READ currentProfileId NOTIFY currentProfileChanged)

public:
    explicit ProfileManager(QObject* parent = nullptr);

    QList<EmissionProfile> loadAll();
    EmissionProfile loadById(const QString& id);
    void save(const EmissionProfile& profile);
    void remove(const QString& id);

    Q_INVOKABLE EmissionProfile createNew(const QString& name);
    Q_INVOKABLE bool loadProfile(const QString& id);
    Q_INVOKABLE QStringList profileNames() const;

    QString currentProfileId() const { return m_currentId; }
    const EmissionProfile& current() const { return m_current; }
    EmissionProfile& current() { return m_current; }

signals:
    void profilesChanged();
    void currentProfileChanged();
    void profileLoaded(const QString& name);

private:
    QString profilesDir() const;

    EmissionProfile         m_current;
    QString                 m_currentId;
    QList<EmissionProfile>  m_cache;
};

} // namespace prestige
