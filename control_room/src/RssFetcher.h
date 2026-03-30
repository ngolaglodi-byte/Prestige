#pragma once

// ============================================================
// Prestige AI — RSS Feed Fetcher
// Fetches headlines from RSS/Atom feeds for the broadcast ticker.
// Supports multiple feeds, auto-refresh, and error reporting.
// ============================================================

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QDateTime>

namespace prestige {

class RssFetcher : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString rssUrl READ rssUrl WRITE setRssUrl NOTIFY rssUrlChanged)
    Q_PROPERTY(int refreshIntervalSec READ refreshIntervalSec WRITE setRefreshIntervalSec NOTIFY refreshIntervalChanged)
    Q_PROPERTY(QString headlines READ headlines NOTIFY headlinesChanged)
    Q_PROPERTY(int feedCount READ feedCount NOTIFY headlinesChanged)
    Q_PROPERTY(QString lastFetchTime READ lastFetchTime NOTIFY headlinesChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorOccurred)
    Q_PROPERTY(bool fetching READ isFetching NOTIFY fetchingChanged)
    Q_PROPERTY(QString separator READ separator WRITE setSeparator NOTIFY separatorChanged)

public:
    explicit RssFetcher(QObject* parent = nullptr);

    QString rssUrl() const { return m_rssUrl; }
    void setRssUrl(const QString& url);

    int refreshIntervalSec() const { return m_refreshSec; }
    void setRefreshIntervalSec(int sec);

    QString headlines() const { return m_headlines; }
    int feedCount() const { return m_headlineCount; }
    QString lastFetchTime() const { return m_lastFetch.isValid() ? m_lastFetch.toString("HH:mm:ss") : "—"; }
    QString errorString() const { return m_error; }
    bool isFetching() const { return m_fetching; }

    QString separator() const { return m_separator; }
    void setSeparator(const QString& sep);

    Q_INVOKABLE void forceRefresh();
    Q_INVOKABLE void addFeed(const QString& url);
    Q_INVOKABLE void removeFeed(const QString& url);
    Q_INVOKABLE QStringList feedList() const { return m_feeds; }

signals:
    void rssUrlChanged();
    void refreshIntervalChanged();
    void headlinesChanged();
    void errorOccurred(const QString& error);
    void fetchingChanged();
    void separatorChanged();

private slots:
    void fetchFeeds();

private:
    void parseRssXml(const QByteArray& data, QStringList& titles);
    void parseAtomXml(const QByteArray& data, QStringList& titles);
    void updateHeadlines(const QStringList& titles);

    QNetworkAccessManager* m_nam = nullptr;
    QTimer*     m_timer       = nullptr;
    QString     m_rssUrl;
    QStringList m_feeds;
    QString     m_headlines;
    int         m_headlineCount = 0;
    int         m_refreshSec  = 60;
    QDateTime   m_lastFetch;
    QString     m_error;
    bool        m_fetching    = false;
    QString     m_separator   = QStringLiteral("  \u2022  ");  // bullet separator
    int         m_pendingReplies = 0;
    QStringList m_pendingTitles;
};

} // namespace prestige
