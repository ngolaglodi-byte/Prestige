import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: sub

    Flickable {
        anchors.fill: parent; contentHeight: subCol.implicitHeight; clip: true
        flickableDirection: Flickable.VerticalFlick
        ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
    ColumnLayout {
        id: subCol; width: parent.width; spacing: 8
        Item { implicitHeight: 4 }

        // ── Header ────────────────────────────────────────────
        RowLayout { spacing: 8; Layout.leftMargin: 8
            Label { text: "Sous-titrage IA"; font.pixelSize: 14; font.bold: true; color: window.darkMode ? "white" : "#1A1A1A" }
            Rectangle {
                Layout.preferredWidth: statusLbl.implicitWidth + 14; Layout.preferredHeight: 18; radius: 9
                color: subtitleController.whisperReady ? Qt.rgba(29/255,185/255,84/255,0.2) : Qt.rgba(1,1,1,0.06)
                Label {
                    id: statusLbl; anchors.centerIn: parent
                    text: subtitleController.whisperReady ? "Whisper OK" : "Whisper..."
                    font.pixelSize: 8; color: subtitleController.whisperReady ? "#1DB954" : "#666"
                }
            }
        }

        // ── Enable toggle ─────────────────────────────────────
        Switch {
            text: "Activer les sous-titres"
            checked: subtitleController.enabled
            onToggled: subtitleController.enabled = checked
            leftPadding: 8
        }

        // ── Model & Language ──────────────────────────────────
        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#222" : "#CCC"; Layout.leftMargin: 8; Layout.rightMargin: 8 }
        Label { text: "Moteur Whisper"; font.pixelSize: 11; color: window.darkMode ? "#AAA" : "#444"; leftPadding: 8 }

        RowLayout { spacing: 4; Layout.leftMargin: 8
            Label { text: "Modele:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 10 }
            ComboBox {
                id: modelCombo
                model: ["tiny", "base", "small", "medium"]
                currentIndex: {
                    var m = subtitleController.whisperModel
                    var idx = ["tiny", "base", "small", "medium"].indexOf(m)
                    return idx >= 0 ? idx : 1
                }
                onCurrentTextChanged: subtitleController.whisperModel = currentText
                Layout.fillWidth: true
                background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
            }
        }
        Label {
            text: {
                var sizes = {"tiny": "39 Mo, rapide", "base": "74 Mo, equilibre", "small": "244 Mo, precis", "medium": "769 Mo, tres precis"}
                return "  " + (sizes[modelCombo.currentText] || "")
            }
            font.pixelSize: 9; color: window.darkMode ? "#555" : "#999"; leftPadding: 8
        }

        RowLayout { spacing: 4; Layout.leftMargin: 8
            Label { text: "Langue:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 10 }
            ComboBox {
                id: langCombo
                model: ["Auto", "Francais", "English", "Espanol", "Deutsch", "Arabic", "Chinese"]
                currentIndex: {
                    var codes = {"auto": 0, "fr": 1, "en": 2, "es": 3, "de": 4, "ar": 5, "zh": 6}
                    return codes[subtitleController.language] || 0
                }
                onCurrentIndexChanged: {
                    var langCodes = ["auto", "fr", "en", "es", "de", "ar", "zh"]
                    subtitleController.language = langCodes[currentIndex]
                }
                Layout.fillWidth: true
                background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
            }
        }

        // ── Appearance ────────────────────────────────────────
        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#222" : "#CCC"; Layout.leftMargin: 8; Layout.rightMargin: 8 }
        Label { text: "Apparence"; font.pixelSize: 11; color: window.darkMode ? "#AAA" : "#444"; leftPadding: 8 }

        RowLayout { spacing: 4; Layout.leftMargin: 8
            Label { text: "Taille:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 10 }
            Slider {
                from: 12; to: 36; stepSize: 1
                value: subtitleController.fontSize
                Layout.fillWidth: true
                onMoved: subtitleController.fontSize = value
            }
            Label { text: subtitleController.fontSize + "px"; color: window.darkMode ? "#888" : "#555"; font.pixelSize: 10 }
        }
        RowLayout { spacing: 4; Layout.leftMargin: 8
            Label { text: "Position:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 10 }
            ComboBox {
                model: ["Bas", "Haut"]; Layout.fillWidth: true
                currentIndex: subtitleController.position === "top" ? 1 : 0
                onCurrentIndexChanged: subtitleController.position = (currentIndex === 0 ? "bottom" : "top")
                background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
            }
        }
        RowLayout { spacing: 4; Layout.leftMargin: 8
            Label { text: "Opacite fond:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 10 }
            Slider {
                from: 0.2; to: 1.0
                value: subtitleController.bgOpacity
                Layout.fillWidth: true
                onMoved: subtitleController.bgOpacity = value
            }
            Label { text: Math.round(subtitleController.bgOpacity * 100) + "%"; color: window.darkMode ? "#888" : "#555"; font.pixelSize: 10 }
        }
        RowLayout { spacing: 4; Layout.leftMargin: 8
            Label { text: "Texte:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 10 }
            Repeater {
                model: ["#FFFFFF", "#FFFF00", "#00FF00"]
                Rectangle {
                    width: 18; height: 18; radius: 4; color: modelData
                    border.color: modelData === subtitleController.textColor ? "white" : "transparent"; border.width: 2
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: subtitleController.textColor = modelData }
                }
            }
        }

        // ── Live Preview ──────────────────────────────────────
        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#222" : "#CCC"; Layout.leftMargin: 8; Layout.rightMargin: 8 }
        Rectangle {
            Layout.fillWidth: true; Layout.leftMargin: 8; Layout.rightMargin: 8
            Layout.preferredHeight: 44; radius: 6
            color: Qt.rgba(0, 0, 0, subtitleController.bgOpacity)
            Label {
                anchors.centerIn: parent; width: parent.width - 16
                text: subtitleController.currentText ? subtitleController.currentText : "Les sous-titres apparaitront ici..."
                color: subtitleController.currentText ? subtitleController.textColor : "#555"
                font.pixelSize: subtitleController.fontSize * 0.7
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                elide: Text.ElideRight
                maximumLineCount: 2
            }
        }

        // ── Confidence badge ──────────────────────────────────
        RowLayout { spacing: 6; Layout.leftMargin: 8
            Label { text: "Confiance:"; color: window.darkMode ? "#555" : "#999"; font.pixelSize: 9 }
            Rectangle {
                Layout.preferredWidth: 40; Layout.preferredHeight: 14; radius: 3
                color: subtitleController.confidence > 0.8 ? Qt.rgba(29/255,185/255,84/255,0.2) :
                       subtitleController.confidence > 0.5 ? Qt.rgba(1,0.7,0,0.2) : Qt.rgba(1,0,0,0.2)
                Label {
                    anchors.centerIn: parent
                    text: Math.round(subtitleController.confidence * 100) + "%"
                    font.pixelSize: 8; color: window.darkMode ? "#AAA" : "#444"
                }
            }
            Item { Layout.fillWidth: true }
            Label { text: "Moteur: Whisper IA (local)"; font.pixelSize: 9; color: window.darkMode ? "#555" : "#999" }
        }
        Item { implicitHeight: 10 }
    }
    }
}
