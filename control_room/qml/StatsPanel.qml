import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

// Analytics Panel -- Real-time broadcast analytics
Item {
    id: statsPanel

    function formatTime(sec) {
        var h = Math.floor(sec / 3600)
        var m = Math.floor((sec % 3600) / 60)
        var s = sec % 60
        return (h < 10 ? "0" : "") + h + ":" + (m < 10 ? "0" : "") + m + ":" + (s < 10 ? "0" : "") + s
    }

    ColumnLayout {
        anchors.fill: parent; anchors.margins: 12; spacing: 8

        // Header
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Analytics"
                font.pixelSize: 15; font.bold: true
                color: window.darkMode ? "white" : "#1A1A1A"
            }
            Item { Layout.fillWidth: true }
            Rectangle {
                Layout.preferredWidth: elapsedLabel.implicitWidth + 16; Layout.preferredHeight: 24; radius: 12
                color: analyticsEngine.elapsedSeconds > 0 ? Qt.rgba(0.11, 0.73, 0.33, 0.15) : Qt.rgba(1,1,1,0.04)
                Label {
                    id: elapsedLabel
                    anchors.centerIn: parent
                    text: formatTime(analyticsEngine.elapsedSeconds)
                    font.pixelSize: 11; font.family: "Menlo"; font.bold: true
                    color: analyticsEngine.elapsedSeconds > 0 ? "#1DB954" : "#666"
                }
            }
        }

        // General stats
        Grid {
            Layout.fillWidth: true; columns: 2; columnSpacing: 8; rowSpacing: 6
            Repeater {
                model: [
                    { label: "Detections", value: analyticsEngine.totalDetections.toString() },
                    { label: "Overlays", value: analyticsEngine.totalOverlays.toString() },
                    { label: "FPS moyen", value: analyticsEngine.avgFps.toFixed(1) },
                    { label: "Latence", value: analyticsEngine.avgLatency.toFixed(0) + "ms" }
                ]
                delegate: Rectangle {
                    width: (statsPanel.width - 32) / 2; height: 40; radius: 6
                    color: window.darkMode ? Qt.rgba(1,1,1,0.03) : Qt.rgba(0,0,0,0.03)
                    RowLayout {
                        anchors.fill: parent; anchors.margins: 8
                        Label { text: modelData.label; font.pixelSize: 10; color: window.darkMode ? "#777" : "#888"; Layout.fillWidth: true }
                        Label { text: modelData.value; font.pixelSize: 13; font.bold: true; font.family: "Menlo"; color: window.darkMode ? "white" : "#1A1A1A" }
                    }
                }
            }
        }

        // Current talent
        Rectangle {
            Layout.fillWidth: true; height: 32; radius: 6
            color: analyticsEngine.currentTalent !== "" ? Qt.rgba(0.36, 0.31, 0.86, 0.1) : Qt.rgba(1,1,1,0.02)
            RowLayout {
                anchors.fill: parent; anchors.margins: 8
                Label { text: "En cours:"; font.pixelSize: 10; color: "#888" }
                Label { text: analyticsEngine.currentTalent || "---"; font.pixelSize: 12; font.bold: true; color: "#8B80E0"; Layout.fillWidth: true }
            }
        }

        // Talent stats list with bar chart
        Label { text: "Temps d'antenne"; font.pixelSize: 12; font.bold: true; color: window.darkMode ? "#CCC" : "#333"; topPadding: 4 }

        Flickable {
            Layout.fillWidth: true; Layout.fillHeight: true
            contentHeight: talentCol.implicitHeight; clip: true
            flickableDirection: Flickable.VerticalFlick

            ColumnLayout {
                id: talentCol; width: parent.width; spacing: 4

                Repeater {
                    model: analyticsEngine.talentStats

                    delegate: Rectangle {
                        Layout.fillWidth: true; height: 56; radius: 6
                        color: window.darkMode ? Qt.rgba(1,1,1,0.03) : Qt.rgba(0,0,0,0.03)

                        ColumnLayout {
                            anchors.fill: parent; anchors.margins: 8; spacing: 2

                            RowLayout {
                                Layout.fillWidth: true
                                Label { text: modelData.name; font.pixelSize: 11; font.bold: true; color: window.darkMode ? "white" : "#1A1A1A"; Layout.fillWidth: true }
                                Label { text: modelData.detections + " det."; font.pixelSize: 11; color: "#888" }
                                Label { text: modelData.avgConfidence.toFixed(0) + "%"; font.pixelSize: 11; color: "#1DB954"; font.bold: true }
                            }

                            // Screen time bar
                            Rectangle {
                                Layout.fillWidth: true; Layout.preferredHeight: 12; radius: 3
                                color: window.darkMode ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.06)

                                Rectangle {
                                    width: parent.width * Math.min(modelData.screenTimePercent / 100.0, 1.0)
                                    height: parent.height; radius: 3
                                    color: "#6C5CE7"
                                    Behavior on width { NumberAnimation { duration: 300 } }
                                }

                                Label {
                                    anchors.centerIn: parent
                                    text: modelData.screenTimePercent.toFixed(0) + "% (" + formatTime(modelData.screenTime) + ")"
                                    font.pixelSize: 10; font.family: "Menlo"
                                    color: "white"
                                }
                            }
                        }
                    }
                }

                // Empty state
                Label {
                    visible: analyticsEngine.talentStats.length === 0
                    text: "Aucun talent detecte"
                    font.pixelSize: 11; color: "#555"
                    Layout.alignment: Qt.AlignHCenter; topPadding: 20
                }
            }
        }

        // Action buttons
        RowLayout {
            Layout.fillWidth: true; spacing: 8

            Rectangle {
                Layout.fillWidth: true; Layout.preferredHeight: 30; radius: 6
                color: window.darkMode ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.04)
                Label { anchors.centerIn: parent; text: "Exporter rapport"; font.pixelSize: 10; color: window.darkMode ? "#888" : "#666" }
                MouseArea {
                    anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        var path = Qt.resolvedUrl(".").toString().replace("file://", "") + "rapport_prestige.txt"
                        analyticsEngine.exportReportToFile(path)
                    }
                }
            }

            Rectangle {
                Layout.preferredWidth: 80; Layout.preferredHeight: 30; radius: 6
                color: Qt.rgba(0.8, 0.2, 0.2, 0.1)
                border.color: Qt.rgba(0.8, 0.2, 0.2, 0.2)
                Label { anchors.centerIn: parent; text: "Reset"; font.pixelSize: 10; color: "#CC5555" }
                MouseArea {
                    anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                    onClicked: analyticsEngine.reset()
                }
            }
        }
    }
}
