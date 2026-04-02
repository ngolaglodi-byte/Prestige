import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs

// ============================================================
// Prestige AI — Professional Color Picker Button
// Presets + full ColorDialog (Adobe-style palette)
// ============================================================

Item {
    id: cpBtn
    width: 26; height: 26

    property color currentColor: "#FFFFFF"
    property var presets: []

    signal colorSelected(color c)

    // The colored button that opens the picker
    Rectangle {
        anchors.fill: parent; radius: 4
        color: cpBtn.currentColor
        border.color: cpMa.containsMouse ? "white" : Qt.rgba(1,1,1,0.3); border.width: 1.5

        // "+" icon when no presets, or colored swatch
        Label {
            anchors.centerIn: parent
            text: "\u270E"; font.pixelSize: 10; color: Qt.rgba(1,1,1,0.8)
            visible: cpMa.containsMouse
        }

        MouseArea {
            id: cpMa; anchors.fill: parent; hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: colorDialog.open()
        }
    }

    ColorDialog {
        id: colorDialog
        title: "Choisir une couleur"
        selectedColor: cpBtn.currentColor
        onAccepted: {
            cpBtn.currentColor = selectedColor
            cpBtn.colorSelected(selectedColor)
        }
    }
}
