#pragma once

// ============================================================
// Prestige AI — Overlay Renderer
// Manages QML overlay items for compositing onto video
// Uses Qt Quick RHI pipeline (Metal on macOS, D3D/Vulkan on Windows)
// ============================================================

#include <QObject>
#include <QQuickItem>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QImage>
#include <QSize>
#include <QList>
#include <QElapsedTimer>

#include "TalentData.h"

namespace prestige {

class OverlayRenderer : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool globalVisible READ globalVisible WRITE setGlobalVisible NOTIFY globalVisibleChanged)

public:
    explicit OverlayRenderer(QObject* parent = nullptr);
    ~OverlayRenderer() override;

    bool initialize(QQmlEngine* engine, const QSize& resolution);
    void shutdown();

    void updateTalents(const QList<TalentOverlay>& talents);
    void setGlobalVisible(bool visible);
    bool globalVisible() const { return m_globalVisible; }
    void setStyle(const QString& styleName);

    QSize resolution() const { return m_resolution; }

signals:
    void globalVisibleChanged(bool visible);
    void talentsUpdated(int count);

private:
    void ensureNameplateCapacity(int count);

    QQmlEngine*    m_engine             = nullptr;
    QQmlComponent* m_nameplateComponent = nullptr;
    QQuickItem*    m_rootItem           = nullptr;

    QList<QQuickItem*> m_nameplates;
    QSize              m_resolution    = {1920, 1080};
    QString            m_currentStyle  = QStringLiteral("bfm");
    bool               m_globalVisible = true;
    bool               m_initialized  = false;
};

} // namespace prestige
