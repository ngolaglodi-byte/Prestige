import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: weatherRoot

    Flickable {
        anchors.fill: parent; contentHeight: wCol.implicitHeight; clip: true
        flickableDirection: Flickable.VerticalFlick
        ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
    ColumnLayout {
        id: wCol; width: parent.width; spacing: 10
        Item { implicitHeight: 4 }

        // ── Header ──────────────────────────────────
        RowLayout { spacing: 8; Layout.leftMargin: 12
            Rectangle {
                Layout.preferredWidth: 28; Layout.preferredHeight: 28; radius: 7
                color: Qt.rgba(108/255,92/255,231/255,0.12)
                Label { anchors.centerIn: parent; text: "\u2601"; font.pixelSize: 14 }
            }
            ColumnLayout { spacing: 0
                Label { text: "Weather"; font.pixelSize: 15; font.weight: Font.Bold; color: window.darkMode ? "#F0F0F5" : "#0F0F14" }
                Label {
                    text: weatherFetcher.autoMode ? "API mode — " + (weatherFetcher.apiKey.length > 20 ? "connected" : "no key") : "Manual mode"
                    font.pixelSize: 10; color: weatherFetcher.autoMode && weatherFetcher.apiKey.length > 20 ? "#00D68F" : (window.darkMode ? "#505060" : "#999")
                }
            }
        }

        // ── ON/OFF ──────────────────────────────────
        Switch { text: "Show weather overlay"; checked: setupController.weatherVisible; onToggled: setupController.weatherVisible = checked; leftPadding: 12 }

        // ── Mode ────────────────────────────────────
        RowLayout { spacing: 6; Layout.leftMargin: 12
            Label { text: "Mode:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11; Layout.preferredWidth: 55 }
            Repeater {
                model: [{ label: "Manual", auto: false }, { label: "API Auto", auto: true }]
                Rectangle {
                    Layout.preferredWidth: 80; Layout.preferredHeight: 30; radius: 8
                    color: weatherFetcher.autoMode === modelData.auto
                        ? Qt.rgba(108/255,92/255,231/255,0.18)
                        : (window.darkMode ? Qt.rgba(255,255,255,0.03) : Qt.rgba(0,0,0,0.03))
                    border.color: weatherFetcher.autoMode === modelData.auto ? Qt.rgba(108/255,92/255,231/255,0.4) : "transparent"
                    Label { anchors.centerIn: parent; text: modelData.label; font.pixelSize: 11; font.weight: Font.DemiBold; color: weatherFetcher.autoMode === modelData.auto ? "#6C5CE7" : (window.darkMode ? "#888" : "#666") }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: weatherFetcher.autoMode = modelData.auto }
                }
            }
        }

        // ── Design ──────────────────────────────────
        RowLayout { spacing: 6; Layout.leftMargin: 12
            Label { text: "Design:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11; Layout.preferredWidth: 55 }
            ComboBox {
                model: ["Shadow Only","Glass Dark","Pill Frost","Minimal","Card Dark","Neon Glow","Solid Dark","Transparent"]
                property var vals: ["wt_shadow_only","wt_glass_dark","wt_pill_frost","wt_minimal","wt_card_dark","wt_neon_glow","wt_solid_dark","wt_transparent"]
                currentIndex: Math.max(0, vals.indexOf(setupController.weatherDesign)); Layout.fillWidth: true
                onActivated: setupController.weatherDesign = vals[currentIndex]
                background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 6; border.color: window.darkMode ? "#333" : "#CCC" }
            }
        }

        // ── API Config (auto mode) ──────────────────
        ColumnLayout {
            visible: weatherFetcher.autoMode; spacing: 8; Layout.fillWidth: true

            Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#1A1A22" : "#E0E0E8"; Layout.leftMargin: 12; Layout.rightMargin: 12 }
            Label { text: "OpenWeatherMap API"; font.pixelSize: 12; font.weight: Font.DemiBold; color: window.darkMode ? "#F0F0F5" : "#0F0F14"; leftPadding: 12 }

            RowLayout { spacing: 6; Layout.leftMargin: 12; Layout.rightMargin: 12
                Label { text: "Key:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11; Layout.preferredWidth: 55 }
                TextField {
                    Layout.fillWidth: true; text: weatherFetcher.apiKey; onTextChanged: weatherFetcher.apiKey = text
                    placeholderText: "Paste your API key here"
                    font.pixelSize: 11; font.family: "SF Mono, Menlo, monospace"; color: window.darkMode ? "#CCC" : "#333"
                    background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 6; border.color: weatherFetcher.apiKey.length > 20 ? "#00D68F" : (window.darkMode ? "#333" : "#CCC") }
                }
            }

            // Status
            RowLayout { spacing: 6; Layout.leftMargin: 12
                Rectangle { Layout.preferredWidth: 6; Layout.preferredHeight: 6; radius: 3; color: weatherFetcher.apiKey.length > 20 ? "#00D68F" : "#FF3D71" }
                Label {
                    text: weatherFetcher.apiKey.length === 0 ? "No API key" : (weatherFetcher.apiKey.length < 20 ? "Key incomplete (32 chars needed)" : "Key configured")
                    font.pixelSize: 10; color: window.darkMode ? "#666" : "#999"
                }
            }

            RowLayout { spacing: 6; Layout.leftMargin: 12; Layout.rightMargin: 12
                Label { text: "City:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11; Layout.preferredWidth: 55 }
                TextField {
                    Layout.fillWidth: true; text: weatherFetcher.city; onTextChanged: weatherFetcher.city = text
                    placeholderText: "Paris, London, New York..."; font.pixelSize: 12; color: window.darkMode ? "#CCC" : "#333"
                    background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 6; border.color: window.darkMode ? "#333" : "#CCC" }
                }
                Rectangle {
                    Layout.preferredWidth: 30; Layout.preferredHeight: 30; radius: 6
                    color: fetchMa.containsMouse ? (window.darkMode ? Qt.rgba(255,255,255,0.08) : Qt.rgba(0,0,0,0.08)) : (window.darkMode ? Qt.rgba(255,255,255,0.03) : Qt.rgba(0,0,0,0.03))
                    Label { anchors.centerIn: parent; text: "\u21BB"; color: weatherFetcher.fetching ? "#6C5CE7" : (window.darkMode ? "#888" : "#555"); font.pixelSize: 16 }
                    MouseArea { id: fetchMa; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: weatherFetcher.fetchNow() }
                }
            }

            RowLayout { spacing: 6; Layout.leftMargin: 12
                Label { text: "Unit:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11; Layout.preferredWidth: 55 }
                Repeater {
                    model: [{ label: "\u00B0C", val: "\u00B0C" }, { label: "\u00B0F", val: "\u00B0F" }]
                    Rectangle {
                        Layout.preferredWidth: 45; Layout.preferredHeight: 28; radius: 6
                        color: weatherFetcher.unit === modelData.val ? Qt.rgba(108/255,92/255,231/255,0.18) : (window.darkMode ? Qt.rgba(255,255,255,0.03) : Qt.rgba(0,0,0,0.03))
                        Label { anchors.centerIn: parent; text: modelData.label; font.pixelSize: 12; font.weight: Font.Bold; color: weatherFetcher.unit === modelData.val ? "#6C5CE7" : (window.darkMode ? "#888" : "#666") }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: weatherFetcher.unit = modelData.val }
                    }
                }
                Item { Layout.fillWidth: true }
                Label { text: "Refresh:"; color: window.darkMode ? "#666" : "#999"; font.pixelSize: 10 }
                ComboBox {
                    model: ["5 min","15 min","30 min","60 min"]; currentIndex: 1; Layout.preferredWidth: 70
                    onCurrentIndexChanged: { var m = [5,15,30,60]; weatherFetcher.refreshMinutes = m[currentIndex] }
                    background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
                }
            }

            // Last update
            RowLayout { spacing: 6; Layout.leftMargin: 12
                Rectangle { Layout.preferredWidth: 6; Layout.preferredHeight: 6; radius: 3; color: weatherFetcher.lastUpdate !== "" ? "#00D68F" : (window.darkMode ? "#333" : "#CCC") }
                Label { text: weatherFetcher.lastUpdate !== "" ? ("Updated: " + weatherFetcher.lastUpdate) : "Not yet fetched"; font.pixelSize: 10; color: window.darkMode ? "#666" : "#999" }
            }

            Label { visible: weatherFetcher.errorString !== ""; text: "Error: " + weatherFetcher.errorString; font.pixelSize: 10; color: "#FF3D71"; leftPadding: 12; Layout.fillWidth: true; wrapMode: Text.WordWrap }
        }

        // ── Manual Config ───────────────────────────
        ColumnLayout {
            visible: !weatherFetcher.autoMode; spacing: 8; Layout.fillWidth: true

            Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#1A1A22" : "#E0E0E8"; Layout.leftMargin: 12; Layout.rightMargin: 12 }
            Label { text: "Manual weather data"; font.pixelSize: 12; font.weight: Font.DemiBold; color: window.darkMode ? "#F0F0F5" : "#0F0F14"; leftPadding: 12 }

            RowLayout { spacing: 6; Layout.leftMargin: 12; Layout.rightMargin: 12
                Label { text: "City:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11; Layout.preferredWidth: 55 }
                TextField {
                    Layout.fillWidth: true; text: weatherFetcher.city; onTextChanged: weatherFetcher.city = text
                    font.pixelSize: 12; color: window.darkMode ? "#CCC" : "#333"
                    background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 6; border.color: window.darkMode ? "#333" : "#CCC" }
                }
            }

            RowLayout { spacing: 6; Layout.leftMargin: 12
                Label { text: "Temp:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11; Layout.preferredWidth: 55 }
                SpinBox {
                    id: tempSpin; from: -40; to: 50; value: Math.round(weatherFetcher.temperature)
                    onValueModified: weatherFetcher.temperature = value; Layout.preferredWidth: 100
                    background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 6; border.color: window.darkMode ? "#333" : "#CCC" }
                    contentItem: Label { text: tempSpin.textFromValue(tempSpin.value, tempSpin.locale); color: window.darkMode ? "#FFF" : "#1A1A1A"; font.pixelSize: 13; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                }
                Repeater {
                    model: [{ label: "\u00B0C", val: "\u00B0C" }, { label: "\u00B0F", val: "\u00B0F" }]
                    Rectangle {
                        Layout.preferredWidth: 40; Layout.preferredHeight: 28; radius: 6
                        color: weatherFetcher.unit === modelData.val ? Qt.rgba(108/255,92/255,231/255,0.18) : (window.darkMode ? Qt.rgba(255,255,255,0.03) : Qt.rgba(0,0,0,0.03))
                        Label { anchors.centerIn: parent; text: modelData.label; font.pixelSize: 12; font.weight: Font.Bold; color: weatherFetcher.unit === modelData.val ? "#6C5CE7" : (window.darkMode ? "#888" : "#666") }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: weatherFetcher.unit = modelData.val }
                    }
                }
            }

            RowLayout { spacing: 6; Layout.leftMargin: 12
                Label { text: "Cond:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11; Layout.preferredWidth: 55 }
                ComboBox {
                    Layout.fillWidth: true
                    model: ["Sunny","Cloudy","Rain","Snow","Storm","Fog"]
                    property var vals: ["ensoleille","nuageux","pluie","neige","orage","brouillard"]
                    onActivated: weatherFetcher.condition = vals[currentIndex]
                    background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 6; border.color: window.darkMode ? "#333" : "#CCC" }
                }
            }
        }

        // ── Preview ─────────────────────────────────
        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#1A1A22" : "#E0E0E8"; Layout.leftMargin: 12; Layout.rightMargin: 12 }
        Label { text: "Preview"; font.pixelSize: 12; font.weight: Font.DemiBold; color: window.darkMode ? "#F0F0F5" : "#0F0F14"; leftPadding: 12 }

        Rectangle {
            Layout.fillWidth: true; Layout.leftMargin: 12; Layout.rightMargin: 12
            Layout.preferredHeight: 72; radius: 8
            color: Qt.rgba(0, 0, 0, 0.6)

            RowLayout {
                anchors.fill: parent; anchors.margins: 12; spacing: 12
                Label { text: weatherFetcher.conditionIcon; font.pixelSize: 32; color: "#FFD700" }
                ColumnLayout { Layout.fillWidth: true; spacing: 2
                    Label { text: weatherFetcher.city || "City"; font.pixelSize: 14; font.weight: Font.Bold; color: "white" }
                    Label { text: Math.round(weatherFetcher.temperature) + weatherFetcher.unit + "  \u00B7  " + weatherFetcher.description; font.pixelSize: 11; color: "#CCC" }
                    RowLayout {
                        visible: weatherFetcher.autoMode && weatherFetcher.humidity > 0; spacing: 8
                        Label { text: "Feels " + Math.round(weatherFetcher.feelsLike) + weatherFetcher.unit; font.pixelSize: 10; color: "#888" }
                        Label { text: "Humidity " + weatherFetcher.humidity + "%"; font.pixelSize: 10; color: "#888" }
                        Label { text: "Wind " + weatherFetcher.windSpeed + " km/h"; font.pixelSize: 10; color: "#888" }
                    }
                }
            }
        }

        Item { implicitHeight: 12 }
    }
    }
}
