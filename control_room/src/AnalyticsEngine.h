#pragma once

// ============================================================
// Prestige AI -- Analytics Engine
// Real-time broadcast analytics and report generation.
// ============================================================

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <QDateTime>
#include <QTimer>
#include <QMap>

namespace prestige {

class AnalyticsEngine : public QObject {
    Q_OBJECT
    Q_PROPERTY(int totalDetections READ totalDetections NOTIFY statsChanged)
    Q_PROPERTY(int totalOverlays READ totalOverlays NOTIFY statsChanged)
    Q_PROPERTY(int elapsedSeconds READ elapsedSeconds NOTIFY statsChanged)
    Q_PROPERTY(double avgFps READ avgFps NOTIFY statsChanged)
    Q_PROPERTY(double avgLatency READ avgLatency NOTIFY statsChanged)
    Q_PROPERTY(QVariantList talentStats READ talentStats NOTIFY statsChanged)
    Q_PROPERTY(QVariantList detectionTimeline READ detectionTimeline NOTIFY statsChanged)
    Q_PROPERTY(QString currentTalent READ currentTalent NOTIFY statsChanged)

public:
    explicit AnalyticsEngine(QObject* parent = nullptr);

    int totalDetections() const { return m_totalDetections; }
    int totalOverlays() const { return m_totalOverlays; }
    int elapsedSeconds() const { return m_elapsedSeconds; }
    double avgFps() const { return m_avgFps; }
    double avgLatency() const { return m_avgLatency; }
    QVariantList talentStats() const;
    QVariantList detectionTimeline() const;
    QString currentTalent() const { return m_currentTalent; }

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void reset();
    Q_INVOKABLE void recordDetection(const QString& talentName, const QString& role, double confidence);
    Q_INVOKABLE void recordFps(double fps);
    Q_INVOKABLE void recordLatency(double ms);
    Q_INVOKABLE void recordOverlayShown();
    Q_INVOKABLE QString generateReport() const;
    Q_INVOKABLE void exportReportToFile(const QString& path) const;

signals:
    void statsChanged();
    void reportGenerated(const QString& content);

private:
    struct TalentStat {
        QString name;
        QString role;
        int detectionCount = 0;
        int screenTimeSeconds = 0;
        double avgConfidence = 0.0;
        double totalConfidence = 0.0;
        QDateTime firstSeen;
        QDateTime lastSeen;
    };

    struct TimelineEntry {
        int secondsFromStart;
        QString talentName;
        QString event; // "detected", "lost"
    };

    QTimer* m_timer = nullptr;
    QMap<QString, TalentStat> m_talentMap;
    QList<TimelineEntry> m_timeline;
    int m_totalDetections = 0;
    int m_totalOverlays = 0;
    int m_elapsedSeconds = 0;
    double m_avgFps = 0.0;
    double m_avgLatency = 0.0;
    int m_fpsCount = 0;
    double m_fpsSum = 0.0;
    int m_latencyCount = 0;
    double m_latencySum = 0.0;
    QString m_currentTalent;
    QDateTime m_startTime;
    bool m_running = false;
};

} // namespace prestige
