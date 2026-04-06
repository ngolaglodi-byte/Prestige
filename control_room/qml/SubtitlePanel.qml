import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: sub

    Flickable {
        anchors.fill: parent; contentHeight: subCol.implicitHeight; clip: true
        flickableDirection: Flickable.VerticalFlick
        ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
    ColumnLayout {
        id: subCol; width: parent.width; spacing: 10
        Item { implicitHeight: 4 }

        // ── Header ──────────────────────────────────
        RowLayout { spacing: 8; Layout.leftMargin: 12
            Rectangle {
                Layout.preferredWidth: 28; Layout.preferredHeight: 28; radius: 7
                color: Qt.rgba(108/255,92/255,231/255,0.12)
                Label { anchors.centerIn: parent; text: "\uD83D\uDCAC"; font.pixelSize: 13 }
            }
            ColumnLayout { spacing: 0
                Label { text: "Subtitles"; font.pixelSize: 15; font.weight: Font.Bold; color: window.darkMode ? "#F0F0F5" : "#0F0F14" }
                Label {
                    text: subtitleController.whisperReady ? "Whisper AI — Ready" : "Whisper AI — Loading..."
                    font.pixelSize: 10; color: subtitleController.whisperReady ? "#00D68F" : (window.darkMode ? "#505060" : "#999")
                }
            }
        }

        // ── ON/OFF ──────────────────────────────────
        Switch { text: "Enable live subtitles"; checked: subtitleController.enabled; onToggled: subtitleController.enabled = checked; leftPadding: 12 }

        // ── Engine ──────────────────────────────────
        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#1A1A22" : "#E0E0E8"; Layout.leftMargin: 12; Layout.rightMargin: 12 }
        Label { text: "Whisper Engine"; font.pixelSize: 12; font.weight: Font.DemiBold; color: window.darkMode ? "#F0F0F5" : "#0F0F14"; leftPadding: 12 }

        RowLayout { spacing: 6; Layout.leftMargin: 12
            Label { text: "Model:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11; Layout.preferredWidth: 55 }
            Repeater {
                model: [
                    { label: "Tiny", val: "tiny", desc: "39MB — Fast" },
                    { label: "Base", val: "base", desc: "74MB — Balanced" },
                    { label: "Small", val: "small", desc: "244MB — Accurate" },
                    { label: "Medium", val: "medium", desc: "769MB — Best" }
                ]
                Rectangle {
                    Layout.preferredWidth: 60; Layout.preferredHeight: 38; radius: 6
                    color: subtitleController.whisperModel === modelData.val
                        ? Qt.rgba(108/255,92/255,231/255,0.18)
                        : (window.darkMode ? Qt.rgba(255,255,255,0.03) : Qt.rgba(0,0,0,0.03))
                    border.color: subtitleController.whisperModel === modelData.val ? Qt.rgba(108/255,92/255,231/255,0.4) : "transparent"
                    ColumnLayout { anchors.centerIn: parent; spacing: 0
                        Label { text: modelData.label; font.pixelSize: 10; font.weight: Font.Bold; color: subtitleController.whisperModel === modelData.val ? "#6C5CE7" : (window.darkMode ? "#888" : "#666"); Layout.alignment: Qt.AlignHCenter }
                        Label { text: modelData.desc.split(" — ")[1]; font.pixelSize: 7; color: window.darkMode ? "#505060" : "#AAA"; Layout.alignment: Qt.AlignHCenter }
                    }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: subtitleController.whisperModel = modelData.val }
                }
            }
        }

        RowLayout { spacing: 6; Layout.leftMargin: 12
            Label { text: "Language:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11; Layout.preferredWidth: 55 }
            ComboBox {
                model: ["Auto-detect", "English", "French", "Spanish", "German", "Arabic", "Chinese", "Portuguese", "Japanese", "Korean"]
                property var codes: ["auto", "en", "fr", "es", "de", "ar", "zh", "pt", "ja", "ko"]
                currentIndex: Math.max(0, codes.indexOf(subtitleController.language))
                onCurrentIndexChanged: subtitleController.language = codes[currentIndex]
                Layout.fillWidth: true
                background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 6; border.color: window.darkMode ? "#333" : "#CCC" }
            }
        }

        // ── Appearance ──────────────────────────────
        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#1A1A22" : "#E0E0E8"; Layout.leftMargin: 12; Layout.rightMargin: 12 }
        Label { text: "Appearance"; font.pixelSize: 12; font.weight: Font.DemiBold; color: window.darkMode ? "#F0F0F5" : "#0F0F14"; leftPadding: 12 }

        // Position
        RowLayout { spacing: 6; Layout.leftMargin: 12
            Label { text: "Position:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11; Layout.preferredWidth: 55 }
            Repeater {
                model: [{ label: "Bottom", val: "bottom" }, { label: "Top", val: "top" }]
                Rectangle {
                    Layout.preferredWidth: 75; Layout.preferredHeight: 28; radius: 6
                    color: subtitleController.position === modelData.val ? Qt.rgba(108/255,92/255,231/255,0.18) : (window.darkMode ? Qt.rgba(255,255,255,0.03) : Qt.rgba(0,0,0,0.03))
                    border.color: subtitleController.position === modelData.val ? Qt.rgba(108/255,92/255,231/255,0.4) : "transparent"
                    Label { anchors.centerIn: parent; text: modelData.label; font.pixelSize: 11; color: subtitleController.position === modelData.val ? "#6C5CE7" : (window.darkMode ? "#888" : "#666") }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: subtitleController.position = modelData.val }
                }
            }
        }

        // Font size
        RowLayout { spacing: 4; Layout.leftMargin: 12
            Label { text: "Size:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11; Layout.preferredWidth: 55 }
            Slider { from: 14; to: 48; stepSize: 1; value: subtitleController.fontSize; Layout.fillWidth: true; onMoved: subtitleController.fontSize = value }
            Label { text: subtitleController.fontSize + "pt"; color: window.darkMode ? "#888" : "#555"; font.pixelSize: 10; Layout.preferredWidth: 30 }
        }

        // Background opacity
        RowLayout { spacing: 4; Layout.leftMargin: 12
            Label { text: "BG opacity:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11; Layout.preferredWidth: 55 }
            Slider { from: 0.2; to: 1.0; value: subtitleController.bgOpacity; Layout.fillWidth: true; onMoved: subtitleController.bgOpacity = value }
            Label { text: Math.round(subtitleController.bgOpacity * 100) + "%"; color: window.darkMode ? "#888" : "#555"; font.pixelSize: 10; Layout.preferredWidth: 30 }
        }

        // Text color
        RowLayout { spacing: 6; Layout.leftMargin: 12; Layout.rightMargin: 12
            Label { text: "Color:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11; Layout.preferredWidth: 55 }
            Rectangle { Layout.preferredWidth: 24; Layout.preferredHeight: 24; radius: 6; color: subtitleController.textColor; border.color: window.darkMode ? "#444" : "#BBB"; border.width: 1 }
            TextField {
                Layout.fillWidth: true; text: subtitleController.textColor; font.pixelSize: 11; font.family: "SF Mono, Menlo, monospace"; color: window.darkMode ? "white" : "#1A1A1A"
                onTextEdited: { if (text.match(/^#[0-9A-Fa-f]{6}$/)) subtitleController.textColor = text }
                background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
            }
            ColorPickerButton { currentColor: subtitleController.textColor; onColorSelected: function(c) { subtitleController.textColor = c } }
        }

        // ── Live Preview ────────────────────────────
        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#1A1A22" : "#E0E0E8"; Layout.leftMargin: 12; Layout.rightMargin: 12 }
        Label { text: "Preview"; font.pixelSize: 12; font.weight: Font.DemiBold; color: window.darkMode ? "#F0F0F5" : "#0F0F14"; leftPadding: 12 }

        Rectangle {
            Layout.fillWidth: true; Layout.leftMargin: 12; Layout.rightMargin: 12
            Layout.preferredHeight: 48; radius: 8
            color: Qt.rgba(0, 0, 0, subtitleController.bgOpacity)
            Label {
                anchors.centerIn: parent; width: parent.width - 20
                text: subtitleController.currentText ? subtitleController.currentText : "Subtitles will appear here in real-time..."
                color: subtitleController.currentText ? subtitleController.textColor : "#555"
                font.pixelSize: subtitleController.fontSize * 0.65
                font.family: "SF Pro Display, Inter, Helvetica Neue"
                horizontalAlignment: Text.AlignHCenter; wrapMode: Text.WordWrap
                elide: Text.ElideRight; maximumLineCount: 2
            }
        }

        // ── Status ──────────────────────────────────
        RowLayout { spacing: 6; Layout.leftMargin: 12
            Rectangle { Layout.preferredWidth: 6; Layout.preferredHeight: 6; radius: 3; color: subtitleController.confidence > 0.8 ? "#00D68F" : (subtitleController.confidence > 0.5 ? "#FFAA00" : "#FF3D71") }
            Label { text: "Confidence: " + Math.round(subtitleController.confidence * 100) + "%"; font.pixelSize: 10; color: window.darkMode ? "#666" : "#999" }
            Item { Layout.fillWidth: true }
            Label { text: "Engine: Whisper AI (offline)"; font.pixelSize: 10; color: window.darkMode ? "#505060" : "#AAA" }
        }

        Item { implicitHeight: 12 }
    }
    }
}
