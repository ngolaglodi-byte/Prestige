// ============================================================
// Prestige AI — Macro Engine Implementation
// ============================================================

#include "MacroEngine.h"
#include "LiveController.h"
#include "OverlayController.h"

#include <QJsonObject>
#include <QDebug>

namespace prestige {

MacroEngine::MacroEngine(LiveController* live, OverlayController* overlay, QObject* parent)
    : QObject(parent)
    , m_live(live)
    , m_overlay(overlay)
{
    // Default macros
    {
        MacroDef m;
        m.name = "Afficher overlays";
        m.icon = "\u2728";
        QJsonObject a;
        a["type"] = "overlays_on";
        a["delayMs"] = 0;
        m.actions = QJsonArray{a};
        m_macros.append(m);
    }
    {
        MacroDef m;
        m.name = "Masquer overlays";
        m.icon = "\u{1F6AB}";
        QJsonObject a;
        a["type"] = "overlays_off";
        a["delayMs"] = 0;
        m.actions = QJsonArray{a};
        m_macros.append(m);
    }
    {
        MacroDef m;
        m.name = "Flash lower third 5s";
        m.icon = "\u26A1";
        QJsonObject a1, a2, a3;
        a1["type"] = "overlays_on"; a1["delayMs"] = 0;
        a2["type"] = "wait"; a2["delayMs"] = 5000;
        a3["type"] = "overlays_off"; a3["delayMs"] = 0;
        m.actions = QJsonArray{a1, a2, a3};
        m_macros.append(m);
    }
    {
        MacroDef m;
        m.name = "Bypass 10s";
        m.icon = "\u23F1";
        QJsonObject a1, a2, a3;
        a1["type"] = "bypass_on"; a1["delayMs"] = 0;
        a2["type"] = "wait"; a2["delayMs"] = 10000;
        a3["type"] = "bypass_off"; a3["delayMs"] = 0;
        m.actions = QJsonArray{a1, a2, a3};
        m_macros.append(m);
    }
}

QVariantList MacroEngine::macros() const
{
    QVariantList result;
    for (int i = 0; i < m_macros.size(); ++i) {
        QVariantMap map;
        map["name"] = m_macros[i].name;
        map["icon"] = m_macros[i].icon;
        map["actionCount"] = m_macros[i].actions.size();
        result.append(map);
    }
    return result;
}

void MacroEngine::executeMacro(int index)
{
    if (index < 0 || index >= m_macros.size()) return;
    if (m_executing) stopMacro();

    m_executing = true;
    m_currentIndex = index;
    m_actionIndex = 0;
    m_currentActions = m_macros[index].actions;

    emit executingChanged();
    qInfo() << "[MacroEngine] Executing macro:" << m_macros[index].name;

    executeNextAction();
}

void MacroEngine::stopMacro()
{
    m_executing = false;
    m_currentIndex = -1;
    m_actionIndex = 0;
    m_currentActions = QJsonArray();
    emit executingChanged();
    qInfo() << "[MacroEngine] Macro stopped";
}

void MacroEngine::addMacro(const QString& name, const QJsonArray& actions)
{
    MacroDef m;
    m.name = name;
    m.icon = "\u{1F3AC}";
    m.actions = actions;
    m_macros.append(m);
    emit macrosChanged();
}

void MacroEngine::removeMacro(int index)
{
    if (index < 0 || index >= m_macros.size()) return;
    m_macros.removeAt(index);
    emit macrosChanged();
}

void MacroEngine::executeNextAction()
{
    if (!m_executing) return;
    if (m_actionIndex >= m_currentActions.size()) {
        // Macro complete
        stopMacro();
        return;
    }

    QJsonObject action = m_currentActions[m_actionIndex].toObject();
    QString type = action["type"].toString();
    int delayMs = action["delayMs"].toInt(0);

    if (type == "overlays_on") {
        m_live->setOverlaysActive(true);
    } else if (type == "overlays_off") {
        m_live->setOverlaysActive(false);
    } else if (type == "bypass_on") {
        m_live->setOverlaysActive(false);
    } else if (type == "bypass_off") {
        m_live->setOverlaysActive(true);
    } else if (type == "wait") {
        // Just wait for delayMs then continue
        m_actionIndex++;
        QTimer::singleShot(delayMs, this, &MacroEngine::executeNextAction);
        return;
    }

    m_actionIndex++;

    if (delayMs > 0) {
        QTimer::singleShot(delayMs, this, &MacroEngine::executeNextAction);
    } else {
        // Use singleShot(0) to avoid deep recursion
        QTimer::singleShot(0, this, &MacroEngine::executeNextAction);
    }
}

} // namespace prestige
