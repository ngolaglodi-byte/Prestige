import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

// Feature 10: Statistiques émission
Item {
    property int totalDetections: 0
    property int totalOverlays: 0
    property int emissionDurationSec: 0
    property double avgFps: 0
    property double avgLatency: 0

    ColumnLayout {
        anchors.fill: parent; anchors.margins: 12; spacing: 10
        Label { text: "Statistiques de l'émission"; font.pixelSize: 15; font.bold: true; color: window.darkMode ? "white" : "#1A1A1A" }

        Repeater {
            model: [
                { label: "Durée totale", value: Math.floor(emissionDurationSec/3600) + "h " + Math.floor((emissionDurationSec%3600)/60) + "min" },
                { label: "Visages détectés", value: totalDetections.toString() },
                { label: "Overlays affichés", value: totalOverlays.toString() },
                { label: "FPS moyen", value: avgFps.toFixed(1) },
                { label: "Latence moyenne", value: avgLatency.toFixed(0) + "ms" }
            ]
            delegate: Rectangle {
                Layout.fillWidth: true; height: 44; radius: 6; color: window.darkMode ? Qt.rgba(1,1,1,0.03) : Qt.rgba(0,0,0,0.03)
                RowLayout { anchors.fill: parent; anchors.margins: 12
                    Label { text: modelData.label; font.pixelSize: 13; color: window.darkMode ? "#999" : "#666"; Layout.fillWidth: true }
                    Label { text: modelData.value; font.pixelSize: 15; font.bold: true; font.family: "Menlo"; color: window.darkMode ? "white" : "#1A1A1A" }
                }
            }
        }

        Rectangle { Layout.preferredWidth: 100; Layout.preferredHeight: 32; radius: 6; color: window.darkMode ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.06); Layout.alignment: Qt.AlignRight
            Label { anchors.centerIn: parent; text: "Exporter CSV"; color: window.darkMode ? "#888" : "#555"; font.pixelSize: 11 }
            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor }
        }
    }
}
