pragma Singleton
import QtQuick 2.15

QtObject {
    property bool isDark: true

    // ── BACKGROUNDS ─────────────────────────────────────────
    readonly property color bg:         isDark ? "#07070A" : "#F0F0F4"
    readonly property color surface:    isDark ? "#0E0E12" : "#FFFFFF"
    readonly property color surfaceAlt: isDark ? "#141418" : "#F5F5F8"
    readonly property color card:       isDark ? "#1A1A1F" : "#EEEEF2"
    readonly property color cardHover:  isDark ? "#222228" : "#E6E6EC"
    readonly property color cardActive: isDark ? "#2A2A32" : "#DDDDE5"
    readonly property color elevated:   isDark ? "#1E1E24" : "#FFFFFF"

    // ── TEXT ─────────────────────────────────────────────────
    readonly property color textPrimary:   isDark ? "#F0F0F5" : "#0F0F14"
    readonly property color textSecondary: isDark ? "#8E8E9A" : "#5C5C6E"
    readonly property color textMuted:     isDark ? "#505060" : "#A0A0B0"
    readonly property color textOnAccent:  "#FFFFFF"

    // ── ACCENT ──────────────────────────────────────────────
    readonly property color accent:       "#6C5CE7"
    readonly property color accentHover:  "#7B6DEF"
    readonly property color accentActive: "#5A4BD4"
    readonly property color accentLight:  isDark ? Qt.rgba(108/255, 92/255, 231/255, 0.12) : Qt.rgba(108/255, 92/255, 231/255, 0.08)
    readonly property color accentGlow:   isDark ? Qt.rgba(108/255, 92/255, 231/255, 0.25) : Qt.rgba(108/255, 92/255, 231/255, 0.15)

    // ── BORDERS ─────────────────────────────────────────────
    readonly property color border:      isDark ? Qt.rgba(255, 255, 255, 0.06) : Qt.rgba(0, 0, 0, 0.08)
    readonly property color borderAlt:   isDark ? Qt.rgba(255, 255, 255, 0.10) : Qt.rgba(0, 0, 0, 0.12)
    readonly property color borderFocus: accent

    // ── STATUS COLORS ───────────────────────────────────────
    readonly property color success:     "#00D68F"
    readonly property color successBg:   isDark ? Qt.rgba(0, 214, 143, 0.10) : Qt.rgba(0, 214, 143, 0.08)
    readonly property color danger:      "#FF3D71"
    readonly property color dangerBg:    isDark ? Qt.rgba(255, 61, 113, 0.10) : Qt.rgba(255, 61, 113, 0.08)
    readonly property color warning:     "#FFAA00"
    readonly property color warningBg:   isDark ? Qt.rgba(255, 170, 0, 0.10) : Qt.rgba(255, 170, 0, 0.08)
    readonly property color info:        "#0095FF"
    readonly property color infoBg:      isDark ? Qt.rgba(0, 149, 255, 0.10) : Qt.rgba(0, 149, 255, 0.08)

    // ── BROADCAST-SPECIFIC ──────────────────────────────────
    readonly property color onAir:       "#FF2D55"
    readonly property color onAirGlow:   Qt.rgba(255, 45, 85, 0.35)
    readonly property color onAirBg:     isDark ? "#1A0008" : "#FFF0F3"
    readonly property color recording:   "#FF3B30"
    readonly property color passthrough: isDark ? "#2C2C34" : "#D0D0D8"
    readonly property color tally:       "#00FF66"

    // ── OVERLAY ─────────────────────────────────────────────
    readonly property color modalBg:     isDark ? Qt.rgba(0, 0, 0, 0.70) : Qt.rgba(0, 0, 0, 0.40)
    readonly property color tooltipBg:   isDark ? "#2A2A32" : "#1A1A22"
    readonly property color tooltipText: "#F0F0F5"

    // ── ELEVATION SHADOWS ───────────────────────────────────
    readonly property color shadowLight: isDark ? Qt.rgba(0, 0, 0, 0.30) : Qt.rgba(0, 0, 0, 0.08)
    readonly property color shadowMedium:isDark ? Qt.rgba(0, 0, 0, 0.50) : Qt.rgba(0, 0, 0, 0.12)
    readonly property color shadowHeavy: isDark ? Qt.rgba(0, 0, 0, 0.70) : Qt.rgba(0, 0, 0, 0.20)

    // ── TYPOGRAPHY SCALE ────────────────────────────────────
    readonly property int fontXs:    10
    readonly property int fontSm:    12
    readonly property int fontBase:  14
    readonly property int fontMd:    16
    readonly property int fontLg:    20
    readonly property int fontXl:    24
    readonly property int fontXxl:   32
    readonly property int fontHero:  48

    readonly property string fontFamily:     "Inter, SF Pro Display, Helvetica Neue, Arial, sans-serif"
    readonly property string fontMono:       "SF Mono, Menlo, Consolas, monospace"

    // ── SPACING SYSTEM (4px grid) ───────────────────────────
    readonly property int spaceXxs:  2
    readonly property int spaceXs:   4
    readonly property int spaceSm:   8
    readonly property int spaceMd:   12
    readonly property int spaceLg:   16
    readonly property int spaceXl:   24
    readonly property int spaceXxl:  32
    readonly property int spaceHuge: 48

    // ── RADIUS ──────────────────────────────────────────────
    readonly property int radiusSm:   4
    readonly property int radiusMd:   8
    readonly property int radiusLg:  12
    readonly property int radiusXl:  16
    readonly property int radiusFull: 9999

    // ── ANIMATION DURATIONS ─────────────────────────────────
    readonly property int animFast:    120
    readonly property int animNormal:  200
    readonly property int animSlow:    350
    readonly property int animSmoother:500

    // ── GLASS EFFECT (frosted glass / glassmorphism) ────────
    readonly property color glassBg:     isDark ? Qt.rgba(14, 14, 18, 0.75) : Qt.rgba(255, 255, 255, 0.80)
    readonly property color glassBorder: isDark ? Qt.rgba(255, 255, 255, 0.08) : Qt.rgba(0, 0, 0, 0.06)

    // ── NAV SIDEBAR ─────────────────────────────────────────
    readonly property int sidebarWidth:       56
    readonly property int sidebarExpandedWidth: 220
    readonly property color sidebarBg:        isDark ? "#0A0A0E" : "#E8E8EE"
    readonly property color sidebarItemHover: isDark ? Qt.rgba(255, 255, 255, 0.06) : Qt.rgba(0, 0, 0, 0.06)
    readonly property color sidebarItemActive:isDark ? Qt.rgba(108/255, 92/255, 231/255, 0.15) : Qt.rgba(108/255, 92/255, 231/255, 0.10)
}
