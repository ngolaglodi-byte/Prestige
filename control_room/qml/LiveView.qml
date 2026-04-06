import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: liveView

    property int elapsedSeconds: 0
    // ── DEMO STATE MACHINE ──────────────────────────────────
    // Simulates the REAL broadcast cycle exactly:
    //   idle → delay → talent shows + title hides → talent exits → title returns → idle
    property string demoPhase: "idle"
    property string demoMode: "single"  // "single" or "multi"
    property bool demoTalentVisible: demoPhase === "talent_in" || demoPhase === "hold" || demoPhase === "talent_exit"
    property bool demoTitleHidden: demoPhase === "talent_in" || demoPhase === "hold" || demoPhase === "talent_exit" || demoPhase === "title_wait"
    property int demoFrameCounter: 0

    // Lottie frame timer — only runs when talent is visible
    Timer {
        id: demoTimer; interval: 50; repeat: true
        running: liveView.demoTalentVisible
        onTriggered: liveView.demoFrameCounter++
    }

    // Single timer for all phases — manually started, NOT reactive
    Timer {
        id: demoPhaseTimer; repeat: false
        onTriggered: {
            if (liveView.demoPhase === "delay") {
                liveView.demoFrameCounter = 0
                liveView.demoPhase = "talent_in"
                // Schedule transition to hold after entry (~3s)
                demoPhaseTimer.interval = 3000
                demoPhaseTimer.start()
            } else if (liveView.demoPhase === "talent_in") {
                liveView.demoPhase = "hold"
                // Schedule transition to exit after talentDisplayDurationSec
                demoPhaseTimer.interval = setupController.talentDisplayDurationSec * 1000
                demoPhaseTimer.start()
            } else if (liveView.demoPhase === "hold") {
                liveView.demoPhase = "talent_exit"
                // Schedule transition to title_wait after exit animation (2s)
                demoPhaseTimer.interval = 2000
                demoPhaseTimer.start()
            } else if (liveView.demoPhase === "talent_exit") {
                liveView.demoPhase = "title_wait"
                // Schedule transition to idle after titleReappearDelaySec
                demoPhaseTimer.interval = setupController.titleReappearDelaySec * 1000
                demoPhaseTimer.start()
            } else if (liveView.demoPhase === "title_wait") {
                liveView.demoPhase = "idle"
                // DONE — no more timer starts
            }
        }
    }

    function startDemo(mode) {
        demoPhaseTimer.stop()
        if (demoPhase !== "idle") { demoPhase = "idle"; return }  // STOP
        demoMode = mode || "single"
        demoFrameCounter = 0
        demoPhase = "delay"
        var delayMs = (demoMode === "multi") ? setupController.multiFaceDelayMs : setupController.singleFaceDelayMs
        demoPhaseTimer.interval = Math.max(100, delayMs)
        demoPhaseTimer.start()
    }
    Timer { interval: 1000; running: mainWindow.overlaysActive; repeat: true; onTriggered: liveView.elapsedSeconds++ }
    Connections { target: mainWindow; function onOverlaysActiveChanged() { if (mainWindow.overlaysActive) liveView.elapsedSeconds = 0 } }

    // ── Keyboard Shortcuts (Feature 5) ────────────────────
    Shortcut {
        sequence: "Space"
        enabled: mainWindow.isLiveMode
        onActivated: liveController.toggleOverlays()
    }
    Shortcut {
        sequence: "R"
        enabled: mainWindow.isLiveMode
        onActivated: liveController.toggleRecording()
    }
    Shortcut {
        sequence: "B"
        enabled: mainWindow.isLiveMode
        onActivated: {
            bypassBtn.bypassed = !bypassBtn.bypassed
            liveController.setBypassed(bypassBtn.bypassed)
            if (bypassBtn.bypassed)
                liveController.setOverlaysActive(false)
            else
                liveController.setOverlaysActive(true)
        }
    }
    Shortcut {
        sequence: "T"
        enabled: mainWindow.isLiveMode
        onActivated: { /* tools now in nav drawer */ }
    }
    Shortcut {
        sequence: "1"
        enabled: mainWindow.isLiveMode
        onActivated: { /* tools now in nav drawer */ }
    }
    Shortcut {
        sequence: "2"
        enabled: mainWindow.isLiveMode
        onActivated: { /* tools now in nav drawer */ }
    }
    Shortcut {
        sequence: "3"
        enabled: mainWindow.isLiveMode
        onActivated: { /* tools now in nav drawer */ }
    }
    Shortcut {
        sequence: "4"
        enabled: mainWindow.isLiveMode
        onActivated: { /* tools now in nav drawer */ }
    }
    Shortcut {
        sequence: "5"
        enabled: mainWindow.isLiveMode
        onActivated: { /* tools now in nav drawer */ }
    }
    Shortcut {
        sequence: "6"
        enabled: mainWindow.isLiveMode
        onActivated: { /* tools now in nav drawer */ }
    }
    Shortcut {
        sequence: "N"
        enabled: mainWindow.isLiveMode
        onActivated: graphicsQueue.takeNext()
    }

    ColumnLayout {
        anchors.fill: parent; spacing: 0

        // ── ON AIR Bar ─────────────────────────────────────
        Rectangle {
            id: onAirBar
            Layout.fillWidth: true; Layout.preferredHeight: 48

            property bool isLive: mainWindow.overlaysActive

            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: onAirBar.isLive ? "#B71C1C" : "#1E1E24" }
                GradientStop { position: 0.3; color: onAirBar.isLive ? "#D32F2F" : "#242428" }
                GradientStop { position: 0.7; color: onAirBar.isLive ? "#D32F2F" : "#242428" }
                GradientStop { position: 1.0; color: onAirBar.isLive ? "#B71C1C" : "#1E1E24" }
            }

            // Top highlight line (subtle broadcast feel)
            Rectangle {
                anchors.top: parent.top; width: parent.width; height: 1
                color: onAirBar.isLive ? Qt.rgba(255, 255, 255, 0.15) : Qt.rgba(255, 255, 255, 0.04)
            }

            RowLayout {
                anchors.fill: parent; anchors.leftMargin: 20; anchors.rightMargin: 20; spacing: 12

                // Tally light indicator
                Item {
                    Layout.preferredWidth: 18; Layout.preferredHeight: 18

                    // Outer glow ring
                    Rectangle {
                        anchors.centerIn: parent; width: 18; height: 18; radius: 9
                        color: "transparent"
                        border.color: onAirBar.isLive ? Qt.rgba(255, 255, 255, 0.25) : Qt.rgba(255, 255, 255, 0.08)
                        border.width: 1
                    }

                    // Inner tally dot
                    Rectangle {
                        anchors.centerIn: parent; width: 10; height: 10; radius: 5
                        color: onAirBar.isLive ? "#FFFFFF" : "#555"
                        SequentialAnimation on opacity {
                            loops: Animation.Infinite; running: onAirBar.isLive
                            NumberAnimation { to: 0.3; duration: 900; easing.type: Easing.InOutSine }
                            NumberAnimation { to: 1.0; duration: 900; easing.type: Easing.InOutSine }
                        }
                    }

                    // Glow effect behind tally when live
                    Rectangle {
                        anchors.centerIn: parent; width: 24; height: 24; radius: 12
                        color: Qt.rgba(255, 45, 85, 0.3); visible: onAirBar.isLive; z: -1
                        SequentialAnimation on opacity {
                            loops: Animation.Infinite; running: onAirBar.isLive
                            NumberAnimation { to: 0.1; duration: 900; easing.type: Easing.InOutSine }
                            NumberAnimation { to: 0.6; duration: 900; easing.type: Easing.InOutSine }
                        }
                    }
                }

                // ON AIR / PASSTHROUGH label
                Label {
                    text: onAirBar.isLive ? "ON AIR" : window.t("passthrough")
                    font.pixelSize: 14; font.weight: Font.Bold
                    font.letterSpacing: onAirBar.isLive ? 2.5 : 1.2
                    color: Qt.rgba(1, 1, 1, onAirBar.isLive ? 1.0 : 0.6)
                }

                // Separator
                Rectangle {
                    Layout.preferredWidth: 1; Layout.preferredHeight: 22
                    color: Qt.rgba(255, 255, 255, onAirBar.isLive ? 0.20 : 0.08)
                    visible: configManager.channelName !== ""
                }

                // Channel name
                Label {
                    text: configManager.channelName
                    font.pixelSize: 13; font.weight: Font.DemiBold
                    color: Qt.rgba(1, 1, 1, 0.85)
                    visible: configManager.channelName !== ""
                }

                // Separator
                Rectangle {
                    Layout.preferredWidth: 1; Layout.preferredHeight: 22
                    color: Qt.rgba(255, 255, 255, onAirBar.isLive ? 0.20 : 0.08)
                }

                // Profile name
                Label {
                    text: setupController.currentProfileName
                    font.pixelSize: 12; color: Qt.rgba(1, 1, 1, 0.55)
                }

                Item { Layout.fillWidth: true }

                // Recording indicator (when recording)
                Rectangle {
                    Layout.preferredWidth: recRow.implicitWidth + 14; Layout.preferredHeight: 26; radius: 13
                    color: Qt.rgba(255, 59, 48, 0.2)
                    border.color: Qt.rgba(255, 59, 48, 0.4)
                    visible: liveController.isRecording

                    RowLayout {
                        id: recRow; anchors.centerIn: parent; spacing: 5
                        Rectangle {
                            Layout.preferredWidth: 7; Layout.preferredHeight: 7; radius: 4; color: "#FF3B30"
                            SequentialAnimation on opacity {
                                loops: Animation.Infinite; running: liveController.isRecording
                                NumberAnimation { to: 0.3; duration: 600 }
                                NumberAnimation { to: 1.0; duration: 600 }
                            }
                        }
                        Label { text: "REC"; font.pixelSize: 10; font.weight: Font.Bold; font.letterSpacing: 1; color: "#FF6B60" }
                    }
                }

                // Timer with monospace - broadcast style
                Rectangle {
                    Layout.preferredWidth: timerLbl.implicitWidth + 20; Layout.preferredHeight: 28; radius: 6
                    color: onAirBar.isLive ? Qt.rgba(0, 0, 0, 0.25) : Qt.rgba(255, 255, 255, 0.04)

                    Label {
                        id: timerLbl; anchors.centerIn: parent
                        text: liveController.formattedDuration(liveView.elapsedSeconds)
                        font.pixelSize: 18; font.weight: Font.Bold
                        font.family: "SF Mono, Menlo, Consolas, monospace"
                        font.letterSpacing: 1.5
                        color: onAirBar.isLive ? "white" : Qt.rgba(1, 1, 1, 0.5)
                    }
                }
            }

            // Bottom edge line
            Rectangle {
                anchors.bottom: parent.bottom; width: parent.width; height: 1
                color: onAirBar.isLive ? Qt.rgba(255, 255, 255, 0.08) : Qt.rgba(255, 255, 255, 0.02)
            }
        }

        // ── Video output ────────────────
        Rectangle {
            id: videoContainer
            Layout.fillWidth: true; Layout.fillHeight: true
            color: window.darkMode ? "#0A0A0E" : "#E8E8EE"

            Image {
                id: liveImage
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                cache: false
                source: previewMonitor.active ? "image://preview/frame?" + liveFrameCounter : ""
                visible: previewMonitor.active

                property int liveFrameCounter: 0
                Connections {
                    target: previewMonitor
                    function onFrameUpdated() { liveImage.liveFrameCounter++ }
                }
            }

            // ── DEMO TALENT OVERLAY ─────────────────────
            // Plays the full Lottie animation lifecycle: entry → hold → exit
            // Visible during talent_in, hold, and talent_out phases
            Item {
                id: demoOverlayContainer
                anchors.fill: parent
                visible: liveView.demoTalentVisible
                z: 5

                // Apply user's nameplate scale + offset
                transform: [
                    Translate {
                        // Same normalization as Compositor: both axes use width/1920
                        x: setupController.nameplateOffsetX * (demoOverlayContainer.width / 1920)
                        y: setupController.nameplateOffsetY * (demoOverlayContainer.width / 1920)
                    },
                    Scale {
                        xScale: setupController.nameplateScale
                        yScale: setupController.nameplateScale
                        origin.x: demoOverlayContainer.width / 2
                        origin.y: demoOverlayContainer.height * 0.85
                    }
                ]

                Image {
                    id: demoOverlay
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectFit
                    cache: false
                    source: demoOverlayContainer.visible
                        ? "image://lottie/native:" + liveView.demoMode + ":" + setupController.lottiePreset + "?" + liveView.demoFrameCounter
                        : ""
                }

                opacity: setupController.backgroundOpacity
            }

            // DEMO badge — shows current phase
            Rectangle {
                anchors.top: parent.top; anchors.right: parent.right; anchors.margins: 10
                width: demoLabel.implicitWidth + 16; height: 22; radius: 11
                color: Qt.rgba(108/255, 92/255, 231/255, 0.85)
                visible: liveView.demoPhase !== "idle"; z: 10
                Label {
                    id: demoLabel; anchors.centerIn: parent
                    text: {
                        if (liveView.demoPhase === "delay") return "DEMO — Detection..."
                        if (liveView.demoPhase === "talent_in") return "DEMO — Talent entree"
                        if (liveView.demoPhase === "hold") return "DEMO — Talent affiche"
                        if (liveView.demoPhase === "talent_exit") return "DEMO — Talent sortie"
                        if (liveView.demoPhase === "title_wait") return "DEMO — Retour titre..."
                        return "DEMO"
                    }
                    font.pixelSize: 9; font.weight: Font.Bold; font.letterSpacing: 1; color: "white"
                }
            }

            // ── WYSIWYG Preview Overlays ─────────────────
            // Only shown when Vision Engine is NOT connected (design mode).
            // When Vision Engine sends composited frames, overlays are already
            // rendered in the frame by the Compositor — hide QML overlays to avoid doubles.
            Item {
                id: wysiwygOverlay
                anchors.fill: parent
                visible: !previewMonitor.active  // Hidden when Vision Engine is sending composited frames

                // Standard broadcast proportions (based on 1920x1080 reference)
                property real pw: parent.width   // Preview width
                property real ph: parent.height  // Preview height

                // ── Layer 1: Channel Logo ────────────────────
                Item {
                    id: previewLogoContainer
                    visible: setupController.channelLogoPath !== "" && !(liveController.isBypassed && !setupController.keepLogoDuringAds)

                    property string pos: setupController.channelLogoPosition
                    property real baseMargin: wysiwygOverlay.pw * 0.008
                    property real logoH: setupController.channelLogoSize * (wysiwygOverlay.ph / 1080.0)

                    width: logoH; height: logoH

                    x: {
                        var ox = setupController.channelLogoOffsetX * (wysiwygOverlay.pw / 1920.0)
                        if (pos === "top_left" || pos === "bottom_left")
                            return baseMargin + ox
                        else
                            return wysiwygOverlay.pw - width - baseMargin + ox
                    }
                    y: {
                        var oy = setupController.channelLogoOffsetY * (wysiwygOverlay.ph / 1080.0)
                        if (pos === "top_left" || pos === "top_right")
                            return baseMargin + oy
                        else
                            return wysiwygOverlay.ph - height - baseMargin + oy
                    }

                    Image {
                        id: previewLogo
                        anchors.fill: parent
                        source: setupController.channelLogoPath ? ("file:///" + setupController.channelLogoPath) : ""
                        fillMode: Image.PreserveAspectFit

                        // Loop: Pulse
                        SequentialAnimation on scale {
                            loops: Animation.Infinite
                            running: setupController.logoLoopAnim === "pulse" || setupController.logoLoopAnim === "rotate"
                            NumberAnimation { to: 1.04; duration: 1200; easing.type: Easing.InOutSine }
                            NumberAnimation { to: 1.0; duration: 1200; easing.type: Easing.InOutSine }
                        }
                    }

                    // Loop: Glow (overlay rectangle that pulses opacity)
                    Rectangle {
                        anchors.fill: parent; radius: 4
                        color: "transparent"; border.color: "#6C5CE7"; border.width: 2
                        visible: setupController.logoLoopAnim === "glow" || setupController.logoLoopAnim === "shimmer"
                        SequentialAnimation on opacity {
                            loops: Animation.Infinite; running: setupController.logoLoopAnim === "glow" || setupController.logoLoopAnim === "shimmer"
                            NumberAnimation { to: 0.0; duration: 1000; easing.type: Easing.InOutSine }
                            NumberAnimation { to: 0.6; duration: 1000; easing.type: Easing.InOutSine }
                        }
                    }

                    // Loop: Bounce (animate the container's y via transform)
                    transform: Translate {
                        id: logoBounceTransform
                        y: 0
                        SequentialAnimation on y {
                            loops: Animation.Infinite
                            running: setupController.logoLoopAnim === "bounce"
                            NumberAnimation { to: -4; duration: 600; easing.type: Easing.InOutSine }
                            NumberAnimation { to: 0; duration: 600; easing.type: Easing.InOutSine }
                        }
                    }

                    // Entry animation
                    opacity: 0
                    Component.onCompleted: logoEntryTimer.start()
                    Timer { id: logoEntryTimer; interval: 300; onTriggered: previewLogoContainer.opacity = 1 }
                    Behavior on opacity { NumberAnimation { duration: setupController.logoEntryAnim === "none" ? 0 : 800; easing.type: Easing.OutCubic } }
                }

                // ── Layer 1b: Channel Name ───────────────────
                Item {
                    id: previewChannelNameContainer
                    visible: setupController.showChannelNameText && configManager.channelName !== "" && !(liveController.isBypassed && !setupController.keepLogoDuringAds)

                    property string pos: setupController.channelLogoPosition
                    property real baseMargin: wysiwygOverlay.pw * 0.005
                    property real nameW: nameLbl.implicitWidth + wysiwygOverlay.pw * 0.012
                    property real nameH: wysiwygOverlay.ph * 0.033

                    width: nameW; height: nameH

                    x: {
                        var ox = setupController.channelNameOffsetX * (wysiwygOverlay.pw / 1920.0)
                        if (pos === "top_left" || pos === "bottom_left")
                            return baseMargin + ox
                        else
                            return wysiwygOverlay.pw - width - baseMargin + ox
                    }
                    y: {
                        var oy = setupController.channelNameOffsetY * (wysiwygOverlay.ph / 1080.0)
                        var logoBottom = previewLogoContainer.visible ? (previewLogoContainer.y + previewLogoContainer.height + wysiwygOverlay.ph * 0.004) : 0
                        if (pos === "top_left" || pos === "top_right")
                            return (previewLogoContainer.visible ? logoBottom : baseMargin) + oy
                        else
                            return wysiwygOverlay.ph - height - baseMargin + oy
                    }

                    Rectangle {
                        id: previewChannelName
                        anchors.fill: parent
                        radius: {
                            var shape = setupController.channelNameShape
                            if (shape === "pill") return height / 2
                            if (shape === "square" || shape === "rectangle") return wysiwygOverlay.ph * 0.003
                            return 0
                        }
                        color: setupController.channelNameShape === "frameless" ? "transparent" : setupController.channelNameBgColor
                        border.color: setupController.channelNameShape === "frameless" ? "transparent" : setupController.channelNameBorderColor
                        border.width: setupController.channelNameShape === "frameless" ? 0 : 1

                        Label {
                            id: nameLbl; anchors.centerIn: parent
                            text: configManager.channelName
                            font.pixelSize: setupController.channelNameFontSize * (wysiwygOverlay.ph / 1080.0)
                            font.weight: Font.Bold
                            color: setupController.channelNameTextColor
                        }

                        // Loop: Pulse
                        SequentialAnimation on scale {
                            loops: Animation.Infinite; running: setupController.nameLoopAnim === "pulse"
                            NumberAnimation { to: 1.04; duration: 1200; easing.type: Easing.InOutSine }
                            NumberAnimation { to: 1.0; duration: 1200; easing.type: Easing.InOutSine }
                        }
                    }

                    // Loop: Glow border
                    Rectangle {
                        anchors.fill: parent; radius: previewChannelName.radius
                        color: "transparent"; border.color: setupController.channelNameBorderColor || "#6C5CE7"; border.width: 2
                        visible: setupController.nameLoopAnim === "glow"
                        SequentialAnimation on opacity {
                            loops: Animation.Infinite; running: setupController.nameLoopAnim === "glow"
                            NumberAnimation { to: 0.0; duration: 1000; easing.type: Easing.InOutSine }
                            NumberAnimation { to: 0.8; duration: 1000; easing.type: Easing.InOutSine }
                        }
                    }

                    // Loop: Bounce (via transform, no binding conflict)
                    transform: [
                        Translate {
                            y: 0
                            SequentialAnimation on y {
                                loops: Animation.Infinite; running: setupController.nameLoopAnim === "bounce"
                                NumberAnimation { to: -4; duration: 600; easing.type: Easing.InOutSine }
                                NumberAnimation { to: 0; duration: 600; easing.type: Easing.InOutSine }
                            }
                        },
                        Rotation {
                            angle: setupController.channelNameShape === "angled" ? -5 : 0
                            origin.x: previewChannelNameContainer.width / 2; origin.y: previewChannelNameContainer.height / 2
                        }
                    ]

                    // Entry animation
                    opacity: 0
                    Component.onCompleted: nameEntryTimer.start()
                    Timer { id: nameEntryTimer; interval: 500; onTriggered: previewChannelNameContainer.opacity = 1 }
                    Behavior on opacity { NumberAnimation { duration: setupController.nameEntryAnim === "none" ? 0 : 800; easing.type: Easing.OutCubic } }
                }

                // ── Layer 2: Show Title ──────────────────────
                Rectangle {
                    id: previewShowTitle
                    visible: setupController.showTitleEnabled && setupController.showTitle !== "" && liveController.showTitleVisible && !liveController.isBypassed

                    property string pos: setupController.showTitlePosition
                    property real baseMargin: wysiwygOverlay.pw * 0.008
                    x: {
                        if (pos === "bottom_left" || pos === "top_left")
                            return baseMargin + setupController.showTitleOffsetX * (wysiwygOverlay.pw / 1920.0)
                        else
                            return parent.width - width - baseMargin + setupController.showTitleOffsetX * (wysiwygOverlay.pw / 1920.0)
                    }
                    y: {
                        if (pos === "top_left" || pos === "top_right")
                            return baseMargin + setupController.showTitleOffsetY * (wysiwygOverlay.ph / 1080.0)
                        else {
                            var tickerH = wysiwygTickerBar.visible ? wysiwygTickerBar.height + wysiwygOverlay.ph * 0.007 : 0
                            return parent.height - height - baseMargin - tickerH + setupController.showTitleOffsetY * (wysiwygOverlay.ph / 1080.0)
                        }
                    }

                    width: Math.max(wysiwygOverlay.pw * 0.21, titleCol.implicitWidth + wysiwygOverlay.pw * 0.013)
                    height: wysiwygOverlay.ph * 0.046
                    radius: {
                        var shape = setupController.showTitleShape
                        if (shape === "pill") return height / 2
                        if (shape === "rectangle" || shape === "square") return wysiwygOverlay.ph * 0.004
                        return 0
                    }
                    color: setupController.showTitleShape === "frameless" ? "transparent" : setupController.showTitleBgColor
                    border.color: setupController.showTitleShape === "frameless" ? "transparent" : setupController.showTitleBorderColor
                    border.width: 1
                    opacity: liveController.showTitleVisible ? 1.0 : 0.0
                    Behavior on opacity { NumberAnimation { duration: setupController.showTitleEntryAnim === "none" ? 0 : 500; easing.type: Easing.OutCubic } }

                    // Loop: Pulse
                    SequentialAnimation on scale {
                        loops: Animation.Infinite; running: setupController.showTitleLoopAnim === "pulse"
                        NumberAnimation { to: 1.03; duration: 1200; easing.type: Easing.InOutSine }
                        NumberAnimation { to: 1.0; duration: 1200; easing.type: Easing.InOutSine }
                    }
                    // Loop: Glow border
                    Rectangle {
                        anchors.fill: parent; radius: parent.radius
                        color: "transparent"; border.color: setupController.showTitleBorderColor || "#6C5CE7"; border.width: 2
                        visible: setupController.showTitleLoopAnim === "glow"
                        SequentialAnimation on opacity {
                            loops: Animation.Infinite; running: setupController.showTitleLoopAnim === "glow"
                            NumberAnimation { to: 0.0; duration: 1000; easing.type: Easing.InOutSine }
                            NumberAnimation { to: 0.8; duration: 1000; easing.type: Easing.InOutSine }
                        }
                    }
                    // Loop: Bounce
                    transform: Translate {
                        y: 0
                        SequentialAnimation on y {
                            loops: Animation.Infinite; running: setupController.showTitleLoopAnim === "bounce"
                            NumberAnimation { to: -3; duration: 600; easing.type: Easing.InOutSine }
                            NumberAnimation { to: 0; duration: 600; easing.type: Easing.InOutSine }
                        }
                    }

                    // Accent bar on left
                    Rectangle {
                        width: wysiwygOverlay.pw * 0.002; height: parent.height
                        color: setupController.accentColor.toString() !== "#000000" ? setupController.accentColor : "#6C5CE7"
                        visible: setupController.showTitleShape !== "frameless"
                    }

                    ColumnLayout {
                        id: titleCol; anchors.centerIn: parent; spacing: 2
                        Label {
                            text: setupController.showTitle
                            font.pixelSize: setupController.showTitleFontSize * (wysiwygOverlay.ph / 1080.0)
                            font.weight: Font.Bold; color: setupController.showTitleTextColor
                        }
                        Label {
                            visible: setupController.showSubtitle !== ""
                            text: setupController.showSubtitle
                            font.pixelSize: (setupController.showTitleFontSize - 3) * (wysiwygOverlay.ph / 1080.0)
                            color: Qt.rgba(1,1,1,0.7)
                        }
                    }
                }

                // ── Layer 3: Talent Nameplate (from IA detection) ──
                Rectangle {
                    id: previewNameplate
                    visible: liveController.talentNameplateVisible && liveController.talentDetected && !liveController.isBypassed

                    property real npScale: setupController.nameplateScale

                    anchors.bottom: wysiwygTickerBar.top; anchors.left: parent.left
                    anchors.margins: wysiwygOverlay.pw * 0.008

                    width: Math.max(wysiwygOverlay.pw * 0.198 * npScale, talentCol.implicitWidth + wysiwygOverlay.pw * 0.017)
                    height: wysiwygOverlay.ph * 0.041 * npScale
                    radius: wysiwygOverlay.ph * 0.004
                    color: Qt.rgba(0, 0, 0, setupController.backgroundOpacity > 0 ? setupController.backgroundOpacity : 0.82)

                    // Accent bar left
                    Rectangle {
                        width: wysiwygOverlay.pw * 0.002; height: parent.height; radius: 2
                        color: setupController.accentColor.toString() !== "#000000" ? setupController.accentColor : "#E30613"
                    }

                    ColumnLayout {
                        id: talentCol; anchors.centerIn: parent; spacing: 1
                        Label {
                            text: liveController.detectedName || ""
                            font.pixelSize: wysiwygOverlay.ph * 0.015 * previewNameplate.npScale; font.weight: Font.Bold; color: "white"
                        }
                        Label {
                            visible: liveController.detectedRole !== ""
                            text: liveController.detectedRole || ""
                            font.pixelSize: wysiwygOverlay.ph * 0.011 * previewNameplate.npScale; color: Qt.rgba(1,1,1,0.7)
                        }
                    }

                    opacity: liveController.talentNameplateVisible ? 1.0 : 0.0
                    Behavior on opacity { NumberAnimation { duration: 300 } }
                }

                // ── Ticker bar (bottom) ─────────────────────
                Rectangle {
                    id: wysiwygTickerBar
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: (wysiwygSubtitleBar.visible ? (wysiwygSubtitleBar.height + wysiwygOverlay.ph * 0.007) : 0) - setupController.tickerOffsetY * (wysiwygOverlay.ph / 1080.0)
                    anchors.left: parent.left; anchors.right: parent.right
                    height: visible ? wysiwygOverlay.ph * 0.033 : 0
                    visible: setupController.tickerVisible && (rssFetcher.headlines !== "" || setupController.tickerManualText !== "") && !liveController.isBypassed
                    color: setupController.tickerBgColor
                    clip: true

                    Label {
                        id: tickerText
                        text: rssFetcher.headlines || setupController.tickerManualText || ""
                        font.pixelSize: setupController.tickerFontSize * (wysiwygOverlay.ph / 1080.0); font.weight: Font.Bold; color: setupController.tickerTextColor
                        y: (parent.height - height) / 2
                        NumberAnimation on x {
                            from: wysiwygTickerBar.width
                            to: -tickerText.implicitWidth
                            duration: Math.max(4000, tickerText.implicitWidth * (60 / Math.max(1, setupController.tickerSpeed)))
                            loops: Animation.Infinite; running: wysiwygTickerBar.visible
                        }
                    }
                }

                // ── Subtitles ────────────────────────────────
                Rectangle {
                    id: wysiwygSubtitleBar
                    x: (parent.width - width) / 2 + setupController.subtitleOffsetX * (wysiwygOverlay.pw / 1920.0)
                    y: {
                        var oy = setupController.subtitleOffsetY * (wysiwygOverlay.ph / 1080.0)
                        if (subtitleController.position === "top")
                            return wysiwygOverlay.ph * 0.015 + oy
                        else {
                            var tickerH = wysiwygTickerBar.visible ? wysiwygTickerBar.height + wysiwygOverlay.ph * 0.007 : 0
                            return wysiwygOverlay.ph - height - wysiwygOverlay.ph * 0.008 - tickerH + oy
                        }
                    }
                    visible: subtitleController.enabled && subtitleController.currentText !== "" && !liveController.isBypassed

                    width: Math.max(wysiwygOverlay.pw * 0.313, subLbl.implicitWidth + wysiwygOverlay.pw * 0.013)
                    height: Math.max(wysiwygOverlay.ph * 0.037, subLbl.implicitHeight + wysiwygOverlay.ph * 0.011)
                    radius: wysiwygOverlay.ph * 0.006
                    color: Qt.rgba(0, 0, 0, subtitleController.bgOpacity)

                    Label {
                        id: subLbl; anchors.centerIn: parent
                        text: subtitleController.currentText
                        font.pixelSize: subtitleController.fontSize * (wysiwygOverlay.ph / 1080.0)
                        color: subtitleController.textColor
                    }
                }

                // ── Countdown (top-left) — broadcast pill: accent color, subtle glass ──
                Rectangle {
                    id: wysiwygCountdown
                    visible: liveController.countdownActive && !liveController.isBypassed

                    property real cdScale: setupController.countdownScale

                    x: wysiwygOverlay.pw * 0.008 + setupController.countdownOffsetX * (wysiwygOverlay.pw / 1920.0)
                    y: (previewLogoContainer.visible ? previewLogoContainer.height + wysiwygOverlay.ph * 0.022 : wysiwygOverlay.ph * 0.015) + setupController.countdownOffsetY * (wysiwygOverlay.ph / 1080.0)

                    width: Math.max(wysiwygOverlay.pw * 0.068 * cdScale, cdLbl.implicitWidth + wysiwygOverlay.pw * 0.009)
                    height: wysiwygOverlay.ph * 0.028 * cdScale
                    radius: height / 2
                    color: Qt.rgba(200/255, 0, 0, 0.55)
                    border.color: Qt.rgba(255, 80, 80, 0.25)
                    border.width: 1

                    Label {
                        id: cdLbl; anchors.centerIn: parent
                        text: {
                            var s = liveController.countdownSeconds
                            var mm = Math.floor(s / 60)
                            var ss = s % 60
                            var label = liveController.countdownLabel
                            var time = (mm < 10 ? "0" : "") + mm + ":" + (ss < 10 ? "0" : "") + ss
                            return label ? label + " " + time : time
                        }
                        font.pixelSize: wysiwygOverlay.ph * 0.012 * wysiwygCountdown.cdScale; font.weight: Font.Bold; font.family: "Menlo"; color: "white"
                    }
                }

                // ── Clock (top-right) — broadcast style: text + drop shadow, no box ──
                Item {
                    id: wysiwygClock
                    visible: setupController.clockVisible && !liveController.isBypassed

                    property real ckScale: setupController.clockScale

                    x: parent.width - width - wysiwygOverlay.pw * 0.012 + setupController.clockOffsetX * (wysiwygOverlay.pw / 1920.0)
                    y: wysiwygOverlay.ph * 0.015 + setupController.clockOffsetY * (wysiwygOverlay.ph / 1080.0)

                    width: clockLbl.implicitWidth + wysiwygOverlay.pw * 0.006
                    height: clockLbl.implicitHeight + wysiwygOverlay.ph * 0.004

                    // Drop shadow (broadcast standard)
                    Label {
                        anchors.centerIn: parent
                        anchors.horizontalCenterOffset: 1; anchors.verticalCenterOffset: 1
                        text: clockLbl.text
                        font: clockLbl.font; color: Qt.rgba(0, 0, 0, 0.7)
                    }
                    Label {
                        id: clockLbl; anchors.centerIn: parent
                        text: Qt.formatTime(new Date(), setupController.clockFormat)
                        font.pixelSize: wysiwygOverlay.ph * 0.014 * wysiwygClock.ckScale; font.weight: Font.Bold; font.family: "Menlo"; color: "white"
                    }
                    Timer { interval: 1000; running: true; repeat: true; onTriggered: clockLbl.text = Qt.formatTime(new Date(), setupController.clockFormat) }
                }

                // ── QR Code placeholder (when active) ────────
                Rectangle {
                    id: wysiwygQrCode
                    visible: liveController.qrCodeVisible && liveController.qrCodeUrl !== "" && !liveController.isBypassed

                    property real qrScale: setupController.qrCodeScale
                    property string pos: liveController.qrCodePosition || "bottom_right"
                    property real qrBaseMargin: wysiwygOverlay.pw * 0.008
                    x: {
                        if (pos === "bottom_left" || pos === "top_left")
                            return qrBaseMargin + setupController.qrCodeOffsetX * (wysiwygOverlay.pw / 1920.0)
                        else
                            return parent.width - width - qrBaseMargin + setupController.qrCodeOffsetX * (wysiwygOverlay.pw / 1920.0)
                    }
                    y: {
                        if (pos === "top_right" || pos === "top_left")
                            return qrBaseMargin + setupController.qrCodeOffsetY * (wysiwygOverlay.ph / 1080.0)
                        else {
                            var tickerH = wysiwygTickerBar.visible ? wysiwygTickerBar.height : 0
                            return parent.height - height - qrBaseMargin - tickerH + setupController.qrCodeOffsetY * (wysiwygOverlay.ph / 1080.0)
                        }
                    }

                    width: wysiwygOverlay.pw * 0.052 * qrScale; height: wysiwygOverlay.pw * 0.052 * qrScale
                    radius: wysiwygOverlay.ph * 0.006
                    color: "white"

                    // QR pattern placeholder
                    Grid {
                        anchors.centerIn: parent; columns: 5; spacing: wysiwygOverlay.pw * 0.001
                        Repeater {
                            model: 25
                            Rectangle {
                                width: wysiwygOverlay.pw * 0.005 * wysiwygQrCode.qrScale; height: wysiwygOverlay.pw * 0.005 * wysiwygQrCode.qrScale
                                color: (index % 3 === 0 || index % 7 === 0) ? "#000" : "white"
                            }
                        }
                    }

                    Label {
                        anchors.bottom: parent.bottom; anchors.horizontalCenter: parent.horizontalCenter
                        anchors.bottomMargin: wysiwygOverlay.ph * 0.002
                        text: "SCAN"; font.pixelSize: wysiwygOverlay.ph * 0.007 * wysiwygQrCode.qrScale; font.weight: Font.Bold; color: "#6C5CE7"
                    }
                }

                // ── Scoreboard overlay — glass morphism (FIFA/UEFA style) ──
                Rectangle {
                    id: wysiwygScoreboard
                    visible: setupController.scoreboardVisible && !liveController.isBypassed

                    property real sbScale: setupController.scoreboardScale
                    property string sbPos: setupController.scoreboardPosition || "top_left"
                    x: {
                        if (sbPos === "top_left" || sbPos === "bottom_left")
                            return wysiwygOverlay.pw * 0.008 + setupController.scoreboardOffsetX * (wysiwygOverlay.pw / 1920.0)
                        else
                            return parent.width - width - wysiwygOverlay.pw * 0.008 + setupController.scoreboardOffsetX * (wysiwygOverlay.pw / 1920.0)
                    }
                    y: {
                        if (sbPos === "top_left" || sbPos === "top_right")
                            return wysiwygOverlay.ph * 0.056 + setupController.scoreboardOffsetY * (wysiwygOverlay.ph / 1080.0)
                        else
                            return parent.height - height - wysiwygOverlay.ph * 0.056 + setupController.scoreboardOffsetY * (wysiwygOverlay.ph / 1080.0)
                    }

                    width: wysiwygOverlay.pw * 0.188 * sbScale; height: wysiwygOverlay.ph * 0.079 * sbScale
                    radius: wysiwygOverlay.ph * 0.005
                    color: Qt.rgba(0, 0, 0, 0.35)
                    border.color: Qt.rgba(1, 1, 1, 0.08)
                    border.width: 1

                    // Top accent line
                    Rectangle {
                        anchors.top: parent.top; anchors.left: parent.left; anchors.right: parent.right
                        height: 1; color: Qt.rgba(1, 1, 1, 0.12)
                    }

                    RowLayout {
                        anchors.centerIn: parent; spacing: wysiwygOverlay.pw * 0.004

                        // Team A block
                        ColumnLayout {
                            spacing: 0
                            // Team A color accent bar
                            Rectangle {
                                Layout.preferredWidth: wysiwygOverlay.pw * 0.035 * wysiwygScoreboard.sbScale
                                Layout.preferredHeight: 2; color: setupController.scoreboardColorA
                                Layout.alignment: Qt.AlignHCenter
                            }
                            Label { text: setupController.scoreboardTeamA; font.pixelSize: wysiwygOverlay.ph * 0.008 * wysiwygScoreboard.sbScale; color: Qt.rgba(1,1,1,0.7); font.weight: Font.DemiBold; Layout.alignment: Qt.AlignHCenter; font.letterSpacing: 0.5 }
                            Label { text: setupController.scoreboardScoreA.toString(); font.pixelSize: wysiwygOverlay.ph * 0.022 * wysiwygScoreboard.sbScale; font.weight: Font.Bold; color: "white"; Layout.alignment: Qt.AlignHCenter }
                        }

                        // Separator
                        Rectangle { Layout.preferredWidth: 1; Layout.preferredHeight: wysiwygOverlay.ph * 0.035 * wysiwygScoreboard.sbScale; color: Qt.rgba(1,1,1,0.12) }

                        // Team B block
                        ColumnLayout {
                            spacing: 0
                            Rectangle {
                                Layout.preferredWidth: wysiwygOverlay.pw * 0.035 * wysiwygScoreboard.sbScale
                                Layout.preferredHeight: 2; color: setupController.scoreboardColorB
                                Layout.alignment: Qt.AlignHCenter
                            }
                            Label { text: setupController.scoreboardTeamB; font.pixelSize: wysiwygOverlay.ph * 0.008 * wysiwygScoreboard.sbScale; color: Qt.rgba(1,1,1,0.7); font.weight: Font.DemiBold; Layout.alignment: Qt.AlignHCenter; font.letterSpacing: 0.5 }
                            Label { text: setupController.scoreboardScoreB.toString(); font.pixelSize: wysiwygOverlay.ph * 0.022 * wysiwygScoreboard.sbScale; font.weight: Font.Bold; color: "white"; Layout.alignment: Qt.AlignHCenter }
                        }
                    }
                    // Timer + Period — subtle at bottom
                    Label {
                        anchors.bottom: parent.bottom; anchors.horizontalCenter: parent.horizontalCenter
                        anchors.bottomMargin: wysiwygOverlay.ph * 0.003
                        text: setupController.scoreboardMatchTime + "  P" + setupController.scoreboardPeriod
                        font.pixelSize: wysiwygOverlay.ph * 0.007 * wysiwygScoreboard.sbScale; font.family: "Menlo"; color: Qt.rgba(0, 229, 255, 0.8)
                    }
                }

                // ── Weather overlay — broadcast style: text + drop shadow, no box ──
                Item {
                    id: wysiwygWeather
                    visible: setupController.weatherVisible && weatherFetcher.city !== "" && !liveController.isBypassed

                    property real wScale: setupController.weatherScale

                    x: parent.width - width - wysiwygOverlay.pw * 0.012 + setupController.weatherOffsetX * (wysiwygOverlay.pw / 1920.0)
                    y: parent.height - height - wysiwygOverlay.ph * 0.074 + setupController.weatherOffsetY * (wysiwygOverlay.ph / 1080.0)

                    width: weatherRow.implicitWidth + wysiwygOverlay.pw * 0.006
                    height: weatherRow.implicitHeight + wysiwygOverlay.ph * 0.006

                    // Drop shadow layer
                    RowLayout {
                        anchors.centerIn: parent
                        anchors.horizontalCenterOffset: 1; anchors.verticalCenterOffset: 1
                        spacing: wysiwygOverlay.pw * 0.003
                        Label { text: weatherFetcher.conditionIcon; font.pixelSize: wysiwygOverlay.ph * 0.022 * wysiwygWeather.wScale; color: Qt.rgba(0,0,0,0.6) }
                        ColumnLayout {
                            spacing: 0
                            Label { text: weatherFetcher.city; font.pixelSize: wysiwygOverlay.ph * 0.011 * wysiwygWeather.wScale; font.weight: Font.Bold; color: Qt.rgba(0,0,0,0.6) }
                            Label { text: Math.round(weatherFetcher.temperature) + weatherFetcher.unit; font.pixelSize: wysiwygOverlay.ph * 0.009 * wysiwygWeather.wScale; color: Qt.rgba(0,0,0,0.6) }
                        }
                    }
                    // Main content
                    RowLayout {
                        id: weatherRow; anchors.centerIn: parent
                        spacing: wysiwygOverlay.pw * 0.003
                        Label { text: weatherFetcher.conditionIcon; font.pixelSize: wysiwygOverlay.ph * 0.022 * wysiwygWeather.wScale }
                        ColumnLayout {
                            spacing: 0
                            Label { text: weatherFetcher.city; font.pixelSize: wysiwygOverlay.ph * 0.011 * wysiwygWeather.wScale; font.weight: Font.Bold; color: "white" }
                            Label { text: Math.round(weatherFetcher.temperature) + weatherFetcher.unit; font.pixelSize: wysiwygOverlay.ph * 0.009 * wysiwygWeather.wScale; color: Qt.rgba(1,1,1,0.8) }
                        }
                    }
                }

                // ── Queue: Lower Third overlay (cross-fade) ──
                Rectangle {
                    id: queueLowerThird
                    visible: opacity > 0
                    opacity: graphicsQueue.currentIndex >= 0 && graphicsQueue.currentItem.type === "lower_third" ? 1.0 : 0.0
                    Behavior on opacity { NumberAnimation { duration: 500; easing.type: Easing.InOutQuad } }

                    anchors.bottom: wysiwygTickerBar.visible ? wysiwygTickerBar.top : parent.bottom
                    anchors.left: parent.left
                    anchors.margins: wysiwygOverlay.pw * 0.008

                    width: Math.max(wysiwygOverlay.pw * 0.2, queueNameLbl.implicitWidth + wysiwygOverlay.pw * 0.02)
                    height: wysiwygOverlay.ph * 0.045
                    radius: wysiwygOverlay.ph * 0.004
                    color: Qt.rgba(0, 0, 0, setupController.backgroundOpacity > 0 ? setupController.backgroundOpacity : 0.82)

                    Rectangle { width: wysiwygOverlay.pw * 0.002; height: parent.height; color: setupController.accentColor.toString() !== "#000000" ? setupController.accentColor : "#6C5CE7" }

                    ColumnLayout {
                        anchors.centerIn: parent; spacing: 1
                        Label { id: queueNameLbl; text: graphicsQueue.currentItem.name || ""; font.pixelSize: wysiwygOverlay.ph * 0.015; font.weight: Font.Bold; color: "white" }
                        Label { text: graphicsQueue.currentItem.role || ""; font.pixelSize: wysiwygOverlay.ph * 0.011; color: Qt.rgba(1,1,1,0.7); visible: text !== "" }
                    }
                }

                // ── Queue: Message overlay (cross-fade, centered) ──
                Rectangle {
                    visible: opacity > 0
                    opacity: graphicsQueue.currentIndex >= 0 && graphicsQueue.currentItem.type === "message" ? 1.0 : 0.0
                    Behavior on opacity { NumberAnimation { duration: 500; easing.type: Easing.InOutQuad } }
                    anchors.centerIn: parent
                    width: Math.max(wysiwygOverlay.pw * 0.3, queueMsgLbl.implicitWidth + wysiwygOverlay.pw * 0.02)
                    height: queueMsgLbl.implicitHeight + wysiwygOverlay.ph * 0.02
                    radius: wysiwygOverlay.ph * 0.006
                    color: Qt.rgba(0, 0, 0, 0.8)
                    Label { id: queueMsgLbl; anchors.centerIn: parent; text: graphicsQueue.currentItem.text || ""; font.pixelSize: wysiwygOverlay.ph * 0.018; font.weight: Font.Bold; color: "white"; wrapMode: Text.WordWrap; width: parent.width - wysiwygOverlay.pw * 0.02; horizontalAlignment: Text.AlignHCenter }
                }

                // ── Queue: Ticker overlay (cross-fade) ──
                Rectangle {
                    visible: opacity > 0
                    opacity: graphicsQueue.currentIndex >= 0 && graphicsQueue.currentItem.type === "ticker" ? 1.0 : 0.0
                    Behavior on opacity { NumberAnimation { duration: 500; easing.type: Easing.InOutQuad } }
                    anchors.bottom: parent.bottom; anchors.left: parent.left; anchors.right: parent.right
                    height: wysiwygOverlay.ph * 0.033
                    color: setupController.tickerBgColor
                    clip: true
                    Label {
                        id: queueTickerText
                        text: graphicsQueue.currentItem.text || ""
                        font.pixelSize: setupController.tickerFontSize * (wysiwygOverlay.ph / 1080.0); font.weight: Font.Bold; color: setupController.tickerTextColor
                        y: (parent.height - height) / 2
                        NumberAnimation on x {
                            from: wysiwygOverlay.pw
                            to: -queueTickerText.implicitWidth
                            duration: Math.max(4000, queueTickerText.implicitWidth * 30)
                            loops: Animation.Infinite; running: queueTickerText.parent.opacity > 0
                        }
                    }
                }

                // ── Drag handles for overlay repositioning ──
                // Drag handle: Channel Name
                Rectangle {
                    visible: previewChannelName.visible
                    x: previewChannelName.x + previewChannelName.width - 8
                    y: previewChannelName.y - 4
                    width: 12; height: 12; radius: 2; z: 20
                    color: dragNameMa.containsMouse ? "#6C5CE7" : Qt.rgba(1,1,1,0.3)
                    MouseArea {
                        id: dragNameMa; anchors.fill: parent; hoverEnabled: true
                        drag.target: previewChannelNameContainer
                        cursorShape: pressed ? Qt.ClosedHandCursor : Qt.SizeAllCursor
                        onReleased: {
                            var pos = setupController.channelLogoPosition
                            var baseMargin = wysiwygOverlay.pw * 0.005
                            if (pos === "top_left" || pos === "bottom_left")
                                setupController.channelNameOffsetX = Math.round((previewChannelName.x - baseMargin) * 1920.0 / wysiwygOverlay.pw)
                            else
                                setupController.channelNameOffsetX = Math.round((previewChannelName.x - (wysiwygOverlay.pw - previewChannelName.width - baseMargin)) * 1920.0 / wysiwygOverlay.pw)
                            setupController.channelNameOffsetY = Math.round(previewChannelName.y * 1080.0 / wysiwygOverlay.ph)
                        }
                    }
                }

                // Drag handle: Logo
                Rectangle {
                    visible: previewLogoContainer.visible
                    x: previewLogoContainer.x + previewLogoContainer.width - 8
                    y: previewLogoContainer.y - 4
                    width: 12; height: 12; radius: 2; z: 20
                    color: dragLogoMa.containsMouse ? "#6C5CE7" : Qt.rgba(1,1,1,0.3)
                    MouseArea {
                        id: dragLogoMa; anchors.fill: parent; hoverEnabled: true
                        drag.target: previewLogoContainer
                        cursorShape: pressed ? Qt.ClosedHandCursor : Qt.SizeAllCursor
                        onReleased: {
                            var pos = setupController.channelLogoPosition
                            var baseMargin = wysiwygOverlay.pw * 0.008
                            if (pos === "top_left" || pos === "bottom_left")
                                setupController.channelLogoOffsetX = Math.round((previewLogoContainer.x - baseMargin) * 1920.0 / wysiwygOverlay.pw)
                            else
                                setupController.channelLogoOffsetX = Math.round((previewLogoContainer.x - (wysiwygOverlay.pw - previewLogoContainer.width - baseMargin)) * 1920.0 / wysiwygOverlay.pw)
                            if (pos === "top_left" || pos === "top_right")
                                setupController.channelLogoOffsetY = Math.round((previewLogoContainer.y - baseMargin) * 1080.0 / wysiwygOverlay.ph)
                            else
                                setupController.channelLogoOffsetY = Math.round((previewLogoContainer.y - (wysiwygOverlay.ph - previewLogoContainer.height - baseMargin)) * 1080.0 / wysiwygOverlay.ph)
                        }
                    }
                }

                // Drag handle: Show Title
                Rectangle {
                    visible: previewShowTitle.visible
                    x: previewShowTitle.x + previewShowTitle.width - 8
                    y: previewShowTitle.y - 4
                    width: 12; height: 12; radius: 2; z: 20
                    color: dragTitleMa.containsMouse ? "#6C5CE7" : Qt.rgba(1,1,1,0.3)
                    MouseArea {
                        id: dragTitleMa; anchors.fill: parent; hoverEnabled: true
                        drag.target: previewShowTitle
                        cursorShape: pressed ? Qt.ClosedHandCursor : Qt.SizeAllCursor
                        onReleased: {
                            var pos = setupController.showTitlePosition
                            var baseMargin = wysiwygOverlay.pw * 0.008
                            if (pos === "bottom_left" || pos === "top_left")
                                setupController.showTitleOffsetX = Math.round((previewShowTitle.x - baseMargin) * 1920.0 / wysiwygOverlay.pw)
                            else
                                setupController.showTitleOffsetX = Math.round((previewShowTitle.x - (wysiwygOverlay.pw - previewShowTitle.width - baseMargin)) * 1920.0 / wysiwygOverlay.pw)
                            if (pos === "top_left" || pos === "top_right")
                                setupController.showTitleOffsetY = Math.round((previewShowTitle.y - baseMargin) * 1080.0 / wysiwygOverlay.ph)
                            else
                                setupController.showTitleOffsetY = Math.round((previewShowTitle.y - (wysiwygOverlay.ph - previewShowTitle.height - baseMargin)) * 1080.0 / wysiwygOverlay.ph)
                        }
                    }
                }

                // Drag handle: Scoreboard
                Rectangle {
                    visible: wysiwygScoreboard.visible
                    x: wysiwygScoreboard.x + wysiwygScoreboard.width - 8
                    y: wysiwygScoreboard.y - 4
                    width: 12; height: 12; radius: 2; z: 20
                    color: dragScoreMa.containsMouse ? "#6C5CE7" : Qt.rgba(1,1,1,0.3)
                    MouseArea {
                        id: dragScoreMa; anchors.fill: parent; hoverEnabled: true
                        drag.target: wysiwygScoreboard
                        cursorShape: pressed ? Qt.ClosedHandCursor : Qt.SizeAllCursor
                        onReleased: {
                            var sbPos = wysiwygScoreboard.sbPos
                            var baseMargin = wysiwygOverlay.pw * 0.008
                            var baseMarginY = wysiwygOverlay.ph * 0.056
                            if (sbPos === "top_left" || sbPos === "bottom_left")
                                setupController.scoreboardOffsetX = Math.round((wysiwygScoreboard.x - baseMargin) * 1920.0 / wysiwygOverlay.pw)
                            else
                                setupController.scoreboardOffsetX = Math.round((wysiwygScoreboard.x - (wysiwygOverlay.pw - wysiwygScoreboard.width - baseMargin)) * 1920.0 / wysiwygOverlay.pw)
                            if (sbPos === "top_left" || sbPos === "top_right")
                                setupController.scoreboardOffsetY = Math.round((wysiwygScoreboard.y - baseMarginY) * 1080.0 / wysiwygOverlay.ph)
                            else
                                setupController.scoreboardOffsetY = Math.round((wysiwygScoreboard.y - (wysiwygOverlay.ph - wysiwygScoreboard.height - baseMarginY)) * 1080.0 / wysiwygOverlay.ph)
                        }
                    }
                }

                // Drag handle: Weather
                Rectangle {
                    visible: wysiwygWeather.visible
                    x: wysiwygWeather.x + wysiwygWeather.width - 8
                    y: wysiwygWeather.y - 4
                    width: 12; height: 12; radius: 2; z: 20
                    color: dragWeatherMa.containsMouse ? "#6C5CE7" : Qt.rgba(1,1,1,0.3)
                    MouseArea {
                        id: dragWeatherMa; anchors.fill: parent; hoverEnabled: true
                        drag.target: wysiwygWeather
                        cursorShape: pressed ? Qt.ClosedHandCursor : Qt.SizeAllCursor
                        onReleased: {
                            setupController.weatherOffsetX = Math.round((wysiwygWeather.x - (wysiwygOverlay.pw - wysiwygWeather.width - wysiwygOverlay.pw * 0.012)) * 1920.0 / wysiwygOverlay.pw)
                            setupController.weatherOffsetY = Math.round((wysiwygWeather.y - (wysiwygOverlay.ph - wysiwygWeather.height - wysiwygOverlay.ph * 0.074)) * 1080.0 / wysiwygOverlay.ph)
                        }
                    }
                }

                // Drag handle: Clock
                Rectangle {
                    visible: wysiwygClock.visible
                    x: wysiwygClock.x + wysiwygClock.width - 8
                    y: wysiwygClock.y - 4
                    width: 12; height: 12; radius: 2; z: 20
                    color: dragClockMa.containsMouse ? "#6C5CE7" : Qt.rgba(1,1,1,0.3)
                    MouseArea {
                        id: dragClockMa; anchors.fill: parent; hoverEnabled: true
                        drag.target: wysiwygClock
                        cursorShape: pressed ? Qt.ClosedHandCursor : Qt.SizeAllCursor
                        onReleased: {
                            setupController.clockOffsetX = Math.round((wysiwygClock.x - (wysiwygOverlay.pw - wysiwygClock.width - wysiwygOverlay.pw * 0.012)) * 1920.0 / wysiwygOverlay.pw)
                            setupController.clockOffsetY = Math.round((wysiwygClock.y - wysiwygOverlay.ph * 0.015) * 1080.0 / wysiwygOverlay.ph)
                        }
                    }
                }

                // Drag handle: Countdown
                Rectangle {
                    visible: wysiwygCountdown.visible
                    x: wysiwygCountdown.x + wysiwygCountdown.width - 8
                    y: wysiwygCountdown.y - 4
                    width: 12; height: 12; radius: 2; z: 20
                    color: dragCountdownMa.containsMouse ? "#6C5CE7" : Qt.rgba(1,1,1,0.3)
                    MouseArea {
                        id: dragCountdownMa; anchors.fill: parent; hoverEnabled: true
                        drag.target: wysiwygCountdown
                        cursorShape: pressed ? Qt.ClosedHandCursor : Qt.SizeAllCursor
                        onReleased: {
                            setupController.countdownOffsetX = Math.round((wysiwygCountdown.x - wysiwygOverlay.pw * 0.008) * 1920.0 / wysiwygOverlay.pw)
                            var baseY = previewLogoContainer.visible ? previewLogoContainer.height + wysiwygOverlay.ph * 0.022 : wysiwygOverlay.ph * 0.015
                            setupController.countdownOffsetY = Math.round((wysiwygCountdown.y - baseY) * 1080.0 / wysiwygOverlay.ph)
                        }
                    }
                }

                // Drag handle: QR Code
                Rectangle {
                    visible: wysiwygQrCode.visible
                    x: wysiwygQrCode.x + wysiwygQrCode.width - 8
                    y: wysiwygQrCode.y - 4
                    width: 12; height: 12; radius: 2; z: 20
                    color: dragQrMa.containsMouse ? "#6C5CE7" : Qt.rgba(1,1,1,0.3)
                    MouseArea {
                        id: dragQrMa; anchors.fill: parent; hoverEnabled: true
                        drag.target: wysiwygQrCode
                        cursorShape: pressed ? Qt.ClosedHandCursor : Qt.SizeAllCursor
                        onReleased: {
                            var qrPos = liveController.qrCodePosition || "bottom_right"
                            var qrBaseMargin = wysiwygOverlay.pw * 0.008
                            if (qrPos === "bottom_left" || qrPos === "top_left")
                                setupController.qrCodeOffsetX = Math.round((wysiwygQrCode.x - qrBaseMargin) * 1920.0 / wysiwygOverlay.pw)
                            else
                                setupController.qrCodeOffsetX = Math.round((wysiwygQrCode.x - (wysiwygOverlay.pw - wysiwygQrCode.width - qrBaseMargin)) * 1920.0 / wysiwygOverlay.pw)
                            if (qrPos === "top_right" || qrPos === "top_left")
                                setupController.qrCodeOffsetY = Math.round((wysiwygQrCode.y - qrBaseMargin) * 1080.0 / wysiwygOverlay.ph)
                            else {
                                var tickerH = wysiwygTickerBar.visible ? wysiwygTickerBar.height : 0
                                setupController.qrCodeOffsetY = Math.round((wysiwygQrCode.y - (wysiwygOverlay.ph - wysiwygQrCode.height - qrBaseMargin - tickerH)) * 1080.0 / wysiwygOverlay.ph)
                            }
                        }
                    }
                }

                // Drag handle: Subtitle
                Rectangle {
                    visible: wysiwygSubtitleBar.visible
                    x: wysiwygSubtitleBar.x + wysiwygSubtitleBar.width - 8
                    y: wysiwygSubtitleBar.y - 4
                    width: 12; height: 12; radius: 2; z: 20
                    color: dragSubMa.containsMouse ? "#6C5CE7" : Qt.rgba(1,1,1,0.3)
                    MouseArea {
                        id: dragSubMa; anchors.fill: parent; hoverEnabled: true
                        drag.target: wysiwygSubtitleBar
                        cursorShape: pressed ? Qt.ClosedHandCursor : Qt.SizeAllCursor
                        onReleased: {
                            setupController.subtitleOffsetX = Math.round((wysiwygSubtitleBar.x - (wysiwygOverlay.pw - wysiwygSubtitleBar.width) / 2) * 1920.0 / wysiwygOverlay.pw)
                            setupController.subtitleOffsetY = Math.round((wysiwygSubtitleBar.y - (wysiwygOverlay.ph - wysiwygSubtitleBar.height - wysiwygOverlay.ph * 0.008)) * 1080.0 / wysiwygOverlay.ph)
                        }
                    }
                }
            }

            // Resolution + FPS badge (on top of everything)
            Rectangle {
                anchors.top: parent.top; anchors.right: parent.right; anchors.margins: 10
                width: fpsLbl.implicitWidth + 14; height: 22; radius: 6
                color: Qt.rgba(0, 0, 0, 0.5)
                visible: previewMonitor.active
                z: 10
                Label {
                    id: fpsLbl; anchors.centerIn: parent
                    text: previewMonitor.sourceWidth + "\u00D7" + previewMonitor.sourceHeight + " @ " + previewMonitor.fps.toFixed(0) + "fps"
                    font.pixelSize: 11; font.family: "Menlo"; color: window.darkMode ? "#888" : "#555"
                }
            }

            // Placeholder (when Vision Engine not running)
            Column {
                anchors.centerIn: parent; spacing: 12; visible: !previewMonitor.active
                opacity: 0.4

                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: 64; height: 64; radius: 32
                    color: "transparent"; border.color: window.darkMode ? "#333" : "#CCC"; border.width: 2
                    Label { anchors.centerIn: parent; text: "\u25B6"; font.pixelSize: 24; color: window.darkMode ? "#333" : "#CCC" }
                }
                Label { anchors.horizontalCenter: parent.horizontalCenter; text: window.t("output_live"); color: window.darkMode ? "#555" : "#999"; font.pixelSize: 16; font.weight: Font.DemiBold }
                Label { anchors.horizontalCenter: parent.horizontalCenter; text: window.t("launch_vision"); color: window.darkMode ? "#444" : "#AAA"; font.pixelSize: 12 }
            }
        } // videoContainer

        // ── Detection & Control panel ────────────────────────────────
        Rectangle {
            Layout.fillWidth: true; Layout.preferredHeight: 82
            color: window.darkMode ? "#0B0B10" : "#F0F0F6"
            Rectangle { anchors.top: parent.top; width: parent.width; height: 1; color: window.darkMode ? Qt.rgba(255,255,255,0.04) : Qt.rgba(0,0,0,0.06) }

            RowLayout {
                anchors.fill: parent; anchors.leftMargin: 20; anchors.rightMargin: 20; spacing: 14

                // Detection indicator
                Item {
                    Layout.preferredWidth: 16; Layout.preferredHeight: 16

                    Rectangle {
                        anchors.centerIn: parent; width: 10; height: 10; radius: 5
                        color: liveController.talentDetected ? "#00D68F" : (window.darkMode ? "#2C2C34" : "#CCC")
                        Behavior on color { ColorAnimation { duration: 300 } }
                    }
                    // Pulse ring
                    Rectangle {
                        anchors.centerIn: parent; width: 16; height: 16; radius: 8
                        color: "transparent"
                        border.color: liveController.talentDetected ? Qt.rgba(0, 214, 143, 0.4) : "transparent"
                        border.width: 1
                        Behavior on border.color { ColorAnimation { duration: 300 } }
                        SequentialAnimation on scale {
                            loops: Animation.Infinite; running: liveController.talentDetected
                            NumberAnimation { from: 1; to: 1.6; duration: 1200; easing.type: Easing.OutQuad }
                            NumberAnimation { from: 1.6; to: 1; duration: 0 }
                        }
                        SequentialAnimation on opacity {
                            loops: Animation.Infinite; running: liveController.talentDetected
                            NumberAnimation { from: 0.6; to: 0; duration: 1200; easing.type: Easing.OutQuad }
                            NumberAnimation { from: 0; to: 0.6; duration: 0 }
                        }
                    }
                }

                // Scene mode pill
                Rectangle {
                    visible: liveController.talentDetected
                    Layout.preferredWidth: modeLbl.implicitWidth + 18; Layout.preferredHeight: 26; radius: 13
                    color: liveController.isMultiFace ? Qt.rgba(108/255,92/255,231/255,0.12) : Qt.rgba(0, 214, 143, 0.10)
                    border.color: liveController.isMultiFace ? Qt.rgba(108/255,92/255,231/255,0.25) : Qt.rgba(0, 214, 143, 0.20)
                    Behavior on color { ColorAnimation { duration: 300 } }

                    Label {
                        id: modeLbl; anchors.centerIn: parent
                        text: liveController.isMultiFace ? liveController.faceCount + " " + window.t("faces") : "1 " + window.t("face")
                        color: liveController.isMultiFace ? "#8B80E0" : "#00D68F"
                        font.pixelSize: 10; font.weight: Font.Bold; font.letterSpacing: 0.3
                    }
                }

                // Talent info
                ColumnLayout {
                    spacing: 2; Layout.fillWidth: true
                    Label {
                        text: liveController.talentDetected ? liveController.detectedName : window.t("waiting")
                        font.pixelSize: 16; font.weight: Font.Bold
                        color: liveController.talentDetected ? (window.darkMode ? "#F0F0F5" : "#0F0F14") : (window.darkMode ? "#3A3A44" : "#AAA")
                        Behavior on color { ColorAnimation { duration: 400 } }
                    }
                    Label {
                        text: liveController.talentDetected ? liveController.detectedRole : ""
                        font.pixelSize: 12; color: window.darkMode ? "#6A6A78" : "#888"
                    }
                }

                // Confidence pill
                Rectangle {
                    visible: liveController.talentDetected && !liveController.isMultiFace
                    Layout.preferredWidth: confLbl.implicitWidth + 16; Layout.preferredHeight: 26; radius: 6
                    color: liveController.confidence > 0.85 ? Qt.rgba(0, 214, 143, 0.10) : Qt.rgba(255, 170, 0, 0.10)
                    border.color: liveController.confidence > 0.85 ? Qt.rgba(0, 214, 143, 0.20) : Qt.rgba(255, 170, 0, 0.20)

                    Label {
                        id: confLbl; anchors.centerIn: parent
                        text: Math.round(liveController.confidence * 100) + "%"
                        color: liveController.confidence > 0.85 ? "#00D68F" : "#FFAA00"
                        font.pixelSize: 11; font.weight: Font.Bold; font.family: "SF Mono, Menlo, monospace"
                    }
                }

                // ── DEMO 1 VISAGE ──
                Rectangle {
                    property bool isActive: liveView.demoPhase !== "idle" && liveView.demoMode === "single"
                    Layout.preferredWidth: demoTxt.implicitWidth + 16; Layout.preferredHeight: 36; radius: 8
                    color: isActive
                           ? (msDemo.containsMouse ? Qt.rgba(108/255,92/255,231/255,0.25) : Qt.rgba(108/255,92/255,231/255,0.18))
                           : (msDemo.containsMouse ? (window.darkMode ? Qt.rgba(255,255,255,0.07) : Qt.rgba(0,0,0,0.07)) : (window.darkMode ? Qt.rgba(255,255,255,0.03) : Qt.rgba(0,0,0,0.03)))
                    border.color: isActive ? Qt.rgba(108/255,92/255,231/255,0.5) : (window.darkMode ? Qt.rgba(255,255,255,0.08) : Qt.rgba(0,0,0,0.08))
                    Behavior on color { ColorAnimation { duration: 150 } }
                    Label {
                        id: demoTxt; anchors.centerIn: parent
                        text: parent.isActive ? "STOP" : "DEMO 1"
                        color: parent.isActive ? "#6C5CE7" : (window.darkMode ? "#6A6A78" : "#666")
                        font.pixelSize: 9; font.weight: Font.Bold; font.letterSpacing: 0.8
                    }
                    MouseArea {
                        id: msDemo; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                        onClicked: liveView.startDemo("single")
                    }
                }

                // ── DEMO MULTI VISAGE ──
                Rectangle {
                    property bool isActive: liveView.demoPhase !== "idle" && liveView.demoMode === "multi"
                    Layout.preferredWidth: demoMultiTxt.implicitWidth + 16; Layout.preferredHeight: 36; radius: 8
                    color: isActive
                           ? (msDemoMulti.containsMouse ? Qt.rgba(108/255,92/255,231/255,0.25) : Qt.rgba(108/255,92/255,231/255,0.18))
                           : (msDemoMulti.containsMouse ? (window.darkMode ? Qt.rgba(255,255,255,0.07) : Qt.rgba(0,0,0,0.07)) : (window.darkMode ? Qt.rgba(255,255,255,0.03) : Qt.rgba(0,0,0,0.03)))
                    border.color: isActive ? Qt.rgba(108/255,92/255,231/255,0.5) : (window.darkMode ? Qt.rgba(255,255,255,0.08) : Qt.rgba(0,0,0,0.08))
                    Behavior on color { ColorAnimation { duration: 150 } }
                    Label {
                        id: demoMultiTxt; anchors.centerIn: parent
                        text: parent.isActive ? "STOP" : "DEMO 2+"
                        color: parent.isActive ? "#6C5CE7" : (window.darkMode ? "#6A6A78" : "#666")
                        font.pixelSize: 9; font.weight: Font.Bold; font.letterSpacing: 0.8
                    }
                    MouseArea {
                        id: msDemoMulti; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                        onClicked: liveView.startDemo("multi")
                    }
                }

                // Separator
                Rectangle { Layout.preferredWidth: 1; Layout.preferredHeight: 36; color: window.darkMode ? Qt.rgba(255,255,255,0.06) : Qt.rgba(0,0,0,0.08) }

                // BYPASS button
                Rectangle {
                    id: bypassBtn
                    property bool bypassed: false
                    Layout.preferredWidth: 90; Layout.preferredHeight: 36; radius: 8
                    color: bypassed
                           ? (msBypass.containsMouse ? "#E53935" : "#D32F2F")
                           : (msBypass.containsMouse ? (window.darkMode ? Qt.rgba(255,255,255,0.07) : Qt.rgba(0,0,0,0.07)) : (window.darkMode ? Qt.rgba(255,255,255,0.03) : Qt.rgba(0,0,0,0.03)))
                    border.color: bypassed ? Qt.rgba(255, 61, 113, 0.4) : (window.darkMode ? Qt.rgba(255,255,255,0.08) : Qt.rgba(0,0,0,0.08))
                    Behavior on color { ColorAnimation { duration: 150 } }

                    Label {
                        anchors.centerIn: parent
                        text: "BYPASS"
                        color: bypassBtn.bypassed ? "white" : (window.darkMode ? "#6A6A78" : "#666")
                        font.pixelSize: 10; font.weight: Font.Bold; font.letterSpacing: 1.2
                    }
                    MouseArea {
                        id: msBypass; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            bypassBtn.bypassed = !bypassBtn.bypassed
                            liveController.setBypassed(bypassBtn.bypassed)
                            if (bypassBtn.bypassed)
                                liveController.setOverlaysActive(false)
                            else
                                liveController.setOverlaysActive(true)
                        }
                    }
                }

                // Overlay toggle
                Rectangle {
                    Layout.preferredWidth: 120; Layout.preferredHeight: 36; radius: 8
                    visible: !bypassBtn.bypassed
                    color: liveController.overlaysActive
                           ? (msOverlay.containsMouse ? Qt.rgba(0, 214, 143, 0.14) : Qt.rgba(0, 214, 143, 0.08))
                           : (msOverlay.containsMouse ? Qt.rgba(255, 61, 113, 0.14) : Qt.rgba(255, 61, 113, 0.08))
                    border.color: liveController.overlaysActive ? Qt.rgba(0, 214, 143, 0.25) : Qt.rgba(255, 61, 113, 0.25)
                    Behavior on color { ColorAnimation { duration: 200 } }

                    Label {
                        anchors.centerIn: parent
                        text: liveController.overlaysActive ? window.t("overlays_on") : window.t("overlays_off")
                        color: liveController.overlaysActive ? "#00D68F" : "#FF3D71"
                        font.pixelSize: 10; font.weight: Font.Bold; font.letterSpacing: 0.5
                    }
                    MouseArea { id: msOverlay; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: liveController.toggleOverlays() }
                }
            }
        }

        // ── Graphics Queue Bar ───────────────────────────────
        Rectangle {
            id: queueBar
            Layout.fillWidth: true; Layout.preferredHeight: queueBar.queueExpanded ? 160 : 34
            color: window.darkMode ? "#0A0A0E" : "#EAEAF0"
            property bool queueExpanded: false

            Behavior on Layout.preferredHeight { NumberAnimation { duration: 250; easing.type: Easing.OutCubic } }

            Rectangle { anchors.top: parent.top; width: parent.width; height: 1; color: window.darkMode ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.06) }

            ColumnLayout {
                anchors.fill: parent; spacing: 0

                // Header (always visible)
                Rectangle {
                    Layout.fillWidth: true; Layout.preferredHeight: 36
                    color: "transparent"
                    RowLayout {
                        anchors.fill: parent; anchors.leftMargin: 12; anchors.rightMargin: 12; spacing: 8

                        Label { text: "QUEUE"; font.pixelSize: 11; font.weight: Font.Bold; font.letterSpacing: 2; color: window.darkMode ? "#666" : "#999" }
                        Label { text: graphicsQueue.count + " items"; font.pixelSize: 11; color: window.darkMode ? "#555" : "#AAA" }

                        Item { Layout.fillWidth: true }

                        // Preview next
                        Label {
                            visible: graphicsQueue.nextItem.type !== undefined
                            text: "NEXT: " + (graphicsQueue.nextItem.name || graphicsQueue.nextItem.text || "")
                            font.pixelSize: 11; color: "#6C5CE7"; elide: Text.ElideRight; Layout.maximumWidth: 200
                        }

                        // TAKE button
                        Rectangle {
                            Layout.preferredWidth: 60; Layout.preferredHeight: 36; radius: 6
                            color: queueTakeMa.containsMouse ? "#DD0000" : "#CC0000"
                            visible: graphicsQueue.count > 0
                            Label { anchors.centerIn: parent; text: "TAKE"; color: "white"; font.pixelSize: 11; font.weight: Font.Bold }
                            MouseArea { id: queueTakeMa; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: graphicsQueue.takeNext() }
                        }

                        // Clear
                        Rectangle {
                            Layout.preferredWidth: 50; Layout.preferredHeight: 32; radius: 6
                            color: window.darkMode ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.04)
                            visible: graphicsQueue.currentIndex >= 0
                            Label { anchors.centerIn: parent; text: "CLEAR"; color: window.darkMode ? "#888" : "#666"; font.pixelSize: 11; font.weight: Font.Bold }
                            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: graphicsQueue.clearProgram() }
                        }

                        // Expand/collapse
                        Rectangle {
                            Layout.preferredWidth: 32; Layout.preferredHeight: 32; radius: 4
                            color: window.darkMode ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.04)
                            Label { anchors.centerIn: parent; text: queueBar.queueExpanded ? "\u25BC" : "\u25B2"; font.pixelSize: 11; color: window.darkMode ? "#888" : "#666" }
                            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: queueBar.queueExpanded = !queueBar.queueExpanded }
                        }
                    }
                }

                // Queue items list (expanded)
                ListView {
                    Layout.fillWidth: true; Layout.fillHeight: true; clip: true; spacing: 2
                    visible: queueBar.queueExpanded
                    model: graphicsQueue.items
                    orientation: ListView.Horizontal
                    delegate: Rectangle {
                        width: 160; height: ListView.view ? ListView.view.height - 4 : 100
                        radius: 6
                        color: graphicsQueue.currentIndex === index
                            ? Qt.rgba(204/255,0,0,0.15)
                            : (window.darkMode ? Qt.rgba(1,1,1,0.03) : Qt.rgba(0,0,0,0.03))
                        border.color: graphicsQueue.currentIndex === index ? "#CC0000" : (window.darkMode ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.08))

                        ColumnLayout {
                            anchors.fill: parent; anchors.margins: 8; spacing: 2
                            Label {
                                text: {
                                    if (modelData.type === "lower_third") return "LOWER THIRD"
                                    if (modelData.type === "ticker") return "TICKER"
                                    if (modelData.type === "message") return "MESSAGE"
                                    if (modelData.type === "qr_code") return "QR CODE"
                                    return modelData.type || ""
                                }
                                font.pixelSize: 10; font.weight: Font.Bold; color: "#6C5CE7"
                            }
                            Label {
                                text: modelData.name || modelData.text || modelData.url || ""
                                font.pixelSize: 11; font.weight: Font.DemiBold; color: window.darkMode ? "white" : "#1A1A1A"
                                elide: Text.ElideRight; Layout.fillWidth: true
                            }
                            Label {
                                text: modelData.role || ""
                                font.pixelSize: 11; color: window.darkMode ? "#888" : "#666"
                                visible: modelData.role !== undefined && modelData.role !== ""
                            }
                            Item { Layout.fillHeight: true }
                            Label {
                                text: graphicsQueue.currentIndex === index ? "ON AIR" : (index < graphicsQueue.currentIndex ? "DONE" : "READY")
                                font.pixelSize: 10; font.weight: Font.Bold
                                color: graphicsQueue.currentIndex === index ? "#CC0000" : (index < graphicsQueue.currentIndex ? "#00D68F" : (window.darkMode ? "#666" : "#999"))
                            }
                        }

                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: graphicsQueue.takeItem(index) }
                    }
                }
            }
        }

        // ── Broadcast Status Bar ──────────────────────────
        Rectangle {
            Layout.fillWidth: true; Layout.preferredHeight: 36
            color: window.darkMode ? "#08080C" : "#E2E2E8"
            Rectangle { anchors.top: parent.top; width: parent.width; height: 1; color: window.darkMode ? Qt.rgba(255,255,255,0.03) : Qt.rgba(0,0,0,0.05) }

            RowLayout {
                anchors.fill: parent; anchors.leftMargin: 14; anchors.rightMargin: 14; spacing: 12

                // Output indicators
                Repeater {
                    model: [
                        { label: "SDI",  active: liveController.sdiActive },
                        { label: "NDI",  active: liveController.ndiActive },
                        { label: "RTMP", active: liveController.rtmpActive },
                        { label: "SRT",  active: liveController.srtActive }
                    ]
                    delegate: Row {
                        spacing: 4
                        Rectangle {
                            width: 5; height: 5; radius: 3; anchors.verticalCenter: parent.verticalCenter
                            color: modelData.active ? "#00D68F" : (window.darkMode ? "#252530" : "#CCC")
                            Behavior on color { ColorAnimation { duration: 300 } }
                        }
                        Label {
                            text: modelData.label; font.pixelSize: 10; font.weight: Font.Medium; font.letterSpacing: 0.5
                            color: modelData.active ? (window.darkMode ? "#8E8E9A" : "#444") : (window.darkMode ? "#2E2E38" : "#BBB")
                            Behavior on color { ColorAnimation { duration: 300 } }
                        }
                    }
                }

                Rectangle { Layout.preferredWidth: 1; Layout.preferredHeight: 16; color: window.darkMode ? Qt.rgba(255,255,255,0.06) : Qt.rgba(0,0,0,0.08) }

                // FPS
                RowLayout { spacing: 4
                    Rectangle { Layout.preferredWidth: 5; Layout.preferredHeight: 5; radius: 3; color: liveController.fps >= 24 ? "#00D68F" : (liveController.fps >= 15 ? "#FFAA00" : "#FF3D71") }
                    Label { text: liveController.fps + " fps"; font.pixelSize: 10; font.family: "SF Mono, Menlo, monospace"; color: window.darkMode ? "#8E8E9A" : "#555" }
                }

                // Latency
                RowLayout { spacing: 4
                    Rectangle { Layout.preferredWidth: 5; Layout.preferredHeight: 5; radius: 3; color: liveController.latencyMs < 50 ? "#00D68F" : (liveController.latencyMs < 100 ? "#FFAA00" : "#FF3D71") }
                    Label { text: liveController.latencyMs + " ms"; font.pixelSize: 10; font.family: "SF Mono, Menlo, monospace"; color: window.darkMode ? "#8E8E9A" : "#555" }
                }

                Rectangle { Layout.preferredWidth: 1; Layout.preferredHeight: 16; color: window.darkMode ? Qt.rgba(255,255,255,0.06) : Qt.rgba(0,0,0,0.08) }

                // AI status
                RowLayout { spacing: 4
                    Rectangle { Layout.preferredWidth: 5; Layout.preferredHeight: 5; radius: 3; color: "#00D68F" }
                    Label { text: "AI"; font.pixelSize: 10; font.weight: Font.Bold; font.letterSpacing: 0.5; color: window.darkMode ? "#8E8E9A" : "#555" }
                }

                // Source
                Label { text: setupController.inputType || "\u2014"; font.pixelSize: 10; color: window.darkMode ? "#505060" : "#999" }

                Rectangle { Layout.preferredWidth: 1; Layout.preferredHeight: 16; color: window.darkMode ? Qt.rgba(255,255,255,0.06) : Qt.rgba(0,0,0,0.08) }

                // VU meter bars (professional segmented look)
                RowLayout { spacing: 2
                    Label { text: "VU"; font.pixelSize: 9; font.letterSpacing: 1; color: window.darkMode ? "#505060" : "#999" }
                    // Left channel
                    Row {
                        spacing: 1
                        Repeater {
                            model: 8
                            Rectangle {
                                width: 3; height: 14; radius: 1
                                color: {
                                    var level = audioMeter.levelL * 8
                                    if (index >= level) return window.darkMode ? "#1A1A22" : "#D0D0D8"
                                    if (index >= 6) return "#FF3D71"
                                    if (index >= 4) return "#FFAA00"
                                    return "#00D68F"
                                }
                            }
                        }
                    }
                    // Right channel
                    Row {
                        spacing: 1
                        Repeater {
                            model: 8
                            Rectangle {
                                width: 3; height: 14; radius: 1
                                color: {
                                    var level = audioMeter.levelR * 8
                                    if (index >= level) return window.darkMode ? "#1A1A22" : "#D0D0D8"
                                    if (index >= 6) return "#FF3D71"
                                    if (index >= 4) return "#FFAA00"
                                    return "#00D68F"
                                }
                            }
                        }
                    }
                }

                Item { Layout.fillWidth: true }

                // REC button
                Rectangle {
                    Layout.preferredWidth: recBtnRow.implicitWidth + 14; Layout.preferredHeight: 26; radius: 6
                    color: liveController.isRecording
                           ? (msRecBtn.containsMouse ? Qt.rgba(255, 59, 48, 0.20) : Qt.rgba(255, 59, 48, 0.12))
                           : (msRecBtn.containsMouse ? (window.darkMode ? Qt.rgba(255,255,255,0.05) : Qt.rgba(0,0,0,0.05)) : "transparent")
                    border.color: liveController.isRecording ? Qt.rgba(255, 59, 48, 0.3) : "transparent"
                    Behavior on color { ColorAnimation { duration: 150 } }

                    RowLayout {
                        id: recBtnRow; anchors.centerIn: parent; spacing: 5
                        Rectangle {
                            Layout.preferredWidth: 6; Layout.preferredHeight: 6; radius: 3
                            color: liveController.isRecording ? "#FF3B30" : (window.darkMode ? "#3A3A44" : "#AAA")
                            SequentialAnimation on opacity {
                                loops: Animation.Infinite; running: liveController.isRecording
                                NumberAnimation { to: 0.3; duration: 500 }
                                NumberAnimation { to: 1.0; duration: 500 }
                            }
                        }
                        Label {
                            text: liveController.isRecording ? ("REC " + liveController.recordingDuration) : "REC"
                            font.pixelSize: 10; font.family: "SF Mono, Menlo, monospace"; font.weight: Font.Bold
                            color: liveController.isRecording ? "#FF6B60" : (window.darkMode ? "#3A3A44" : "#AAA")
                        }
                    }
                    MouseArea { id: msRecBtn; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: liveController.toggleRecording() }
                }

                Rectangle { Layout.preferredWidth: 1; Layout.preferredHeight: 16; color: window.darkMode ? Qt.rgba(255,255,255,0.06) : Qt.rgba(0,0,0,0.08) }

                // Elapsed
                Label {
                    text: liveController.formattedDuration(liveView.elapsedSeconds)
                    font.pixelSize: 10; font.family: "SF Mono, Menlo, monospace"
                    color: window.darkMode ? "#6A6A78" : "#666"
                }

                Rectangle { Layout.preferredWidth: 1; Layout.preferredHeight: 16; color: window.darkMode ? Qt.rgba(255,255,255,0.06) : Qt.rgba(0,0,0,0.08) }

                // Stop button
                Rectangle {
                    Layout.preferredWidth: stopLabel.implicitWidth + 18; Layout.preferredHeight: 26; radius: 6
                    color: msStopBtn.containsMouse ? (window.darkMode ? Qt.rgba(255, 61, 113, 0.10) : Qt.rgba(255, 61, 113, 0.08)) : "transparent"
                    Behavior on color { ColorAnimation { duration: 150 } }
                    Label {
                        id: stopLabel; anchors.centerIn: parent
                        text: "\u25A0 " + window.t("stop")
                        color: msStopBtn.containsMouse ? "#FF3D71" : (window.darkMode ? "#505060" : "#999")
                        font.pixelSize: 10; font.weight: Font.Medium
                        Behavior on color { ColorAnimation { duration: 150 } }
                    }
                    MouseArea { id: msStopBtn; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: stopConfirmDialog.open() }
                }
            }
        }
    }
}
