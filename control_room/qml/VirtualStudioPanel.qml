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
        id: vsCol; width: parent.width; spacing: 10
        Item { implicitHeight: 4 }

        // ── Header ──────────────────────────────────
        RowLayout { spacing: 8; Layout.leftMargin: 12
            Rectangle {
                Layout.preferredWidth: 28; Layout.preferredHeight: 28; radius: 7
                color: Qt.rgba(108/255,92/255,231/255,0.12)
                Label { anchors.centerIn: parent; text: "\uD83C\uDFA5"; font.pixelSize: 13 }
            }
            ColumnLayout { spacing: 0
                Label { text: "Virtual Studio"; font.pixelSize: 15; font.weight: Font.Bold; color: window.darkMode ? "#F0F0F5" : "#0F0F14" }
                Label { text: "Replace background with a professional virtual set"; font.pixelSize: 10; color: window.darkMode ? "#505060" : "#999" }
            }
        }

        Switch { text: "Enable Virtual Studio"; checked: setupController.virtualStudioEnabled; onToggled: setupController.virtualStudioEnabled = checked; leftPadding: 12 }

        // ── Chroma Key ──────────────────────────────
        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#1A1A22" : "#E0E0E8"; Layout.leftMargin: 12; Layout.rightMargin: 12 }
        Label { text: "Chroma Key"; font.pixelSize: 12; font.weight: Font.DemiBold; color: window.darkMode ? "#F0F0F5" : "#0F0F14"; leftPadding: 12 }

        Switch { text: "Enable chroma key"; checked: setupController.chromaKeyEnabled; onToggled: setupController.chromaKeyEnabled = checked; leftPadding: 12 }

        RowLayout { spacing: 6; Layout.leftMargin: 12
            Label { text: "Key color:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11; Layout.preferredWidth: 55 }
            Repeater {
                model: [{ label: "Green", val: "green", c: "#00CC44" }, { label: "Blue", val: "blue", c: "#0066CC" }]
                Rectangle {
                    Layout.preferredWidth: 70; Layout.preferredHeight: 30; radius: 8
                    color: setupController.chromaKeyColor === modelData.val ? modelData.c : (window.darkMode ? Qt.rgba(255,255,255,0.03) : Qt.rgba(0,0,0,0.03))
                    Label { anchors.centerIn: parent; text: modelData.label; font.pixelSize: 11; font.weight: Font.DemiBold; color: setupController.chromaKeyColor === modelData.val ? "white" : (window.darkMode ? "#888" : "#666") }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: setupController.chromaKeyColor = modelData.val }
                }
            }
        }

        RowLayout { spacing: 4; Layout.leftMargin: 12
            Label { text: "Tolerance:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11; Layout.preferredWidth: 55 }
            Slider { from: 0.1; to: 0.8; stepSize: 0.01; value: setupController.chromaKeyTolerance; Layout.fillWidth: true; onMoved: setupController.chromaKeyTolerance = value }
            Label { text: Math.round(setupController.chromaKeyTolerance * 100) + "%"; color: window.darkMode ? "#888" : "#555"; font.pixelSize: 10; Layout.preferredWidth: 30 }
        }

        RowLayout { spacing: 4; Layout.leftMargin: 12
            Label { text: "Smooth:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11; Layout.preferredWidth: 55 }
            Slider { from: 0.0; to: 0.2; stepSize: 0.005; value: setupController.chromaKeySmooth; Layout.fillWidth: true; onMoved: setupController.chromaKeySmooth = value }
            Label { text: Math.round(setupController.chromaKeySmooth * 100) + "%"; color: window.darkMode ? "#888" : "#555"; font.pixelSize: 10; Layout.preferredWidth: 30 }
        }

        // ── Studio Templates ────────────────────────
        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#1A1A22" : "#E0E0E8"; Layout.leftMargin: 12; Layout.rightMargin: 12 }
        Label { text: "Studio templates"; font.pixelSize: 12; font.weight: Font.DemiBold; color: window.darkMode ? "#F0F0F5" : "#0F0F14"; leftPadding: 12 }

        GridLayout {
            columns: 2; Layout.leftMargin: 12; Layout.rightMargin: 12; columnSpacing: 6; rowSpacing: 6
            Repeater {
                model: [
                    { sid: 0, name: "News Desk", desc: "Classic news studio", c1: "#0A0F23", c2: "#1A2850" },
                    { sid: 1, name: "Morning Show", desc: "Warm daylight set", c1: "#FFC88C", c2: "#8CB4E6" },
                    { sid: 2, name: "Sports Center", desc: "Dynamic sports set", c1: "#08080F", c2: "#C80000" },
                    { sid: 3, name: "Interview", desc: "Elegant interview set", c1: "#2D2A28", c2: "#C8B48C" },
                    { sid: 4, name: "Weather Map", desc: "Weather forecast set", c1: "#1E50A0", c2: "#286450" },
                    { sid: 5, name: "Election", desc: "Election night set", c1: "#B41420", c2: "#1432B4" },
                    { sid: 6, name: "Tech / Digital", desc: "High-tech studio", c1: "#050A14", c2: "#00E5FF" },
                    { sid: 7, name: "Luxury", desc: "Premium elegant set", c1: "#0F0C0A", c2: "#D4AF37" }
                ]
                delegate: Rectangle {
                    Layout.fillWidth: true; Layout.preferredHeight: 56; radius: 8
                    color: setupController.virtualStudioId === modelData.sid ? Qt.rgba(108/255,92/255,231/255,0.12) : (window.darkMode ? Qt.rgba(255,255,255,0.02) : Qt.rgba(0,0,0,0.02))
                    border.color: setupController.virtualStudioId === modelData.sid ? Qt.rgba(108/255,92/255,231/255,0.3) : "transparent"
                    Behavior on color { ColorAnimation { duration: 150 } }
                    RowLayout {
                        anchors.fill: parent; anchors.margins: 8; spacing: 8
                        Rectangle { Layout.preferredWidth: 36; Layout.preferredHeight: 36; radius: 6; gradient: Gradient { GradientStop { position: 0; color: modelData.c1 }
                            GradientStop { position: 1; color: modelData.c2 } } }
                        ColumnLayout { Layout.fillWidth: true; spacing: 1
                            Label { text: modelData.name; font.pixelSize: 11; font.weight: Font.Bold; color: window.darkMode ? "#F0F0F5" : "#0F0F14" }
                            Label { text: modelData.desc; font.pixelSize: 9; color: window.darkMode ? "#505060" : "#999" }
                        }
                        Label { text: setupController.virtualStudioId === modelData.sid ? "\u2713" : ""; color: "#6C5CE7"; font.pixelSize: 14 }
                    }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: setupController.virtualStudioId = modelData.sid }
                }
            }
        }

        // ── Customization ───────────────────────────
        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#1A1A22" : "#E0E0E8"; Layout.leftMargin: 12; Layout.rightMargin: 12 }
        Label { text: "Customization"; font.pixelSize: 12; font.weight: Font.DemiBold; color: window.darkMode ? "#F0F0F5" : "#0F0F14"; leftPadding: 12 }

        RowLayout { spacing: 4; Layout.leftMargin: 12
            Label { text: "Lighting:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11; Layout.preferredWidth: 55 }
            Slider { from: 0.2; to: 2.0; stepSize: 0.1; value: setupController.vsLightIntensity; Layout.fillWidth: true; onMoved: setupController.vsLightIntensity = value }
            Label { text: Math.round(setupController.vsLightIntensity * 100) + "%"; color: window.darkMode ? "#888" : "#555"; font.pixelSize: 10; Layout.preferredWidth: 30 }
        }

        Switch { text: "Studio animations"; checked: setupController.vsAnimationsEnabled; onToggled: setupController.vsAnimationsEnabled = checked; leftPadding: 12 }

        // ── Custom Background ───────────────────────
        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#1A1A22" : "#E0E0E8"; Layout.leftMargin: 12; Layout.rightMargin: 12 }
        Label { text: "Custom background"; font.pixelSize: 12; font.weight: Font.DemiBold; color: window.darkMode ? "#F0F0F5" : "#0F0F14"; leftPadding: 12 }
        Label { text: "Import a 4K image to replace the template"; font.pixelSize: 10; color: window.darkMode ? "#505060" : "#999"; leftPadding: 12 }

        RowLayout { spacing: 6; Layout.leftMargin: 12; Layout.rightMargin: 12
            TextField {
                Layout.fillWidth: true; text: setupController.vsCustomBackground
                placeholderText: "path/to/background.png (4K recommended)"
                onTextEdited: setupController.vsCustomBackground = text
                font.pixelSize: 11; color: window.darkMode ? "#CCC" : "#333"
                background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 6; border.color: window.darkMode ? "#333" : "#CCC" }
            }
            Rectangle {
                Layout.preferredWidth: 50; Layout.preferredHeight: 28; radius: 6
                color: window.darkMode ? Qt.rgba(255,255,255,0.04) : Qt.rgba(0,0,0,0.06)
                visible: setupController.vsCustomBackground !== ""
                Label { anchors.centerIn: parent; text: "Clear"; font.pixelSize: 10; color: window.darkMode ? "#AAA" : "#555" }
                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: setupController.vsCustomBackground = "" }
            }
        }
        Label {
            visible: setupController.vsCustomBackground !== ""
            text: "Custom image active — template overridden"
            font.pixelSize: 10; color: "#00D68F"; leftPadding: 12
        }

        Item { implicitHeight: 12 }
    }
    }
}
