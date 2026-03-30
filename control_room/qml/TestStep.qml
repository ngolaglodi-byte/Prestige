import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

// ============================================================
// Step 5 — Test & Validation
// ============================================================

Item {
    Flickable {
        anchors.fill: parent; anchors.margins: 24
        contentHeight: testRow.implicitHeight; clip: true
        flickableDirection: Flickable.VerticalFlick
        ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
    RowLayout {
        id: testRow; width: parent.width
        spacing: 24

        // Left: test preview
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 16

            Label { text: window.t("live_test"); font.pixelSize: 16; color: window.darkMode ? "white" : "#1A1A1A" }

            // 16:9 test preview
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: width * 9 / 16
                color: "#000"
                radius: 8

                Column {
                    anchors.centerIn: parent
                    spacing: 8
                    Label {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "\u25B6"
                        font.pixelSize: 48; color: window.darkMode ? "#333" : "#CCC"
                    }
                    Label {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: window.t("click_test")
                        font.pixelSize: 14; color: window.darkMode ? "#555" : "#999"
                    }
                }

                Rectangle {
                    anchors.top: parent.top; anchors.right: parent.right
                    anchors.margins: 8
                    width: 56; height: 22; radius: 4; color: window.darkMode ? "#222" : "#E0E0E6"
                    Label { anchors.centerIn: parent; text: "TEST"; font.pixelSize: 10; color: window.darkMode ? "#666" : "#999" }
                }
            }

            Button {
                text: window.t("launch_test")
                Layout.fillWidth: true
                background: Rectangle { color: window.darkMode ? "#1A2A1A" : "#E8F5E9"; radius: 6; border.color: "#1DB954" }
                contentItem: Label { text: parent.text; color: "#1DB954"; font.pixelSize: 13; horizontalAlignment: Text.AlignHCenter }
            }
        }

        // Right: checklist
        ColumnLayout {
            Layout.preferredWidth: 320
            Layout.fillHeight: true
            spacing: 12

            Label { text: window.t("pre_checklist"); font.pixelSize: 16; color: window.darkMode ? "white" : "#1A1A1A" }

            Repeater {
                model: [
                    { label: window.t("style_chosen"),   ok: setupController.selectedStyle !== "" },
                    { label: window.t("talent_added"), ok: talentManager.talents.length > 0 },
                    { label: window.t("source_configured"),   ok: setupController.inputType !== "" },
                    { label: window.t("output_active"),  ok: setupController.isReadyToGo() },
                    { label: window.t("ai_server"),       ok: talentManager.connected }
                ]
                delegate: Rectangle {
                    Layout.fillWidth: true
                    height: 48; radius: 6
                    color: window.darkMode ? "#111114" : "#FFFFFF"; border.color: window.darkMode ? "#1E1E22" : "#DDD"

                    RowLayout {
                        anchors.fill: parent; anchors.margins: 12; spacing: 10

                        Rectangle {
                            Layout.preferredWidth: 24; Layout.preferredHeight: 24; radius: 12
                            color: modelData.ok ? "#1DB954" : (window.darkMode ? "#2A2A30" : "#E0E0E6")
                            Label {
                                anchors.centerIn: parent
                                text: modelData.ok ? "\u2713" : ""
                                font.pixelSize: 12; color: "white"
                            }
                        }

                        Label {
                            text: modelData.label
                            font.pixelSize: 13
                            color: modelData.ok ? (window.darkMode ? "#CCC" : "#333") : (window.darkMode ? "#666" : "#999")
                            Layout.fillWidth: true
                        }
                    }
                }
            }

            Item { Layout.fillHeight: true }

            // Summary
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 100; radius: 8
                color: setupController.isReadyToGo() ? (window.darkMode ? "#0A1A0A" : "#E8F5E9") : (window.darkMode ? "#1A1010" : "#FFEBEE")
                border.color: setupController.isReadyToGo() ? "#1DB954" : (window.darkMode ? "#553333" : "#E0B0B0")

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 6

                    Label {
                        text: setupController.isReadyToGo() ? window.t("ready") : window.t("not_ready")
                        font.pixelSize: 16; font.bold: true
                        color: setupController.isReadyToGo() ? "#1DB954" : "#CC3333"
                        Layout.alignment: Qt.AlignHCenter
                    }
                    Label {
                        text: setupController.isReadyToGo()
                              ? window.t("can_launch")
                              : window.t("complete_steps")
                        font.pixelSize: 12
                        color: window.darkMode ? "#888" : "#555"
                        Layout.alignment: Qt.AlignHCenter
                    }
                }
            }
        }
    } // end testRow
    } // end Flickable
}
