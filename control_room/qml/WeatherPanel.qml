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
        id: wCol; width: parent.width; spacing: 8
        Item { implicitHeight: 4 }

        // ── Header ────────────────────────────────────────────
        RowLayout { spacing: 8; Layout.leftMargin: 8
            Label { text: "Meteo"; font.pixelSize: 14; font.bold: true; color: window.darkMode ? "white" : "#1A1A1A" }
            Rectangle {
                Layout.preferredWidth: wBadge.implicitWidth + 14; Layout.preferredHeight: 18; radius: 9
                color: weatherFetcher.autoMode ? Qt.rgba(29/255,185/255,84/255,0.2) : Qt.rgba(1,1,1,0.06)
                Label {
                    id: wBadge; anchors.centerIn: parent
                    text: weatherFetcher.autoMode ? "Auto" : "Manuel"
                    font.pixelSize: 10; color: weatherFetcher.autoMode ? "#1DB954" : "#666"
                }
            }
        }

        Switch { text: "Afficher sur la sortie"; checked: setupController.weatherVisible; onToggled: setupController.weatherVisible = checked; leftPadding: 8 }

        RowLayout { spacing: 4; Layout.leftMargin: 8
            Label { text: "Design:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11 }
            ComboBox { model: ["Shadow Only","Glass Dark","Pill Frost","Minimal","Card Dark","Neon Glow","Solid Dark","Transparent"]
                property var vals: ["wt_shadow_only","wt_glass_dark","wt_pill_frost","wt_minimal","wt_card_dark","wt_neon_glow","wt_solid_dark","wt_transparent"]
                currentIndex: Math.max(0, vals.indexOf(setupController.weatherDesign)); Layout.fillWidth: true
                onActivated: setupController.weatherDesign = vals[currentIndex]
                background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
            }
        }

        // ── Mode toggle ───────────────────────────────────────
        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#222" : "#CCC"; Layout.leftMargin: 8; Layout.rightMargin: 8 }

        RowLayout { spacing: 8; Layout.leftMargin: 8
            Label { text: "Mode:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11 }
            Rectangle {
                Layout.preferredWidth: 70; Layout.preferredHeight: 24; radius: 4
                color: !weatherFetcher.autoMode ? "#5B4FDB" : (window.darkMode ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.06))
                Label { anchors.centerIn: parent; text: "Manuel"; font.pixelSize: 11; color: !weatherFetcher.autoMode ? "white" : (window.darkMode ? "#888" : "#555") }
                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: weatherFetcher.autoMode = false }
            }
            Rectangle {
                Layout.preferredWidth: 70; Layout.preferredHeight: 24; radius: 4
                color: weatherFetcher.autoMode ? "#5B4FDB" : (window.darkMode ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.06))
                Label { anchors.centerIn: parent; text: "Auto API"; font.pixelSize: 11; color: weatherFetcher.autoMode ? "white" : (window.darkMode ? "#888" : "#555") }
                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: weatherFetcher.autoMode = true }
            }
        }

        // ── Auto mode config ──────────────────────────────────
        ColumnLayout {
            visible: weatherFetcher.autoMode; spacing: 6
            Layout.fillWidth: true

            // ── API Key with instructions ──────────────────
            Rectangle {
                Layout.fillWidth: true; Layout.leftMargin: 8; Layout.rightMargin: 8
                Layout.preferredHeight: instrCol.implicitHeight + 16; radius: 6
                color: Qt.rgba(91/255, 79/255, 219/255, 0.06)
                border.color: Qt.rgba(91/255, 79/255, 219/255, 0.15)
                ColumnLayout {
                    id: instrCol; anchors.fill: parent; anchors.margins: 8; spacing: 4
                    Label { text: "Comment obtenir une cle API (gratuit) :"; font.pixelSize: 11; font.bold: true; color: window.darkMode ? "#AAA" : "#444" }
                    Label { text: "1. Allez sur  openweathermap.org/appid"; font.pixelSize: 11; color: window.darkMode ? "#888" : "#555" }
                    Label { text: "2. Creez un compte gratuit (email + mot de passe)"; font.pixelSize: 11; color: window.darkMode ? "#888" : "#555" }
                    Label { text: "3. Dans 'API Keys', copiez votre cle"; font.pixelSize: 11; color: window.darkMode ? "#888" : "#555" }
                    Label { text: "4. Collez-la ci-dessous — la meteo sera automatique"; font.pixelSize: 11; color: window.darkMode ? "#888" : "#555" }
                    Label { text: "Gratuit : 1 000 appels/jour — largement suffisant"; font.pixelSize: 10; color: "#5B4FDB" }
                }
            }

            RowLayout { spacing: 4; Layout.leftMargin: 8; Layout.rightMargin: 8
                Label { text: "Cle API:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11 }
                TextField {
                    Layout.fillWidth: true
                    text: weatherFetcher.apiKey
                    onTextChanged: weatherFetcher.apiKey = text
                    placeholderText: "Collez votre cle API ici"
                    font.pixelSize: 11; color: window.darkMode ? "#CCC" : "#333"
                    background: Rectangle {
                        color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4
                        border.color: weatherFetcher.apiKey.length > 0 ? "#1DB954" : (window.darkMode ? "#333" : "#CCC")
                        border.width: weatherFetcher.apiKey.length > 0 ? 1.5 : 1
                    }
                }
            }
            // API key status badge
            RowLayout { spacing: 6; Layout.leftMargin: 8
                Rectangle {
                    Layout.preferredWidth: 8; Layout.preferredHeight: 8; radius: 4
                    color: weatherFetcher.apiKey.length > 20 ? "#1DB954" : (weatherFetcher.apiKey.length > 0 ? "#FF6B00" : "#CC3333")
                }
                Label {
                    text: {
                        if (weatherFetcher.apiKey.length === 0) return "Aucune cle — saisissez votre cle OpenWeatherMap"
                        if (weatherFetcher.apiKey.length < 20) return "Cle incomplete — une cle valide fait 32 caracteres"
                        return "Cle configuree — prete a utiliser"
                    }
                    font.pixelSize: 11
                    color: weatherFetcher.apiKey.length > 20 ? "#1DB954" : "#888"
                }
            }

            RowLayout { spacing: 4; Layout.leftMargin: 8
                Label { text: "Ville:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11 }
                TextField {
                    Layout.fillWidth: true
                    text: weatherFetcher.city
                    onTextChanged: weatherFetcher.city = text
                    placeholderText: "Paris"
                    font.pixelSize: 11; color: window.darkMode ? "#CCC" : "#333"
                    background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
                }
                Rectangle {
                    Layout.preferredWidth: 24; Layout.preferredHeight: 24; radius: 4
                    color: fetchMa.containsMouse ? (window.darkMode ? Qt.rgba(1,1,1,0.08) : Qt.rgba(0,0,0,0.08)) : (window.darkMode ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.06))
                    Label { anchors.centerIn: parent; text: "\u21BB"; color: weatherFetcher.fetching ? "#5B4FDB" : (window.darkMode ? "#888" : "#555"); font.pixelSize: 14 }
                    MouseArea { id: fetchMa; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: weatherFetcher.fetchNow() }
                }
            }

            RowLayout { spacing: 4; Layout.leftMargin: 8
                Label { text: "Refresh:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11 }
                ComboBox {
                    model: ["5 min", "15 min", "30 min", "60 min"]
                    currentIndex: 1
                    onCurrentIndexChanged: {
                        var mins = [5, 15, 30, 60]
                        weatherFetcher.refreshMinutes = mins[currentIndex]
                    }
                    Layout.preferredWidth: 80
                    background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
                }
                Label { text: "Unite:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11; Layout.leftMargin: 8 }
                ComboBox {
                    model: ["\u00B0C", "\u00B0F"]
                    currentIndex: weatherFetcher.unit.indexOf("F") >= 0 ? 1 : 0
                    onCurrentTextChanged: weatherFetcher.unit = currentText
                    Layout.preferredWidth: 60
                    background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
                }
            }

            // Status
            RowLayout { spacing: 6; Layout.leftMargin: 8
                Label { text: "MAJ:"; color: window.darkMode ? "#555" : "#999"; font.pixelSize: 11 }
                Label { text: weatherFetcher.lastUpdate; color: window.darkMode ? "#888" : "#555"; font.pixelSize: 11 }
                Item { Layout.fillWidth: true }
                Label { visible: weatherFetcher.country !== ""; text: weatherFetcher.country; color: window.darkMode ? "#888" : "#555"; font.pixelSize: 11 }
            }

            // Error
            Label {
                visible: weatherFetcher.errorString !== ""
                text: "Erreur: " + weatherFetcher.errorString
                font.pixelSize: 11; color: "#CC3333"; leftPadding: 8
                Layout.fillWidth: true; wrapMode: Text.WordWrap
            }
        }

        // ── Manual mode config ────────────────────────────────
        ColumnLayout {
            visible: !weatherFetcher.autoMode; spacing: 6
            Layout.fillWidth: true

            RowLayout { spacing: 4; Layout.leftMargin: 8
                Label { text: "Ville:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11 }
                TextField {
                    Layout.fillWidth: true
                    text: weatherFetcher.city
                    onTextChanged: weatherFetcher.city = text
                    font.pixelSize: 11; color: window.darkMode ? "#CCC" : "#333"
                    background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
                }
            }

            RowLayout { spacing: 4; Layout.leftMargin: 8
                Label { text: "Temperature:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11 }
                SpinBox {
                    id: tempSpin; from: -40; to: 50
                    value: Math.round(weatherFetcher.temperature)
                    onValueModified: weatherFetcher.temperature = value
                    Layout.preferredWidth: 100
                    background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
                    contentItem: Label {
                        text: tempSpin.textFromValue(tempSpin.value, tempSpin.locale)
                        color: "#FFF"; font.pixelSize: 12
                        horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                    }
                }
                ComboBox {
                    model: ["\u00B0C", "\u00B0F"]; Layout.preferredWidth: 60
                    currentIndex: weatherFetcher.unit.indexOf("F") >= 0 ? 1 : 0
                    onCurrentTextChanged: weatherFetcher.unit = currentText
                    background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
                }
            }

            RowLayout { spacing: 4; Layout.leftMargin: 8
                Label { text: "Conditions:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11 }
                ComboBox {
                    Layout.fillWidth: true
                    model: ["Ensoleille", "Nuageux", "Pluie", "Neige", "Orage", "Brouillard"]
                    property var vals: ["ensoleille", "nuageux", "pluie", "neige", "orage", "brouillard"]
                    onActivated: weatherFetcher.condition = vals[currentIndex]
                    background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
                }
            }
        }

        // ── Preview ───────────────────────────────────────────
        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#222" : "#CCC"; Layout.leftMargin: 8; Layout.rightMargin: 8 }

        Rectangle {
            Layout.fillWidth: true; Layout.leftMargin: 8; Layout.rightMargin: 8
            Layout.preferredHeight: 72; radius: 8
            color: Qt.rgba(0, 0, 0, 0.6)

            RowLayout {
                anchors.fill: parent; anchors.margins: 10; spacing: 12

                // Weather icon
                Label {
                    text: weatherFetcher.conditionIcon
                    font.pixelSize: 36; color: "#FFD700"
                }

                // Main info
                ColumnLayout {
                    Layout.fillWidth: true; spacing: 2
                    Label {
                        text: weatherFetcher.city
                        font.pixelSize: 14; font.bold: true; color: "white"  // Preview text stays white on dark preview bg
                    }
                    Label {
                        text: Math.round(weatherFetcher.temperature) + weatherFetcher.unit + "  ·  " + weatherFetcher.description
                        font.pixelSize: 11; color: "#CCC"
                    }
                    RowLayout {
                        visible: weatherFetcher.autoMode && weatherFetcher.humidity > 0
                        spacing: 8
                        Label { text: "Ressenti " + Math.round(weatherFetcher.feelsLike) + weatherFetcher.unit; font.pixelSize: 11; color: "#888" }
                        Label { text: "Humidite " + weatherFetcher.humidity + "%"; font.pixelSize: 11; color: "#888" }
                        Label { text: "Vent " + weatherFetcher.windSpeed + " km/h " + weatherFetcher.windDir; font.pixelSize: 11; color: "#888" }
                    }
                }
            }
        }

        Item { implicitHeight: 10 }
    }
    }
}
