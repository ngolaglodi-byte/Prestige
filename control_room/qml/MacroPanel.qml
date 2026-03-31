import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

// Feature 3: Macros — connected to MacroEngine backend
Item {
    ColumnLayout {
        anchors.fill: parent; anchors.margins: 12; spacing: 10
        Label { text: "Macros & Séquences"; font.pixelSize: 15; font.bold: true; color: window.darkMode ? "white" : "#1A1A1A" }

        // Executing indicator
        Rectangle {
            Layout.fillWidth: true; Layout.preferredHeight: 28; radius: 6
            color: macroEngine.executing ? Qt.rgba(91/255,79/255,219/255,0.15) : "transparent"
            visible: macroEngine.executing
            RowLayout {
                anchors.centerIn: parent; spacing: 6
                Rectangle { Layout.preferredWidth: 8; Layout.preferredHeight: 8; radius: 4; color: "#5B4FDB"
                    SequentialAnimation on opacity {
                        loops: Animation.Infinite
                        NumberAnimation { to: 0.3; duration: 500 }
                        NumberAnimation { to: 1.0; duration: 500 }
                    }
                }
                Label { text: "Macro en cours..."; font.pixelSize: 11; color: "#8B80E0" }
                Rectangle { Layout.preferredWidth: 50; Layout.preferredHeight: 22; radius: 4; color: "#CC0000"
                    Label { anchors.centerIn: parent; text: "STOP"; color: "white"; font.pixelSize: 11; font.bold: true }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: macroEngine.stopMacro() }
                }
            }
        }

        ListView {
            Layout.fillWidth: true; Layout.fillHeight: true; clip: true; spacing: 4
            model: macroEngine.macros

            delegate: Rectangle {
                width: ListView.view.width; height: 48; radius: 6
                color: macroEngine.executing && macroEngine.currentMacroIndex === index
                       ? Qt.rgba(91/255,79/255,219/255,0.12)
                       : (window.darkMode ? Qt.rgba(1,1,1,0.03) : Qt.rgba(0,0,0,0.03))
                border.color: macroEngine.executing && macroEngine.currentMacroIndex === index
                              ? Qt.rgba(91/255,79/255,219/255,0.3)
                              : (window.darkMode ? Qt.rgba(1,1,1,0.05) : Qt.rgba(0,0,0,0.08))
                Behavior on color { ColorAnimation { duration: 200 } }

                RowLayout {
                    anchors.fill: parent; anchors.margins: 10; spacing: 8

                    Label {
                        text: modelData.icon || "\u25B6"
                        font.pixelSize: 18
                    }

                    ColumnLayout { Layout.fillWidth: true; spacing: 2
                        Label { text: modelData.name; font.pixelSize: 13; font.bold: true; color: window.darkMode ? "white" : "#1A1A1A" }
                        Label { text: modelData.actionCount + " action(s)"; font.pixelSize: 11; color: window.darkMode ? "#777" : "#888" }
                    }

                    Rectangle {
                        Layout.preferredWidth: 50; Layout.preferredHeight: 24; radius: 4
                        color: goMouse.containsMouse ? "#6B5FEB" : "#5B4FDB"
                        Behavior on color { ColorAnimation { duration: 150 } }
                        Label { anchors.centerIn: parent; text: "\u25B6 GO"; color: "white"; font.pixelSize: 11; font.bold: true }
                        MouseArea {
                            id: goMouse; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                            onClicked: macroEngine.executeMacro(index)
                        }
                    }
                }
            }
        }
    }
}
