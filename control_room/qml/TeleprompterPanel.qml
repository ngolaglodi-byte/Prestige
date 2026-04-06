import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: tp
    property string scriptText: "Welcome to the evening news.\n\nTonight's headlines..."
    property double scrollSpeed: 2.0
    property bool isScrolling: false
    property bool mirrorMode: false
    property int fontSize: 28
    property string fontColor: "#FFFFFF"
    property string bgColor: "#000000"

    function importScript(path) {
        var xhr = new XMLHttpRequest()
        xhr.open("GET", "file:///" + path)
        xhr.onreadystatechange = function() {
            if (xhr.readyState === XMLHttpRequest.DONE && xhr.responseText) tp.scriptText = xhr.responseText
        }
        xhr.send()
    }

    ColumnLayout {
        anchors.fill: parent; spacing: 0

        // ── Header ──────────────────────────────────
        Rectangle {
            Layout.fillWidth: true; Layout.preferredHeight: 50
            color: "transparent"
            RowLayout { anchors.fill: parent; anchors.leftMargin: 12; anchors.rightMargin: 12; spacing: 8
                Rectangle {
                    Layout.preferredWidth: 28; Layout.preferredHeight: 28; radius: 7
                    color: Qt.rgba(108/255,92/255,231/255,0.12)
                    Label { anchors.centerIn: parent; text: "\uD83D\uDCDD"; font.pixelSize: 13 }
                }
                ColumnLayout { spacing: 0; Layout.fillWidth: true
                    Label { text: "Teleprompter"; font.pixelSize: 15; font.weight: Font.Bold; color: window.darkMode ? "#F0F0F5" : "#0F0F14" }
                    Label { text: tp.isScrolling ? "SCROLLING" : "Ready"; font.pixelSize: 10; color: tp.isScrolling ? "#00D68F" : (window.darkMode ? "#505060" : "#999") }
                }
                // Scroll button
                Rectangle {
                    Layout.preferredWidth: 80; Layout.preferredHeight: 32; radius: 8
                    color: tp.isScrolling ? "#FF3D71" : "#00D68F"
                    Label { anchors.centerIn: parent; text: tp.isScrolling ? "STOP" : "SCROLL"; color: "white"; font.pixelSize: 11; font.weight: Font.Bold; font.letterSpacing: 0.5 }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: tp.isScrolling = !tp.isScrolling }
                }
            }
        }

        // ── Script Area ─────────────────────────────
        TextArea {
            Layout.fillWidth: true; Layout.fillHeight: true; Layout.leftMargin: 8; Layout.rightMargin: 8
            text: tp.scriptText; onTextChanged: tp.scriptText = text
            font.pixelSize: tp.fontSize * 0.55; font.family: "SF Pro Display, Inter, Helvetica Neue"
            color: tp.fontColor; wrapMode: TextArea.Wrap
            transform: Scale { xScale: tp.mirrorMode ? -1 : 1; origin.x: parent.width / 2 }
            background: Rectangle { color: tp.bgColor; radius: 6; border.color: window.darkMode ? "#222" : "#CCC" }
        }

        // ── Controls ────────────────────────────────
        Rectangle {
            Layout.fillWidth: true; Layout.preferredHeight: controlCol.implicitHeight + 16
            color: "transparent"
            ColumnLayout {
                id: controlCol; anchors.fill: parent; anchors.margins: 8; spacing: 8

                // Speed + Size
                RowLayout { spacing: 12
                    RowLayout { spacing: 4; Layout.fillWidth: true
                        Label { text: "Speed:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 10 }
                        Slider { from: 0.5; to: 5; value: tp.scrollSpeed; Layout.fillWidth: true; onMoved: tp.scrollSpeed = value }
                        Label { text: tp.scrollSpeed.toFixed(1) + "x"; color: window.darkMode ? "#888" : "#555"; font.pixelSize: 10; Layout.preferredWidth: 25 }
                    }
                    RowLayout { spacing: 4; Layout.fillWidth: true
                        Label { text: "Size:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 10 }
                        Slider { from: 18; to: 48; stepSize: 1; value: tp.fontSize; Layout.fillWidth: true; onMoved: tp.fontSize = value }
                        Label { text: tp.fontSize + "pt"; color: window.darkMode ? "#888" : "#555"; font.pixelSize: 10; Layout.preferredWidth: 25 }
                    }
                }

                // Colors + Options
                RowLayout { spacing: 8
                    // Text color
                    Label { text: "Text:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 10 }
                    Repeater {
                        model: ["#FFFFFF", "#FFFF00", "#00FF00", "#00CCFF"]
                        Rectangle { width: 16; height: 16; radius: 4; color: modelData; border.color: modelData === tp.fontColor ? "white" : "transparent"; border.width: 1.5
                            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: tp.fontColor = modelData } }
                    }

                    Rectangle { Layout.preferredWidth: 1; Layout.preferredHeight: 16; color: window.darkMode ? "#222" : "#DDD" }

                    // Background
                    Label { text: "BG:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 10 }
                    Repeater {
                        model: ["#000000", "#0A0A14", "#1A1A2E", "#001122"]
                        Rectangle { width: 16; height: 16; radius: 4; color: modelData; border.color: modelData === tp.bgColor ? "#6C5CE7" : (window.darkMode ? "#444" : "#BBB"); border.width: 1.5
                            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: tp.bgColor = modelData } }
                    }

                    Rectangle { Layout.preferredWidth: 1; Layout.preferredHeight: 16; color: window.darkMode ? "#222" : "#DDD" }

                    // Mirror
                    Switch { text: "Mirror"; checked: tp.mirrorMode; onToggled: tp.mirrorMode = checked; font.pixelSize: 10 }

                    Item { Layout.fillWidth: true }

                    // Import
                    Rectangle {
                        Layout.preferredWidth: importLbl.implicitWidth + 14; Layout.preferredHeight: 26; radius: 6
                        color: window.darkMode ? Qt.rgba(255,255,255,0.04) : Qt.rgba(0,0,0,0.05)
                        Label { id: importLbl; anchors.centerIn: parent; text: "Import .txt"; color: window.darkMode ? "#888" : "#555"; font.pixelSize: 9 }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: importRow.visible = !importRow.visible }
                    }
                }

                // Import path (hidden by default)
                RowLayout { id: importRow; visible: false; spacing: 4
                    TextField {
                        id: scriptPathField; Layout.fillWidth: true; placeholderText: "path/to/script.txt"
                        font.pixelSize: 10; color: window.darkMode ? "#CCC" : "#333"
                        background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
                    }
                    Rectangle {
                        Layout.preferredWidth: 40; Layout.preferredHeight: 26; radius: 6; color: "#6C5CE7"
                        Label { anchors.centerIn: parent; text: "Load"; color: "white"; font.pixelSize: 10; font.bold: true }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { importScript(scriptPathField.text); importRow.visible = false } }
                    }
                }
            }
        }
    }
}
