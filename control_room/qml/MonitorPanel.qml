import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

// ============================================================
// Prestige AI — Monitor Panel
// Live video preview in the Control Room
// ============================================================

Rectangle {
    id: root
    color: window.darkMode ? "#111114" : "#FFFFFF"
    radius: 4
    border.color: window.darkMode ? "#2A2A2E" : "#DDD"
    border.width: 1

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 1
        spacing: 0

        // Title bar
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 32
            color: window.darkMode ? "#1E1E22" : "#E0E0E6"
            radius: 4

            // Only round top corners
            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: parent.radius
                color: parent.color
            }

            Text {
                anchors.centerIn: parent
                text: "PREVIEW MONITOR"
                color: window.darkMode ? "#888888" : "#555555"
                font.pixelSize: 11
                font.weight: Font.Bold
                font.letterSpacing: 1.5
            }
        }

        // Video area
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#000000"

            // Placeholder when no video
            Column {
                anchors.centerIn: parent
                spacing: 8
                visible: !previewMonitor.active

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "⬤"
                    color: window.darkMode ? "#333333" : "#CCCCCC"
                    font.pixelSize: 48
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Aucun signal"
                    color: window.darkMode ? "#555555" : "#999999"
                    font.pixelSize: 14
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Lancez le Vision Engine pour voir le flux"
                    color: window.darkMode ? "#444444" : "#AAAAAA"
                    font.pixelSize: 11
                }
            }

            // Resolution badge
            Rectangle {
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.margins: 8
                width: resLabel.implicitWidth + 12
                height: 20
                radius: 3
                color: Qt.rgba(0, 0, 0, 0.6)
                visible: previewMonitor.active

                Text {
                    id: resLabel
                    anchors.centerIn: parent
                    text: previewMonitor.sourceWidth + "×" + previewMonitor.sourceHeight
                    color: window.darkMode ? "#AAAAAA" : "#555555"
                    font.pixelSize: 10
                    font.family: "Menlo"
                }
            }
        }
    }
}
