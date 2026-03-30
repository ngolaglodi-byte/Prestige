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
        onActivated: toolStack.currentIndex = 3
    }
    Shortcut {
        sequence: "1"
        enabled: mainWindow.isLiveMode
        onActivated: toolStack.currentIndex = 0
    }
    Shortcut {
        sequence: "2"
        enabled: mainWindow.isLiveMode
        onActivated: toolStack.currentIndex = 1
    }
    Shortcut {
        sequence: "3"
        enabled: mainWindow.isLiveMode
        onActivated: toolStack.currentIndex = 2
    }
    Shortcut {
        sequence: "4"
        enabled: mainWindow.isLiveMode
        onActivated: toolStack.currentIndex = 3
    }
    Shortcut {
        sequence: "5"
        enabled: mainWindow.isLiveMode
        onActivated: toolStack.currentIndex = 4
    }
    Shortcut {
        sequence: "6"
        enabled: mainWindow.isLiveMode
        onActivated: toolStack.currentIndex = 5
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

        // ── Video + Tools side by side ────────────────────
        RowLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 0

        // Video output
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

            // Resolution + FPS badge
            Rectangle {
                anchors.top: parent.top; anchors.right: parent.right; anchors.margins: 10
                width: fpsLbl.implicitWidth + 14; height: 22; radius: 6
                color: Qt.rgba(0, 0, 0, 0.5)
                visible: previewMonitor.active
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

        // ── Tools panel (right side) ──────────────────────
        Rectangle {
            Layout.preferredWidth: 280; Layout.fillHeight: true
            color: window.darkMode ? Qt.rgba(1,1,1,0.02) : Qt.rgba(0,0,0,0.02)
            Rectangle { anchors.left: parent.left; width: 1; height: parent.height; color: window.darkMode ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.06) }

            ColumnLayout {
                anchors.fill: parent; spacing: 0

                // Tool tabs
                RowLayout {
                    Layout.fillWidth: true; Layout.preferredHeight: 32; spacing: 0
                    Repeater {
                        model: ["\u26BD", "\u2601", "\u{1F4AC}", "\u{1F4F0}", "\u{1F4CA}", "\u{1F4E8}"]
                        Rectangle {
                            Layout.fillWidth: true; Layout.preferredHeight: 32
                            color: toolStack.currentIndex === index ? (window.darkMode ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.08)) : "transparent"
                            Label { anchors.centerIn: parent; text: modelData; font.pixelSize: 14; color: toolStack.currentIndex === index ? (window.darkMode ? "white" : "#1A1A1A") : (window.darkMode ? "#555" : "#999") }
                            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: toolStack.currentIndex = index }
                        }
                    }
                }
                Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.06) }

                StackLayout {
                    id: toolStack; Layout.fillWidth: true; Layout.fillHeight: true
                    ScoreboardPanel {}
                    WeatherPanel {}
                    SubtitlePanel {}
                    TickerPanel {}
                    StatsPanel {}
                    SocialChatPanel {}
                }

                // ── QR Code quick-action (Feature 7) ──────
                Rectangle {
                    Layout.fillWidth: true; Layout.preferredHeight: 44
                    color: window.darkMode ? Qt.rgba(1,1,1,0.02) : Qt.rgba(0,0,0,0.02)
                    Rectangle { anchors.top: parent.top; width: parent.width; height: 1; color: window.darkMode ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.06) }

                    RowLayout {
                        anchors.fill: parent; anchors.leftMargin: 10; anchors.rightMargin: 10; spacing: 6

                        Label { text: "QR"; font.pixelSize: 11; font.bold: true; color: window.darkMode ? "#666" : "#999" }

                        TextField {
                            id: qrUrlInput
                            Layout.fillWidth: true; Layout.preferredHeight: 28
                            placeholderText: "URL..."
                            font.pixelSize: 11; color: window.darkMode ? "white" : "#1A1A1A"
                            background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
                        }

                        ComboBox {
                            id: qrPosCombo
                            Layout.preferredWidth: 60; Layout.preferredHeight: 28
                            model: ["BR", "BL", "TR", "TL"]
                            font.pixelSize: 10
                            property var posMap: ({"BR": "bottom_right", "BL": "bottom_left", "TR": "top_right", "TL": "top_left"})
                            background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
                            contentItem: Label { text: qrPosCombo.currentText; font.pixelSize: 10; color: window.darkMode ? "white" : "#1A1A1A"; verticalAlignment: Text.AlignVCenter; leftPadding: 6 }
                        }

                        Rectangle {
                            Layout.preferredWidth: 44; Layout.preferredHeight: 28; radius: 6
                            color: liveController.qrCodeVisible
                                   ? (msQr.containsMouse ? "#DD0000" : "#CC0000")
                                   : (msQr.containsMouse ? Qt.rgba(91/255,79/255,219/255,0.25) : Qt.rgba(91/255,79/255,219/255,0.12))
                            Behavior on color { ColorAnimation { duration: 150 } }
                            Label {
                                anchors.centerIn: parent
                                text: liveController.qrCodeVisible ? "OFF" : "ON"
                                color: liveController.qrCodeVisible ? "white" : "#8B80E0"
                                font.pixelSize: 11; font.weight: Font.Bold
                            }
                            MouseArea {
                                id: msQr; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    if (liveController.qrCodeVisible) {
                                        liveController.setQrCode("", false, "bottom_right")
                                    } else {
                                        var url = qrUrlInput.text || ""
                                        var pos = qrPosCombo.posMap[qrPosCombo.currentText] || "bottom_right"
                                        if (url.length > 0) liveController.setQrCode(url, true, pos)
                                    }
                                }
                            }
                        }
                    }
                }

                // ── Countdown quick-action (Feature 8) ──────
                Rectangle {
                    Layout.fillWidth: true; Layout.preferredHeight: 44
                    color: window.darkMode ? Qt.rgba(1,1,1,0.02) : Qt.rgba(0,0,0,0.02)
                    Rectangle { anchors.top: parent.top; width: parent.width; height: 1; color: window.darkMode ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.06) }

                    RowLayout {
                        anchors.fill: parent; anchors.leftMargin: 10; anchors.rightMargin: 10; spacing: 6

                        Label { text: "\u23F1"; font.pixelSize: 14; color: window.darkMode ? "#666" : "#999" }

                        TextField {
                            id: countdownMinInput
                            Layout.preferredWidth: 50; Layout.preferredHeight: 28
                            placeholderText: "min"
                            font.pixelSize: 12; color: window.darkMode ? "white" : "#1A1A1A"
                            horizontalAlignment: Text.AlignHCenter
                            validator: IntValidator { bottom: 0; top: 999 }
                            background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
                        }

                        Label { text: ":"; font.pixelSize: 14; color: window.darkMode ? "#555" : "#999" }

                        TextField {
                            id: countdownSecInput
                            Layout.preferredWidth: 50; Layout.preferredHeight: 28
                            placeholderText: "sec"
                            font.pixelSize: 12; color: window.darkMode ? "white" : "#1A1A1A"
                            horizontalAlignment: Text.AlignHCenter
                            validator: IntValidator { bottom: 0; top: 59 }
                            background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
                        }

                        Rectangle {
                            Layout.preferredWidth: 44; Layout.preferredHeight: 28; radius: 6
                            color: liveController.countdownActive
                                   ? (msCountdown.containsMouse ? "#DD0000" : "#CC0000")
                                   : (msCountdown.containsMouse ? Qt.rgba(29/255,185/255,84/255,0.2) : Qt.rgba(29/255,185/255,84/255,0.12))
                            Behavior on color { ColorAnimation { duration: 150 } }
                            Label {
                                anchors.centerIn: parent
                                text: liveController.countdownActive ? "STOP" : "GO"
                                color: liveController.countdownActive ? "white" : "#1DB954"
                                font.pixelSize: 11; font.weight: Font.Bold
                            }
                            MouseArea {
                                id: msCountdown; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    if (liveController.countdownActive) {
                                        liveController.stopCountdown()
                                    } else {
                                        var mins = parseInt(countdownMinInput.text) || 0
                                        var secs = parseInt(countdownSecInput.text) || 0
                                        var total = mins * 60 + secs
                                        if (total > 0) liveController.startCountdown(total, "")
                                    }
                                }
                            }
                        }

                        // Countdown display
                        Label {
                            visible: liveController.countdownActive
                            text: {
                                var s = liveController.countdownSeconds
                                var mm = Math.floor(s / 60)
                                var ss = s % 60
                                return (mm < 10 ? "0" : "") + mm + ":" + (ss < 10 ? "0" : "") + ss
                            }
                            font.pixelSize: 13; font.family: "Menlo"; font.weight: Font.Bold
                            color: liveController.countdownSeconds <= 10 ? "#FF3333" : "#1DB954"
                        }
                    }
                }
            }
        }

        } // end RowLayout video+tools

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
