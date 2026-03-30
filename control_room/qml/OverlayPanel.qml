import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

// ============================================================
// Prestige AI — Overlay Panel
// Talent list + overlay controls
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

            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: parent.radius
                color: parent.color
            }

            Text {
                anchors.centerIn: parent
                text: "TALENTS DÉTECTÉS"
                color: window.darkMode ? "#888888" : "#555555"
                font.pixelSize: 11
                font.weight: Font.Bold
                font.letterSpacing: 1.5
            }
        }

        // Talent list
        ListView {
            id: talentList
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 8
            clip: true
            spacing: 4

            model: ListModel {
                id: talentModel
                // Populated dynamically from ZMQ messages
                ListElement { talentName: "Marie Dupont"; talentRole: "Présentatrice"; isActive: true }
                ListElement { talentName: "Jean Martin";  talentRole: "Invité";        isActive: false }
            }

            delegate: Rectangle {
                width: talentList.width
                height: 44
                radius: 4
                color: isActive ? (window.darkMode ? "#1B3A1B" : "#E8F5E9") : (window.darkMode ? "#1E1E22" : "#F0F0F4")
                border.color: isActive ? "#2E7D32" : "transparent"
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12
                    spacing: 8

                    // Status dot
                    Rectangle {
                        Layout.preferredWidth: 8; Layout.preferredHeight: 8; radius: 4
                        color: isActive ? "#4CAF50" : (window.darkMode ? "#555555" : "#AAAAAA")
                    }

                    // Name + role
                    Column {
                        Layout.fillWidth: true
                        Text {
                            text: talentName
                            color: window.darkMode ? "#FFFFFF" : "#1A1A1A"
                            font.pixelSize: 13
                            font.weight: Font.DemiBold
                        }
                        Text {
                            text: talentRole
                            color: window.darkMode ? "#999999" : "#666666"
                            font.pixelSize: 11
                        }
                    }
                }
            }
        }

        // Separator
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: window.darkMode ? "#2A2A2E" : "#DDD"
        }

        // Overlay controls
        ColumnLayout {
            Layout.fillWidth: true
            Layout.margins: 12
            spacing: 6

            Text {
                text: "OVERLAYS"
                color: window.darkMode ? "#888888" : "#555555"
                font.pixelSize: 11
                font.weight: Font.Bold
                font.letterSpacing: 1.5
            }

            RowLayout {
                Layout.fillWidth: true
                CheckBox {
                    id: nameplateCheck
                    checked: overlayController.nameplatesEnabled
                    onToggled: overlayController.nameplatesEnabled = checked
                    indicator: Rectangle {
                        implicitWidth: 16; implicitHeight: 16; radius: 3
                        color: nameplateCheck.checked ? "#4CAF50" : (window.darkMode ? "#333" : "#CCC")
                        border.color: window.darkMode ? "#555" : "#999"
                    }
                }
                Text { text: "Noms talents"; color: window.darkMode ? "#CCCCCC" : "#333333"; font.pixelSize: 12 }
                Item { Layout.fillWidth: true }
                Text { text: "Éditer"; color: "#5C9CE6"; font.pixelSize: 11; MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor } }
            }

            RowLayout {
                Layout.fillWidth: true
                CheckBox {
                    id: lowerCheck
                    checked: overlayController.lowerThirdsEnabled
                    onToggled: overlayController.lowerThirdsEnabled = checked
                    indicator: Rectangle {
                        implicitWidth: 16; implicitHeight: 16; radius: 3
                        color: lowerCheck.checked ? "#4CAF50" : (window.darkMode ? "#333" : "#CCC")
                        border.color: window.darkMode ? "#555" : "#999"
                    }
                }
                Text { text: "Lower thirds"; color: window.darkMode ? "#CCCCCC" : "#333333"; font.pixelSize: 12 }
                Item { Layout.fillWidth: true }
                Text { text: "Éditer"; color: "#5C9CE6"; font.pixelSize: 11; MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor } }
            }

            RowLayout {
                Layout.fillWidth: true
                CheckBox {
                    id: tickerCheck
                    checked: overlayController.tickerEnabled
                    onToggled: overlayController.tickerEnabled = checked
                    indicator: Rectangle {
                        implicitWidth: 16; implicitHeight: 16; radius: 3
                        color: tickerCheck.checked ? "#4CAF50" : (window.darkMode ? "#333" : "#CCC")
                        border.color: window.darkMode ? "#555" : "#999"
                    }
                }
                Text { text: "Ticker"; color: window.darkMode ? "#CCCCCC" : "#333333"; font.pixelSize: 12 }
                Item { Layout.fillWidth: true }
                Text { text: "Éditer"; color: "#5C9CE6"; font.pixelSize: 11; MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor } }
            }
        }
    }
}
