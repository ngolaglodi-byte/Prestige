import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: tk

    Flickable {
        anchors.fill: parent; contentHeight: tkCol.implicitHeight; clip: true
        flickableDirection: Flickable.VerticalFlick
        ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
    ColumnLayout {
        id: tkCol; width: parent.width; spacing: 10
        Item { implicitHeight: 4 }

        // ── Header ──────────────────────────────────────
        RowLayout { spacing: 8; Layout.leftMargin: 12
            Rectangle {
                Layout.preferredWidth: 28; Layout.preferredHeight: 28; radius: 7
                color: Qt.rgba(108/255,92/255,231/255,0.12)
                Label { anchors.centerIn: parent; text: "\uD83D\uDCE2"; font.pixelSize: 12 }
            }
            ColumnLayout { spacing: 0
                Label { text: "Ticker"; font.pixelSize: 15; font.weight: Font.Bold; color: window.darkMode ? "#F0F0F5" : "#0F0F14" }
                Label { text: rssFetcher.feedCount > 0 ? rssFetcher.feedCount + " headlines RSS" : "Manual text mode"; font.pixelSize: 10; color: window.darkMode ? "#505060" : "#999" }
            }
        }

        // ── ON/OFF ──────────────────────────────────────
        Switch { text: "Enable ticker"; checked: setupController.tickerVisible; onToggled: setupController.tickerVisible = checked; leftPadding: 12 }

        // ── Position (2 choices only) ───────────────────
        RowLayout { spacing: 8; Layout.leftMargin: 12
            Label { text: "Position:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11; Layout.preferredWidth: 55 }
            Repeater {
                model: [{ label: "Bottom", val: "bottom" }, { label: "Top", val: "top" }]
                Rectangle {
                    Layout.preferredWidth: 80; Layout.preferredHeight: 30; radius: 8
                    color: setupController.tickerPosition === modelData.val
                        ? Qt.rgba(108/255,92/255,231/255,0.18)
                        : (window.darkMode ? Qt.rgba(255,255,255,0.03) : Qt.rgba(0,0,0,0.03))
                    border.color: setupController.tickerPosition === modelData.val ? Qt.rgba(108/255,92/255,231/255,0.4) : "transparent"
                    Behavior on color { ColorAnimation { duration: 150 } }
                    Label { anchors.centerIn: parent; text: modelData.label; font.pixelSize: 11; font.weight: Font.DemiBold; color: setupController.tickerPosition === modelData.val ? "#6C5CE7" : (window.darkMode ? "#888" : "#666") }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: setupController.tickerPosition = modelData.val }
                }
            }
        }

        // ── Design (broadcast presets) ──────────────────
        RowLayout { spacing: 6; Layout.leftMargin: 12
            Label { text: "Style:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11; Layout.preferredWidth: 55 }
            ComboBox {
                model: ["News Red","News Blue","Dark Minimal","Sport Green","Breaking Red","Tech Dark","White Clean","Gold Luxury","Purple Pop","Orange Warm"]
                property var vals: ["tk_news_red","tk_news_blue","tk_dark_minimal","tk_sport_green","tk_breaking_red","tk_tech_dark","tk_white_clean","tk_gold_luxury","tk_purple_pop","tk_orange_warm"]
                currentIndex: Math.max(0, vals.indexOf(setupController.tickerDesign)); Layout.fillWidth: true
                onActivated: setupController.tickerDesign = vals[currentIndex]
                background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 6; border.color: window.darkMode ? "#333" : "#CCC" }
            }
        }

        // ── Speed ───────────────────────────────────────
        RowLayout { spacing: 4; Layout.leftMargin: 12
            Label { text: "Speed:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11; Layout.preferredWidth: 55 }
            Repeater {
                model: [{ label: "Slow", val: 1 }, { label: "Normal", val: 2 }, { label: "Fast", val: 3 }, { label: "Rush", val: 5 }]
                Rectangle {
                    Layout.preferredWidth: 55; Layout.preferredHeight: 28; radius: 6
                    color: setupController.tickerSpeed === modelData.val
                        ? Qt.rgba(108/255,92/255,231/255,0.18)
                        : (window.darkMode ? Qt.rgba(255,255,255,0.03) : Qt.rgba(0,0,0,0.03))
                    border.color: setupController.tickerSpeed === modelData.val ? Qt.rgba(108/255,92/255,231/255,0.3) : "transparent"
                    Label { anchors.centerIn: parent; text: modelData.label; font.pixelSize: 10; color: setupController.tickerSpeed === modelData.val ? "#6C5CE7" : (window.darkMode ? "#888" : "#666") }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: setupController.tickerSpeed = modelData.val }
                }
            }
        }

        // ── Colors (pro hex pickers) ────────────────────
        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#1A1A22" : "#E0E0E8"; Layout.leftMargin: 12; Layout.rightMargin: 12 }
        RowLayout { spacing: 6; Layout.leftMargin: 12; Layout.rightMargin: 12
            Label { text: "Band:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11; Layout.preferredWidth: 55 }
            Rectangle { Layout.preferredWidth: 24; Layout.preferredHeight: 24; radius: 6; color: setupController.tickerBgColor; border.color: window.darkMode ? "#444" : "#BBB"; border.width: 1 }
            TextField {
                Layout.fillWidth: true; text: setupController.tickerBgColor; font.pixelSize: 11; font.family: "SF Mono, Menlo, monospace"; color: window.darkMode ? "white" : "#1A1A1A"
                onTextEdited: { if (text.match(/^#[0-9A-Fa-f]{6}$/)) setupController.tickerBgColor = text }
                background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
            }
            ColorPickerButton { currentColor: setupController.tickerBgColor; onColorSelected: function(c) { setupController.tickerBgColor = c } }
        }
        RowLayout { spacing: 6; Layout.leftMargin: 12; Layout.rightMargin: 12
            Label { text: "Text:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11; Layout.preferredWidth: 55 }
            Rectangle { Layout.preferredWidth: 24; Layout.preferredHeight: 24; radius: 6; color: setupController.tickerTextColor; border.color: window.darkMode ? "#444" : "#BBB"; border.width: 1 }
            TextField {
                Layout.fillWidth: true; text: setupController.tickerTextColor; font.pixelSize: 11; font.family: "SF Mono, Menlo, monospace"; color: window.darkMode ? "white" : "#1A1A1A"
                onTextEdited: { if (text.match(/^#[0-9A-Fa-f]{6}$/)) setupController.tickerTextColor = text }
                background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
            }
            ColorPickerButton { currentColor: setupController.tickerTextColor; onColorSelected: function(c) { setupController.tickerTextColor = c } }
        }

        // ── Font Size ───────────────────────────────────
        RowLayout { spacing: 4; Layout.leftMargin: 12
            Label { text: "Size:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11; Layout.preferredWidth: 55 }
            Slider { from: 12; to: 36; stepSize: 1; value: setupController.tickerFontSize; Layout.fillWidth: true; onMoved: setupController.tickerFontSize = value }
            Label { text: setupController.tickerFontSize + "pt"; color: window.darkMode ? "#888" : "#555"; font.pixelSize: 10; Layout.preferredWidth: 30 }
        }

        // ── Manual Text ─────────────────────────────────
        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#1A1A22" : "#E0E0E8"; Layout.leftMargin: 12; Layout.rightMargin: 12 }
        Label { text: "Manual text"; font.pixelSize: 12; font.weight: Font.DemiBold; color: window.darkMode ? "#F0F0F5" : "#0F0F14"; leftPadding: 12 }
        TextArea {
            Layout.fillWidth: true; Layout.preferredHeight: 60; Layout.leftMargin: 12; Layout.rightMargin: 12
            text: setupController.tickerManualText; onTextChanged: setupController.tickerManualText = text
            placeholderText: "BREAKING NEWS — Your ticker text here..."
            font.pixelSize: 12; color: window.darkMode ? "#FFF" : "#1A1A1A"; wrapMode: TextArea.Wrap
            background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 6; border.color: window.darkMode ? "#333" : "#CCC" }
        }

        // ── RSS Feed ────────────────────────────────────
        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#1A1A22" : "#E0E0E8"; Layout.leftMargin: 12; Layout.rightMargin: 12 }
        Label { text: "RSS Feed"; font.pixelSize: 12; font.weight: Font.DemiBold; color: window.darkMode ? "#F0F0F5" : "#0F0F14"; leftPadding: 12 }
        RowLayout { spacing: 6; Layout.leftMargin: 12; Layout.rightMargin: 12
            TextField {
                id: rssInput; Layout.fillWidth: true
                text: rssFetcher.rssUrl; onTextChanged: rssFetcher.rssUrl = text
                placeholderText: "https://feeds.reuters.com/reuters/topNews"
                font.pixelSize: 11; color: window.darkMode ? "#CCC" : "#333"
                background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 6; border.color: window.darkMode ? "#333" : "#CCC" }
            }
            Rectangle {
                Layout.preferredWidth: 28; Layout.preferredHeight: 28; radius: 6
                color: refreshMa.containsMouse ? (window.darkMode ? Qt.rgba(255,255,255,0.08) : Qt.rgba(0,0,0,0.08)) : (window.darkMode ? Qt.rgba(255,255,255,0.03) : Qt.rgba(0,0,0,0.03))
                Label { anchors.centerIn: parent; text: "\u21BB"; color: rssFetcher.fetching ? "#6C5CE7" : (window.darkMode ? "#888" : "#555"); font.pixelSize: 14 }
                MouseArea { id: refreshMa; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: rssFetcher.forceRefresh() }
            }
        }
        RowLayout { spacing: 6; Layout.leftMargin: 12
            Rectangle { Layout.preferredWidth: 6; Layout.preferredHeight: 6; radius: 3; color: rssFetcher.feedCount > 0 ? "#00D68F" : (window.darkMode ? "#333" : "#CCC") }
            Label { text: rssFetcher.feedCount > 0 ? (rssFetcher.feedCount + " headlines — " + rssFetcher.lastFetchTime) : "No feed connected"; font.pixelSize: 10; color: window.darkMode ? "#666" : "#999" }
            Item { Layout.fillWidth: true }
            ComboBox {
                model: ["30s", "1 min", "5 min", "15 min"]; currentIndex: 1; Layout.preferredWidth: 70
                onCurrentIndexChanged: { var s = [30, 60, 300, 900]; rssFetcher.refreshIntervalSec = s[currentIndex] }
                background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
            }
        }

        // Error
        Label {
            visible: rssFetcher.errorString !== ""
            text: "Error: " + rssFetcher.errorString
            font.pixelSize: 10; color: "#FF3D71"; leftPadding: 12
            Layout.fillWidth: true; wrapMode: Text.WordWrap
        }

        // ── Live Preview ────────────────────────────────
        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#1A1A22" : "#E0E0E8"; Layout.leftMargin: 12; Layout.rightMargin: 12 }
        Label { text: "Preview"; font.pixelSize: 12; font.weight: Font.DemiBold; color: window.darkMode ? "#F0F0F5" : "#0F0F14"; leftPadding: 12 }
        Rectangle {
            Layout.fillWidth: true; Layout.leftMargin: 12; Layout.rightMargin: 12
            Layout.preferredHeight: 36; radius: 6; color: setupController.tickerBgColor
            clip: true
            Label {
                id: previewTicker
                text: rssFetcher.headlines ? rssFetcher.headlines : (setupController.tickerManualText ? setupController.tickerManualText : "Ticker text will appear here...")
                color: (rssFetcher.headlines || setupController.tickerManualText) ? setupController.tickerTextColor : "#555"
                font.pixelSize: setupController.tickerFontSize * 0.8; font.weight: Font.Bold
                font.family: "SF Pro Display, Inter, Helvetica Neue, Arial"
                y: (parent.height - height) / 2
                NumberAnimation on x {
                    from: previewTicker.parent.width
                    to: -previewTicker.implicitWidth
                    duration: Math.max(3000, previewTicker.implicitWidth * (50 / Math.max(1, setupController.tickerSpeed)))
                    loops: Animation.Infinite
                    running: setupController.tickerVisible
                }
            }
        }

        Item { implicitHeight: 12 }
    }
    }
}
