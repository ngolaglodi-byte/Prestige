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
    Shortcut {
        sequence: "N"
        enabled: mainWindow.isLiveMode
        onActivated: graphicsQueue.takeNext()
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
                        color: "transparent"; border.color: "#5B4FDB"; border.width: 2
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
                        color: "transparent"; border.color: setupController.channelNameBorderColor || "#5B4FDB"; border.width: 2
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
                    visible: setupController.tickerVisible && rssFetcher.headlines !== "" && !liveController.isBypassed
                    color: setupController.tickerBgColor
                    clip: true

                    Label {
                        id: tickerText
                        text: rssFetcher.headlines || ""
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
                    y: (previewLogoContainer.visible ? previewLogoContainer.height + wysiwygOverlay.ph * 0.022 : wysiwygOverlay.ph * 0.015) + setupController.countdownOffsetY * (wysiwygOverlay.ph / 1080.0)

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
                    visible: setupController.clockVisible && !liveController.isBypassed
                    x: parent.width - width - wysiwygOverlay.pw * 0.008 + setupController.clockOffsetX * (wysiwygOverlay.pw / 1920.0)
                    y: wysiwygOverlay.ph * 0.015 + setupController.clockOffsetY * (wysiwygOverlay.ph / 1080.0)

                    width: wysiwygOverlay.pw * 0.063
                    height: wysiwygOverlay.ph * 0.030
                    radius: wysiwygOverlay.ph * 0.004
                    color: Qt.rgba(0, 0, 0, 0.6)

                    Label {
                        id: clockLbl; anchors.centerIn: parent
                        text: Qt.formatTime(new Date(), setupController.clockFormat)
                        font.pixelSize: wysiwygOverlay.ph * 0.013; font.weight: Font.Bold; font.family: "Menlo"; color: "white"
                    }
                    Timer { interval: 1000; running: true; repeat: true; onTriggered: clockLbl.text = Qt.formatTime(new Date(), setupController.clockFormat) }
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
                    visible: setupController.scoreboardVisible && !liveController.isBypassed

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

                    width: wysiwygOverlay.pw * 0.167; height: wysiwygOverlay.ph * 0.074
                    radius: wysiwygOverlay.ph * 0.006
                    color: Qt.rgba(0, 0, 0, 0.75)
                    border.color: Qt.rgba(1,1,1,0.1)

                    RowLayout {
                        anchors.centerIn: parent; spacing: wysiwygOverlay.pw * 0.005
                        ColumnLayout {
                            spacing: 1
                            Label { text: setupController.scoreboardTeamA; font.pixelSize: wysiwygOverlay.ph * 0.009; color: setupController.scoreboardColorA; font.weight: Font.Bold; Layout.alignment: Qt.AlignHCenter }
                            Label { text: setupController.scoreboardScoreA.toString(); font.pixelSize: wysiwygOverlay.ph * 0.022; font.weight: Font.Bold; color: "white"; Layout.alignment: Qt.AlignHCenter }
                        }
                        Label { text: "-"; font.pixelSize: wysiwygOverlay.ph * 0.015; color: "#555" }
                        ColumnLayout {
                            spacing: 1
                            Label { text: setupController.scoreboardTeamB; font.pixelSize: wysiwygOverlay.ph * 0.009; color: setupController.scoreboardColorB; font.weight: Font.Bold; Layout.alignment: Qt.AlignHCenter }
                            Label { text: setupController.scoreboardScoreB.toString(); font.pixelSize: wysiwygOverlay.ph * 0.022; font.weight: Font.Bold; color: "white"; Layout.alignment: Qt.AlignHCenter }
                        }
                    }
                }

                // ── Weather overlay ──────────────────────────
                Rectangle {
                    id: wysiwygWeather
                    visible: setupController.weatherVisible && weatherFetcher.city !== "" && !liveController.isBypassed

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

                    Rectangle { width: wysiwygOverlay.pw * 0.002; height: parent.height; color: setupController.accentColor.toString() !== "#000000" ? setupController.accentColor : "#5B4FDB" }

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
                    color: dragNameMa.containsMouse ? "#5B4FDB" : Qt.rgba(1,1,1,0.3)
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
                    color: dragLogoMa.containsMouse ? "#5B4FDB" : Qt.rgba(1,1,1,0.3)
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
                    color: dragTitleMa.containsMouse ? "#5B4FDB" : Qt.rgba(1,1,1,0.3)
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
                    color: dragScoreMa.containsMouse ? "#5B4FDB" : Qt.rgba(1,1,1,0.3)
                    MouseArea {
                        id: dragScoreMa; anchors.fill: parent; hoverEnabled: true
                        drag.target: wysiwygScoreboard
                        cursorShape: pressed ? Qt.ClosedHandCursor : Qt.SizeAllCursor
                        onReleased: {
                            setupController.scoreboardOffsetX = Math.round(wysiwygScoreboard.x * 1920.0 / wysiwygOverlay.pw)
                            setupController.scoreboardOffsetY = Math.round(wysiwygScoreboard.y * 1080.0 / wysiwygOverlay.ph)
                        }
                    }
                }

                // Drag handle: Weather
                Rectangle {
                    visible: wysiwygWeather.visible
                    x: wysiwygWeather.x + wysiwygWeather.width - 8
                    y: wysiwygWeather.y - 4
                    width: 12; height: 12; radius: 2; z: 20
                    color: dragWeatherMa.containsMouse ? "#5B4FDB" : Qt.rgba(1,1,1,0.3)
                    MouseArea {
                        id: dragWeatherMa; anchors.fill: parent; hoverEnabled: true
                        drag.target: wysiwygWeather
                        cursorShape: pressed ? Qt.ClosedHandCursor : Qt.SizeAllCursor
                        onReleased: {
                            setupController.weatherOffsetX = Math.round((wysiwygWeather.x - (wysiwygOverlay.pw - wysiwygWeather.width - wysiwygOverlay.pw * 0.008)) * 1920.0 / wysiwygOverlay.pw)
                            setupController.weatherOffsetY = Math.round((wysiwygWeather.y - (wysiwygOverlay.ph - wysiwygWeather.height - wysiwygOverlay.ph * 0.074)) * 1080.0 / wysiwygOverlay.ph)
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
        }

        // ── Detection panel ────────────────────────────────
        Rectangle {
            Layout.fillWidth: true; Layout.preferredHeight: 90
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
                    Layout.preferredWidth: modeLbl.implicitWidth + 16; Layout.preferredHeight: 28; radius: 14
                    color: liveController.isMultiFace ? Qt.rgba(91/255,79/255,219/255,0.15) : Qt.rgba(29/255,185/255,84/255,0.15)
                    border.color: liveController.isMultiFace ? Qt.rgba(91/255,79/255,219/255,0.3) : Qt.rgba(29/255,185/255,84/255,0.3)
                    Behavior on color { ColorAnimation { duration: 300 } }

                    Label {
                        id: modeLbl; anchors.centerIn: parent
                        text: liveController.isMultiFace ? liveController.faceCount + " " + window.t("faces") : "1 " + window.t("face")
                        color: liveController.isMultiFace ? "#8B80E0" : "#1DB954"
                        font.pixelSize: 11; font.weight: Font.Bold
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

        // ── Graphics Queue Bar ───────────────────────────────
        Rectangle {
            id: queueBar
            Layout.fillWidth: true; Layout.preferredHeight: queueBar.queueExpanded ? 160 : 36
            color: window.darkMode ? "#0A0A0E" : "#E8E8EE"
            property bool queueExpanded: false

            Behavior on Layout.preferredHeight { NumberAnimation { duration: 200 } }

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
                            font.pixelSize: 11; color: "#5B4FDB"; elide: Text.ElideRight; Layout.maximumWidth: 200
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
                                font.pixelSize: 10; font.weight: Font.Bold; color: "#5B4FDB"
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
                                color: graphicsQueue.currentIndex === index ? "#CC0000" : (index < graphicsQueue.currentIndex ? "#1DB954" : (window.darkMode ? "#666" : "#999"))
                            }
                        }

                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: graphicsQueue.takeItem(index) }
                    }
                }
            }
        }

        // ── Broadcast Status Bar ──────────────────────────
        Rectangle {
            Layout.fillWidth: true; Layout.preferredHeight: 38
            color: window.darkMode ? "#08080C" : "#E0E0E6"
            Rectangle { anchors.top: parent.top; width: parent.width; height: 1; color: window.darkMode ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.06) }

            RowLayout {
                anchors.fill: parent; anchors.leftMargin: 16; anchors.rightMargin: 16; spacing: 16

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

                Rectangle { Layout.preferredWidth: 1; Layout.preferredHeight: 18; color: window.darkMode ? "#333" : "#CCC" }

                // FPS with color coding
                RowLayout { spacing: 4
                    Rectangle { Layout.preferredWidth: 6; Layout.preferredHeight: 6; radius: 3; color: liveController.fps >= 24 ? "#1DB954" : (liveController.fps >= 15 ? "#FFB800" : "#CC0000") }
                    Label { text: "FPS " + liveController.fps; font.pixelSize: 11; font.family: "Menlo"; color: window.darkMode ? "#AAA" : "#555" }
                }

                // Latency with color coding
                RowLayout { spacing: 4
                    Rectangle { Layout.preferredWidth: 6; Layout.preferredHeight: 6; radius: 3; color: liveController.latencyMs < 50 ? "#1DB954" : (liveController.latencyMs < 100 ? "#FFB800" : "#CC0000") }
                    Label { text: liveController.latencyMs + "ms"; font.pixelSize: 11; font.family: "Menlo"; color: window.darkMode ? "#AAA" : "#555" }
                }

                Rectangle { Layout.preferredWidth: 1; Layout.preferredHeight: 18; color: window.darkMode ? "#333" : "#CCC" }

                // AI status
                RowLayout { spacing: 4
                    Rectangle { Layout.preferredWidth: 6; Layout.preferredHeight: 6; radius: 3; color: "#1DB954" }
                    Label { text: "IA"; font.pixelSize: 11; font.weight: Font.Bold; color: window.darkMode ? "#AAA" : "#555" }
                }

                // Source
                Label { text: setupController.inputType || "\u2014"; font.pixelSize: 11; color: window.darkMode ? "#666" : "#999" }

                Rectangle { Layout.preferredWidth: 1; Layout.preferredHeight: 18; color: window.darkMode ? "#333" : "#CCC" }

                // VU meter bars (simplified)
                RowLayout { spacing: 2
                    Label { text: "VU"; font.pixelSize: 11; color: window.darkMode ? "#666" : "#999" }
                    Rectangle { Layout.preferredWidth: 4; Layout.preferredHeight: audioMeter.levelL * 18; Layout.maximumHeight: 18; radius: 1; color: audioMeter.levelL > 0.8 ? "#CC0000" : "#1DB954"; Layout.alignment: Qt.AlignBottom }
                    Rectangle { Layout.preferredWidth: 4; Layout.preferredHeight: audioMeter.levelR * 18; Layout.maximumHeight: 18; radius: 1; color: audioMeter.levelR > 0.8 ? "#CC0000" : "#1DB954"; Layout.alignment: Qt.AlignBottom }
                }

                Item { Layout.fillWidth: true }

                // REC
                Rectangle {
                    Layout.preferredWidth: recRow.implicitWidth + 16; Layout.preferredHeight: 28; radius: 6
                    color: liveController.isRecording
                           ? (msRecBtn.containsMouse ? Qt.rgba(204/255,0,0,0.25) : Qt.rgba(204/255,0,0,0.15))
                           : (msRecBtn.containsMouse ? (window.darkMode ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.06)) : "transparent")
                    border.color: liveController.isRecording ? "#CC0000" : "transparent"
                    Behavior on color { ColorAnimation { duration: 150 } }

                    RowLayout {
                        id: recRow; anchors.centerIn: parent; spacing: 6
                        Rectangle {
                            visible: liveController.isRecording; Layout.preferredWidth: 8; Layout.preferredHeight: 8; radius: 4; color: "#CC0000"
                            SequentialAnimation on opacity {
                                loops: Animation.Infinite; running: liveController.isRecording
                                NumberAnimation { to: 0.3; duration: 500 }
                                NumberAnimation { to: 1.0; duration: 500 }
                            }
                        }
                        Rectangle {
                            visible: !liveController.isRecording; Layout.preferredWidth: 8; Layout.preferredHeight: 8; radius: 4; color: window.darkMode ? "#555" : "#AAA"
                        }
                        Label { text: liveController.isRecording ? ("REC " + liveController.recordingDuration) : "REC"; font.pixelSize: 11; font.family: "Menlo"; color: liveController.isRecording ? "#CC0000" : (window.darkMode ? "#555" : "#AAA") }
                    }
                    MouseArea { id: msRecBtn; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: liveController.toggleRecording() }
                }

                Rectangle { Layout.preferredWidth: 1; Layout.preferredHeight: 18; color: window.darkMode ? "#333" : "#CCC" }

                // Elapsed
                Label { text: liveController.formattedDuration(liveView.elapsedSeconds); font.pixelSize: 11; font.family: "Menlo"; color: window.darkMode ? "#888" : "#666" }

                Rectangle { Layout.preferredWidth: 1; Layout.preferredHeight: 18; color: window.darkMode ? "#333" : "#CCC" }

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
