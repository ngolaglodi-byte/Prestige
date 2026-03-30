import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Effects

Item {
    id: setupView

    property int currentStep: 0
    property var steps: [window.t("step_style"), window.t("step_talents"), window.t("step_source"), window.t("step_outputs"), window.t("step_test")]

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // ── Sidebar ────────────────────────────────────────
        Rectangle {
            Layout.preferredWidth: 230
            Layout.fillHeight: true
            color: window.darkMode ? Qt.rgba(1, 1, 1, 0.02) : Qt.rgba(0, 0, 0, 0.02)

            // Subtle left border glow
            Rectangle {
                anchors.right: parent.right; width: 1
                height: parent.height; color: window.darkMode ? Qt.rgba(1, 1, 1, 0.04) : Qt.rgba(0, 0, 0, 0.06)
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 6

                // Logo
                Item {
                    Layout.fillWidth: true; Layout.preferredHeight: 60
                    Label {
                        anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
                        text: "PRESTIGE"
                        font.pixelSize: 18; font.weight: Font.Bold; font.letterSpacing: 3
                        color: "#5B4FDB"
                    }
                    Label {
                        anchors.left: parent.left; anchors.bottom: parent.bottom; anchors.bottomMargin: 6
                        text: window.t("config_emission")
                        font.pixelSize: 11; color: window.darkMode ? "#555" : "#999"
                    }
                }

                Item { implicitHeight: 12 }

                // Scrollable steps area
                Flickable {
                    Layout.fillWidth: true; Layout.fillHeight: true
                    contentHeight: sidebarStepsCol.implicitHeight; clip: true
                    flickableDirection: Flickable.VerticalFlick
                    ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
                    ColumnLayout {
                        id: sidebarStepsCol; width: parent.width; spacing: 6

                        // Steps
                        Repeater {
                            model: setupView.steps
                            delegate: Rectangle {
                                Layout.fillWidth: true; Layout.preferredHeight: 48; radius: 10
                                color: index === setupView.currentStep ? Qt.rgba(91/255, 79/255, 219/255, 0.12) : (msStep.containsMouse ? (window.darkMode ? Qt.rgba(1,1,1,0.03) : Qt.rgba(0,0,0,0.03)) : "transparent")
                                border.color: index === setupView.currentStep ? Qt.rgba(91/255, 79/255, 219/255, 0.3) : "transparent"
                                Behavior on color { ColorAnimation { duration: 200 } }
                                Behavior on border.color { ColorAnimation { duration: 200 } }

                                RowLayout {
                                    anchors.fill: parent; anchors.leftMargin: 12; anchors.rightMargin: 12; spacing: 10

                                    // Step number circle
                                    Rectangle {
                                        Layout.preferredWidth: 26; Layout.preferredHeight: 26; radius: 13
                                        color: index < setupView.currentStep ? "#1DB954" : (index === setupView.currentStep ? "#5B4FDB" : (window.darkMode ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.08)))
                                        Behavior on color { ColorAnimation { duration: 300 } }

                                        Label {
                                            anchors.centerIn: parent
                                            text: index < setupView.currentStep ? "\u2713" : (index + 1).toString()
                                            color: "white"; font.pixelSize: 11; font.weight: Font.Bold
                                        }
                                    }

                                    Label {
                                        text: modelData; Layout.fillWidth: true
                                        color: index === setupView.currentStep ? (window.darkMode ? "white" : "#1A1A1A") : (index < setupView.currentStep ? (window.darkMode ? "#AAA" : "#444") : (window.darkMode ? "#555" : "#999"))
                                        font.pixelSize: 13; font.weight: index === setupView.currentStep ? Font.DemiBold : Font.Normal
                                        Behavior on color { ColorAnimation { duration: 200 } }
                                        elide: Text.ElideRight
                                    }
                                }

                                MouseArea {
                                    id: msStep; anchors.fill: parent; hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: setupView.currentStep = index
                                }
                            }
                        }
                    }
                }

                // Profile card
                Rectangle {
                    Layout.fillWidth: true; Layout.preferredHeight: 80; radius: 12
                    color: window.darkMode ? Qt.rgba(1, 1, 1, 0.03) : Qt.rgba(0, 0, 0, 0.03)
                    border.color: window.darkMode ? Qt.rgba(1, 1, 1, 0.05) : Qt.rgba(0, 0, 0, 0.08)

                    ColumnLayout {
                        anchors.fill: parent; anchors.margins: 12; spacing: 4
                        Label { text: window.t("active_profile"); font.pixelSize: 10; color: window.darkMode ? "#555" : "#999"; font.letterSpacing: 1 }
                        Label { text: setupController.currentProfileName; font.pixelSize: 13; font.weight: Font.DemiBold; color: window.darkMode ? "white" : "#1A1A1A"; elide: Text.ElideRight; Layout.fillWidth: true }
                        Label { text: window.t("modify"); font.pixelSize: 11; color: "#5B4FDB"
                            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor }
                        }
                    }
                }
            }
        }

        // ── Main content ───────────────────────────────────
        ColumnLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 0

            // Step header
            Item {
                Layout.fillWidth: true; Layout.preferredHeight: 64

                RowLayout {
                    anchors.fill: parent; anchors.leftMargin: 28; anchors.rightMargin: 28

                    Label {
                        text: setupView.steps[setupView.currentStep]
                        font.pixelSize: 22; font.weight: Font.DemiBold; color: window.darkMode ? "white" : "#1A1A1A"
                    }
                    Item { Layout.fillWidth: true }

                    // Progress dots
                    Row {
                        spacing: 6
                        Repeater {
                            model: setupView.steps.length
                            Rectangle {
                                width: index === setupView.currentStep ? 24 : 8; height: 8; radius: 4
                                color: index <= setupView.currentStep ? "#5B4FDB" : (window.darkMode ? Qt.rgba(1,1,1,0.08) : Qt.rgba(0,0,0,0.1))
                                Behavior on width { NumberAnimation { duration: 250; easing.type: Easing.OutCubic } }
                                Behavior on color { ColorAnimation { duration: 250 } }
                            }
                        }
                    }
                }

                // Bottom separator
                Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: window.darkMode ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.06) }
            }

            // Step content
            StackLayout {
                Layout.fillWidth: true; Layout.fillHeight: true
                currentIndex: setupView.currentStep
                StyleStep {}
                TalentStep {}
                SourceStep {}
                OutputStep {}
                TestStep {}
            }

            // Bottom navigation
            Rectangle {
                Layout.fillWidth: true; Layout.preferredHeight: 72
                color: window.darkMode ? Qt.rgba(1, 1, 1, 0.02) : Qt.rgba(0, 0, 0, 0.02)
                Rectangle { anchors.top: parent.top; width: parent.width; height: 1; color: window.darkMode ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.06) }

                RowLayout {
                    anchors.fill: parent; anchors.leftMargin: 28; anchors.rightMargin: 28

                    // Previous
                    Rectangle {
                        visible: setupView.currentStep > 0
                        Layout.preferredWidth: 120; Layout.preferredHeight: 40; radius: 8
                        color: msPrev.containsMouse ? (window.darkMode ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.06)) : "transparent"
                        Behavior on color { ColorAnimation { duration: 150 } }
                        Label { anchors.centerIn: parent; text: "\u2190  " + window.t("previous"); color: window.darkMode ? "#888" : "#555"; font.pixelSize: 13 }
                        MouseArea { id: msPrev; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: setupView.currentStep-- }
                    }

                    Item { Layout.fillWidth: true }

                    // Save
                    Rectangle {
                        Layout.preferredWidth: 120; Layout.preferredHeight: 40; radius: 8
                        color: msSave.containsMouse ? (window.darkMode ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.06)) : "transparent"
                        Behavior on color { ColorAnimation { duration: 150 } }
                        Label { anchors.centerIn: parent; text: window.t("save"); color: window.darkMode ? "#888" : "#555"; font.pixelSize: 13 }
                        MouseArea { id: msSave; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: setupController.saveProfile() }
                    }

                    // Next
                    Rectangle {
                        visible: setupView.currentStep < setupView.steps.length - 1
                        Layout.preferredWidth: 120; Layout.preferredHeight: 40; radius: 8
                        color: msNext.containsMouse ? "#6B5FEB" : "#5B4FDB"
                        Behavior on color { ColorAnimation { duration: 150 } }
                        scale: msNext.pressed ? 0.97 : 1.0
                        Behavior on scale { NumberAnimation { duration: 100 } }
                        Label { anchors.centerIn: parent; text: window.t("next") + "  \u2192"; color: "white"; font.pixelSize: 13; font.weight: Font.DemiBold }
                        MouseArea { id: msNext; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: setupView.currentStep++ }
                    }

                    // Launch
                    Rectangle {
                        visible: setupView.currentStep === setupView.steps.length - 1
                        Layout.preferredWidth: 200; Layout.preferredHeight: 44; radius: 10
                        enabled: setupController.isReadyToGo()
                        color: msLaunch2.containsMouse ? "#DD0000" : "#CC0000"
                        opacity: enabled ? 1.0 : 0.4
                        Behavior on color { ColorAnimation { duration: 150 } }
                        scale: msLaunch2.pressed ? 0.97 : 1.0
                        Behavior on scale { NumberAnimation { duration: 100 } }

                        RowLayout {
                            anchors.centerIn: parent; spacing: 8
                            Rectangle { Layout.preferredWidth: 8; Layout.preferredHeight: 8; radius: 4; color: "white" }
                            Label { text: window.t("launch_emission_btn"); color: "white"; font.pixelSize: 14; font.weight: Font.Bold; font.letterSpacing: 0.5 }
                        }
                        MouseArea { id: msLaunch2; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; enabled: parent.enabled; onClicked: confirmLaunchDialog.open() }
                    }
                }
            }
        }
    }
}
