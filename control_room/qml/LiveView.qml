import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Effects

Item {
    id: liveView

    property int elapsedSeconds: 0
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

    ColumnLayout {
        anchors.fill: parent; spacing: 0

        // ── ON AIR Bar ─────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true; Layout.preferredHeight: 52
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: mainWindow.overlaysActive ? "#CC0000" : "#333333" }
                GradientStop { position: 0.5; color: mainWindow.overlaysActive ? "#DD0000" : "#444444" }
                GradientStop { position: 1.0; color: mainWindow.overlaysActive ? "#BB0000" : "#333333" }
            }

            RowLayout {
                anchors.fill: parent; anchors.leftMargin: 24; anchors.rightMargin: 24; spacing: 14

                // Pulsing dot with glow
                Item {
                    Layout.preferredWidth: 16; Layout.preferredHeight: 16
                    Rectangle {
                        anchors.centerIn: parent; width: 16; height: 16; radius: 8
                        color: mainWindow.overlaysActive ? Qt.rgba(1, 1, 1, 0.2) : Qt.rgba(1, 1, 1, 0.1)
                    }
                    Rectangle {
                        anchors.centerIn: parent; width: 10; height: 10; radius: 5
                        color: mainWindow.overlaysActive ? "white" : "#888"
                        SequentialAnimation on opacity {
                            loops: Animation.Infinite; running: mainWindow.overlaysActive
                            NumberAnimation { to: 0.3; duration: 800; easing.type: Easing.InOutSine }
                            NumberAnimation { to: 1.0; duration: 800; easing.type: Easing.InOutSine }
                        }
                    }
                }

                Label {
                    text: mainWindow.overlaysActive ? window.t("overlays_active") : window.t("passthrough")
                    font.pixelSize: 15; font.weight: Font.Bold
                    font.letterSpacing: 1; color: "white"
                }

                Rectangle { Layout.preferredWidth: 1; Layout.preferredHeight: 24; color: Qt.rgba(1,1,1,0.25); visible: configManager.channelName !== "" }

                Label { text: configManager.channelName; font.pixelSize: 15; font.weight: Font.DemiBold; color: Qt.rgba(1,1,1,0.9); visible: configManager.channelName !== "" }

                Rectangle { Layout.preferredWidth: 1; Layout.preferredHeight: 24; color: Qt.rgba(1,1,1,0.25) }

                Label { text: setupController.currentProfileName; font.pixelSize: 14; color: Qt.rgba(1,1,1,0.7) }

                Item { Layout.fillWidth: true }

                // Timer with monospace
                Label {
                    text: liveController.formattedDuration(liveView.elapsedSeconds)
                    font.pixelSize: 20; font.weight: Font.Bold; font.family: "Menlo"; color: "white"
                }
            }
        }

        // ── Video output (full width — no side panel) ────────────────
        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true
            color: "#000000"

            // Live composited video from Vision Engine (:5558)
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

            // ── WYSIWYG Preview Overlays ─────────────────
            // Shows exactly what the viewer sees on TV
            Item {
                id: wysiwygOverlay
                anchors.fill: parent
                visible: true  // Always visible to show branding even without Vision Engine

                // Standard broadcast proportions (based on 1920x1080 reference)
                property real pw: parent.width   // Preview width
                property real ph: parent.height  // Preview height

                // ── Layer 1: Channel Logo ────────────────────
                Image {
                    id: previewLogo
                    visible: setupController.channelLogoPath !== "" && !(liveController.isBypassed && !setupController.keepLogoDuringAds)
                    source: setupController.channelLogoPath ? ("file:///" + setupController.channelLogoPath) : ""

                    width: wysiwygOverlay.pw * 0.042
                    height: wysiwygOverlay.pw * 0.042
                    fillMode: Image.PreserveAspectFit

                    // Base position + offset
                    property string pos: setupController.channelLogoPosition
                    property real baseMargin: wysiwygOverlay.pw * 0.008
                    x: {
                        if (pos === "top_left" || pos === "bottom_left")
                            return baseMargin + setupController.channelLogoOffsetX * (wysiwygOverlay.pw / 1920.0)
                        else
                            return parent.width - width - baseMargin + setupController.channelLogoOffsetX * (wysiwygOverlay.pw / 1920.0)
                    }
                    y: {
                        if (pos === "top_left" || pos === "top_right")
                            return baseMargin + setupController.channelLogoOffsetY * (wysiwygOverlay.ph / 1080.0)
                        else
                            return parent.height - height - baseMargin + setupController.channelLogoOffsetY * (wysiwygOverlay.ph / 1080.0)
                    }

                    // Loop animation
                    property string loopAnim: setupController.logoLoopAnim
                    SequentialAnimation on scale {
                        loops: Animation.Infinite; running: previewLogo.loopAnim === "pulse"
                        NumberAnimation { to: 1.03; duration: 1200; easing.type: Easing.InOutSine }
                        NumberAnimation { to: 1.0; duration: 1200; easing.type: Easing.InOutSine }
                    }
                    opacity: 1.0
                    // Entry animation
                    NumberAnimation on opacity { from: 0; to: 1; duration: 800; running: true }
                }

                // ── Layer 1b: Channel Name ───────────────────
                Rectangle {
                    id: previewChannelName
                    visible: setupController.showChannelNameText && configManager.channelName !== "" && !(liveController.isBypassed && !setupController.keepLogoDuringAds)

                    width: nameLbl.implicitWidth + wysiwygOverlay.pw * 0.008
                    height: wysiwygOverlay.ph * 0.033

                    property string pos: setupController.channelLogoPosition
                    property real baseMargin: wysiwygOverlay.pw * 0.005
                    x: {
                        if (pos === "top_left" || pos === "bottom_left")
                            return baseMargin + setupController.channelNameOffsetX * (wysiwygOverlay.pw / 1920.0)
                        else
                            return parent.width - width - baseMargin + setupController.channelNameOffsetX * (wysiwygOverlay.pw / 1920.0)
                    }
                    y: {
                        var logoBottom = previewLogo.visible ? (previewLogo.y + previewLogo.height + wysiwygOverlay.ph * 0.004) : 0
                        if (pos === "top_left" || pos === "top_right")
                            return (previewLogo.visible ? logoBottom : baseMargin) + setupController.channelNameOffsetY * (wysiwygOverlay.ph / 1080.0)
                        else
                            return parent.height - height - baseMargin + setupController.channelNameOffsetY * (wysiwygOverlay.ph / 1080.0)
                    }
                    radius: {
                        var shape = setupController.channelNameShape
                        if (shape === "pill") return height / 2
                        if (shape === "square" || shape === "rectangle") return wysiwygOverlay.ph * 0.003
                        return 0
                    }
                    color: setupController.channelNameShape === "frameless" ? "transparent" : setupController.channelNameBgColor
                    border.color: setupController.channelNameShape === "frameless" ? "transparent" : setupController.channelNameBorderColor
                    border.width: setupController.channelNameShape === "frameless" ? 0 : 1

                    // Skew for angled shape
                    transform: Rotation {
                        angle: setupController.channelNameShape === "angled" ? -5 : 0
                        origin.x: previewChannelName.width / 2; origin.y: previewChannelName.height / 2
                    }

                    Label {
                        id: nameLbl; anchors.centerIn: parent
                        text: configManager.channelName
                        font.pixelSize: wysiwygOverlay.ph * 0.013
                        font.weight: Font.Bold
                        color: setupController.channelNameTextColor
                    }

                    // Pulse animation
                    SequentialAnimation on scale {
                        loops: Animation.Infinite; running: setupController.nameLoopAnim === "pulse"
                        NumberAnimation { to: 1.03; duration: 1200; easing.type: Easing.InOutSine }
                        NumberAnimation { to: 1.0; duration: 1200; easing.type: Easing.InOutSine }
                    }
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
                    Behavior on opacity { NumberAnimation { duration: 400 } }

                    // Accent bar on left
                    Rectangle {
                        width: wysiwygOverlay.pw * 0.002; height: parent.height
                        color: setupController.accentColor.toString() !== "#000000" ? setupController.accentColor : "#5B4FDB"
                        visible: setupController.showTitleShape !== "frameless"
                    }

                    ColumnLayout {
                        id: titleCol; anchors.centerIn: parent; spacing: 2
                        Label {
                            text: setupController.showTitle
                            font.pixelSize: wysiwygOverlay.ph * 0.015
                            font.weight: Font.Bold; color: setupController.showTitleTextColor
                        }
                        Label {
                            visible: setupController.showSubtitle !== ""
                            text: setupController.showSubtitle
                            font.pixelSize: wysiwygOverlay.ph * 0.012
                            color: Qt.rgba(1,1,1,0.7)
                        }
                    }
                }

                // ── Layer 3: Talent Nameplate (from IA detection) ──
                Rectangle {
                    id: previewNameplate
                    visible: liveController.talentNameplateVisible && liveController.talentDetected && !liveController.isBypassed

                    anchors.bottom: wysiwygTickerBar.top; anchors.left: parent.left
                    anchors.margins: wysiwygOverlay.pw * 0.008

                    width: Math.max(wysiwygOverlay.pw * 0.198, talentCol.implicitWidth + wysiwygOverlay.pw * 0.017)
                    height: wysiwygOverlay.ph * 0.041
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
                            font.pixelSize: wysiwygOverlay.ph * 0.015; font.weight: Font.Bold; color: "white"
                        }
                        Label {
                            visible: liveController.detectedRole !== ""
                            text: liveController.detectedRole || ""
                            font.pixelSize: wysiwygOverlay.ph * 0.011; color: Qt.rgba(1,1,1,0.7)
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
                    visible: rssFetcher.headlines !== "" && !liveController.isBypassed
                    color: "#CC0000"
                    clip: true

                    Label {
                        id: tickerText
                        text: rssFetcher.headlines || ""
                        font.pixelSize: wysiwygOverlay.ph * 0.012; font.weight: Font.Bold; color: "white"
                        y: (parent.height - height) / 2
                        NumberAnimation on x {
                            from: wysiwygTickerBar.width
                            to: -tickerText.implicitWidth
                            duration: Math.max(8000, tickerText.implicitWidth * 30)
                            loops: Animation.Infinite; running: wysiwygTickerBar.visible
                        }
                    }
                }

                // ── Subtitles ────────────────────────────────
                Rectangle {
                    id: wysiwygSubtitleBar
                    x: (parent.width - width) / 2 + setupController.subtitleOffsetX * (wysiwygOverlay.pw / 1920.0)
                    y: parent.height - height - wysiwygOverlay.ph * 0.008 + setupController.subtitleOffsetY * (wysiwygOverlay.ph / 1080.0)
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

                // ── Countdown (top-left area) ────────────────
                Rectangle {
                    visible: liveController.countdownActive && !liveController.isBypassed
                    x: wysiwygOverlay.pw * 0.008 + setupController.countdownOffsetX * (wysiwygOverlay.pw / 1920.0)
                    y: (previewLogo.visible ? previewLogo.height + wysiwygOverlay.ph * 0.022 : wysiwygOverlay.ph * 0.015) + setupController.countdownOffsetY * (wysiwygOverlay.ph / 1080.0)

                    width: Math.max(wysiwygOverlay.pw * 0.073, cdLbl.implicitWidth + wysiwygOverlay.pw * 0.011)
                    height: wysiwygOverlay.ph * 0.033
                    radius: height / 2
                    color: Qt.rgba(204/255, 0, 0, 0.85)

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
                        font.pixelSize: wysiwygOverlay.ph * 0.013; font.weight: Font.Bold; font.family: "Menlo"; color: "white"
                    }
                }

                // ── Clock (top-right) ────────────────────────
                Rectangle {
                    visible: true
                    x: parent.width - width - wysiwygOverlay.pw * 0.008 + setupController.clockOffsetX * (wysiwygOverlay.pw / 1920.0)
                    y: wysiwygOverlay.ph * 0.015 + setupController.clockOffsetY * (wysiwygOverlay.ph / 1080.0)

                    width: wysiwygOverlay.pw * 0.063
                    height: wysiwygOverlay.ph * 0.030
                    radius: wysiwygOverlay.ph * 0.004
                    color: Qt.rgba(0, 0, 0, 0.6)

                    Label {
                        id: clockLbl; anchors.centerIn: parent
                        text: Qt.formatTime(new Date(), "HH:mm:ss")
                        font.pixelSize: wysiwygOverlay.ph * 0.013; font.weight: Font.Bold; font.family: "Menlo"; color: "white"
                    }
                    Timer { interval: 1000; running: true; repeat: true; onTriggered: clockLbl.text = Qt.formatTime(new Date(), "HH:mm:ss") }
                }

                // ── QR Code placeholder (when active) ────────
                Rectangle {
                    visible: liveController.qrCodeVisible && liveController.qrCodeUrl !== "" && !liveController.isBypassed

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

                    width: wysiwygOverlay.pw * 0.052; height: wysiwygOverlay.pw * 0.052
                    radius: wysiwygOverlay.ph * 0.006
                    color: "white"

                    // QR pattern placeholder
                    Grid {
                        anchors.centerIn: parent; columns: 5; spacing: wysiwygOverlay.pw * 0.001
                        Repeater {
                            model: 25
                            Rectangle {
                                width: wysiwygOverlay.pw * 0.005; height: wysiwygOverlay.pw * 0.005
                                color: (index % 3 === 0 || index % 7 === 0) ? "#000" : "white"
                            }
                        }
                    }

                    Label {
                        anchors.bottom: parent.bottom; anchors.horizontalCenter: parent.horizontalCenter
                        anchors.bottomMargin: wysiwygOverlay.ph * 0.002
                        text: "SCAN"; font.pixelSize: wysiwygOverlay.ph * 0.007; font.weight: Font.Bold; color: "#5B4FDB"
                    }
                }

                // ── Scoreboard overlay ──────────────────────
                Rectangle {
                    id: wysiwygScoreboard
                    visible: mainWindow.overlaysActive && !liveController.isBypassed

                    x: (wysiwygOverlay.pw * 0.008) + setupController.scoreboardOffsetX * (wysiwygOverlay.pw / 1920.0)
                    y: (wysiwygOverlay.ph * 0.056) + setupController.scoreboardOffsetY * (wysiwygOverlay.ph / 1080.0)

                    width: wysiwygOverlay.pw * 0.167; height: wysiwygOverlay.ph * 0.074
                    radius: wysiwygOverlay.ph * 0.006
                    color: Qt.rgba(0, 0, 0, 0.75)
                    border.color: Qt.rgba(1,1,1,0.1)

                    RowLayout {
                        anchors.centerIn: parent; spacing: wysiwygOverlay.pw * 0.005
                        ColumnLayout {
                            spacing: 1
                            Label { text: "TEAM A"; font.pixelSize: wysiwygOverlay.ph * 0.009; color: "#CC0000"; font.weight: Font.Bold; Layout.alignment: Qt.AlignHCenter }
                            Label { text: "0"; font.pixelSize: wysiwygOverlay.ph * 0.022; font.weight: Font.Bold; color: "white"; Layout.alignment: Qt.AlignHCenter }
                        }
                        Label { text: "-"; font.pixelSize: wysiwygOverlay.ph * 0.015; color: "#555" }
                        ColumnLayout {
                            spacing: 1
                            Label { text: "TEAM B"; font.pixelSize: wysiwygOverlay.ph * 0.009; color: "#0066CC"; font.weight: Font.Bold; Layout.alignment: Qt.AlignHCenter }
                            Label { text: "0"; font.pixelSize: wysiwygOverlay.ph * 0.022; font.weight: Font.Bold; color: "white"; Layout.alignment: Qt.AlignHCenter }
                        }
                    }
                }

                // ── Weather overlay ──────────────────────────
                Rectangle {
                    id: wysiwygWeather
                    visible: weatherFetcher.city !== "" && !liveController.isBypassed

                    x: parent.width - width - wysiwygOverlay.pw * 0.008 + setupController.weatherOffsetX * (wysiwygOverlay.pw / 1920.0)
                    y: parent.height - height - wysiwygOverlay.ph * 0.074 + setupController.weatherOffsetY * (wysiwygOverlay.ph / 1080.0)

                    width: wysiwygOverlay.pw * 0.104; height: wysiwygOverlay.ph * 0.065
                    radius: wysiwygOverlay.ph * 0.006
                    color: Qt.rgba(0, 0, 0, 0.7)

                    RowLayout {
                        anchors.centerIn: parent; spacing: wysiwygOverlay.pw * 0.004
                        Label { text: weatherFetcher.conditionIcon; font.pixelSize: wysiwygOverlay.ph * 0.020 }
                        ColumnLayout {
                            spacing: 0
                            Label { text: weatherFetcher.city; font.pixelSize: wysiwygOverlay.ph * 0.011; font.weight: Font.Bold; color: "white" }
                            Label { text: Math.round(weatherFetcher.temperature) + weatherFetcher.unit; font.pixelSize: wysiwygOverlay.ph * 0.009; color: "#CCC" }
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
                    font.pixelSize: 10; font.family: "Menlo"; color: window.darkMode ? "#888" : "#555"
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
        }

        // ── Detection panel ────────────────────────────────
        Rectangle {
            Layout.fillWidth: true; Layout.preferredHeight: 80
            color: window.darkMode ? Qt.rgba(1, 1, 1, 0.02) : Qt.rgba(0, 0, 0, 0.02)
            Rectangle { anchors.top: parent.top; width: parent.width; height: 1; color: window.darkMode ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.06) }

            RowLayout {
                anchors.fill: parent; anchors.leftMargin: 24; anchors.rightMargin: 24; spacing: 16

                // Detection indicator (animated)
                Rectangle {
                    Layout.preferredWidth: 12; Layout.preferredHeight: 12; radius: 6
                    color: liveController.talentDetected ? "#1DB954" : (window.darkMode ? "#333" : "#CCC")
                    Behavior on color { ColorAnimation { duration: 300 } }
                    // Pulse when detected
                    Rectangle {
                        anchors.centerIn: parent; width: 20; height: 20; radius: 10
                        color: "transparent"; border.color: liveController.talentDetected ? "#1DB954" : "transparent"; border.width: 1
                        opacity: 0.4
                        Behavior on border.color { ColorAnimation { duration: 300 } }
                    }
                }

                // Scene mode pill
                Rectangle {
                    visible: liveController.talentDetected
                    Layout.preferredWidth: modeLbl.implicitWidth + 16; Layout.preferredHeight: 24; radius: 12
                    color: liveController.isMultiFace ? Qt.rgba(91/255,79/255,219/255,0.15) : Qt.rgba(29/255,185/255,84/255,0.15)
                    border.color: liveController.isMultiFace ? Qt.rgba(91/255,79/255,219/255,0.3) : Qt.rgba(29/255,185/255,84/255,0.3)
                    Behavior on color { ColorAnimation { duration: 300 } }

                    Label {
                        id: modeLbl; anchors.centerIn: parent
                        text: liveController.isMultiFace ? liveController.faceCount + " " + window.t("faces") : "1 " + window.t("face")
                        color: liveController.isMultiFace ? "#8B80E0" : "#1DB954"
                        font.pixelSize: 10; font.weight: Font.Bold
                    }
                }

                // Talent info
                ColumnLayout {
                    spacing: 3; Layout.fillWidth: true
                    Label {
                        text: liveController.talentDetected ? liveController.detectedName : window.t("waiting")
                        font.pixelSize: 18; font.weight: Font.DemiBold
                        color: liveController.talentDetected ? (window.darkMode ? "white" : "#1A1A1A") : (window.darkMode ? "#444" : "#AAA")
                        Behavior on color { ColorAnimation { duration: 400 } }
                    }
                    Label {
                        text: liveController.talentDetected ? liveController.detectedRole : ""
                        font.pixelSize: 13; color: window.darkMode ? "#777" : "#888"
                    }
                }

                // Confidence pill
                Rectangle {
                    visible: liveController.talentDetected && !liveController.isMultiFace
                    Layout.preferredWidth: 52; Layout.preferredHeight: 28; radius: 8
                    color: liveController.confidence > 0.85 ? Qt.rgba(29/255,185/255,84/255,0.12) : Qt.rgba(255/255,165/255,0/255,0.12)
                    border.color: liveController.confidence > 0.85 ? Qt.rgba(29/255,185/255,84/255,0.25) : Qt.rgba(255/255,165/255,0/255,0.25)

                    Label {
                        anchors.centerIn: parent
                        text: Math.round(liveController.confidence * 100) + "%"
                        color: liveController.confidence > 0.85 ? "#1DB954" : "#FFA500"
                        font.pixelSize: 12; font.weight: Font.Bold
                    }
                }

                // BYPASS button — cuts ALL overlays for ads/pubs
                Rectangle {
                    id: bypassBtn
                    property bool bypassed: false
                    Layout.preferredWidth: 100; Layout.preferredHeight: 38; radius: 8
                    color: bypassed
                           ? (msBypass.containsMouse ? "#DD0000" : "#CC0000")
                           : (msBypass.containsMouse ? (window.darkMode ? Qt.rgba(1,1,1,0.08) : Qt.rgba(0,0,0,0.08)) : (window.darkMode ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.04)))
                    border.color: bypassed ? "#FF3333" : (window.darkMode ? Qt.rgba(1,1,1,0.1) : Qt.rgba(0,0,0,0.1))
                    Behavior on color { ColorAnimation { duration: 150 } }

                    Label {
                        anchors.centerIn: parent
                        text: bypassBtn.bypassed ? "BYPASS" : "BYPASS"
                        color: bypassBtn.bypassed ? "white" : (window.darkMode ? "#888" : "#555")
                        font.pixelSize: 11; font.weight: Font.Bold
                    }
                    MouseArea {
                        id: msBypass; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            bypassBtn.bypassed = !bypassBtn.bypassed
                            liveController.setBypassed(bypassBtn.bypassed)
                            if (bypassBtn.bypassed) {
                                // CUT everything — pub mode
                                liveController.setOverlaysActive(false)
                            } else {
                                // Back to normal — end of pub
                                liveController.setOverlaysActive(true)
                            }
                        }
                    }
                }

                // Overlay toggle
                Rectangle {
                    Layout.preferredWidth: 130; Layout.preferredHeight: 38; radius: 8
                    visible: !bypassBtn.bypassed
                    color: liveController.overlaysActive
                           ? (msOverlay.containsMouse ? Qt.rgba(29/255,185/255,84/255,0.15) : Qt.rgba(29/255,185/255,84/255,0.08))
                           : (msOverlay.containsMouse ? Qt.rgba(204/255,51/255,51/255,0.15) : Qt.rgba(204/255,51/255,51/255,0.08))
                    border.color: liveController.overlaysActive ? Qt.rgba(29/255,185/255,84/255,0.3) : Qt.rgba(204/255,51/255,51/255,0.3)
                    Behavior on color { ColorAnimation { duration: 200 } }

                    Label {
                        anchors.centerIn: parent
                        text: liveController.overlaysActive ? window.t("overlays_on") : window.t("overlays_off")
                        color: liveController.overlaysActive ? "#1DB954" : "#CC3333"
                        font.pixelSize: 11; font.weight: Font.Bold
                    }
                    MouseArea { id: msOverlay; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: liveController.toggleOverlays() }
                }
            }
        }

        // ── Technical bar ──────────────────────────────────
        Rectangle {
            Layout.fillWidth: true; Layout.preferredHeight: 42
            color: window.darkMode ? Qt.rgba(0, 0, 0, 0.3) : Qt.rgba(0, 0, 0, 0.06)
            Rectangle { anchors.top: parent.top; width: parent.width; height: 1; color: window.darkMode ? Qt.rgba(1,1,1,0.03) : Qt.rgba(0,0,0,0.06) }

            RowLayout {
                anchors.fill: parent; anchors.leftMargin: 24; anchors.rightMargin: 24; spacing: 20

                // Output indicators with animated dots
                Repeater {
                    model: [
                        { label: "SDI",  active: liveController.sdiActive },
                        { label: "NDI",  active: liveController.ndiActive },
                        { label: "RTMP", active: liveController.rtmpActive },
                        { label: "SRT",  active: liveController.srtActive }
                    ]
                    delegate: Row {
                        spacing: 5
                        Rectangle {
                            width: 6; height: 6; radius: 3; anchors.verticalCenter: parent.verticalCenter
                            color: modelData.active ? "#1DB954" : (window.darkMode ? "#2A2A2E" : "#CCC")
                            Behavior on color { ColorAnimation { duration: 300 } }
                        }
                        Label { text: modelData.label; font.pixelSize: 11; color: modelData.active ? (window.darkMode ? "#AAA" : "#444") : (window.darkMode ? "#3A3A3E" : "#BBB") }
                    }
                }

                Rectangle { Layout.preferredWidth: 1; Layout.preferredHeight: 16; color: window.darkMode ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.08) }

                Label { text: "FPS " + liveController.fps + "/25"; font.pixelSize: 11; font.family: "Menlo"; color: liveController.fps >= 24 ? (window.darkMode ? "#888" : "#555") : "#FF6B35" }
                Label { text: "LAT " + liveController.latencyMs + "ms"; font.pixelSize: 11; font.family: "Menlo"; color: liveController.latencyMs < 33 ? (window.darkMode ? "#888" : "#555") : "#FF6B35" }
                Label { text: "CPU " + liveController.cpuPercent + "%"; font.pixelSize: 11; font.family: "Menlo"; color: window.darkMode ? "#666" : "#999" }
                Label { text: "GPU " + liveController.gpuPercent + "%"; font.pixelSize: 11; font.family: "Menlo"; color: window.darkMode ? "#666" : "#999" }

                // VU Meter (Feature 4)
                Rectangle { Layout.preferredWidth: 1; Layout.preferredHeight: 16; color: window.darkMode ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.08) }
                Row {
                    spacing: 2
                    Layout.alignment: Qt.AlignVCenter

                    // Left channel
                    Rectangle {
                        width: 4; height: 24; color: "transparent"
                        anchors.verticalCenter: parent.verticalCenter
                        Rectangle {
                            width: parent.width; anchors.bottom: parent.bottom
                            height: parent.height * audioMeter.levelL
                            gradient: Gradient {
                                GradientStop { position: 0.0; color: "#FF3333" }
                                GradientStop { position: 0.3; color: "#FFAA00" }
                                GradientStop { position: 0.6; color: "#1DB954" }
                                GradientStop { position: 1.0; color: "#1DB954" }
                            }
                        }
                        // Peak indicator
                        Rectangle {
                            width: parent.width; height: 1; color: "#FF3333"
                            y: parent.height * (1.0 - audioMeter.peakL)
                            visible: audioMeter.active
                        }
                    }

                    // Right channel
                    Rectangle {
                        width: 4; height: 24; color: "transparent"
                        anchors.verticalCenter: parent.verticalCenter
                        Rectangle {
                            width: parent.width; anchors.bottom: parent.bottom
                            height: parent.height * audioMeter.levelR
                            gradient: Gradient {
                                GradientStop { position: 0.0; color: "#FF3333" }
                                GradientStop { position: 0.3; color: "#FFAA00" }
                                GradientStop { position: 0.6; color: "#1DB954" }
                                GradientStop { position: 1.0; color: "#1DB954" }
                            }
                        }
                        Rectangle {
                            width: parent.width; height: 1; color: "#FF3333"
                            y: parent.height * (1.0 - audioMeter.peakR)
                            visible: audioMeter.active
                        }
                    }

                    Label { text: "VU"; font.pixelSize: 8; color: audioMeter.active ? (window.darkMode ? "#888" : "#555") : (window.darkMode ? "#444" : "#AAA"); anchors.verticalCenter: parent.verticalCenter }
                }

                Item { Layout.fillWidth: true }

                // REC button (Feature 2)
                Rectangle {
                    Layout.preferredWidth: recRow.implicitWidth + 16; Layout.preferredHeight: 28; radius: 6
                    color: liveController.isRecording
                           ? (msRecBtn.containsMouse ? Qt.rgba(204/255,0,0,0.25) : Qt.rgba(204/255,0,0,0.15))
                           : (msRecBtn.containsMouse ? (window.darkMode ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.06)) : "transparent")
                    border.color: liveController.isRecording ? "#CC0000" : "transparent"
                    Behavior on color { ColorAnimation { duration: 150 } }

                    Row {
                        id: recRow; anchors.centerIn: parent; spacing: 6

                        // Red dot (pulses when recording)
                        Rectangle {
                            width: 8; height: 8; radius: 4; anchors.verticalCenter: parent.verticalCenter
                            color: liveController.isRecording ? "#FF0000" : (window.darkMode ? "#666" : "#999")
                            SequentialAnimation on opacity {
                                loops: Animation.Infinite; running: liveController.isRecording
                                NumberAnimation { to: 0.3; duration: 600; easing.type: Easing.InOutSine }
                                NumberAnimation { to: 1.0; duration: 600; easing.type: Easing.InOutSine }
                            }
                        }

                        Label {
                            text: liveController.isRecording ? ("REC " + liveController.recordingDuration) : "REC"
                            color: liveController.isRecording ? "#FF3333" : (window.darkMode ? "#666" : "#999")
                            font.pixelSize: 11; font.weight: liveController.isRecording ? Font.Bold : Font.Normal
                            font.family: liveController.isRecording ? "Menlo" : "Helvetica Neue"
                        }
                    }
                    MouseArea { id: msRecBtn; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: liveController.toggleRecording() }
                }

                Rectangle { Layout.preferredWidth: 1; Layout.preferredHeight: 16; color: window.darkMode ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.08) }

                // Stop button (subtle)
                Rectangle {
                    Layout.preferredWidth: stopLabel.implicitWidth + 20; Layout.preferredHeight: 28; radius: 6
                    color: msStopBtn.containsMouse ? (window.darkMode ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.06)) : "transparent"
                    Behavior on color { ColorAnimation { duration: 150 } }
                    Label { id: stopLabel; anchors.centerIn: parent; text: "\u25A0  " + window.t("stop"); color: window.darkMode ? "#666" : "#999"; font.pixelSize: 11 }
                    MouseArea { id: msStopBtn; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: stopConfirmDialog.open() }
                }
            }
        }
    }
}
