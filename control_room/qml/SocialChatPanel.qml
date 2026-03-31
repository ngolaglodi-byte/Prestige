import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: chatPanel

    ColumnLayout {
        anchors.fill: parent; anchors.margins: 10; spacing: 8

        // Header
        Label { text: "Social Chat"; font.pixelSize: 14; font.weight: Font.Bold; color: window.darkMode ? "white" : "#1A1A1A" }

        // Platform selector
        RowLayout {
            Layout.fillWidth: true; spacing: 6
            Label { text: "Plateforme"; font.pixelSize: 11; color: window.darkMode ? "#888" : "#555" }
            ComboBox {
                id: platformCombo
                Layout.fillWidth: true; Layout.preferredHeight: 28
                model: ["youtube", "twitch"]
                currentIndex: socialChatController.platform === "twitch" ? 1 : 0
                font.pixelSize: 11
                background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
                contentItem: Label { text: platformCombo.currentText; font.pixelSize: 11; color: window.darkMode ? "white" : "#1A1A1A"; verticalAlignment: Text.AlignVCenter; leftPadding: 6 }
                onCurrentTextChanged: socialChatController.platform = currentText
            }
        }

        // Channel ID
        RowLayout {
            Layout.fillWidth: true; spacing: 6
            Label { text: "Channel"; font.pixelSize: 11; color: window.darkMode ? "#888" : "#555" }
            TextField {
                Layout.fillWidth: true; Layout.preferredHeight: 28
                placeholderText: "ID / nom..."
                text: socialChatController.channelId
                font.pixelSize: 11; color: window.darkMode ? "white" : "#1A1A1A"
                background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
                onTextChanged: socialChatController.channelId = text
            }
        }

        // Enable toggle
        RowLayout {
            Layout.fillWidth: true; spacing: 8
            Label { text: "Activer"; font.pixelSize: 11; color: window.darkMode ? "#888" : "#555" }
            Switch {
                checked: socialChatController.enabled
                onCheckedChanged: socialChatController.enabled = checked
            }
        }

        // Show on output toggle
        RowLayout {
            Layout.fillWidth: true; spacing: 8
            Label { text: "Afficher sur sortie"; font.pixelSize: 11; color: window.darkMode ? "#888" : "#555" }
            Switch {
                checked: socialChatController.showOnOutput
                onCheckedChanged: socialChatController.showOnOutput = checked
            }
        }

        // Max messages slider
        RowLayout {
            Layout.fillWidth: true; spacing: 6
            Label { text: "Max msg"; font.pixelSize: 11; color: window.darkMode ? "#888" : "#555" }
            Slider {
                Layout.fillWidth: true
                from: 5; to: 50; stepSize: 1
                value: socialChatController.maxMessages
                onValueChanged: socialChatController.maxMessages = value
            }
            Label { text: socialChatController.maxMessages.toString(); font.pixelSize: 11; color: window.darkMode ? "#AAA" : "#444"; Layout.preferredWidth: 24 }
        }

        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.08) }

        // Live message preview
        Label { text: "Messages recents"; font.pixelSize: 11; color: window.darkMode ? "#666" : "#999" }

        ListView {
            Layout.fillWidth: true; Layout.fillHeight: true
            model: socialChatController.recentMessages
            clip: true
            spacing: 2

            delegate: Rectangle {
                width: ListView.view.width; height: msgCol.implicitHeight + 8; radius: 4
                color: window.darkMode ? Qt.rgba(1,1,1,0.03) : Qt.rgba(0,0,0,0.03)

                ColumnLayout {
                    id: msgCol
                    anchors.fill: parent; anchors.margins: 4; spacing: 1
                    RowLayout {
                        spacing: 4
                        Rectangle { Layout.preferredWidth: 6; Layout.preferredHeight: 6; radius: 3; color: modelData.platform === "twitch" ? "#9147FF" : "#FF0000" }
                        Label { text: modelData.author; font.pixelSize: 11; font.weight: Font.Bold; color: modelData.color || "#AAA" }
                    }
                    Label { text: modelData.message; font.pixelSize: 11; color: window.darkMode ? "#CCC" : "#333"; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                }
            }

            // Empty state
            Label {
                anchors.centerIn: parent
                visible: !socialChatController.recentMessages || socialChatController.recentMessages.length === 0
                text: "Aucun message"
                font.pixelSize: 12; color: window.darkMode ? "#444" : "#AAA"
            }
        }
    }
}
