import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    ColumnLayout {
        anchors.fill: parent; spacing: 0

        // ── Header ──────────────────────────────────
        Rectangle {
            Layout.fillWidth: true; Layout.preferredHeight: 50; color: "transparent"
            RowLayout { anchors.fill: parent; anchors.leftMargin: 12; anchors.rightMargin: 12; spacing: 8
                Rectangle {
                    Layout.preferredWidth: 28; Layout.preferredHeight: 28; radius: 7
                    color: Qt.rgba(108/255,92/255,231/255,0.12)
                    Label { anchors.centerIn: parent; text: "\uD83C\uDFAC"; font.pixelSize: 13 }
                }
                ColumnLayout { spacing: 0; Layout.fillWidth: true
                    Label { text: "Macros & Sequences"; font.pixelSize: 15; font.weight: Font.Bold; color: window.darkMode ? "#F0F0F5" : "#0F0F14" }
                    Label { text: macroEngine.executing ? "Running..." : macroEngine.macros.length + " macros available"; font.pixelSize: 10; color: macroEngine.executing ? "#00D68F" : (window.darkMode ? "#505060" : "#999") }
                }

                // Stop button (visible when running)
                Rectangle {
                    Layout.preferredWidth: 60; Layout.preferredHeight: 30; radius: 8
                    color: "#FF3D71"; visible: macroEngine.executing
                    Label { anchors.centerIn: parent; text: "STOP"; color: "white"; font.pixelSize: 10; font.weight: Font.Bold; font.letterSpacing: 0.5 }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: macroEngine.stopMacro() }
                }
            }
        }

        // ── Executing indicator ─────────────────────
        Rectangle {
            Layout.fillWidth: true; Layout.preferredHeight: 36; Layout.leftMargin: 12; Layout.rightMargin: 12
            radius: 8; visible: macroEngine.executing
            color: Qt.rgba(108/255,92/255,231/255,0.10)
            border.color: Qt.rgba(108/255,92/255,231/255,0.25)
            RowLayout { anchors.centerIn: parent; spacing: 6
                Rectangle { Layout.preferredWidth: 8; Layout.preferredHeight: 8; radius: 4; color: "#6C5CE7"
                    SequentialAnimation on opacity { loops: Animation.Infinite; NumberAnimation { to: 0.3; duration: 500 }
                        NumberAnimation { to: 1.0; duration: 500 } }
                }
                Label { text: "Macro executing — step " + (macroEngine.currentMacroIndex + 1); font.pixelSize: 11; color: "#8B80E0" }
            }
        }

        Item { implicitHeight: 8 }

        // ── Macro List ──────────────────────────────
        ListView {
            Layout.fillWidth: true; Layout.fillHeight: true; Layout.leftMargin: 8; Layout.rightMargin: 8
            clip: true; spacing: 4
            model: macroEngine.macros

            delegate: Rectangle {
                width: ListView.view.width; height: 52; radius: 8
                color: macroEngine.executing && macroEngine.currentMacroIndex === index
                       ? Qt.rgba(108/255,92/255,231/255,0.12)
                       : (macroHover.containsMouse ? (window.darkMode ? Qt.rgba(255,255,255,0.04) : Qt.rgba(0,0,0,0.04)) : (window.darkMode ? Qt.rgba(255,255,255,0.02) : Qt.rgba(0,0,0,0.02)))
                border.color: macroEngine.executing && macroEngine.currentMacroIndex === index
                              ? Qt.rgba(108/255,92/255,231/255,0.3) : "transparent"
                Behavior on color { ColorAnimation { duration: 150 } }

                RowLayout {
                    anchors.fill: parent; anchors.leftMargin: 12; anchors.rightMargin: 12; spacing: 10

                    // Icon
                    Rectangle {
                        Layout.preferredWidth: 32; Layout.preferredHeight: 32; radius: 8
                        color: window.darkMode ? Qt.rgba(255,255,255,0.04) : Qt.rgba(0,0,0,0.04)
                        Label { anchors.centerIn: parent; text: modelData.icon || "\u25B6"; font.pixelSize: 16 }
                    }

                    ColumnLayout { Layout.fillWidth: true; spacing: 2
                        Label { text: modelData.name; font.pixelSize: 13; font.weight: Font.Bold; color: window.darkMode ? "#F0F0F5" : "#0F0F14" }
                        Label { text: modelData.actionCount + " actions"; font.pixelSize: 10; color: window.darkMode ? "#505060" : "#999" }
                    }

                    Rectangle {
                        Layout.preferredWidth: 56; Layout.preferredHeight: 30; radius: 8
                        color: goMouse.containsMouse ? "#7B6DEF" : "#6C5CE7"
                        Behavior on color { ColorAnimation { duration: 100 } }
                        Label { anchors.centerIn: parent; text: "\u25B6 GO"; color: "white"; font.pixelSize: 10; font.weight: Font.Bold; font.letterSpacing: 0.5 }
                        MouseArea { id: goMouse; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: macroEngine.executeMacro(index) }
                    }
                }

                MouseArea { id: macroHover; anchors.fill: parent; hoverEnabled: true; acceptedButtons: Qt.NoButton }
            }
        }
    }
}
