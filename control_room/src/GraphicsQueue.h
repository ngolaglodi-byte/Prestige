#pragma once

// ============================================================
// Prestige AI — Graphics Queue
// Manages a queue of overlay graphics for sequential playback
// ============================================================

#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include <QJsonArray>

namespace prestige {

class GraphicsQueue : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList items READ items NOTIFY queueChanged)
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY queueChanged)
    Q_PROPERTY(int count READ count NOTIFY queueChanged)
    Q_PROPERTY(QVariantMap currentItem READ currentItem NOTIFY queueChanged)
    Q_PROPERTY(QVariantMap nextItem READ nextItem NOTIFY queueChanged)

public:
    explicit GraphicsQueue(QObject* parent = nullptr);

    QVariantList items() const { return m_items; }
    int currentIndex() const { return m_currentIndex; }
    int count() const { return m_items.size(); }
    QVariantMap currentItem() const;
    QVariantMap nextItem() const;

    // Queue management
    Q_INVOKABLE void addLowerThird(const QString& name, const QString& role);
    Q_INVOKABLE void addTicker(const QString& text);
    Q_INVOKABLE void addMessage(const QString& text);
    Q_INVOKABLE void addQrCode(const QString& url);
    Q_INVOKABLE void removeItem(int index);
    Q_INVOKABLE void moveItem(int from, int to);
    Q_INVOKABLE void clearQueue();

    // Playback
    Q_INVOKABLE void takeNext();      // Send next item to program (air)
    Q_INVOKABLE void takePrevious();
    Q_INVOKABLE void takeItem(int index);
    Q_INVOKABLE void clearProgram();   // Remove current from air

signals:
    void queueChanged();
    void itemTaken(const QVariantMap& item); // Emitted when an item goes to air

private:
    QVariantList m_items;
    int m_currentIndex = -1;
};

} // namespace prestige
