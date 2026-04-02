import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

// ============================================================
// Prestige AI — Professional Color Picker Button
// Built-in color picker (no external Qt module dependency)
// ============================================================

Item {
    id: cpBtn
    width: 26; height: 26

    property color currentColor: "#FFFFFF"

    signal colorSelected(color c)

    Rectangle {
        anchors.fill: parent; radius: 4
        color: cpBtn.currentColor
        border.color: cpMa.containsMouse ? "white" : Qt.rgba(1,1,1,0.3); border.width: 1.5

        Label {
            anchors.centerIn: parent
            text: "\u270E"; font.pixelSize: 10; color: Qt.rgba(1,1,1,0.8)
            visible: cpMa.containsMouse
        }

        MouseArea {
            id: cpMa; anchors.fill: parent; hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: pickerPopup.open()
        }
    }

    Popup {
        id: pickerPopup
        x: -180; y: cpBtn.height + 4
        width: 210; height: 280; padding: 8
        modal: true; closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle { color: "#1A1A1E"; radius: 8; border.color: "#333" }

        ColumnLayout {
            anchors.fill: parent; spacing: 6

            Label { text: "Couleur"; font.pixelSize: 12; font.bold: true; color: "white" }

            // Hex input
            RowLayout { spacing: 4
                Label { text: "#"; color: "#888"; font.pixelSize: 12; font.family: "Menlo" }
                TextField {
                    id: hexField; Layout.fillWidth: true
                    text: cpBtn.currentColor.toString().substring(1).toUpperCase()
                    font.pixelSize: 11; font.family: "Menlo"; color: "white"
                    maximumLength: 6
                    background: Rectangle { color: "#111114"; radius: 4; border.color: "#444" }
                    onTextEdited: {
                        if (text.length === 6) {
                            var c = "#" + text
                            cpBtn.currentColor = c
                            cpBtn.colorSelected(c)
                        }
                    }
                }
            }

            // Color palette (quick presets)
            GridLayout {
                columns: 8; columnSpacing: 3; rowSpacing: 3; Layout.fillWidth: true
                Repeater {
                    model: [
                        "#FF0000","#FF4400","#FF8800","#FFCC00","#FFFF00","#88FF00","#00FF00","#00FF88",
                        "#00FFFF","#0088FF","#0044FF","#0000FF","#4400FF","#8800FF","#FF00FF","#FF0088",
                        "#CC0000","#CC4400","#CC8800","#CCCC00","#88CC00","#00CC00","#00CC88","#00CCCC",
                        "#FFFFFF","#CCCCCC","#999999","#666666","#333333","#1A1A1A","#0D0D0D","#000000",
                        "#E30613","#5B4FDB","#0066CC","#1DB954","#FF6B00","#D4AF37","#00E5FF","#C0C0C0"
                    ]
                    Rectangle {
                        width: 20; height: 20; radius: 3; color: modelData
                        border.color: modelData === cpBtn.currentColor.toString() ? "white" : Qt.rgba(1,1,1,0.15); border.width: modelData === cpBtn.currentColor.toString() ? 2 : 0.5
                        MouseArea {
                            anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                            onClicked: { cpBtn.currentColor = modelData; cpBtn.colorSelected(modelData); hexField.text = modelData.substring(1).toUpperCase() }
                        }
                    }
                }
            }

            // Preview
            Rectangle {
                Layout.fillWidth: true; Layout.preferredHeight: 24; radius: 4
                color: cpBtn.currentColor
                Label { anchors.centerIn: parent; text: cpBtn.currentColor.toString().toUpperCase(); font.pixelSize: 10; font.family: "Menlo"; color: cpBtn.currentColor.hslLightness > 0.5 ? "black" : "white" }
            }

            // RGB sliders
            RowLayout { spacing: 2; Layout.fillWidth: true
                Label { text: "R"; color: "#FF4444"; font.pixelSize: 9; font.bold: true }
                Slider { from: 0; to: 255; value: cpBtn.currentColor.r * 255; Layout.fillWidth: true; onMoved: { var c = Qt.rgba(value/255, cpBtn.currentColor.g, cpBtn.currentColor.b, 1); cpBtn.currentColor = c; cpBtn.colorSelected(c); hexField.text = c.toString().substring(1,7).toUpperCase() } }
            }
            RowLayout { spacing: 2; Layout.fillWidth: true
                Label { text: "G"; color: "#44FF44"; font.pixelSize: 9; font.bold: true }
                Slider { from: 0; to: 255; value: cpBtn.currentColor.g * 255; Layout.fillWidth: true; onMoved: { var c = Qt.rgba(cpBtn.currentColor.r, value/255, cpBtn.currentColor.b, 1); cpBtn.currentColor = c; cpBtn.colorSelected(c); hexField.text = c.toString().substring(1,7).toUpperCase() } }
            }
            RowLayout { spacing: 2; Layout.fillWidth: true
                Label { text: "B"; color: "#4444FF"; font.pixelSize: 9; font.bold: true }
                Slider { from: 0; to: 255; value: cpBtn.currentColor.b * 255; Layout.fillWidth: true; onMoved: { var c = Qt.rgba(cpBtn.currentColor.r, cpBtn.currentColor.g, value/255, 1); cpBtn.currentColor = c; cpBtn.colorSelected(c); hexField.text = c.toString().substring(1,7).toUpperCase() } }
            }
        }
    }
}
