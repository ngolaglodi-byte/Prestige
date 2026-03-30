// ============================================================
// Prestige AI — RSS Feed Fetcher Implementation
// ============================================================

#include "RssFetcher.h"
#include <QNetworkReply>
#include <QXmlStreamReader>
#include <QDebug>
#include <QUrl>

namespace prestige {

RssFetcher::RssFetcher(QObject* parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
    , m_timer(new QTimer(this))
{
    m_timer->setInterval(m_refreshSec * 1000);
    connect(m_timer, &QTimer::timeout, this, &RssFetcher::fetchFeeds);
}

void RssFetcher::setRssUrl(const QString& url)
{
    if (m_rssUrl == url) return;
    m_rssUrl = url;

    // Parse comma-separated URLs into feed list
    m_feeds.clear();
    const auto parts = url.split(',', Qt::SkipEmptyParts);
    for (const auto& p : parts) {
        QString trimmed = p.trimmed();
        if (!trimmed.isEmpty())
            m_feeds.append(trimmed);
    }

    emit rssUrlChanged();

    if (!m_feeds.isEmpty()) {
        fetchFeeds();
        m_timer->start();
    } else {
        m_timer->stop();
    }
}

void RssFetcher::setRefreshIntervalSec(int sec)
{
    sec = qBound(10, sec, 3600);
    if (m_refreshSec == sec) return;
    m_refreshSec = sec;
    m_timer->setInterval(sec * 1000);
    emit refreshIntervalChanged();
}

void RssFetcher::setSeparator(const QString& sep)
{
    if (m_separator == sep) return;
    m_separator = sep;
    emit separatorChanged();
}

void RssFetcher::addFeed(const QString& url)
{
    if (url.trimmed().isEmpty() || m_feeds.contains(url.trimmed())) return;
    m_feeds.append(url.trimmed());
    m_rssUrl = m_feeds.join(", ");
    emit rssUrlChanged();
    fetchFeeds();
    if (!m_timer->isActive()) m_timer->start();
}

void RssFetcher::removeFeed(const QString& url)
{
    if (m_feeds.removeAll(url.trimmed()) > 0) {
        m_rssUrl = m_feeds.join(", ");
        emit rssUrlChanged();
        if (m_feeds.isEmpty()) {
            m_timer->stop();
            m_headlines.clear();
            m_headlineCount = 0;
            emit headlinesChanged();
        } else {
            fetchFeeds();
        }
    }
}

void RssFetcher::forceRefresh()
{
    if (!m_feeds.isEmpty())
        fetchFeeds();
}

void RssFetcher::fetchFeeds()
{
    if (m_feeds.isEmpty()) return;

    m_fetching = true;
    emit fetchingChanged();

    m_pendingReplies = m_feeds.size();
    m_pendingTitles.clear();
    m_error.clear();

    for (const auto& feedUrl : m_feeds) {
        QUrl url(feedUrl);
        if (!url.isValid()) {
            m_pendingReplies--;
            continue;
        }

        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::UserAgentHeader, "PrestigeAI/1.0 Broadcast RSS Reader");
        request.setRawHeader("Accept", "application/rss+xml, application/atom+xml, application/xml, text/xml");

        auto* reply = m_nam->get(request);
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            reply->deleteLater();

            if (reply->error() != QNetworkReply::NoError) {
                m_error = reply->errorString();
                qWarning() << "[RSS] Fetch error:" << m_error << reply->url();
                emit errorOccurred(m_error);
            } else {
                QByteArray data = reply->readAll();

                QStringList titles;
                // Try RSS first, then Atom
                parseRssXml(data, titles);
                if (titles.isEmpty())
                    parseAtomXml(data, titles);

                m_pendingTitles.append(titles);

                if (!titles.isEmpty()) {
                    qInfo() << "[RSS]" << titles.size() << "headlines from" << reply->url().host();
                }
            }

            m_pendingReplies--;
            if (m_pendingReplies <= 0) {
                updateHeadlines(m_pendingTitles);
                m_lastFetch = QDateTime::currentDateTime();
                m_fetching = false;
                emit fetchingChanged();
            }
        });
    }
}

void RssFetcher::parseRssXml(const QByteArray& data, QStringList& titles)
{
    QXmlStreamReader xml(data);
    bool inItem = false;
    bool inTitle = false;

    while (!xml.atEnd()) {
        auto token = xml.readNext();
        if (token == QXmlStreamReader::StartElement) {
            if (xml.name() == u"item")
                inItem = true;
            else if (inItem && xml.name() == u"title")
                inTitle = true;
        } else if (token == QXmlStreamReader::EndElement) {
            if (xml.name() == u"item")
                inItem = false;
            else if (xml.name() == u"title")
                inTitle = false;
        } else if (token == QXmlStreamReader::Characters && inTitle) {
            QString text = xml.text().toString().trimmed();
            if (!text.isEmpty())
                titles.append(text);
        }
    }
}

void RssFetcher::parseAtomXml(const QByteArray& data, QStringList& titles)
{
    QXmlStreamReader xml(data);
    bool inEntry = false;
    bool inTitle = false;

    while (!xml.atEnd()) {
        auto token = xml.readNext();
        if (token == QXmlStreamReader::StartElement) {
            if (xml.name() == u"entry")
                inEntry = true;
            else if (inEntry && xml.name() == u"title")
                inTitle = true;
        } else if (token == QXmlStreamReader::EndElement) {
            if (xml.name() == u"entry")
                inEntry = false;
            else if (xml.name() == u"title")
                inTitle = false;
        } else if (token == QXmlStreamReader::Characters && inTitle) {
            QString text = xml.text().toString().trimmed();
            if (!text.isEmpty())
                titles.append(text);
        }
    }
}

void RssFetcher::updateHeadlines(const QStringList& titles)
{
    m_headlineCount = titles.size();
    m_headlines = titles.join(m_separator);
    emit headlinesChanged();
}

} // namespace prestige
