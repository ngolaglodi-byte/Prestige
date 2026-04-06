import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

// ============================================================
// Step 4 — Output: TV (SDI/NDI/SRT) + Réseaux Sociaux
// ============================================================

Item {
    Flickable {
        anchors.fill: parent; anchors.margins: 24
        contentHeight: mainCol.implicitHeight; clip: true

        ColumnLayout {
            id: mainCol; width: parent.width; spacing: 16

            // ═══════════════════════════════════════════════════
            // SECTION 1: Sorties broadcast (TV)
            // ═══════════════════════════════════════════════════
            Label { text: window.t("broadcast_outputs"); font.pixelSize: 16; color: window.darkMode ? "white" : "#1A1A1A" }
            Label { text: window.t("broadcast_desc"); font.pixelSize: 12; color: window.darkMode ? "#777" : "#888" }

            Repeater {
                model: [
                    { type: "sdi",  label: "SDI",  desc: "Sortie vidéo SDI via carte DeckLink / AJA", prop: "outputSDI" },
                    { type: "ndi",  label: "NDI",  desc: "Sortie réseau NDI — visible par tout appareil NDI", prop: "outputNDI" },
                    { type: "srt",  label: "SRT",  desc: "Protocole SRT pour liaisons longue distance", prop: "outputSRT" }
                ]
                delegate: Rectangle {
                    Layout.fillWidth: true; height: 56; radius: 8
                    color: window.darkMode ? "#111114" : "#FFFFFF"; border.color: window.darkMode ? "#1E1E22" : "#DDD"
                    RowLayout {
                        anchors.fill: parent; anchors.margins: 12
                        Switch {
                            checked: {
                                if (modelData.prop === "outputSDI") return setupController.outputSDI
                                if (modelData.prop === "outputNDI") return setupController.outputNDI
                                if (modelData.prop === "outputSRT") return setupController.outputSRT
                                return false
                            }
                            onToggled: {
                                if (modelData.prop === "outputSDI") setupController.outputSDI = checked
                                if (modelData.prop === "outputNDI") setupController.outputNDI = checked
                                if (modelData.prop === "outputSRT") setupController.outputSRT = checked
                            }
                        }
                        ColumnLayout { Layout.fillWidth: true; spacing: 1
                            Label { text: modelData.label; font.pixelSize: 14; font.bold: true; color: window.darkMode ? "white" : "#1A1A1A" }
                            Label { text: modelData.desc; font.pixelSize: 11; color: window.darkMode ? "#777" : "#888" }
                        }
                    }
                }
            }

            // ═══════════════════════════════════════════════════
            // SECTION 2: Réseaux sociaux (streaming simultané)
            // ═══════════════════════════════════════════════════
            Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#222" : "#CCC"; Layout.topMargin: 8 }

            Label { text: window.t("social_outputs"); font.pixelSize: 16; color: window.darkMode ? "white" : "#1A1A1A" }
            Label { text: window.t("social_desc"); font.pixelSize: 12; color: window.darkMode ? "#777" : "#888" }

            Repeater {
                id: socialRepeater
                model: ListModel {
                    id: socialModel
                    ListElement { platform: "youtube";   name: "YouTube Live";    icon: "\u25B6";  rtmpBase: "rtmp://a.rtmp.youtube.com/live2/"; color: "#FF0000" }
                    ListElement { platform: "twitch";    name: "Twitch";          icon: "\u25C6";  rtmpBase: "rtmp://live.twitch.tv/app/";       color: "#9146FF" }
                    ListElement { platform: "facebook";  name: "Facebook Live";   icon: "f";       rtmpBase: "rtmps://live-api-s.facebook.com:443/rtmp/"; color: "#1877F2" }
                    ListElement { platform: "instagram"; name: "Instagram Live";  icon: "\u25CB";  rtmpBase: "rtmps://live-upload.instagram.com:443/rtmp/"; color: "#E4405F" }
                    ListElement { platform: "tiktok";    name: "TikTok LIVE";     icon: "\u266A";  rtmpBase: "rtmp://push.rtmp.tiktok.com/live/"; color: "#010101" }
                    ListElement { platform: "x";         name: "X (Twitter)";     icon: "X";       rtmpBase: "rtmp://va.pscp.tv:80/x/";         color: "#000000" }
                    ListElement { platform: "custom";    name: "RTMP personnalisé"; icon: "\u2B22"; rtmpBase: "";                                 color: "#6C5CE7" }
                }
                delegate: Rectangle {
                    Layout.fillWidth: true
                    height: socialCol.implicitHeight + 20
                    radius: 8; color: window.darkMode ? "#111114" : "#FFFFFF"; border.color: window.darkMode ? "#1E1E22" : "#DDD"

                    ColumnLayout {
                        id: socialCol
                        anchors.left: parent.left; anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter; anchors.margins: 12
                        spacing: 8

                        RowLayout {
                            Layout.fillWidth: true; spacing: 10

                            // Platform icon
                            Rectangle {
                                Layout.preferredWidth: 32; Layout.preferredHeight: 32; radius: 6; color: model.color
                                Label { anchors.centerIn: parent; text: model.icon; color: "white"; font.pixelSize: 14; font.bold: true }
                            }

                            // Platform name + status
                            ColumnLayout {
                                Layout.fillWidth: true; spacing: 1
                                Label { text: model.name; font.pixelSize: 14; font.bold: true; color: window.darkMode ? "white" : "#1A1A1A" }
                                Label {
                                    text: {
                                        var key = getSocialKey(model.platform)
                                        return key ? "Clé configurée ✓" : "Non configuré"
                                    }
                                    font.pixelSize: 11
                                    color: getSocialKey(model.platform) ? "#1DB954" : (window.darkMode ? "#666" : "#999")
                                }
                            }

                            // Enable switch
                            Switch {
                                id: socialSwitch
                                checked: getSocialEnabled(model.platform)
                                onToggled: setSocialEnabled(model.platform, checked)
                            }
                        }

                        // Stream key field (visible when enabled)
                        ColumnLayout {
                            visible: socialSwitch.checked
                            Layout.fillWidth: true; spacing: 6

                            // URL (pre-filled for known platforms, editable for custom)
                            RowLayout {
                                visible: model.platform === "custom"
                                Layout.fillWidth: true; spacing: 6
                                Label { text: "URL RTMP:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11 }
                                TextField {
                                    Layout.fillWidth: true
                                    placeholderText: "rtmp://votre-serveur.com/live/"
                                    Component.onCompleted: text = getSocialUrl(model.platform)
                                    onTextEdited: setSocialUrl(model.platform, text)
                                    font.pixelSize: 11; color: window.darkMode ? "#CCC" : "#333"
                                    background: Rectangle { color: window.darkMode ? "#1A1A1E" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true; spacing: 6
                                Label { text: "Clé de stream:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11 }
                                TextField {
                                    Layout.fillWidth: true
                                    placeholderText: "Collez votre clé de stream ici"
                                    Component.onCompleted: text = getSocialKey(model.platform)
                                    onTextEdited: setSocialKey(model.platform, text)
                                    echoMode: TextInput.Password
                                    font.pixelSize: 11; color: window.darkMode ? "#CCC" : "#333"
                                    background: Rectangle { color: window.darkMode ? "#1A1A1E" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
                                }
                                // Show/hide key
                                Rectangle {
                                    Layout.preferredWidth: 24; Layout.preferredHeight: 24; radius: 4
                                    color: eyeMa.containsMouse ? (window.darkMode ? Qt.rgba(1,1,1,0.08) : Qt.rgba(0,0,0,0.08)) : "transparent"
                                    Label { anchors.centerIn: parent; text: "\u25CE"; color: window.darkMode ? "#888" : "#555"; font.pixelSize: 12 }
                                    MouseArea { id: eyeMa; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: socialSwitch.toggle() }
                                }
                            }

                            // Pre-filled URL display for known platforms
                            Label {
                                visible: model.platform !== "custom"
                                text: "URL: " + model.rtmpBase + "..."
                                font.pixelSize: 11; color: window.darkMode ? "#555" : "#999"
                            }
                        }
                    }
                }
            }

            // ═══════════════════════════════════════════════════
            // SECTION 3: Qualité de sortie (FPS / Bitrate)
            // ═══════════════════════════════════════════════════
            Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#222" : "#CCC"; Layout.topMargin: 8 }

            Label { text: "Qualité de sortie"; font.pixelSize: 16; color: window.darkMode ? "white" : "#1A1A1A" }

            RowLayout { spacing: 8; Layout.leftMargin: 8
                Label { text: "Fréquence:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 12 }
                Repeater {
                    model: [{"label": "24", "fps": 24}, {"label": "25 PAL", "fps": 25}, {"label": "30 NTSC", "fps": 30}, {"label": "50", "fps": 50}, {"label": "60", "fps": 60}]
                    Rectangle {
                        Layout.preferredWidth: fpsLbl.implicitWidth + 16; Layout.preferredHeight: 28; radius: 6
                        color: setupController.outputFps === modelData.fps ? "#6C5CE7" : (window.darkMode ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.06))
                        Label { id: fpsLbl; anchors.centerIn: parent; text: modelData.label + " fps"; font.pixelSize: 11; color: setupController.outputFps === modelData.fps ? "white" : (window.darkMode ? "#888" : "#555") }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: setupController.outputFps = modelData.fps }
                    }
                }
            }

            RowLayout { spacing: 8; Layout.leftMargin: 8
                Label { text: "Débit vidéo:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 12 }
                Slider { id: brSlider; from: 2; to: 50; stepSize: 1; value: setupController.outputBitrate; Layout.preferredWidth: 180; onMoved: setupController.outputBitrate = value }
                Label { text: setupController.outputBitrate + " Mbps"; color: window.darkMode ? "#888" : "#555"; font.pixelSize: 12; Layout.preferredWidth: 60 }
            }
            Label {
                text: {
                    var br = setupController.outputBitrate
                    if (br <= 4) return "  SD / réseaux mobiles"
                    if (br <= 8) return "  1080p standard (YouTube, Twitch)"
                    if (br <= 15) return "  1080p haute qualité"
                    if (br <= 30) return "  4K standard"
                    return "  4K haute qualité / broadcast"
                }
                font.pixelSize: 11; color: "#6C5CE7"; leftPadding: 8
            }

            // Summary
            Rectangle {
                Layout.fillWidth: true; Layout.preferredHeight: summaryCol.implicitHeight + 16
                radius: 8; color: window.darkMode ? Qt.rgba(1,1,1,0.02) : Qt.rgba(0,0,0,0.02); border.color: window.darkMode ? Qt.rgba(1,1,1,0.05) : Qt.rgba(0,0,0,0.08)

                ColumnLayout {
                    id: summaryCol
                    anchors.fill: parent; anchors.margins: 12; spacing: 4
                    Label { text: window.t("output_summary"); font.pixelSize: 13; font.bold: true; color: window.darkMode ? "#AAA" : "#444" }
                    Label {
                        text: {
                            var outputs = []
                            if (setupController.outputSDI) outputs.push("SDI")
                            if (setupController.outputNDI) outputs.push("NDI")
                            if (setupController.outputSRT) outputs.push("SRT")
                            // Count social media
                            var socials = []
                            for (var p of ["youtube","twitch","facebook","instagram","tiktok","x","custom"]) {
                                if (getSocialEnabled(p) && getSocialKey(p)) socials.push(p)
                            }
                            if (socials.length > 0) outputs.push(socials.length + " réseau(x) social(aux)")
                            return outputs.length > 0 ? outputs.join(" + ") : "Aucune sortie configurée"
                        }
                        font.pixelSize: 12; color: window.darkMode ? "#888" : "#555"
                    }
                }
            }

            Item { implicitHeight: 20 }
        }
    }

    // ── Social media config storage — persisted via SetupController ──
    property var socialConfigs: ({})

    // Default URLs per platform
    readonly property var defaultUrls: ({
        "youtube":   "rtmp://a.rtmp.youtube.com/live2/",
        "twitch":    "rtmp://live.twitch.tv/app/",
        "facebook":  "rtmps://live-api-s.facebook.com:443/rtmp/",
        "instagram": "rtmps://live-upload.instagram.com:443/rtmp/",
        "tiktok":    "rtmp://push.rtmp.tiktok.com/live/",
        "x":         "rtmp://va.pscp.tv:80/x/",
        "custom":    ""
    })

    Component.onCompleted: {
        // Load saved configs from SetupController (persisted in profile)
        var saved = setupController.socialConfigsJson
        if (saved && saved.length > 2) {
            try { socialConfigs = JSON.parse(saved) } catch(e) { socialConfigs = {} }
        }
        // Ensure all platforms exist with defaults
        var platforms = ["youtube","twitch","facebook","instagram","tiktok","x","custom"]
        for (var i = 0; i < platforms.length; i++) {
            var p = platforms[i]
            if (!socialConfigs[p])
                socialConfigs[p] = { enabled: false, key: "", url: defaultUrls[p] || "" }
            if (!socialConfigs[p].url)
                socialConfigs[p].url = defaultUrls[p] || ""
        }
        socialConfigs = socialConfigs
        // Publish immediately so VE gets the saved config
        publishSocialToController()
    }

    function getSocialEnabled(p) { return socialConfigs[p] ? socialConfigs[p].enabled : false }
    function getSocialKey(p) { return socialConfigs[p] ? socialConfigs[p].key : "" }
    function getSocialUrl(p) { return socialConfigs[p] ? socialConfigs[p].url : "" }

    function setSocialEnabled(p, v) {
        if (!socialConfigs[p]) socialConfigs[p] = { enabled: false, key: "", url: defaultUrls[p] || "" }
        socialConfigs[p].enabled = v
        socialConfigs = socialConfigs
        saveSocialConfigs()
        publishSocialToController()
    }
    function setSocialKey(p, v) {
        if (!socialConfigs[p]) socialConfigs[p] = { enabled: false, key: "", url: defaultUrls[p] || "" }
        socialConfigs[p].key = v
        socialConfigs = socialConfigs
        saveSocialConfigs()
        publishSocialToController()
    }
    function setSocialUrl(p, v) {
        if (!socialConfigs[p]) socialConfigs[p] = { enabled: false, key: "", url: defaultUrls[p] || "" }
        socialConfigs[p].url = v
        socialConfigs = socialConfigs
        saveSocialConfigs()
        publishSocialToController()
    }

    // Save all configs to SetupController for persistence
    function saveSocialConfigs() {
        setupController.socialConfigsJson = JSON.stringify(socialConfigs)
    }

    // Serialize ACTIVE outputs to SetupController for Vision Engine transmission
    function publishSocialToController() {
        var outputs = []
        var platforms = ["youtube","twitch","facebook","instagram","tiktok","x","custom"]
        for (var i = 0; i < platforms.length; i++) {
            var p = platforms[i]
            var cfg = socialConfigs[p]
            if (cfg && cfg.enabled && cfg.key) {
                outputs.push({ platform: p, url: cfg.url + cfg.key })
            }
        }
        setupController.socialOutputsJson = JSON.stringify(outputs)
    }

}
