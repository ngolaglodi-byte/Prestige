import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

// ============================================================
// Prestige AI — Settings Panel
// Quick-access settings at the bottom of the Control Room
// ============================================================

Rectangle {
    id: root
    color: window.darkMode ? "#111114" : "#FFFFFF"
    radius: 4
    border.color: window.darkMode ? "#2A2A2E" : "#DDD"
    border.width: 1

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        Text {
            text: "PARAMÈTRES"
            color: window.darkMode ? "#888888" : "#555555"
            font.pixelSize: 11
            font.weight: Font.Bold
            font.letterSpacing: 1.5
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: 12
            rowSpacing: 6

            // Input source
            Text { text: "Source"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11 }
            ComboBox {
                id: sourceCombo
                Layout.fillWidth: true
                model: ["Webcam", "NDI", "SDI (DeckLink)", "SDI (AJA)"]
                currentIndex: 0
                Layout.preferredHeight: 28

                background: Rectangle {
                    color: window.darkMode ? "#1E1E22" : "#F0F0F4"
                    radius: 3
                    border.color: window.darkMode ? "#333" : "#CCC"
                }
                contentItem: Text {
                    text: sourceCombo.displayText
                    color: window.darkMode ? "#CCC" : "#333"
                    font.pixelSize: 11
                    leftPadding: 8
                    verticalAlignment: Text.AlignVCenter
                }
            }

            // ZMQ address
            Text { text: "ZMQ"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11 }
            TextField {
                Layout.fillWidth: true
                text: "tcp://127.0.0.1:5555"
                Layout.preferredHeight: 28
                font.pixelSize: 11
                font.family: "Menlo"
                color: window.darkMode ? "#CCC" : "#333"
                background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 3; border.color: window.darkMode ? "#333" : "#CCC" }
            }

            // Accent color
            Text { text: "Couleur"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11 }
            Row {
                spacing: 6
                Repeater {
                    model: ["#E30613", "#1976D2", "#388E3C", "#F57C00", "#7B1FA2"]
                    delegate: Rectangle {
                        width: 22; height: 22; radius: 11
                        color: modelData
                        border.color: index === 0 ? "#FFF" : "transparent"
                        border.width: 2
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                        }
                    }
                }
            }
        }
    }
}
