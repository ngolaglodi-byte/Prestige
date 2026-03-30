#pragma once
#include <QObject>
#include <QVariantList>
#include <QJsonArray>
#include <QTimer>

namespace prestige {
class LiveController;
class OverlayController;

class MacroEngine : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList macros READ macros NOTIFY macrosChanged)
    Q_PROPERTY(bool executing READ isExecuting NOTIFY executingChanged)
    Q_PROPERTY(int currentMacroIndex READ currentMacroIndex NOTIFY executingChanged)

public:
    explicit MacroEngine(LiveController* live, OverlayController* overlay, QObject* parent = nullptr);

    QVariantList macros() const;
    bool isExecuting() const { return m_executing; }
    int currentMacroIndex() const { return m_currentIndex; }

    Q_INVOKABLE void executeMacro(int index);
    Q_INVOKABLE void stopMacro();
    Q_INVOKABLE void addMacro(const QString& name, const QJsonArray& actions);
    Q_INVOKABLE void removeMacro(int index);

signals:
    void macrosChanged();
    void executingChanged();

private:
    void executeNextAction();

    struct MacroDef {
        QString name;
        QString icon;
        QJsonArray actions; // [{type, params, delayMs}]
    };

    QList<MacroDef> m_macros;
    LiveController* m_live;
    OverlayController* m_overlay;
    bool m_executing = false;
    int m_currentIndex = -1;
    int m_actionIndex = 0;
    QJsonArray m_currentActions;
};
} // namespace prestige
