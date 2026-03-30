#pragma once

// ============================================================
// Prestige AI — Plugin Interface
// Copyright (c) 2024-2026 Prestige Technologie Company
// All rights reserved.
//
// Feature 15: Plugin system for third-party extensions.
//
// Plugins are shared libraries loaded from ~/.prestige-ai/plugins/
// They can add overlays, panels, data sources, or automation.
// ============================================================

#include <QString>
#include <QJsonObject>

namespace prestige {

// Plugin capabilities
enum class PluginCapability {
    Overlay,        // Adds a custom overlay to the compositor
    Panel,          // Adds a UI panel to the Control Room
    DataSource,     // Provides data (RSS, API, scores, etc.)
    Automation,     // Macro/sequence actions
};

// Context passed to plugins for interacting with Prestige AI
struct PluginContext {
    // Send overlay data to the compositor
    // overlay: {"type":"custom", "text":"...", "x":100, "y":100, ...}
    std::function<void(const QJsonObject& overlay)> sendOverlay;

    // Send a config change to the vision engine
    std::function<void(const QString& key, const QString& value)> setConfig;

    // Log a message
    std::function<void(const QString& message)> log;
};

// ── Plugin Interface ───────────────────────────────────────
class IPrestigePlugin {
public:
    virtual ~IPrestigePlugin() = default;

    // Identity
    virtual QString pluginId() const = 0;
    virtual QString pluginName() const = 0;
    virtual QString pluginVersion() const = 0;
    virtual QString pluginAuthor() const = 0;

    // Lifecycle
    virtual bool initialize(PluginContext& context) = 0;
    virtual void shutdown() = 0;

    // Capabilities
    virtual QList<PluginCapability> capabilities() const = 0;

    // Optional: provide a QML panel URL for the Control Room
    virtual QString qmlPanelUrl() const { return {}; }

    // Optional: called every frame for overlay plugins
    virtual QJsonObject renderOverlay() { return {}; }
};

} // namespace prestige

// Macro for declaring a plugin
#define PRESTIGE_PLUGIN_IID "com.prestigeai.plugin/1.0"
Q_DECLARE_INTERFACE(prestige::IPrestigePlugin, PRESTIGE_PLUGIN_IID)
