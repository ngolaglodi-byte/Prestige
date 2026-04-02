import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
Item {
    id: tp
    property string scriptText: "Bienvenue au journal de 20 heures.\n\nCe soir, les titres de l'actualité..."
    property double scrollSpeed: 2.0
    property bool isScrolling: false
    property bool mirrorMode: false
    property int fontSize: 24
    property string fontColor: "#FFFFFF"

    // File import handled via text field path input (no QtQuick.Dialogs dependency)
    function importScript(path) {
        var xhr = new XMLHttpRequest()
        xhr.open("GET", "file:///" + path)
        xhr.onreadystatechange = function() {
            if (xhr.readyState === XMLHttpRequest.DONE) tp.scriptText = xhr.responseText
        }
        xhr.send()
    }

    ColumnLayout {
        anchors.fill: parent; anchors.margins: 12; spacing: 8
        Label { text: "Téléprompter"; font.pixelSize: 15; font.bold: true; color: window.darkMode ? "white" : "#1A1A1A" }

        TextArea {
            Layout.fillWidth: true; Layout.fillHeight: true
            text: tp.scriptText; onTextChanged: tp.scriptText = text
            font.pixelSize: tp.fontSize * 0.6; color: tp.fontColor; wrapMode: TextArea.Wrap
            transform: Scale { xScale: tp.mirrorMode ? -1 : 1; origin.x: parent.width / 2 }
            background: Rectangle { color: window.darkMode ? "#0A0A0E" : "#E8E8EE"; radius: 6; border.color: window.darkMode ? "#222" : "#CCC" }
        }

        RowLayout { spacing: 4
            Label { text: "Taille:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11 }
            Slider { from: 16; to: 48; stepSize: 1; value: tp.fontSize; Layout.fillWidth: true; onMoved: tp.fontSize = value }
            Label { text: tp.fontSize + "px"; color: window.darkMode ? "#888" : "#555"; font.pixelSize: 11 }
        }
        RowLayout { spacing: 4
            Label { text: "Vitesse:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11 }
            Slider { from: 0.5; to: 5; value: tp.scrollSpeed; Layout.fillWidth: true; onMoved: tp.scrollSpeed = value }
            Label { text: tp.scrollSpeed.toFixed(1) + "x"; color: window.darkMode ? "#888" : "#555"; font.pixelSize: 11 }
        }
        RowLayout { spacing: 4
            Label { text: "Couleur:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11 }
            Repeater { model: ["#FFFFFF", "#FFFF00", "#00FF00", "#00CCFF"]; Rectangle { width: 18; height: 18; radius: 4; color: modelData; border.color: modelData === tp.fontColor ? "white" : "transparent"; border.width: 2; MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: tp.fontColor = modelData } } }
        }

        RowLayout {
            spacing: 6
            Switch { text: "Miroir"; checked: tp.mirrorMode; onToggled: tp.mirrorMode = checked }
            Item { Layout.fillWidth: true }
            Rectangle { Layout.preferredWidth: 80; Layout.preferredHeight: 30; radius: 6; color: window.darkMode ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.06)
                Label { anchors.centerIn: parent; text: "Importer..."; color: window.darkMode ? "#888" : "#555"; font.pixelSize: 11 }
                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: importPathField.visible = !importPathField.visible }
            }
            RowLayout { id: importPathField; visible: false; Layout.fillWidth: true; spacing: 4
                TextField { id: scriptPathField; Layout.fillWidth: true; placeholderText: "Chemin du fichier .txt"; font.pixelSize: 10; color: window.darkMode ? "#CCC" : "#333"
                    background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
                }
                Rectangle { Layout.preferredWidth: 50; Layout.preferredHeight: 28; radius: 4; color: "#5B4FDB"
                    Label { anchors.centerIn: parent; text: "OK"; color: "white"; font.pixelSize: 10; font.bold: true }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { importScript(scriptPathField.text); importPathField.visible = false } }
                }
            }
            Rectangle { Layout.preferredWidth: 80; Layout.preferredHeight: 30; radius: 6; color: tp.isScrolling ? "#CC0000" : "#1DB954"
                Label { anchors.centerIn: parent; text: tp.isScrolling ? "STOP" : "DÉFILER"; color: "white"; font.pixelSize: 11; font.bold: true }
                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: tp.isScrolling = !tp.isScrolling }
            }
        }
    }
}
