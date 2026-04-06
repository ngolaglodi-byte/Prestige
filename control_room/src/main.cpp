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
#include <QDir>
#include <QPainter>

#include "MainWindow.h"
#include "PreviewMonitor.h"
#include "SetupController.h"
#include "LottieEngine.h"

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

// ── Lottie style preview provider ─────────────────────────
// QML uses: Image { source: "image://lottie/presetId?frame" }
// Renders a Lottie animation frame locally for the style preview.

class LottiePreviewProvider : public QQuickImageProvider {
public:
    LottiePreviewProvider(prestige::SetupController* sc = nullptr)
        : QQuickImageProvider(QQuickImageProvider::Image), m_setupController(sc)
    {
        // Load presets from animations directory
        QString appDir = QCoreApplication::applicationDirPath();
        QStringList paths = {
            appDir + "/animations",
            appDir + "/../Resources/animations",
            appDir + "/../../resources/animations",
            appDir + "/../../../../../resources/animations",
            appDir + "/../../../../resources/animations",
        };
        for (const auto& p : paths) {
            if (QDir(p).exists()) {
                m_engine.loadPresets(p);
                qInfo() << "[LottiePreview] Loaded presets from" << p;
                break;
            }
        }
    }

    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override {
        Q_UNUSED(requestedSize)
        // id format: "[native:]presetId?frameCounter"
        QString rawId = id.section('?', 0, 0);
        int frameNum = id.section('?', 1, 1).toInt();

        // "native:" prefix = full-frame transparent overlay (for DEMO on preview)
        // Format: "native:single:presetId" or "native:multi:presetId" or just "presetId"
        bool nativeMode = rawId.startsWith("native:");
        bool multiMode = false;
        QString presetId = rawId;
        if (nativeMode) {
            QString rest = rawId.mid(7); // after "native:"
            if (rest.startsWith("multi:")) {
                multiMode = true;
                presetId = rest.mid(6); // after "multi:"
            } else if (rest.startsWith("single:")) {
                presetId = rest.mid(7); // after "single:"
            } else {
                presetId = rest;
            }
        }

        m_engine.setActivePreset(presetId);
        if (multiMode && m_setupController) {
            // Multi-face mode: use configured multi-face text
            QString mName = m_setupController->multiFaceName();
            QString mRole = m_setupController->multiFaceRole();
            m_engine.setTitle(mName.isEmpty() ? "Debat" : mName);
            m_engine.setSubtitle(mRole.isEmpty() ? "Edition Speciale" : mRole);
        } else {
            m_engine.setTitle("Marie Dupont");
            m_engine.setSubtitle("Journaliste");
        }
        double userOpacity = 1.0;
        if (m_setupController) {
            m_engine.setAccentColor(m_setupController->accentColor());
            m_engine.setFontSize(m_setupController->nameplateFontSize());
            m_engine.setTextColor(QColor(m_setupController->nameplateTextColor()));
            userOpacity = m_setupController->backgroundOpacity();
        }

        // Native mode: render at 16:9 with transparent background (overlay for DEMO)
        if (nativeMode) {
            QSize outSize(1280, 720);
            double fps = 20.0;
            double duration = m_engine.duration();
            // No +3.0 offset — play from frame 0 to show the entry animation
            double timeSec = (duration > 0) ? std::fmod(frameNum / fps, duration) : 0.0;
            QImage frame = m_engine.renderFrame(timeSec, outSize);
            if (size) *size = outSize;
            return frame.isNull() ? QImage(outSize, QImage::Format_ARGB32_Premultiplied) : frame;
        }

        // Render at 960x540 for sharper preview (16:9)
        QSize outSize(960, 540);
        double fps = 20.0; // matches QML timer (50ms = 20fps)
        double duration = m_engine.duration();
        double timeSec = (duration > 0) ? std::fmod((frameNum / fps) + 3.0, duration) : 3.0;

        // Pro broadcast preview background — dark studio gradient + subtle silhouette
        QImage output(outSize, QImage::Format_ARGB32_Premultiplied);
        {
            QPainter bg(&output);
            bg.setRenderHint(QPainter::Antialiasing);
            // Dark studio gradient
            QLinearGradient grad(0, 0, 0, outSize.height());
            grad.setColorAt(0.0, QColor(18, 22, 30));
            grad.setColorAt(0.5, QColor(10, 14, 22));
            grad.setColorAt(1.0, QColor(6, 8, 14));
            bg.fillRect(output.rect(), grad);
            // Subtle person silhouette (head + shoulders)
            bg.setPen(Qt::NoPen);
            int cx = outSize.width() / 2;
            // Head
            bg.setBrush(QColor(255, 255, 255, 8));
            bg.drawEllipse(QPointF(cx, outSize.height() * 0.22), outSize.width() * 0.05, outSize.width() * 0.05);
            // Shoulders
            bg.setBrush(QColor(255, 255, 255, 5));
            bg.drawRoundedRect(QRectF(cx - outSize.width() * 0.1, outSize.height() * 0.32,
                                      outSize.width() * 0.2, outSize.height() * 0.45),
                               outSize.width() * 0.02, outSize.width() * 0.02);
            bg.end();
        }

        // Render Lottie at full AE resolution then auto-center the content
        // This ensures the nameplate is always centered in the preview regardless of AE coords
        QSize aeSize(1914, 1080); // native AE canvas
        QImage frame = m_engine.renderFrame(timeSec, aeSize);

        if (!frame.isNull()) {
            // Find bounding box of non-transparent pixels
            int minX = aeSize.width(), minY = aeSize.height(), maxX = 0, maxY = 0;
            for (int y = 0; y < aeSize.height(); y += 4) { // sample every 4th row for speed
                const QRgb* row = reinterpret_cast<const QRgb*>(frame.constScanLine(y));
                for (int x = 0; x < aeSize.width(); x += 4) { // sample every 4th pixel
                    if (qAlpha(row[x]) > 10) {
                        if (x < minX) minX = x;
                        if (x > maxX) maxX = x;
                        if (y < minY) minY = y;
                        if (y > maxY) maxY = y;
                    }
                }
            }

            QPainter p(&output);
            p.setOpacity(userOpacity);
            if (maxX > minX && maxY > minY) {
                // Add padding around content
                int pad = 40;
                minX = qMax(0, minX - pad);
                minY = qMax(0, minY - pad);
                maxX = qMin(aeSize.width() - 1, maxX + pad);
                maxY = qMin(aeSize.height() - 1, maxY + pad);

                // Crop the content region from the AE frame
                QRect contentRect(minX, minY, maxX - minX, maxY - minY);
                QImage cropped = frame.copy(contentRect);

                // Scale to 70% of preview box so it doesn't fill edge-to-edge
                QSize fitSize(outSize.width() * 0.70, outSize.height() * 0.70);
                QImage scaled = cropped.scaled(fitSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                int dx = (outSize.width() - scaled.width()) / 2;
                int dy = (outSize.height() - scaled.height()) / 2;
                p.drawImage(dx, dy, scaled);
            } else {
                // Fallback: draw as-is scaled to fit
                QImage scaled = frame.scaled(outSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                int dx = (outSize.width() - scaled.width()) / 2;
                int dy = (outSize.height() - scaled.height()) / 2;
                p.drawImage(dx, dy, scaled);
            }
            p.end();
        }

        if (size) *size = output.size();
        return output;
    }

private:
    prestige::LottieEngine m_engine;
    prestige::SetupController* m_setupController = nullptr;
};

int main(int argc, char* argv[])
{
    // Force non-native menus on ALL platforms (native menus break with custom styling)
    qputenv("QT_QUICK_CONTROLS_USE_NATIVE_MENUS", "0");
    qputenv("QT_QUICK_CONTROLS_STYLE", "Basic");

    QQuickStyle::setStyle("Basic");

    QApplication app(argc, argv);
    // Disable native menu bar on macOS (forces Qt Quick Controls menus)
    app.setAttribute(Qt::AA_DontUseNativeMenuBar, true);
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

    // ── Lottie style preview provider ────────────────────────
    auto* setupCtrl = engine.rootContext()->contextProperty("setupController").value<prestige::SetupController*>();
    engine.addImageProvider("lottie", new LottiePreviewProvider(setupCtrl));

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
