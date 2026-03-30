import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: tk
    property bool showOnOutput: false
    property string manualText: ""
    property int scrollSpeed: 2
    property string bgColor: "#CC0000"
    property string textColor: "#FFFFFF"
    property int fontSize: 14

    Flickable {
        anchors.fill: parent; contentHeight: tkCol.implicitHeight; clip: true
        flickableDirection: Flickable.VerticalFlick
        ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
    ColumnLayout {
        id: tkCol; width: parent.width; spacing: 8
        Item { implicitHeight: 4 }

        // ── Header ────────────────────────────────────────────
        RowLayout { spacing: 8; Layout.leftMargin: 8
            Label { text: "Ticker / Bandeau"; font.pixelSize: 14; font.bold: true; color: window.darkMode ? "white" : "#1A1A1A" }
            Rectangle {
                Layout.preferredWidth: feedBadge.implicitWidth + 14; Layout.preferredHeight: 18; radius: 9
                color: rssFetcher.feedCount > 0 ? Qt.rgba(29/255,185/255,84/255,0.2) : Qt.rgba(1,1,1,0.06)
                Label {
                    id: feedBadge; anchors.centerIn: parent
                    text: rssFetcher.feedCount > 0 ? rssFetcher.feedCount + " titres" : "Aucun flux"
                    font.pixelSize: 8; color: rssFetcher.feedCount > 0 ? "#1DB954" : "#666"
                }
            }
        }

        Switch { text: "Afficher le ticker"; checked: tk.showOnOutput; onToggled: tk.showOnOutput = checked; leftPadding: 8 }

        // ── Manual Text ───────────────────────────────────────
        Label { text: "Texte manuel"; font.pixelSize: 11; color: window.darkMode ? "#AAA" : "#444"; leftPadding: 8 }
        TextArea {
            Layout.fillWidth: true; Layout.preferredHeight: 50; Layout.leftMargin: 8; Layout.rightMargin: 8
            text: tk.manualText; onTextChanged: tk.manualText = text
            placeholderText: "DERNIERE MINUTE — Texte ici..."
            font.pixelSize: 11; color: window.darkMode ? "#FFF" : "#1A1A1A"; wrapMode: TextArea.Wrap
            background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
        }

        // ── RSS Feeds ───────────────────────────���─────────────
        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#222" : "#CCC"; Layout.leftMargin: 8; Layout.rightMargin: 8 }
        Label { text: "Flux RSS automatique"; font.pixelSize: 11; color: window.darkMode ? "#AAA" : "#444"; leftPadding: 8 }

        RowLayout { spacing: 4; Layout.leftMargin: 8; Layout.rightMargin: 8
            TextField {
                id: rssInput; Layout.fillWidth: true
                text: rssFetcher.rssUrl
                onTextChanged: rssFetcher.rssUrl = text
                placeholderText: "https://feeds.reuters.com/reuters/topNews"
                font.pixelSize: 10; color: window.darkMode ? "#CCC" : "#333"
                background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
            }
            Rectangle {
                Layout.preferredWidth: 24; Layout.preferredHeight: 24; radius: 4
                color: refreshMa.containsMouse ? (window.darkMode ? Qt.rgba(1,1,1,0.08) : Qt.rgba(0,0,0,0.08)) : (window.darkMode ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.06))
                Label { anchors.centerIn: parent; text: "\u21BB"; color: rssFetcher.fetching ? "#5B4FDB" : (window.darkMode ? "#888" : "#555"); font.pixelSize: 14 }
                MouseArea { id: refreshMa; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: rssFetcher.forceRefresh() }
            }
        }

        // ── Feed Status ───────────────────────────────────────
        RowLayout { spacing: 6; Layout.leftMargin: 8
            Label { text: "Derniere MAJ:"; color: window.darkMode ? "#555" : "#999"; font.pixelSize: 9 }
            Label { text: rssFetcher.lastFetchTime; color: window.darkMode ? "#888" : "#555"; font.pixelSize: 9 }
            Item { Layout.fillWidth: true }
            Label { text: rssFetcher.feedCount + " titres"; color: window.darkMode ? "#888" : "#555"; font.pixelSize: 9 }
        }

        RowLayout { spacing: 4; Layout.leftMargin: 8
            Label { text: "Refresh:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 10 }
            ComboBox {
                id: refreshCombo
                model: ["30s", "1 min", "5 min", "15 min"]
                currentIndex: 1
                onCurrentIndexChanged: {
                    var secs = [30, 60, 300, 900]
                    rssFetcher.refreshIntervalSec = secs[currentIndex]
                }
                Layout.preferredWidth: 80
                background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
            }
            Item { Layout.fillWidth: true }
        }

        // ── Error display ─────────────────────────────────────
        Label {
            visible: rssFetcher.errorString !== ""
            text: "Erreur: " + rssFetcher.errorString
            font.pixelSize: 9; color: "#CC3333"; leftPadding: 8
            Layout.fillWidth: true; wrapMode: Text.WordWrap
        }

        // ── Appearance ────────────────────────────────────────
        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#222" : "#CCC"; Layout.leftMargin: 8; Layout.rightMargin: 8 }
        Label { text: "Apparence"; font.pixelSize: 11; color: window.darkMode ? "#AAA" : "#444"; leftPadding: 8 }

        RowLayout { spacing: 4; Layout.leftMargin: 8
            Label { text: "Fond:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 10 }
            Repeater {
                model: ["#CC0000", "#0066CC", "#000000", "#1DB954", "#FF6B00", "#5B4FDB"]
                Rectangle {
                    width: 18; height: 18; radius: 4; color: modelData
                    border.color: modelData === tk.bgColor ? "white" : "transparent"; border.width: 2
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: tk.bgColor = modelData }
                }
            }
        }
        RowLayout { spacing: 4; Layout.leftMargin: 8
            Label { text: "Vitesse:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 10 }
            Slider { from: 1; to: 5; stepSize: 1; value: tk.scrollSpeed; Layout.fillWidth: true; onMoved: tk.scrollSpeed = value }
            Label { text: tk.scrollSpeed + "x"; color: window.darkMode ? "#888" : "#555"; font.pixelSize: 10 }
        }
        RowLayout { spacing: 4; Layout.leftMargin: 8
            Label { text: "Taille:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 10 }
            Slider { from: 10; to: 24; stepSize: 1; value: tk.fontSize; Layout.fillWidth: true; onMoved: tk.fontSize = value }
            Label { text: tk.fontSize + "px"; color: window.darkMode ? "#888" : "#555"; font.pixelSize: 10 }
        }

        // ── Preview ───────────────────────────────────────────
        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#222" : "#CCC"; Layout.leftMargin: 8; Layout.rightMargin: 8 }
        Rectangle {
            Layout.fillWidth: true; Layout.leftMargin: 8; Layout.rightMargin: 8
            Layout.preferredHeight: 32; radius: 4; color: tk.bgColor
            clip: true
            Label {
                id: previewTicker
                text: rssFetcher.headlines ? rssFetcher.headlines : (tk.manualText ? tk.manualText : "Les titres apparaitront ici...")
                color: rssFetcher.headlines || tk.manualText ? tk.textColor : "#555"
                font.pixelSize: tk.fontSize * 0.8; font.bold: true
                y: (parent.height - height) / 2
                NumberAnimation on x {
                    from: previewTicker.parent.width
                    to: -previewTicker.implicitWidth
                    duration: 12000
                    loops: Animation.Infinite
                    running: tk.showOnOutput
                }
            }
        }

        Item { implicitHeight: 10 }
    }
    }
}
