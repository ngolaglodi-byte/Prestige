import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: vsPanel

    Flickable {
        anchors.fill: parent; contentHeight: vsCol.implicitHeight; clip: true
        flickableDirection: Flickable.VerticalFlick
        ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
    ColumnLayout {
        id: vsCol; width: parent.width; spacing: 8
        Item { implicitHeight: 4 }

        // ── Header ────────────────────────────────────────────
        Label { text: "Virtual Studio"; font.pixelSize: 14; font.bold: true; color: window.darkMode ? "white" : "#1A1A1A"; leftPadding: 8 }
        Label { text: "Remplacez le fond par un plateau virtuel professionnel"; font.pixelSize: 11; color: window.darkMode ? "#888" : "#666"; leftPadding: 8 }

        Switch { text: "Activer le Virtual Studio"; checked: setupController.virtualStudioEnabled; onToggled: setupController.virtualStudioEnabled = checked; leftPadding: 8 }

        // ── Chroma Key ────────────────────────────────────────
        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#222" : "#CCC"; Layout.leftMargin: 8; Layout.rightMargin: 8 }
        Label { text: "Chroma Key"; font.pixelSize: 12; font.bold: true; color: window.darkMode ? "#AAA" : "#444"; leftPadding: 8 }

        Switch { text: "Activer le Chroma Key"; checked: setupController.chromaKeyEnabled; onToggled: setupController.chromaKeyEnabled = checked; leftPadding: 8 }

        RowLayout { spacing: 6; Layout.leftMargin: 8
            Label { text: "Couleur:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11 }
            Rectangle {
                Layout.preferredWidth: 60; Layout.preferredHeight: 28; radius: 6
                color: setupController.chromaKeyColor === "green" ? "#00CC44" : (window.darkMode ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.06))
                Label { anchors.centerIn: parent; text: "Vert"; font.pixelSize: 11; color: setupController.chromaKeyColor === "green" ? "white" : (window.darkMode ? "#888" : "#555") }
                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: setupController.chromaKeyColor = "green" }
            }
            Rectangle {
                Layout.preferredWidth: 60; Layout.preferredHeight: 28; radius: 6
                color: setupController.chromaKeyColor === "blue" ? "#0066CC" : (window.darkMode ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.06))
                Label { anchors.centerIn: parent; text: "Bleu"; font.pixelSize: 11; color: setupController.chromaKeyColor === "blue" ? "white" : (window.darkMode ? "#888" : "#555") }
                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: setupController.chromaKeyColor = "blue" }
            }
        }

        RowLayout { spacing: 4; Layout.leftMargin: 8
            Label { text: "Tolerance:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11 }
            Slider { from: 0.1; to: 0.8; stepSize: 0.01; value: setupController.chromaKeyTolerance; Layout.fillWidth: true; onMoved: setupController.chromaKeyTolerance = value }
            Label { text: Math.round(setupController.chromaKeyTolerance * 100) + "%"; color: window.darkMode ? "#888" : "#555"; font.pixelSize: 11; Layout.preferredWidth: 36 }
        }

        RowLayout { spacing: 4; Layout.leftMargin: 8
            Label { text: "Lissage:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11 }
            Slider { from: 0.0; to: 0.2; stepSize: 0.005; value: setupController.chromaKeySmooth; Layout.fillWidth: true; onMoved: setupController.chromaKeySmooth = value }
            Label { text: Math.round(setupController.chromaKeySmooth * 100) + "%"; color: window.darkMode ? "#888" : "#555"; font.pixelSize: 11; Layout.preferredWidth: 36 }
        }

        // ── Studio Selection ──────────────────────────────────
        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#222" : "#CCC"; Layout.leftMargin: 8; Layout.rightMargin: 8 }
        Label { text: "Plateau virtuel"; font.pixelSize: 12; font.bold: true; color: window.darkMode ? "#AAA" : "#444"; leftPadding: 8 }

        GridLayout {
            columns: 2; Layout.leftMargin: 8; Layout.rightMargin: 8; columnSpacing: 6; rowSpacing: 6

            Repeater {
                model: [
                    { sid: 0, name: "News Desk",     desc: "Plateau info classique",   c1: "#0A0F23", c2: "#1A2850" },
                    { sid: 1, name: "Morning Show",   desc: "Matinale chaleureuse",     c1: "#FFC88C", c2: "#8CB4E6" },
                    { sid: 2, name: "Sports Center",  desc: "Plateau sport dynamique",  c1: "#08080F", c2: "#C80000" },
                    { sid: 3, name: "Interview",      desc: "Entretien sobre",          c1: "#2D2A28", c2: "#C8B48C" },
                    { sid: 4, name: "Weather",        desc: "Carte meteo",              c1: "#1E50A0", c2: "#286450" },
                    { sid: 5, name: "Election",       desc: "Soiree electorale",        c1: "#B41420", c2: "#1432B4" },
                    { sid: 6, name: "Tech / Digital", desc: "Plateau high-tech",        c1: "#050A14", c2: "#00E5FF" },
                    { sid: 7, name: "Luxury",         desc: "Prestige et elegance",     c1: "#0F0C0A", c2: "#D4AF37" }
                ]
                delegate: Rectangle {
                    Layout.fillWidth: true; Layout.preferredHeight: 64; radius: 8
                    color: setupController.virtualStudioId === modelData.sid
                        ? Qt.rgba(91/255, 79/255, 219/255, 0.12)
                        : (window.darkMode ? "#111114" : "#F0F0F4")
                    border.color: setupController.virtualStudioId === modelData.sid
                        ? Qt.rgba(91/255, 79/255, 219/255, 0.3) : "transparent"

                    RowLayout {
                        anchors.fill: parent; anchors.margins: 8; spacing: 8

                        // Color swatch preview
                        Rectangle {
                            Layout.preferredWidth: 40; Layout.preferredHeight: 40; radius: 6
                            gradient: Gradient {
                                GradientStop { position: 0; color: modelData.c1 }
                                GradientStop { position: 1; color: modelData.c2 }
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true; spacing: 1
                            Label { text: modelData.name; font.pixelSize: 12; font.bold: true; color: window.darkMode ? "white" : "#1A1A1A" }
                            Label { text: modelData.desc; font.pixelSize: 10; color: window.darkMode ? "#888" : "#666" }
                        }

                        Label { text: setupController.virtualStudioId === modelData.sid ? "\u2713" : ""; color: "#5B4FDB"; font.pixelSize: 14 }
                    }

                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: setupController.virtualStudioId = modelData.sid }
                }
            }
        }

        Item { implicitHeight: 10 }
    }
    }
}
