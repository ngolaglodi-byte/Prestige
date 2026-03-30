// ============================================================
// Prestige AI — Control Room
// Copyright (c) 2024-2026 Prestige Technologie Company
// All rights reserved.
// ============================================================

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQuickImageProvider>
#include <QUrl>
#include <QDebug>
#include <QMutex>
#include <QMutexLocker>
#include <QQmlContext>
#include <QIcon>

#include "MainWindow.h"
#include "PreviewMonitor.h"

// ── Live preview image provider ────────────────────────────
// QML uses: Image { source: "image://preview/frame" }
// Updated every time PreviewMonitor receives a new frame.

class PreviewImageProvider : public QQuickImageProvider {
public:
    PreviewImageProvider(prestige::PreviewMonitor* monitor)
        : QQuickImageProvider(QQuickImageProvider::Image)
        , m_monitor(monitor) {}

    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override {
        Q_UNUSED(id) Q_UNUSED(requestedSize)
        QImage frame = m_monitor->latestFrame();
        if (size) *size = frame.size();
        return frame;
    }

private:
    prestige::PreviewMonitor* m_monitor;
};

int main(int argc, char* argv[])
{
    QQuickStyle::setStyle("Basic");

    QApplication app(argc, argv);
    app.setOrganizationName("Prestige Technologie Company");
    app.setOrganizationDomain("prestigetech.com");
    app.setApplicationName("Prestige AI");
    app.setApplicationVersion("1.0.0");
    app.setWindowIcon(QIcon(QStringLiteral(":/icons/prestige_icon.png")));

    // ── Main controller ────────────────────────────────────
    prestige::MainWindow mainWindow;

    // ── QML engine ─────────────────────────────────────────
    QQmlApplicationEngine engine;
    mainWindow.initialize(&engine);

    // ── Live preview provider ──────────────────────────────
    // Find the PreviewMonitor from context
    auto* previewMonitor = engine.rootContext()->contextProperty("previewMonitor").value<prestige::PreviewMonitor*>();
    if (previewMonitor) {
        engine.addImageProvider("preview", new PreviewImageProvider(previewMonitor));
        previewMonitor->start(); // Connect to Vision Engine :5558
        qInfo() << "[ControlRoom] Live preview provider registered";
    }

    const QUrl mainQml(QStringLiteral("qrc:/PrestigeControl/qml/MainView.qml"));

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [mainQml](QObject* obj, const QUrl& url) {
            if (!obj && url == mainQml) {
                qCritical() << "[ControlRoom] Failed to load MainView.qml";
                QCoreApplication::exit(-1);
            }
        }, Qt::QueuedConnection);

    engine.load(mainQml);

    qInfo() << "[ControlRoom] Started — Prestige AI Control Room v1.0.0";

    return app.exec();
}
