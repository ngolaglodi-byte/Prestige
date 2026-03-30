// ============================================================
// Prestige AI — Overlay Renderer Implementation
// ============================================================

#include "OverlayRenderer.h"

#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickItem>
#include <QDebug>

namespace prestige {

OverlayRenderer::OverlayRenderer(QObject* parent)
    : QObject(parent)
{
}

OverlayRenderer::~OverlayRenderer()
{
    shutdown();
}

bool OverlayRenderer::initialize(QQmlEngine* engine, const QSize& resolution)
{
    if (m_initialized)
        return true;

    m_engine = engine;
    m_resolution = resolution;

    // Load the nameplate QML component
    m_nameplateComponent = new QQmlComponent(m_engine,
        QUrl(QStringLiteral("qrc:/PrestigeVision/qml/OverlayNameplate.qml")), this);

    if (m_nameplateComponent->isError()) {
        qWarning() << "[OverlayRenderer] Nameplate QML errors:" << m_nameplateComponent->errors();
    }

    m_initialized = true;
    qInfo() << "[OverlayRenderer] Initialized at" << resolution;
    return true;
}

void OverlayRenderer::shutdown()
{
    if (!m_initialized)
        return;

    qDeleteAll(m_nameplates);
    m_nameplates.clear();
    m_initialized = false;
}

void OverlayRenderer::updateTalents(const QList<TalentOverlay>& talents)
{
    if (!m_initialized || !m_globalVisible)
        return;

    ensureNameplateCapacity(talents.size());

    for (int i = 0; i < talents.size(); ++i) {
        QQuickItem* item = m_nameplates[i];
        const auto& t = talents[i];

        item->setProperty("talentName", t.name);
        item->setProperty("talentRole", t.role);
        item->setProperty("bboxX", t.bbox.x());
        item->setProperty("bboxY", t.bbox.y());
        item->setProperty("bboxW", t.bbox.width());
        item->setProperty("bboxH", t.bbox.height());
        item->setProperty("overlayVisible", t.showOverlay);
    }

    // Hide surplus nameplates
    for (int i = talents.size(); i < m_nameplates.size(); ++i) {
        m_nameplates[i]->setProperty("overlayVisible", false);
    }

    emit talentsUpdated(talents.size());
}

void OverlayRenderer::setGlobalVisible(bool visible)
{
    if (m_globalVisible == visible)
        return;

    m_globalVisible = visible;
    emit globalVisibleChanged(visible);

    if (!visible) {
        for (auto* item : m_nameplates)
            item->setProperty("overlayVisible", false);
    }
}

void OverlayRenderer::setStyle(const QString& styleName)
{
    m_currentStyle = styleName;
    qInfo() << "[OverlayRenderer] Style set to:" << styleName;
}

void OverlayRenderer::ensureNameplateCapacity(int count)
{
    while (m_nameplates.size() < count) {
        if (!m_nameplateComponent || !m_nameplateComponent->isReady()) {
            qWarning() << "[OverlayRenderer] Cannot create nameplate — component not ready";
            break;
        }

        auto* item = qobject_cast<QQuickItem*>(
            m_nameplateComponent->create(m_engine->rootContext()));

        if (item) {
            m_nameplates.append(item);
        }
    }
}

} // namespace prestige
