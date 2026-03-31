// ============================================================
// Prestige AI — Graphics Queue Implementation
// ============================================================

#include "GraphicsQueue.h"
#include <QDebug>

namespace prestige {

GraphicsQueue::GraphicsQueue(QObject* parent)
    : QObject(parent)
{
}

QVariantMap GraphicsQueue::currentItem() const
{
    if (m_currentIndex >= 0 && m_currentIndex < m_items.size())
        return m_items.at(m_currentIndex).toMap();
    return QVariantMap();
}

QVariantMap GraphicsQueue::nextItem() const
{
    int next = m_currentIndex + 1;
    if (next >= 0 && next < m_items.size())
        return m_items.at(next).toMap();
    // If nothing is on air yet, the first item is "next"
    if (m_currentIndex < 0 && !m_items.isEmpty())
        return m_items.at(0).toMap();
    return QVariantMap();
}

void GraphicsQueue::addLowerThird(const QString& name, const QString& role)
{
    QVariantMap item;
    item["type"] = QStringLiteral("lower_third");
    item["name"] = name;
    item["role"] = role;
    item["status"] = QStringLiteral("ready");
    m_items.append(item);
    emit queueChanged();
    qInfo() << "[Queue] Added lower third:" << name << "/" << role;
}

void GraphicsQueue::addTicker(const QString& text)
{
    QVariantMap item;
    item["type"] = QStringLiteral("ticker");
    item["text"] = text;
    item["status"] = QStringLiteral("ready");
    m_items.append(item);
    emit queueChanged();
    qInfo() << "[Queue] Added ticker:" << text.left(40);
}

void GraphicsQueue::addMessage(const QString& text)
{
    QVariantMap item;
    item["type"] = QStringLiteral("message");
    item["text"] = text;
    item["status"] = QStringLiteral("ready");
    m_items.append(item);
    emit queueChanged();
    qInfo() << "[Queue] Added message:" << text.left(40);
}

void GraphicsQueue::addQrCode(const QString& url)
{
    QVariantMap item;
    item["type"] = QStringLiteral("qr_code");
    item["url"] = url;
    item["status"] = QStringLiteral("ready");
    m_items.append(item);
    emit queueChanged();
    qInfo() << "[Queue] Added QR code:" << url;
}

void GraphicsQueue::removeItem(int index)
{
    if (index < 0 || index >= m_items.size()) return;
    m_items.removeAt(index);
    if (m_currentIndex >= m_items.size())
        m_currentIndex = m_items.size() - 1;
    emit queueChanged();
}

void GraphicsQueue::moveItem(int from, int to)
{
    if (from < 0 || from >= m_items.size()) return;
    if (to < 0 || to >= m_items.size()) return;
    if (from == to) return;

    QVariant item = m_items.takeAt(from);
    m_items.insert(to, item);

    // Adjust currentIndex if it was affected
    if (m_currentIndex == from)
        m_currentIndex = to;
    else if (from < m_currentIndex && to >= m_currentIndex)
        m_currentIndex--;
    else if (from > m_currentIndex && to <= m_currentIndex)
        m_currentIndex++;

    emit queueChanged();
}

void GraphicsQueue::clearQueue()
{
    m_items.clear();
    m_currentIndex = -1;
    emit queueChanged();
    qInfo() << "[Queue] Cleared";
}

void GraphicsQueue::takeNext()
{
    if (m_items.isEmpty()) return;

    int nextIndex = m_currentIndex + 1;
    // If nothing on air yet, take the first item
    if (m_currentIndex < 0) nextIndex = 0;

    if (nextIndex >= m_items.size()) return;

    // Mark previous item as done
    if (m_currentIndex >= 0 && m_currentIndex < m_items.size()) {
        QVariantMap prev = m_items.at(m_currentIndex).toMap();
        prev["status"] = QStringLiteral("done");
        m_items[m_currentIndex] = prev;
    }

    // Set new current
    m_currentIndex = nextIndex;
    QVariantMap current = m_items.at(m_currentIndex).toMap();
    current["status"] = QStringLiteral("on_air");
    m_items[m_currentIndex] = current;

    emit queueChanged();
    emit itemTaken(current);
    qInfo() << "[Queue] TAKE:" << current["type"].toString() << current["name"].toString() << current["text"].toString();
}

void GraphicsQueue::takePrevious()
{
    if (m_items.isEmpty()) return;
    if (m_currentIndex <= 0) return;

    // Mark current item as ready (going back)
    if (m_currentIndex >= 0 && m_currentIndex < m_items.size()) {
        QVariantMap cur = m_items.at(m_currentIndex).toMap();
        cur["status"] = QStringLiteral("ready");
        m_items[m_currentIndex] = cur;
    }

    m_currentIndex--;
    QVariantMap current = m_items.at(m_currentIndex).toMap();
    current["status"] = QStringLiteral("on_air");
    m_items[m_currentIndex] = current;

    emit queueChanged();
    emit itemTaken(current);
    qInfo() << "[Queue] TAKE PREV:" << current["type"].toString();
}

void GraphicsQueue::takeItem(int index)
{
    if (index < 0 || index >= m_items.size()) return;

    // Mark previous item as done
    if (m_currentIndex >= 0 && m_currentIndex < m_items.size()) {
        QVariantMap prev = m_items.at(m_currentIndex).toMap();
        prev["status"] = QStringLiteral("done");
        m_items[m_currentIndex] = prev;
    }

    m_currentIndex = index;
    QVariantMap current = m_items.at(m_currentIndex).toMap();
    current["status"] = QStringLiteral("on_air");
    m_items[m_currentIndex] = current;

    emit queueChanged();
    emit itemTaken(current);
    qInfo() << "[Queue] TAKE item" << index << ":" << current["type"].toString();
}

void GraphicsQueue::clearProgram()
{
    if (m_currentIndex >= 0 && m_currentIndex < m_items.size()) {
        QVariantMap cur = m_items.at(m_currentIndex).toMap();
        cur["status"] = QStringLiteral("done");
        m_items[m_currentIndex] = cur;
    }
    m_currentIndex = -1;
    emit queueChanged();
    qInfo() << "[Queue] Program cleared";
}

} // namespace prestige
