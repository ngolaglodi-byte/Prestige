// ============================================================
// Prestige AI -- Analytics Engine Implementation
// Copyright (c) 2024-2026 Prestige Technologie Company
// All rights reserved.
// ============================================================

#include "AnalyticsEngine.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <algorithm>

namespace prestige {

AnalyticsEngine::AnalyticsEngine(QObject* parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
{
    m_timer->setInterval(1000);
    connect(m_timer, &QTimer::timeout, this, [this]() {
        m_elapsedSeconds++;

        // Update screen time for current talent
        if (!m_currentTalent.isEmpty() && m_talentMap.contains(m_currentTalent)) {
            m_talentMap[m_currentTalent].screenTimeSeconds++;
        }

        emit statsChanged();
    });
}

void AnalyticsEngine::start()
{
    reset();
    m_startTime = QDateTime::currentDateTime();
    m_running = true;
    m_timer->start();
    qInfo() << "[Analytics] Started";
}

void AnalyticsEngine::stop()
{
    m_running = false;
    m_timer->stop();
    qInfo() << "[Analytics] Stopped at" << m_elapsedSeconds << "seconds";
}

void AnalyticsEngine::reset()
{
    m_timer->stop();
    m_talentMap.clear();
    m_timeline.clear();
    m_totalDetections = 0;
    m_totalOverlays = 0;
    m_elapsedSeconds = 0;
    m_avgFps = 0.0;
    m_avgLatency = 0.0;
    m_fpsCount = 0;
    m_fpsSum = 0.0;
    m_latencyCount = 0;
    m_latencySum = 0.0;
    m_currentTalent.clear();
    m_running = false;
    emit statsChanged();
    qInfo() << "[Analytics] Reset";
}

void AnalyticsEngine::recordDetection(const QString& talentName, const QString& role, double confidence)
{
    if (!m_running || talentName.isEmpty())
        return;

    m_totalDetections++;

    auto& stat = m_talentMap[talentName];
    if (stat.name.isEmpty()) {
        stat.name = talentName;
        stat.role = role;
        stat.firstSeen = QDateTime::currentDateTime();
    }
    stat.detectionCount++;
    stat.totalConfidence += confidence;
    stat.avgConfidence = stat.totalConfidence / stat.detectionCount;
    stat.lastSeen = QDateTime::currentDateTime();

    // Add timeline entry if talent changed
    if (m_currentTalent != talentName) {
        TimelineEntry entry;
        entry.secondsFromStart = m_elapsedSeconds;
        entry.talentName = talentName;
        entry.event = QStringLiteral("detected");
        m_timeline.append(entry);

        if (!m_currentTalent.isEmpty()) {
            TimelineEntry lostEntry;
            lostEntry.secondsFromStart = m_elapsedSeconds;
            lostEntry.talentName = m_currentTalent;
            lostEntry.event = QStringLiteral("lost");
            m_timeline.append(lostEntry);
        }

        m_currentTalent = talentName;
    }

    emit statsChanged();
}

void AnalyticsEngine::recordFps(double fps)
{
    if (!m_running) return;
    m_fpsCount++;
    m_fpsSum += fps;
    m_avgFps = m_fpsSum / m_fpsCount;
}

void AnalyticsEngine::recordLatency(double ms)
{
    if (!m_running) return;
    m_latencyCount++;
    m_latencySum += ms;
    m_avgLatency = m_latencySum / m_latencyCount;
}

void AnalyticsEngine::recordOverlayShown()
{
    if (!m_running) return;
    m_totalOverlays++;
    emit statsChanged();
}

QVariantList AnalyticsEngine::talentStats() const
{
    QVariantList result;

    // Sort talents by screen time descending
    QList<TalentStat> sorted = m_talentMap.values();
    std::sort(sorted.begin(), sorted.end(), [](const TalentStat& a, const TalentStat& b) {
        return a.screenTimeSeconds > b.screenTimeSeconds;
    });

    for (const auto& ts : sorted) {
        QVariantMap m;
        m["name"] = ts.name;
        m["role"] = ts.role;
        m["detections"] = ts.detectionCount;
        m["screenTime"] = ts.screenTimeSeconds;
        m["screenTimePercent"] = m_elapsedSeconds > 0
            ? (ts.screenTimeSeconds * 100.0 / m_elapsedSeconds) : 0.0;
        m["avgConfidence"] = ts.avgConfidence * 100.0;
        result.append(m);
    }

    return result;
}

QVariantList AnalyticsEngine::detectionTimeline() const
{
    QVariantList result;
    for (const auto& entry : m_timeline) {
        QVariantMap m;
        m["time"] = entry.secondsFromStart;
        m["talent"] = entry.talentName;
        m["event"] = entry.event;
        result.append(m);
    }
    return result;
}

static QString formatDuration(int totalSeconds)
{
    int h = totalSeconds / 3600;
    int m = (totalSeconds % 3600) / 60;
    int s = totalSeconds % 60;
    return QStringLiteral("%1:%2:%3")
        .arg(h, 2, 10, QLatin1Char('0'))
        .arg(m, 2, 10, QLatin1Char('0'))
        .arg(s, 2, 10, QLatin1Char('0'));
}

QString AnalyticsEngine::generateReport() const
{
    QString report;
    QTextStream out(&report);

    out << QString::fromUtf8(u8"\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550") << "\n";
    out << QString::fromUtf8(u8"PRESTIGE AI \u2014 RAPPORT D'\u00C9MISSION") << "\n";
    out << QString::fromUtf8(u8"\u00A9 Prestige Technologie Company") << "\n";
    out << "Date: " << QDateTime::currentDateTime().toString("yyyy-MM-dd") << "\n";
    out << QString::fromUtf8(u8"Dur\u00E9e: ") << formatDuration(m_elapsedSeconds) << "\n";
    out << QString::fromUtf8(u8"\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550") << "\n";
    out << "\n";

    out << QString::fromUtf8(u8"STATISTIQUES G\u00C9N\u00C9RALES") << "\n";
    out << QString::fromUtf8(u8"  D\u00E9tections totales: ") << m_totalDetections << "\n";
    out << QString::fromUtf8(u8"  Overlays affich\u00E9s: ") << m_totalOverlays << "\n";
    out << "  FPS moyen: " << QString::number(m_avgFps, 'f', 1) << "\n";
    out << "  Latence moyenne: " << QString::number(m_avgLatency, 'f', 1) << "ms\n";
    out << "\n";

    // Sort talents by screen time
    QList<TalentStat> sorted = m_talentMap.values();
    std::sort(sorted.begin(), sorted.end(), [](const TalentStat& a, const TalentStat& b) {
        return a.screenTimeSeconds > b.screenTimeSeconds;
    });

    out << "TEMPS D'ANTENNE PAR TALENT\n";
    int idx = 1;
    for (const auto& ts : sorted) {
        double pct = m_elapsedSeconds > 0
            ? (ts.screenTimeSeconds * 100.0 / m_elapsedSeconds) : 0.0;
        out << "  " << idx++ << ". " << ts.name << " (" << ts.role << ")\n";
        out << QString::fromUtf8(u8"     - D\u00E9tections: ") << ts.detectionCount << "\n";
        out << "     - Temps d'antenne: " << formatDuration(ts.screenTimeSeconds)
            << " (" << QString::number(pct, 'f', 0) << "%)\n";
        out << "     - Confiance moyenne: " << QString::number(ts.avgConfidence * 100.0, 'f', 1) << "%\n";
        out << "\n";
    }

    out << QString::fromUtf8(u8"TIMELINE DES D\u00C9TECTIONS") << "\n";
    for (const auto& entry : m_timeline) {
        if (entry.event == "detected") {
            out << "  " << formatDuration(entry.secondsFromStart)
                << QString::fromUtf8(u8" \u2014 ") << entry.talentName
                << QString::fromUtf8(u8" d\u00E9tect\u00E9(e)") << "\n";
        }
    }
    out << "\n";

    out << QString::fromUtf8(u8"\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550") << "\n";
    out << QString::fromUtf8(u8"G\u00E9n\u00E9r\u00E9 par Prestige AI v1.0.0") << "\n";

    return report;
}

void AnalyticsEngine::exportReportToFile(const QString& path) const
{
    QString filePath = path;
    // Strip file:// prefix if present (from QML file dialogs)
    if (filePath.startsWith("file://"))
        filePath = filePath.mid(7);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "[Analytics] Failed to write report to" << filePath << ":" << file.errorString();
        return;
    }

    QTextStream out(&file);
    out << generateReport();
    file.close();

    qInfo() << "[Analytics] Report exported to" << filePath;
    const_cast<AnalyticsEngine*>(this)->emit reportGenerated(filePath);
}

} // namespace prestige
