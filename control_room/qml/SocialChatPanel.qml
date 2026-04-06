import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: chatPanel

    ColumnLayout {
        anchors.fill: parent; spacing: 0

        // ── Header ──────────────────────────────────
        Rectangle {
            Layout.fillWidth: true; Layout.preferredHeight: 50; color: "transparent"
            RowLayout { anchors.fill: parent; anchors.leftMargin: 12; anchors.rightMargin: 12; spacing: 8
                Rectangle {
                    Layout.preferredWidth: 28; Layout.preferredHeight: 28; radius: 7
                    color: Qt.rgba(108/255,92/255,231/255,0.12)
                    Label { anchors.centerIn: parent; text: "\uD83D\uDCAC"; font.pixelSize: 13 }
                }
                ColumnLayout { spacing: 0; Layout.fillWidth: true
                    Label { text: "Social Chat"; font.pixelSize: 15; font.weight: Font.Bold; color: window.darkMode ? "#F0F0F5" : "#0F0F14" }
                    Label {
                        text: socialChatController.enabled ? (socialChatController.platform + " — Connected") : "Disconnected"
                        font.pixelSize: 10; color: socialChatController.enabled ? "#00D68F" : (window.darkMode ? "#505060" : "#999")
                    }
                }
            }
        }

        Flickable {
            Layout.fillWidth: true; Layout.preferredHeight: 180; contentHeight: configCol.implicitHeight; clip: true
            flickableDirection: Flickable.VerticalFlick
            ColumnLayout {
                id: configCol; width: parent.width; spacing: 8

                // ── Platform ────────────────────────────
                RowLayout { spacing: 6; Layout.leftMargin: 12
                    Label { text: "Platform:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11; Layout.preferredWidth: 55 }
                    Repeater {
                        model: [
                            { label: "YouTube", val: "youtube", color: "#FF0000" },
                            { label: "Twitch", val: "twitch", color: "#9147FF" }
                        ]
                        Rectangle {
                            Layout.preferredWidth: 80; Layout.preferredHeight: 30; radius: 8
                            color: socialChatController.platform === modelData.val ? Qt.rgba(modelData.color, 0.15) : (window.darkMode ? Qt.rgba(255,255,255,0.03) : Qt.rgba(0,0,0,0.03))
                            border.color: socialChatController.platform === modelData.val ? modelData.color : "transparent"
                            Label { anchors.centerIn: parent; text: modelData.label; font.pixelSize: 11; font.weight: Font.DemiBold; color: socialChatController.platform === modelData.val ? modelData.color : (window.darkMode ? "#888" : "#666") }
                            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: socialChatController.platform = modelData.val }
                        }
                    }
                }

                // ── Channel ─────────────────────────────
                RowLayout { spacing: 6; Layout.leftMargin: 12; Layout.rightMargin: 12
                    Label { text: "Channel:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11; Layout.preferredWidth: 55 }
                    TextField {
                        Layout.fillWidth: true; text: socialChatController.channelId; onTextChanged: socialChatController.channelId = text
                        placeholderText: socialChatController.platform === "twitch" ? "channel_name" : "YouTube Video ID"
                        font.pixelSize: 12; color: window.darkMode ? "white" : "#1A1A1A"
                        background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 6; border.color: window.darkMode ? "#333" : "#CCC" }
                    }
                }

                // ── Toggles ─────────────────────────────
                RowLayout { spacing: 12; Layout.leftMargin: 12
                    Switch { text: "Enable chat"; checked: socialChatController.enabled; onCheckedChanged: socialChatController.enabled = checked }
                    Switch { text: "Show on output"; checked: socialChatController.showOnOutput; onCheckedChanged: socialChatController.showOnOutput = checked }
                }

                // ── Max messages ─────────────────────────
                RowLayout { spacing: 4; Layout.leftMargin: 12
                    Label { text: "Max msgs:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11; Layout.preferredWidth: 55 }
                    Slider { from: 5; to: 50; stepSize: 1; value: socialChatController.maxMessages; Layout.fillWidth: true; onValueChanged: socialChatController.maxMessages = value }
                    Label { text: socialChatController.maxMessages.toString(); color: window.darkMode ? "#888" : "#555"; font.pixelSize: 10; Layout.preferredWidth: 24 }
                }
            }
        }

        // ── Messages ────────────────────────────────
        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#1A1A22" : "#E0E0E8"; Layout.leftMargin: 12; Layout.rightMargin: 12 }
        Label { text: "Recent messages"; font.pixelSize: 11; font.weight: Font.DemiBold; color: window.darkMode ? "#F0F0F5" : "#0F0F14"; leftPadding: 12; topPadding: 6 }

        ListView {
            Layout.fillWidth: true; Layout.fillHeight: true; Layout.leftMargin: 8; Layout.rightMargin: 8
            model: socialChatController.recentMessages; clip: true; spacing: 3

            delegate: Rectangle {
                width: ListView.view.width; height: msgCol.implicitHeight + 10; radius: 6
                color: window.darkMode ? Qt.rgba(255,255,255,0.02) : Qt.rgba(0,0,0,0.02)

                ColumnLayout {
                    id: msgCol; anchors.fill: parent; anchors.margins: 6; spacing: 2
                    RowLayout { spacing: 5
                        Rectangle { Layout.preferredWidth: 6; Layout.preferredHeight: 6; radius: 3; color: modelData.platform === "twitch" ? "#9147FF" : "#FF0000" }
                        Label { text: modelData.author; font.pixelSize: 11; font.weight: Font.Bold; color: modelData.color || "#AAA" }
                    }
                    Label { text: modelData.message; font.pixelSize: 11; color: window.darkMode ? "#CCC" : "#333"; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                }
            }

            Label {
                anchors.centerIn: parent
                visible: !socialChatController.recentMessages || socialChatController.recentMessages.length === 0
                text: socialChatController.enabled ? "Waiting for messages..." : "Enable chat to see messages"
                font.pixelSize: 11; color: window.darkMode ? "#444" : "#AAA"
            }
        }
    }
}
