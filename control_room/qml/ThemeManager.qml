pragma Singleton
import QtQuick 2.15

// Feature 14: Thème clair/sombre
QtObject {
    property bool isDark: true

    // Backgrounds
    readonly property color bg:        isDark ? "#07070A" : "#F0F0F4"
    readonly property color surface:   isDark ? "#111114" : "#FFFFFF"
    readonly property color surfaceAlt:isDark ? "#16161A" : "#F5F5F8"
    readonly property color card:      isDark ? "#1E1E22" : "#EEEEF2"

    // Text
    readonly property color textPrimary:   isDark ? "#FFFFFF" : "#1A1A1A"
    readonly property color textSecondary: isDark ? "#999999" : "#666666"
    readonly property color textMuted:     isDark ? "#555555" : "#AAAAAA"

    // Accent
    readonly property color accent: "#5B4FDB"
    readonly property color accentLight: isDark ? Qt.rgba(91/255,79/255,219/255,0.15) : Qt.rgba(91/255,79/255,219/255,0.1)

    // Borders
    readonly property color border:     isDark ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.08)
    readonly property color borderAlt:  isDark ? "#333333" : "#DDDDDD"

    // Status
    readonly property color success: "#1DB954"
    readonly property color danger:  "#CC0000"
    readonly property color warning: "#FF9800"

    // Overlay
    readonly property color modalBg: isDark ? Qt.rgba(0,0,0,0.6) : Qt.rgba(0,0,0,0.3)
}
