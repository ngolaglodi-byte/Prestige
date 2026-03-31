import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

// ============================================================
// Step 3 — Video Source Configuration (with NDI discovery)
// ============================================================

Item {
    Flickable {
        anchors.fill: parent; anchors.margins: 24
        contentHeight: sourceCol.implicitHeight; clip: true
        flickableDirection: Flickable.VerticalFlick
        ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
    ColumnLayout {
        id: sourceCol; width: parent.width
        spacing: 20

        Label { text: window.t("video_source"); font.pixelSize: 16; color: window.darkMode ? "white" : "#1A1A1A" }
        Label {
            text: window.t("choose_source")
            font.pixelSize: 13; color: window.darkMode ? "#777" : "#888"
        }

        // Source type cards
        GridLayout {
            Layout.fillWidth: true
            columns: 3
            rowSpacing: 12
            columnSpacing: 12

            Repeater {
                model: [
                    { type: "webcam",   label: "Webcam",          desc: "Caméra intégrée ou USB" },
                    { type: "ndi",      label: "NDI",             desc: "Source réseau NewTek NDI" },
                    { type: "decklink", label: "SDI (DeckLink)",  desc: "Carte Blackmagic Design" },
                    { type: "aja",      label: "SDI (AJA)",       desc: "Carte AJA NTV2" },
                    { type: "magewell", label: "USB (Magewell)",  desc: "Capture USB/PCIe" },
                    { type: "test",     label: "Mire de test",    desc: "Signal de test interne" }
                ]
                delegate: Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 72
                    radius: 8
                    color: setupController.inputType === modelData.type ? (window.darkMode ? "#1A1A3A" : "#E8E8F5") : (window.darkMode ? "#111114" : "#FFFFFF")
                    border.color: setupController.inputType === modelData.type ? "#5B4FDB" : (window.darkMode ? "#1E1E22" : "#DDD")
                    border.width: 1

                    RowLayout {
                        anchors.fill: parent; anchors.margins: 14; spacing: 12

                        ColumnLayout {
                            Layout.fillWidth: true; spacing: 2
                            Label { text: modelData.label; font.pixelSize: 14; font.bold: true; color: window.darkMode ? "white" : "#1A1A1A" }
                            Label { text: modelData.desc; font.pixelSize: 12; color: window.darkMode ? "#777" : "#888" }
                        }

                        Rectangle {
                            Layout.preferredWidth: 20; Layout.preferredHeight: 20; radius: 10
                            color: "transparent"
                            border.color: setupController.inputType === modelData.type ? "#5B4FDB" : (window.darkMode ? "#444" : "#AAA")
                            border.width: 2
                            Rectangle {
                                anchors.centerIn: parent
                                width: 10; height: 10; radius: 5
                                color: "#5B4FDB"
                                visible: setupController.inputType === modelData.type
                            }
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: setupController.inputType = modelData.type
                    }
                }
            }
        }

        // ── Detected Devices Section ─────────────────────────
        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#222" : "#CCC"; Layout.leftMargin: 12; Layout.rightMargin: 12 }
        Label { text: "Appareils détectés"; font.pixelSize: 13; font.bold: true; color: window.darkMode ? "white" : "#1A1A1A"; leftPadding: 12 }
        Label { text: hardwareScanner.devices.length + " appareil(s) trouvé(s)"; font.pixelSize: 10; color: window.darkMode ? "#888" : "#666"; leftPadding: 12 }

        ListView {
            Layout.fillWidth: true; Layout.preferredHeight: Math.min(200, hardwareScanner.devices.length * 52); clip: true
            model: hardwareScanner.devices
            spacing: 4
            delegate: Rectangle {
                width: ListView.view.width - 24; height: 48; x: 12; radius: 8
                color: window.darkMode ? Qt.rgba(1,1,1,0.03) : Qt.rgba(0,0,0,0.03)
                border.color: modelData.available ? (window.darkMode ? Qt.rgba(29/255,185/255,84/255,0.2) : Qt.rgba(29/255,185/255,84/255,0.3)) : "transparent"

                RowLayout {
                    anchors.fill: parent; anchors.margins: 10; spacing: 10

                    // Icon based on type
                    Label {
                        text: {
                            var t = modelData.type
                            if (t === "mixer_usb") return "\uD83C\uDFAC"
                            if (t === "capture_card") return "\uD83D\uDCF9"
                            if (t === "decklink") return "\uD83D\uDD33"
                            if (t === "ndi") return "\uD83C\uDF10"
                            if (t === "aja") return "\uD83D\uDD33"
                            if (t === "magewell") return "\uD83D\uDCF9"
                            return "\uD83D\uDCF7"
                        }
                        font.pixelSize: 20
                    }

                    ColumnLayout {
                        Layout.fillWidth: true; spacing: 1
                        Label {
                            text: modelData.name
                            font.pixelSize: 12; font.weight: Font.DemiBold
                            color: window.darkMode ? "white" : "#1A1A1A"
                            elide: Text.ElideRight; Layout.fillWidth: true
                        }
                        Label {
                            text: modelData.driver + (modelData.mixerBrand ? (" \u2014 " + modelData.mixerBrand) : "")
                            font.pixelSize: 9; color: window.darkMode ? "#888" : "#666"
                        }
                    }

                    // Status indicator
                    Rectangle {
                        Layout.preferredWidth: 8; Layout.preferredHeight: 8; radius: 4
                        color: modelData.available ? "#1DB954" : "#CC3333"
                    }
                }

                MouseArea {
                    anchors.fill: parent; cursorShape: modelData.available ? Qt.PointingHandCursor : Qt.ArrowCursor
                    onClicked: {
                        if (modelData.available) {
                            if (modelData.type === "ndi") {
                                setupController.inputType = "ndi"
                                setupController.inputSource = modelData.name
                            } else if (modelData.type === "decklink") {
                                setupController.inputType = "decklink"
                                setupController.inputSource = modelData.name
                            } else if (modelData.type === "aja") {
                                setupController.inputType = "aja"
                            } else if (modelData.type === "magewell") {
                                setupController.inputType = "magewell"
                            } else {
                                setupController.inputType = "webcam"
                                setupController.inputSource = modelData.name
                            }
                        }
                    }
                }
            }
        }

        // Refresh button
        Rectangle {
            Layout.preferredWidth: 140; Layout.preferredHeight: 32; Layout.leftMargin: 12; radius: 6
            color: window.darkMode ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.04)
            Label { anchors.centerIn: parent; text: "\u21BB Scanner le matériel"; font.pixelSize: 10; color: "#5B4FDB" }
            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: hardwareScanner.scan() }
        }

        // ── NDI Source Discovery Panel ──────────────────────
        Rectangle {
            Layout.fillWidth: true
            visible: setupController.inputType === "ndi"
            Layout.preferredHeight: ndiCol.implicitHeight + 24
            color: window.darkMode ? "#111114" : "#FFFFFF"; radius: 8; border.color: window.darkMode ? "#1E1E22" : "#DDD"

            ColumnLayout {
                id: ndiCol
                anchors.left: parent.left; anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.margins: 16
                spacing: 10

                RowLayout {
                    Layout.fillWidth: true
                    Label { text: window.t("ndi_sources"); font.pixelSize: 14; font.bold: true; color: window.darkMode ? "white" : "#1A1A1A" }
                    Item { Layout.fillWidth: true }
                    Button {
                        text: window.t("search")
                        onClicked: {
                            ndiSearching.visible = true
                            ndiCapture.refreshSources()
                            ndiSearchTimer.start()
                        }
                        background: Rectangle { color: window.darkMode ? "#1A2A1A" : "#E8F5E9"; radius: 4; border.color: "#1DB954" }
                        contentItem: Label { text: parent.text; color: "#1DB954"; font.pixelSize: 12; horizontalAlignment: Text.AlignHCenter }
                    }
                }

                // Searching indicator
                Label {
                    id: ndiSearching
                    text: window.t("searching_ndi")
                    font.pixelSize: 12; color: window.darkMode ? "#888" : "#555"
                    visible: false
                }

                Timer {
                    id: ndiSearchTimer
                    interval: 3500
                    onTriggered: ndiSearching.visible = false
                }

                // Source list
                ListView {
                    Layout.fillWidth: true
                    Layout.preferredHeight: Math.min(contentHeight, 200)
                    spacing: 4; clip: true
                    model: ndiCapture.availableSources

                    delegate: Rectangle {
                        width: ListView.view.width
                        height: 40; radius: 6
                        color: setupController.inputSource === modelData ? (window.darkMode ? "#1A1A3A" : "#E8E8F5") : (window.darkMode ? "#0D0D0F" : "#F0F0F4")
                        border.color: setupController.inputSource === modelData ? "#5B4FDB" : (window.darkMode ? "#222" : "#CCC")

                        RowLayout {
                            anchors.fill: parent; anchors.margins: 10; spacing: 8
                            Rectangle { Layout.preferredWidth: 8; Layout.preferredHeight: 8; radius: 4; color: "#1DB954" }
                            Label { text: modelData; font.pixelSize: 13; color: window.darkMode ? "white" : "#1A1A1A"; Layout.fillWidth: true }
                            Button {
                                text: window.t("connect")
                                visible: setupController.inputSource !== modelData
                                flat: true
                                onClicked: {
                                    setupController.inputSource = modelData
                                    ndiCapture.connectSource(modelData)
                                }
                                contentItem: Label { text: parent.text; color: "#5B4FDB"; font.pixelSize: 11 }
                            }
                            Label {
                                text: window.t("connected")
                                visible: setupController.inputSource === modelData
                                color: "#1DB954"; font.pixelSize: 11
                            }
                        }
                    }

                    Label {
                        anchors.centerIn: parent
                        visible: ndiCapture.availableSources.length === 0
                        text: window.t("no_ndi")
                        font.pixelSize: 12; color: window.darkMode ? "#555" : "#999"
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }
        }

        // Resolution + FPS
        Rectangle {
            Layout.fillWidth: true; Layout.preferredHeight: 64
            color: window.darkMode ? "#111114" : "#FFFFFF"; radius: 8; border.color: window.darkMode ? "#1E1E22" : "#DDD"
            RowLayout {
                anchors.fill: parent; anchors.margins: 16; spacing: 24
                Label { text: window.t("resolution") + " :"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 13 }
                ComboBox {
                    model: ["1920x1080 (Full HD)", "3840x2160 (4K UHD)", "1280x720 (HD)"]
                    Layout.preferredWidth: 220
                    background: Rectangle { color: window.darkMode ? "#1A1A1E" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
                    contentItem: Label { text: parent.displayText; color: window.darkMode ? "#CCC" : "#333"; font.pixelSize: 12; leftPadding: 8; verticalAlignment: Text.AlignVCenter }
                }
                Label { text: "FPS :"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 13 }
                ComboBox {
                    model: ["25", "30", "50", "60"]
                    Layout.preferredWidth: 80
                    background: Rectangle { color: window.darkMode ? "#1A1A1E" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
                    contentItem: Label { text: parent.displayText; color: window.darkMode ? "#CCC" : "#333"; font.pixelSize: 12; leftPadding: 8; verticalAlignment: Text.AlignVCenter }
                }
            }
        }

        Item { implicitHeight: 20 }
    } // end sourceCol
    } // end Flickable
}
