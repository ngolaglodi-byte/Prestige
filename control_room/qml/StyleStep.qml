import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root

    property string previewStyleId: setupController.selectedStyle
    property string previewAnimType: setupController.animationType

    RowLayout {
        anchors.fill: parent; anchors.margins: 24; spacing: 24

        // ── Left: Style list (scrollable) — 50% ────────────
        ColumnLayout {
            Layout.preferredWidth: (parent.width - 24) / 2
            Layout.fillHeight: true; spacing: 12
            Label { text: window.t("overlay_style"); font.pixelSize: 16; color: window.darkMode ? "white" : "#1A1A1A" }
            Label { text: window.t("hover_preview"); font.pixelSize: 12; color: window.darkMode ? "#666" : "#999" }

            ListView {
                id: styleList; Layout.fillWidth: true; Layout.fillHeight: true
                spacing: 4; clip: true
                model: ListModel {
                    ListElement { sid: "bfm"; nm: "BFM TV"; ref: "Barre rouge, fond noir" }
                    ListElement { sid: "lci"; nm: "LCI"; ref: "Dégradé, triangle orange" }
                    ListElement { sid: "france2"; nm: "France 2"; ref: "Fond blanc, bleu" }
                    ListElement { sid: "france24"; nm: "France 24"; ref: "Double ligne rouge+bleu" }
                    ListElement { sid: "cnn"; nm: "CNN"; ref: "Fond rouge plein" }
                    ListElement { sid: "bbc"; nm: "BBC News"; ref: "Rouge, barre blanche" }
                    ListElement { sid: "skynews"; nm: "Sky News"; ref: "Bleu Sky, cyan" }
                    ListElement { sid: "aljazeera"; nm: "Al Jazeera"; ref: "Bordeaux, texte or" }
                    ListElement { sid: "sports"; nm: "Sports"; ref: "Badge numéro, accent" }
                    ListElement { sid: "football"; nm: "Football TV"; ref: "Fond pelouse" }
                    ListElement { sid: "olympics"; nm: "Olympiques"; ref: "Blanc, anneaux" }
                    ListElement { sid: "politique"; nm: "Débats politiques"; ref: "Bleu nuit, tricolore" }
                    ListElement { sid: "gouvernement"; nm: "Officiel gouv."; ref: "Blanc, sceau RF" }
                    ListElement { sid: "cinema"; nm: "Cinéma / Festival"; ref: "Noir, lignes or" }
                    ListElement { sid: "luxury"; nm: "Luxury / Prestige"; ref: "Argent, spacing" }
                    ListElement { sid: "tech"; nm: "Tech / Innovation"; ref: "Cyan néon, mono" }
                    ListElement { sid: "minimal"; nm: "Minimaliste"; ref: "Sans fond" }
                    ListElement { sid: "dual"; nm: "Deux personnes"; ref: "Double bloc" }
                    ListElement { sid: "fullscreen"; nm: "Plein écran"; ref: "Gradient, médaillon" }
                    ListElement { sid: "breaking"; nm: "Breaking News"; ref: "Bannière rouge" }
                }
                delegate: Rectangle {
                    width: styleList.width; height: 52; radius: 8
                    color: sid === setupController.selectedStyle ? Qt.rgba(91/255,79/255,219/255,0.12) : ma.containsMouse ? (window.darkMode ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.06)) : (window.darkMode ? "#0D0D10" : "#E0E0E6")
                    border.color: sid === setupController.selectedStyle ? Qt.rgba(91/255,79/255,219/255,0.3) : "transparent"
                    Behavior on color { ColorAnimation { duration: 150 } }
                    RowLayout {
                        anchors.fill: parent; anchors.margins: 8; spacing: 8
                        Rectangle { Layout.preferredWidth: 4; Layout.preferredHeight: 32; radius: 2; color: getAccent(sid) }
                        ColumnLayout { Layout.fillWidth: true; spacing: 1
                            Label { text: nm; font.pixelSize: 13; font.weight: Font.DemiBold; color: window.darkMode ? "white" : "#1A1A1A" }
                            Label { text: ref; font.pixelSize: 10; color: window.darkMode ? "#666" : "#999" }
                        }
                        Label { text: "\u2713"; color: "#5B4FDB"; font.pixelSize: 14; visible: sid === setupController.selectedStyle }
                    }
                    MouseArea {
                        id: ma; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                        onClicked: { setupController.selectedStyle = sid; root.previewStyleId = sid }
                        onEntered: root.previewStyleId = sid
                    }
                }
            }
        }

        // ── Right: Preview + config — 50% (scrollable) ─────────
        Flickable {
            Layout.preferredWidth: (parent.width - 24) / 2
            Layout.fillHeight: true
            contentHeight: rightCol.implicitHeight; clip: true
            flickableDirection: Flickable.VerticalFlick
            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

            ColumnLayout {
                id: rightCol; width: parent.width; spacing: 12

                Label { text: window.t("preview"); font.pixelSize: 16; color: window.darkMode ? "white" : "#1A1A1A" }

                // ── PREVIEW BOX ────────────────────────────────
                // Shows LIVE Vision Engine output (with Lottie animations)
                // when connected, or QML fallback when standalone
                Rectangle {
                    id: pvBox; Layout.fillWidth: true; Layout.preferredHeight: width * 9 / 16
                color: window.darkMode ? "#0A0A0E" : "#E8E8EE"; radius: 10; clip: true

                // LIVE preview from Vision Engine (shows real Lottie animations)
                Image {
                    id: styleLivePreview
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectFit
                    cache: false
                    source: previewMonitor.active ? "image://preview/frame?" + stylePreviewCounter : ""
                    visible: previewMonitor.active
                    property int stylePreviewCounter: 0
                    Connections {
                        target: previewMonitor
                        function onFrameUpdated() { styleLivePreview.stylePreviewCounter++ }
                    }
                }

                // QML fallback (when Vision Engine not connected)
                Item {
                    anchors.fill: parent
                    visible: !previewMonitor.active

                Rectangle { anchors.fill: parent; radius: 10; gradient: Gradient { GradientStop { position: 0; color: "#1A1A24" } GradientStop { position: 1; color: "#0A0A10" } } }
                Rectangle { anchors.horizontalCenter: parent.horizontalCenter; y: parent.height*0.15; width: parent.width*0.2; height: parent.height*0.55; color: Qt.rgba(1,1,1,0.03); radius: width*0.1 }
                Rectangle { anchors.horizontalCenter: parent.horizontalCenter; y: parent.height*0.08; width: parent.width*0.08; height: width; radius: width/2; color: Qt.rgba(1,1,1,0.04) }

                // ── THE NAMEPLATE — reacts to style + accent color + opacity + animation ──
                Rectangle {
                    id: plate
                    property real animX: 0
                    property real animY: 0
                    property real animOpacity: 1
                    property real animScale: 1

                    x: parent.width * 0.05 + animX
                    y: parent.height * 0.85 - Math.max(parent.height * 0.13, 40) + animY
                    width: Math.max(parent.width * 0.48, 220)
                    height: Math.max(parent.height * 0.13, 40)
                    radius: 4
                    color: getBg(root.previewStyleId)
                    opacity: setupController.backgroundOpacity * animOpacity
                    scale: animScale
                    transformOrigin: Item.Left

                    Behavior on animX { id: plateBehaviorX; NumberAnimation { duration: setupController.animEnterFrames * 33; easing.type: Easing.OutCubic } }
                    Behavior on animY { id: plateBehaviorY; NumberAnimation { duration: setupController.animEnterFrames * 33; easing.type: Easing.OutCubic } }
                    Behavior on animOpacity { id: plateBehaviorOp; NumberAnimation { duration: setupController.animEnterFrames * 33; easing.type: Easing.OutCubic } }
                    Behavior on animScale { id: plateBehaviorSc; NumberAnimation { duration: setupController.animEnterFrames * 33; easing.type: Easing.OutCubic } }

                    // Accent bar left — uses user's accent color
                    Rectangle { anchors.left: parent.left; width: 3; height: parent.height; color: setupController.accentColor; visible: !isWhite(root.previewStyleId) && root.previewStyleId !== "cnn" && root.previewStyleId !== "bbc" && root.previewStyleId !== "luxury" && root.previewStyleId !== "minimal" }
                    // BBC white bar
                    Rectangle { anchors.left: parent.left; width: 5; height: parent.height; radius: 2; color: "white"; visible: root.previewStyleId === "bbc" }
                    // BBC badge
                    Rectangle { anchors.right: parent.right; anchors.rightMargin: 5; anchors.verticalCenter: parent.verticalCenter; width: plate.height*0.65; height: plate.height*0.65; radius: 3; color: Qt.rgba(1,1,1,0.15); visible: root.previewStyleId === "bbc"; Text { anchors.centerIn: parent; text: "BBC"; color: "white"; font.pixelSize: Math.max(6, plate.height*0.28); font.bold: true } }
                    // LCI triangle
                    Canvas { anchors.top: parent.top; anchors.right: parent.right; width: plate.height*0.35; height: plate.height*0.35; visible: root.previewStyleId === "lci"; onPaint: { var c=getContext("2d"); c.fillStyle="#FF6D00"; c.beginPath(); c.moveTo(0,0); c.lineTo(width,0); c.lineTo(width,height); c.fill() } }
                    // France24 double line
                    Rectangle { anchors.bottom: parent.bottom; anchors.bottomMargin: 3; width: parent.width; height: 2; color: "#003580"; visible: root.previewStyleId === "france24" }
                    // Tricolore
                    Column { anchors.left: parent.left; width: 5; height: parent.height; visible: root.previewStyleId === "politique" || root.previewStyleId === "gouvernement"; Rectangle { width: 5; height: parent.height/3; color: "#002395" } Rectangle { width: 5; height: parent.height/3; color: "#FFFFFF" } Rectangle { width: 5; height: parent.height/3; color: "#ED2939" } }
                    // Gouvernement RF seal
                    Rectangle { anchors.right: parent.right; anchors.rightMargin: 5; anchors.verticalCenter: parent.verticalCenter; width: plate.height*0.6; height: plate.height*0.6; radius: width/2; color: "#EEE"; border.color: "#002395"; border.width: 1; visible: root.previewStyleId === "gouvernement"; Text { anchors.centerIn: parent; text: "RF"; color: "#002395"; font.pixelSize: Math.max(6, plate.height*0.22); font.bold: true } }
                    // AJ badge
                    Rectangle { anchors.left: parent.left; width: parent.width*0.14; height: parent.height; color: Qt.rgba(0.83,0.63,0.09,0.3); radius: 2; visible: root.previewStyleId === "aljazeera"; Text { anchors.centerIn: parent; text: "AJ"; color: "#D4A017"; font.pixelSize: Math.max(6, plate.height*0.28); font.bold: true } }
                    // Sports badge — uses user's accent color
                    Rectangle { anchors.left: parent.left; width: parent.width*0.14; height: parent.height; color: setupController.accentColor; radius: 2; visible: root.previewStyleId === "sports"; Text { anchors.centerIn: parent; text: "10"; color: "white"; font.pixelSize: Math.max(7, plate.height*0.35); font.bold: true } }
                    // Olympics rings
                    Row { anchors.left: parent.left; anchors.leftMargin: 5; anchors.verticalCenter: parent.verticalCenter; spacing: 1; visible: root.previewStyleId === "olympics"; Repeater { model: ["#0085C7","#222","#DF0024","#F4C300","#009F3D"]; Rectangle { width: Math.max(5, plate.height*0.17); height: width; radius: width/2; color: "transparent"; border.color: modelData; border.width: 1 } } }
                    // Tech scanlines + LIVE
                    Column { anchors.fill: parent; spacing: 2; opacity: 0.08; visible: root.previewStyleId === "tech"; Repeater { model: Math.max(1, Math.floor(plate.height/3)); Rectangle { width: plate.width; height: 1; color: "white" } } }
                    Rectangle { anchors.top: parent.top; anchors.right: parent.right; anchors.margins: 3; width: plate.width*0.11; height: plate.height*0.3; radius: 2; color: Qt.rgba(0,0.9,1,0.15); visible: root.previewStyleId === "tech"; Text { anchors.centerIn: parent; text: "LIVE"; color: "#00E5FF"; font.pixelSize: Math.max(4, plate.height*0.16); font.bold: true } }
                    // Luxury/Cinema lines
                    Rectangle { anchors.top: parent.top; width: parent.width; height: 1; color: "#C0C0C0"; visible: root.previewStyleId === "luxury" }
                    Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: "#C0C0C0"; visible: root.previewStyleId === "luxury" }
                    Rectangle { anchors.top: parent.top; width: parent.width; height: 1; color: "#D4AF37"; visible: root.previewStyleId === "cinema" }
                    Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: "#D4AF37"; visible: root.previewStyleId === "cinema" }
                    // Accent line bottom — uses user's accent color
                    Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 2; color: setupController.accentColor }

                    // ── TALENT NAME + ROLE ──────────────────
                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left; anchors.leftMargin: getTxtMargin(root.previewStyleId)
                        spacing: 1
                        Text {
                            text: getName(root.previewStyleId)
                            color: getNameColor(root.previewStyleId)
                            font.pixelSize: Math.max(9, pvBox.height * 0.032)
                            font.bold: true
                            font.family: root.previewStyleId === "tech" ? "Menlo" : "Helvetica Neue"
                            font.letterSpacing: root.previewStyleId === "luxury" ? 2 : 0
                        }
                        Text {
                            text: getRole(root.previewStyleId)
                            color: getRoleColor(root.previewStyleId)
                            font.pixelSize: Math.max(7, pvBox.height * 0.022)
                            font.family: root.previewStyleId === "tech" ? "Menlo" : "Helvetica Neue"
                            font.italic: root.previewStyleId === "cinema" || root.previewStyleId === "politique"
                            font.capitalization: root.previewStyleId === "luxury" ? Font.AllUppercase : Font.MixedCase
                        }
                    }
                }

                // ── Effect preview — BIG and VISIBLE ──────────
                // Each effect category shows a clearly distinct visual

                // === GLOW: bright thick border + outer halo ===
                Rectangle {
                    visible: ["neon_glow","edge_glow","bloom","shimmer"].indexOf(setupController.animationType) >= 0
                    x: plate.x - 10; y: plate.y - 10
                    width: plate.width + 20; height: plate.height + 20
                    radius: 10; color: "transparent"
                    border.color: setupController.accentColor; border.width: 4
                    opacity: 0.6 + 0.3 * Math.sin(glowTimer.phase)
                    Rectangle { anchors.fill: parent; anchors.margins: -8; radius: 18; color: "transparent"; border.color: setupController.accentColor; border.width: 2; opacity: 0.3 + 0.2 * Math.sin(glowTimer.phase * 1.5) }
                    Rectangle { anchors.fill: parent; anchors.margins: -16; radius: 26; color: "transparent"; border.color: setupController.accentColor; border.width: 1; opacity: 0.15 + 0.1 * Math.sin(glowTimer.phase * 2) }
                }

                // === GLITCH/VHS: strong RGB split + scan lines ===
                Item {
                    visible: ["glitch_rgb","vhs_effect","chromatic_aberration"].indexOf(setupController.animationType) >= 0
                    x: plate.x; y: plate.y; width: plate.width; height: plate.height; z: plate.z + 1
                    Text { text: getName(root.previewStyleId); color: Qt.rgba(1,0,0,0.5); font.pixelSize: Math.max(9, pvBox.height * 0.032); font.bold: true; x: 4 * Math.sin(glowTimer.phase * 4); anchors.verticalCenter: parent.verticalCenter; anchors.verticalCenterOffset: -6 }
                    Text { text: getName(root.previewStyleId); color: Qt.rgba(0,1,1,0.5); font.pixelSize: Math.max(9, pvBox.height * 0.032); font.bold: true; x: -4 * Math.sin(glowTimer.phase * 4); anchors.verticalCenter: parent.verticalCenter; anchors.verticalCenterOffset: -6 }
                    // Scan lines
                    Column { anchors.fill: parent; spacing: 3; opacity: 0.15; Repeater { model: Math.floor(parent.height / 4); Rectangle { width: plate.width; height: 1; color: "white" } } }
                }

                // === PARTICLES: big visible animated dots ===
                Repeater {
                    model: ["sparkles","bokeh","rising_particles","fire_embers","confetti","snow","dust"].indexOf(setupController.animationType) >= 0 ? 20 : 0
                    Rectangle {
                        property real rx: Math.random()
                        property real ry: Math.random()
                        property real rSpeed: 0.3 + Math.random() * 0.7
                        x: plate.x - 20 + rx * (plate.width + 40)
                        y: {
                            var at = setupController.animationType
                            if (at === "snow" || at === "confetti") return plate.y - 30 + ((ry * (plate.height + 60) + glowTimer.phase * 40 * rSpeed) % (plate.height + 60))
                            if (at === "fire_embers" || at === "rising_particles") return plate.y + plate.height + 10 - ((ry * (plate.height + 40) + glowTimer.phase * 35 * rSpeed) % (plate.height + 40))
                            return plate.y - 15 + ry * (plate.height + 30)
                        }
                        width: 4 + Math.random() * 6; height: width; radius: width / 2
                        color: {
                            var at = setupController.animationType
                            if (at === "fire_embers") return index % 2 === 0 ? "#FF8C00" : "#FF3300"
                            if (at === "snow") return "white"
                            if (at === "confetti") return ["#FF0000","#00CC00","#0066FF","#FFCC00","#FF00CC","#00FFFF"][index % 6]
                            if (at === "dust") return "#C0B8A0"
                            return setupController.accentColor
                        }
                        opacity: 0.5 + 0.4 * Math.sin(glowTimer.phase * 3 + index * 0.9)
                    }
                }

                // === BLUR: frosted glass overlay ===
                Rectangle {
                    visible: ["gaussian_blur_in","radial_blur","directional_blur","defocus","blur_in","tilt_shift"].indexOf(setupController.animationType) >= 0
                    x: plate.x; y: plate.y; width: plate.width; height: plate.height; radius: 4
                    color: Qt.rgba(1, 1, 1, 0.08 + 0.04 * Math.sin(glowTimer.phase))
                    border.color: Qt.rgba(1, 1, 1, 0.15); border.width: 1
                }

                // === COLOR: moving gradient overlay ===
                Rectangle {
                    visible: ["color_sweep","gradient_shift","duotone","shadow_drop_animate","outline_stroke"].indexOf(setupController.animationType) >= 0
                    x: plate.x; y: plate.y; width: plate.width; height: plate.height; radius: 4
                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop { position: Math.abs(Math.sin(glowTimer.phase * 0.3)) * 0.5; color: Qt.rgba(setupController.accentColor.r, setupController.accentColor.g, setupController.accentColor.b, 0.4) }
                        GradientStop { position: 0.5 + Math.abs(Math.sin(glowTimer.phase * 0.3)) * 0.5; color: "transparent" }
                    }
                }

                // === LOWER THIRD: thick animated accent line ===
                Rectangle {
                    visible: ["line_draw","bar_slide","underline_grow","split_reveal"].indexOf(setupController.animationType) >= 0
                    x: plate.x; y: plate.y + plate.height - 3
                    width: plate.width * Math.min(1, (glowTimer.phase % 4) / 2); height: 4; radius: 2
                    color: setupController.accentColor
                }

                // === SHAPE: thick animated border ===
                Rectangle {
                    visible: ["bracket_expand","corner_build","box_wipe","shape_morph","rectangle_build","hexagon_pattern","grid_reveal","circle_expand"].indexOf(setupController.animationType) >= 0
                    x: plate.x - 4; y: plate.y - 4; width: plate.width + 8; height: plate.height + 8
                    radius: 4; color: "transparent"
                    border.color: setupController.accentColor; border.width: 3
                    opacity: 0.6 + 0.3 * Math.sin(glowTimer.phase * 1.5)
                }

                // === TRANSITION: wipe bar moving across ===
                Rectangle {
                    visible: ["wipe_linear","push_slide","zoom_through","ink_bleed","spin_transition","cross_dissolve","light_leak"].indexOf(setupController.animationType) >= 0
                    x: plate.x + plate.width * Math.abs(Math.sin(glowTimer.phase * 0.4)) - 5; y: plate.y - 5
                    width: 10; height: plate.height + 10; radius: 5
                    color: Qt.rgba(setupController.accentColor.r, setupController.accentColor.g, setupController.accentColor.b, 0.5)
                }

                // === LIGHT: bright sweep across plate ===
                Rectangle {
                    visible: ["lens_flare","light_rays","light_leak","light_streak"].indexOf(setupController.animationType) >= 0
                    x: plate.x + plate.width * (0.2 + 0.6 * Math.abs(Math.sin(glowTimer.phase * 0.3))); y: plate.y - 15
                    width: 40; height: plate.height + 30; radius: 20
                    gradient: Gradient { orientation: Gradient.Horizontal
                        GradientStop { position: 0; color: "transparent" }
                        GradientStop { position: 0.5; color: Qt.rgba(1, 1, 1, 0.25 + 0.15 * Math.sin(glowTimer.phase)) }
                        GradientStop { position: 1; color: "transparent" }
                    }
                }

                // ═══ AE POST-EFFECT PREVIEW ═══════════════════
                // Visual indicator for the active AE post-effect on overlay

                // Distortion preview: wave/ripple on nameplate
                Item {
                    visible: ["turbulent_displace","twirl","spherize","bulge","ripple","wave_warp","kaleidoscope","mesh_warp","reshape"].indexOf(setupController.aeEffectId) >= 0
                    x: plate.x; y: plate.y; width: plate.width; height: plate.height
                    transform: [
                        Translate { x: Math.sin(glowTimer.phase * 3) * 3 * setupController.aeEffectIntensity; y: Math.cos(glowTimer.phase * 2.5) * 2 * setupController.aeEffectIntensity }
                    ]
                    Rectangle { anchors.fill: parent; radius: 4; color: "transparent"; border.color: "#FF6600"; border.width: 2; opacity: 0.5 + 0.3 * Math.sin(glowTimer.phase * 2) }
                    Label { anchors.centerIn: parent; text: "DISTORTION"; font.pixelSize: 7; font.bold: true; color: "#FF6600"; opacity: 0.7 }
                }

                // Color correction preview: tinted overlay
                Rectangle {
                    visible: ["curves","levels","hue_saturation","brightness_contrast","exposure","tint","tritone","colorama","leave_color","vibrance","photo_filter","gradient_map","black_white","invert","threshold","solarize","color_balance"].indexOf(setupController.aeEffectId) >= 0
                    x: plate.x; y: plate.y; width: plate.width; height: plate.height; radius: 4
                    color: Qt.rgba(setupController.aeEffectColor1.r || 0.9, setupController.aeEffectColor1.g || 0.1, setupController.aeEffectColor1.b || 0.1, 0.25 * setupController.aeEffectIntensity)
                    Label { anchors.centerIn: parent; text: "COLOR"; font.pixelSize: 7; font.bold: true; color: "#FFD700"; opacity: 0.7 }
                }

                // Generate preview: animated pattern
                Item {
                    visible: ["fractal_noise","cell_pattern","grid","gradient_ramp","vegas","radio_waves","audio_spectrum","lens_flare_gen","light_burst","beam","4color_gradient","fill","stroke","circle_burst","checkerboard"].indexOf(setupController.aeEffectId) >= 0
                    x: plate.x; y: plate.y; width: plate.width; height: plate.height
                    Repeater {
                        model: 8
                        Rectangle {
                            property real rx: Math.random()
                            x: rx * (plate.width - 6); y: (index / 8.0) * plate.height
                            width: 6; height: 6; radius: 3
                            color: setupController.aeEffectColor1 || "#00FF88"
                            opacity: 0.4 + 0.5 * Math.sin(glowTimer.phase * 4 + index * 0.8)
                        }
                    }
                    Label { anchors.centerIn: parent; text: "GENERATE"; font.pixelSize: 7; font.bold: true; color: "#00FF88"; opacity: 0.7 }
                }

                // Stylize preview: edge/texture effect
                Rectangle {
                    visible: ["emboss","find_edges","roughen_edges","scatter","stylize_glow","cartoon","halftone","stained_glass","noise","strobe","motion_tile","cross_hatch","oil_paint"].indexOf(setupController.aeEffectId) >= 0
                    x: plate.x - 2; y: plate.y - 2; width: plate.width + 4; height: plate.height + 4; radius: 6
                    color: "transparent"; border.color: "#FF00FF"; border.width: 2
                    opacity: 0.5 + 0.4 * Math.sin(glowTimer.phase * 1.5)
                    Label { anchors.centerIn: parent; text: "STYLIZE"; font.pixelSize: 7; font.bold: true; color: "#FF00FF"; opacity: 0.7 }
                }

                // Perspective preview: skewed plate
                Rectangle {
                    visible: ["cc_sphere","cc_cylinder","bevel_alpha","drop_shadow","radial_shadow","3d_rotation","reflection"].indexOf(setupController.aeEffectId) >= 0
                    x: plate.x + 5; y: plate.y + 5; width: plate.width - 4; height: plate.height - 4; radius: 4
                    color: Qt.rgba(0, 0, 0, 0.3 * setupController.aeEffectIntensity)
                    Label { anchors.centerIn: parent; text: "3D"; font.pixelSize: 9; font.bold: true; color: "#00CCFF"; opacity: 0.7 }
                }

                // Time preview: ghosting trail
                Rectangle {
                    visible: ["echo","trails","force_motion_blur"].indexOf(setupController.aeEffectId) >= 0
                    x: plate.x - 8; y: plate.y; width: plate.width; height: plate.height; radius: 4
                    color: getBg(root.previewStyleId); opacity: 0.3
                }
                Rectangle {
                    visible: ["echo","trails","force_motion_blur"].indexOf(setupController.aeEffectId) >= 0
                    x: plate.x - 4; y: plate.y; width: plate.width; height: plate.height; radius: 4
                    color: getBg(root.previewStyleId); opacity: 0.5
                    Label { anchors.centerIn: parent; text: "TIME"; font.pixelSize: 7; font.bold: true; color: "#FFCC00"; opacity: 0.7 }
                }

                // Blend mode preview: colored overlay with mode label
                Rectangle {
                    visible: setupController.overlayBlendMode !== "normal" && setupController.overlayBlendMode !== ""
                    x: plate.x; y: plate.y - 18; width: bmLbl.implicitWidth + 10; height: 14; radius: 3
                    color: Qt.rgba(0.3, 0.2, 0.8, 0.5)
                    Label { id: bmLbl; anchors.centerIn: parent; text: "BLEND: " + (setupController.overlayBlendMode || "").toUpperCase(); font.pixelSize: 6; font.bold: true; color: "#DDD" }
                }

                // Wiggle preview: jittering nameplate
                property real wiggleX: setupController.wiggleEnabled ? Math.sin(glowTimer.phase * (setupController.wiggleFreq || 3) * 2) * (setupController.wiggleAmp || 5) * 0.3 : 0
                property real wiggleY: setupController.wiggleEnabled ? Math.cos(glowTimer.phase * (setupController.wiggleFreq || 3) * 1.5) * (setupController.wiggleAmp || 5) * 0.2 : 0

                // Effect animation timer
                Timer { id: glowTimer; interval: 50; running: true; repeat: true; property real phase: 0; onTriggered: phase += 0.1 }

                // Effect name badge (BIG)
                Rectangle {
                    visible: setupController.animationType !== "slide_left" && setupController.animationType !== ""
                    anchors.bottom: plate.top; anchors.left: plate.left; anchors.bottomMargin: 4
                    width: fxLbl.implicitWidth + 10; height: 16; radius: 3
                    color: Qt.rgba(91/255, 79/255, 219/255, 0.3)
                    Label { id: fxLbl; anchors.centerIn: parent; text: setupController.animationType.replace(/_/g, " ").toUpperCase(); font.pixelSize: 7; font.bold: true; color: "#AAA" }
                }

                } // End QML fallback Item

                // Badges (always visible)
                Rectangle { anchors.top: parent.top; anchors.right: parent.right; anchors.margins: 8; width: pvLbl.implicitWidth+12; height: 18; radius: 4; color: Qt.rgba(0,0,0,0.4); z: 10
                    Label { id: pvLbl; anchors.centerIn: parent; text: previewMonitor.active ? "LIVE" : "PREVIEW"; font.pixelSize: 10; color: previewMonitor.active ? "#1DB954" : "#666" } }
                Rectangle { anchors.top: parent.top; anchors.left: parent.left; anchors.margins: 8; width: stLbl.implicitWidth+12; height: 18; radius: 4; color: Qt.rgba(0,0,0,0.4); z: 10
                    Label { id: stLbl; anchors.centerIn: parent; text: root.previewStyleId.toUpperCase(); font.pixelSize: 10; font.bold: true; color: "#999" } }
            }

            // Replay animation button
            Row {
                spacing: 12
                Rectangle {
                    width: rpLbl.implicitWidth + 16; height: 28; radius: 6
                    color: rpMa.containsMouse ? Qt.rgba(91/255,79/255,219/255,0.15) : (window.darkMode ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.06))
                    Behavior on color { ColorAnimation { duration: 150 } }
                    Label { id: rpLbl; anchors.centerIn: parent; text: "\u25B6  " + window.t("replay_anim"); font.pixelSize: 11; color: window.darkMode ? "#AAA" : "#444" }
                    MouseArea { id: rpMa; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: replayAnim() }
                }
                Label { text: "Délai: " + (setupController.singleFaceDelayMs / 1000).toFixed(1) + "s"; font.pixelSize: 11; color: window.darkMode ? "#555" : "#999"; anchors.verticalCenter: parent.verticalCenter }
            }

            // ── Config sections ────────────────────────────
                ColumnLayout {
                    id: configCol; Layout.fillWidth: true; spacing: 10

                    Label { text: window.t("accent_color"); font.pixelSize: 13; color: window.darkMode ? "#AAA" : "#444" }
                    Row { spacing: 5
                        Repeater { model: ["#E30613","#5B4FDB","#0066CC","#1DB954","#FF6B00","#C8A84E","#00E5FF","#FFF"]; Rectangle { width: 22; height: 22; radius: 11; color: modelData; border.color: modelData===setupController.accentColor.toString()?"white":"transparent"; border.width: 2; MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: setupController.accentColor=modelData } } }
                        ColorPickerButton { currentColor: setupController.accentColor; onColorSelected: function(c) { setupController.accentColor = c } }
                    }

                    Label { text: window.t("opacity"); font.pixelSize: 13; color: window.darkMode ? "#AAA" : "#444" }
                    RowLayout { Slider { from:0.4;to:1;value:setupController.backgroundOpacity;Layout.preferredWidth:140;onMoved:setupController.backgroundOpacity=value } Label { text: Math.round(setupController.backgroundOpacity*100)+"%";color:window.darkMode?"#888":"#555";font.pixelSize:11 } }

                    Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#222" : "#CCC" }
                    Label { text: "Animation (After Effects)"; font.pixelSize: 13; font.bold: true; color: "#5B4FDB" }
                    ComboBox {
                        id: animCombo
                        Layout.fillWidth: true
                        model: [
                            "Title 01 — Simple Tag",
                            "Title 02 — Stacked Blocks",
                            "Title 03 — Modern Design",
                            "Title 04 — Angled Bars",
                            "Title 05 — Clean Bar",
                            "Title 06 — Text Block",
                            "Title 07 — Stylish Line",
                            "Title 08 — Motion Block",
                            "Title 09 — Wide Bar"
                        ]
                        property var values: [
                            "title_01", "title_02", "title_03", "title_04", "title_05",
                            "title_06", "title_07", "title_08", "title_09"
                        ]
                        currentIndex: Math.max(0, values.indexOf(setupController.lottiePreset))
                        onActivated: { setupController.lottiePreset = values[currentIndex] }
                        background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: "#5B4FDB"; border.width: 1 }
                        contentItem: Label { text: animCombo.displayText; color: window.darkMode ? "#CCC" : "#333"; font.pixelSize: 12; leftPadding: 8; verticalAlignment: Text.AlignVCenter }
                    }

                    // ══════════════════════════════════════════════
                    // AE EASING CURVE (Graph Editor)
                    // ══════════════════════════════════════════════
                    Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#222" : "#CCC" }
                    Label { text: "Courbe d'animation (AE)"; font.pixelSize: 13; font.bold: true; color: "#5B4FDB" }
                    ComboBox {
                        Layout.fillWidth: true
                        model: [
                            "— Standard —", "Linear", "Ease Out Cubic", "Ease In Out Cubic",
                            "— Sine —", "Ease In Sine", "Ease Out Sine", "Ease In Out Sine",
                            "— Quad —", "Ease In Quad", "Ease Out Quad", "Ease In Out Quad",
                            "— Quart —", "Ease In Quart", "Ease Out Quart", "Ease In Out Quart",
                            "— Quint —", "Ease In Quint", "Ease Out Quint", "Ease In Out Quint",
                            "— Expo —", "Ease In Expo", "Ease Out Expo", "Ease In Out Expo",
                            "— Circ —", "Ease In Circ", "Ease Out Circ", "Ease In Out Circ",
                            "— Back —", "Ease In Back", "Ease Out Back", "Ease In Out Back",
                            "— Elastic —", "Ease In Elastic", "Ease Out Elastic", "Ease In Out Elastic",
                            "— Bounce —", "Ease In Bounce", "Ease Out Bounce", "Ease In Out Bounce",
                            "— Avancé —", "Spring", "Overshoot", "Anticipation", "Snap", "Rubber Band", "Smoother Step"
                        ]
                        property var values: [
                            "", "linear", "ease_out_cubic", "ease_in_out_cubic",
                            "", "ease_in_sine", "ease_out_sine", "ease_in_out_sine",
                            "", "ease_in_quad", "ease_out_quad", "ease_in_out_quad",
                            "", "ease_in_quart", "ease_out_quart", "ease_in_out_quart",
                            "", "ease_in_quint", "ease_out_quint", "ease_in_out_quint",
                            "", "ease_in_expo", "ease_out_expo", "ease_in_out_expo",
                            "", "ease_in_circ", "ease_out_circ", "ease_in_out_circ",
                            "", "ease_in_back", "ease_out_back", "ease_in_out_back",
                            "", "ease_in_elastic", "ease_out_elastic", "ease_in_out_elastic",
                            "", "ease_in_bounce", "ease_out_bounce", "ease_in_out_bounce",
                            "", "spring", "overshoot", "anticipation", "snap", "rubber_band", "smoother_step"
                        ]
                        currentIndex: 2
                        onActivated: { if (values[currentIndex] !== "") setupController.easingCurve = values[currentIndex] }
                        background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: "#5B4FDB"; border.width: 1 }
                        contentItem: Label { text: parent.displayText; color: window.darkMode ? "#CCC" : "#333"; font.pixelSize: 12; leftPadding: 8; verticalAlignment: Text.AlignVCenter }
                    }

                    // ══════════════════════════════════════════════
                    // AE BLEND MODE (27 modes)
                    // ══════════════════════════════════════════════
                    Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#222" : "#CCC" }
                    Label { text: "Mode de fusion (AE)"; font.pixelSize: 13; font.bold: true; color: "#5B4FDB" }
                    ComboBox {
                        Layout.fillWidth: true
                        model: [
                            "Normal", "Dissolve",
                            "— Assombrissement —", "Darken", "Multiply", "Color Burn", "Linear Burn", "Darker Color",
                            "— Éclaircissement —", "Lighten", "Screen", "Color Dodge", "Linear Dodge", "Lighter Color",
                            "— Contraste —", "Overlay", "Soft Light", "Hard Light", "Vivid Light", "Linear Light", "Pin Light", "Hard Mix",
                            "— Inversion —", "Difference", "Exclusion", "Subtract", "Divide",
                            "— Composant —", "Hue", "Saturation", "Color", "Luminosity"
                        ]
                        property var values: [
                            "normal", "dissolve",
                            "", "darken", "multiply", "color_burn", "linear_burn", "darker_color",
                            "", "lighten", "screen", "color_dodge", "linear_dodge", "lighter_color",
                            "", "overlay", "soft_light", "hard_light", "vivid_light", "linear_light", "pin_light", "hard_mix",
                            "", "difference", "exclusion", "subtract", "divide",
                            "", "hue", "saturation", "color", "luminosity"
                        ]
                        currentIndex: 0
                        onActivated: { if (values[currentIndex] !== "") setupController.overlayBlendMode = values[currentIndex] }
                        background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: "#5B4FDB"; border.width: 1 }
                        contentItem: Label { text: parent.displayText; color: window.darkMode ? "#CCC" : "#333"; font.pixelSize: 12; leftPadding: 8; verticalAlignment: Text.AlignVCenter }
                    }

                    // ══════════════════════════════════════════════
                    // AE POST-EFFECTS on Overlay
                    // ══════════════════════════════════════════════
                    Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#222" : "#CCC" }
                    Label { text: "Effet AE sur overlay"; font.pixelSize: 13; font.bold: true; color: "#5B4FDB" }
                    ComboBox {
                        id: aeEffectCombo; Layout.fillWidth: true
                        model: [
                            "Aucun",
                            "— Distortion (14) —", "Turbulent Displace", "Twirl", "Spherize", "Bulge", "Ripple", "Wave Warp", "Kaleidoscope", "Mirror", "Pixelate", "Mosaic", "Polar Coords", "Mesh Warp", "Posterize", "Reshape",
                            "— Color (17) —", "Curves", "Levels", "Hue/Saturation", "Brightness/Contrast", "Exposure", "Tint", "Tritone", "Colorama", "Leave Color", "Vibrance", "Photo Filter", "Gradient Map", "Black & White", "Invert", "Threshold", "Solarize", "Color Balance",
                            "— Generate (15) —", "Fractal Noise", "Cell Pattern", "Grid", "Gradient Ramp", "Vegas", "Radio Waves", "Audio Spectrum", "Lens Flare", "Light Burst", "Beam", "4-Color Gradient", "Fill", "Stroke", "Circle Burst", "Checkerboard",
                            "— Stylize (13) —", "Emboss", "Find Edges", "Roughen Edges", "Scatter", "Glow", "Cartoon", "Halftone", "Stained Glass", "Noise", "Strobe", "Motion Tile", "Cross Hatch", "Oil Paint",
                            "— Perspective (7) —", "CC Sphere", "CC Cylinder", "Bevel Alpha", "Drop Shadow", "Radial Shadow", "3D Rotation", "Reflection",
                            "— Time (3) —", "Echo", "Trails", "Force Motion Blur",
                            "— Matte (1) —", "Luma Key"
                        ]
                        property var values: [
                            "",
                            "", "turbulent_displace", "twirl", "spherize", "bulge", "ripple", "wave_warp", "kaleidoscope", "mirror", "pixelate", "mosaic", "polar_coords", "mesh_warp", "posterize", "reshape",
                            "", "curves", "levels", "hue_saturation", "brightness_contrast", "exposure", "tint", "tritone", "colorama", "leave_color", "vibrance", "photo_filter", "gradient_map", "black_white", "invert", "threshold", "solarize", "color_balance",
                            "", "fractal_noise", "cell_pattern", "grid", "gradient_ramp", "vegas", "radio_waves", "audio_spectrum", "lens_flare_gen", "light_burst", "beam", "4color_gradient", "fill", "stroke", "circle_burst", "checkerboard",
                            "", "emboss", "find_edges", "roughen_edges", "scatter", "stylize_glow", "cartoon", "halftone", "stained_glass", "noise", "strobe", "motion_tile", "cross_hatch", "oil_paint",
                            "", "cc_sphere", "cc_cylinder", "bevel_alpha", "drop_shadow", "radial_shadow", "3d_rotation", "reflection",
                            "", "echo", "trails", "force_motion_blur",
                            "", "luma_key"
                        ]
                        currentIndex: 0
                        onActivated: { setupController.aeEffectId = values[currentIndex] || "" }
                        background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: "#5B4FDB"; border.width: 1 }
                        contentItem: Label { text: parent.displayText; color: window.darkMode ? "#CCC" : "#333"; font.pixelSize: 12; leftPadding: 8; verticalAlignment: Text.AlignVCenter }
                    }

                    // AE Effect parameters (visible when an effect is selected)
                    ColumnLayout {
                        visible: setupController.aeEffectId !== ""
                        Layout.fillWidth: true; spacing: 6

                        RowLayout { Label{text:"Intensité:";color:window.darkMode?"#999":"#666";font.pixelSize:12} Slider{from:0;to:1;value:0.5;Layout.preferredWidth:120;onMoved:setupController.aeEffectIntensity=value} Label{text:Math.round(value*100)+"%";color:window.darkMode?"#888":"#555";font.pixelSize:11} }
                        RowLayout { Label{text:"Param 1:";color:window.darkMode?"#999":"#666";font.pixelSize:12} Slider{from:0;to:1;value:0.5;Layout.preferredWidth:120;onMoved:setupController.aeEffectParam1=value} Label{text:Math.round(value*100)+"%";color:window.darkMode?"#888":"#555";font.pixelSize:11} }
                        RowLayout { Label{text:"Param 2:";color:window.darkMode?"#999":"#666";font.pixelSize:12} Slider{from:0;to:1;value:0.5;Layout.preferredWidth:120;onMoved:setupController.aeEffectParam2=value} Label{text:Math.round(value*100)+"%";color:window.darkMode?"#888":"#555";font.pixelSize:11} }

                        Row { spacing: 5
                            Label { text: "Couleur 1:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 12; anchors.verticalCenter: parent.verticalCenter }
                            ColorPickerButton { currentColor: setupController.aeEffectColor1 || "#E30613"; onColorSelected: function(c) { setupController.aeEffectColor1 = c } }
                            Label { text: "Couleur 2:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 12; anchors.verticalCenter: parent.verticalCenter }
                            ColorPickerButton { currentColor: setupController.aeEffectColor2 || "#FFFFFF"; onColorSelected: function(c) { setupController.aeEffectColor2 = c } }
                        }
                    }

                    // ══════════════════════════════════════════════
                    // AE EXPRESSION: Wiggle
                    // ══════════════════════════════════════════════
                    Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#222" : "#CCC" }
                    RowLayout {
                        Label { text: "Expression: Wiggle"; font.pixelSize: 13; font.bold: true; color: "#5B4FDB" }
                        Switch { checked: false; onCheckedChanged: setupController.wiggleEnabled = checked }
                    }
                    RowLayout {
                        visible: setupController.wiggleEnabled
                        Label{text:"Freq:";color:window.darkMode?"#999":"#666";font.pixelSize:12} Slider{from:0.5;to:15;value:3;Layout.preferredWidth:80;onMoved:setupController.wiggleFreq=value} Label{text:value.toFixed(1);color:window.darkMode?"#888":"#555";font.pixelSize:11}
                        Label{text:"Amp:";color:window.darkMode?"#999":"#666";font.pixelSize:12} Slider{from:1;to:30;value:5;Layout.preferredWidth:80;onMoved:setupController.wiggleAmp=value} Label{text:Math.round(value)+"px";color:window.darkMode?"#888":"#555";font.pixelSize:11}
                    }

                    Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#222" : "#CCC" }
                    Label { text: window.t("multi_face"); font.pixelSize: 13; color: window.darkMode ? "#AAA" : "#444" }
                    TextField { Layout.fillWidth: true; text:setupController.multiFaceName; placeholderText:"Titre multi-visage"; onTextChanged:setupController.multiFaceName=text; font.pixelSize:12; color:window.darkMode?"#FFF":"#1A1A1A"; background:Rectangle{color:window.darkMode?"#1E1E22":"#F0F0F4";radius:4;border.color:window.darkMode?"#333":"#CCC"} }
                    TextField { Layout.fillWidth: true; text:setupController.multiFaceRole; placeholderText:"Sous-titre"; onTextChanged:setupController.multiFaceRole=text; font.pixelSize:12; color:window.darkMode?"#FFF":"#1A1A1A"; background:Rectangle{color:window.darkMode?"#1E1E22":"#F0F0F4";radius:4;border.color:window.darkMode?"#333":"#CCC"} }

                    Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#222" : "#CCC" }
                    Label { text: window.t("delays"); font.pixelSize: 13; color: window.darkMode ? "#AAA" : "#444" }
                    RowLayout { Label{text:"1 visage:";color:window.darkMode?"#999":"#666";font.pixelSize:12} Slider{from:0;to:3000;stepSize:100;value:setupController.singleFaceDelayMs;Layout.preferredWidth:100;onMoved:setupController.singleFaceDelayMs=value} Label{text:(setupController.singleFaceDelayMs/1000).toFixed(1)+"s";color:window.darkMode?"#888":"#555";font.pixelSize:11} }
                    RowLayout { Label{text:"Multi:";color:window.darkMode?"#999":"#666";font.pixelSize:12} Slider{from:0;to:3000;stepSize:100;value:setupController.multiFaceDelayMs;Layout.preferredWidth:100;onMoved:setupController.multiFaceDelayMs=value} Label{text:(setupController.multiFaceDelayMs/1000).toFixed(1)+"s";color:window.darkMode?"#888":"#555";font.pixelSize:11} }
                    RowLayout { Label{text:"Masquer:";color:window.darkMode?"#999":"#666";font.pixelSize:12} Slider{from:500;to:5000;stepSize:250;value:setupController.overlayHideDelayMs;Layout.preferredWidth:100;onMoved:setupController.overlayHideDelayMs=value} Label{text:(setupController.overlayHideDelayMs/1000).toFixed(1)+"s";color:window.darkMode?"#888":"#555";font.pixelSize:11} }
                }

                Item { implicitHeight: 20 }
            } // end rightCol
        } // end Flickable
    } // end RowLayout

    // ── Animation replay ─────────────────────────────────

    // Step 1: disable Behaviors, set start position instantly
    // Step 2: after delay (singleFaceDelayMs), re-enable Behaviors + animate to final
    Timer {
        id: replayDelayTimer
        interval: setupController.singleFaceDelayMs
        onTriggered: {
            // Re-enable Behaviors (they animate to the new values)
            plateBehaviorX.enabled = true
            plateBehaviorY.enabled = true
            plateBehaviorOp.enabled = true
            plateBehaviorSc.enabled = true

            // Set final position — Behaviors animate smoothly
            plate.animX = 0
            plate.animY = 0
            plate.animOpacity = 1
            plate.animScale = 1
        }
    }

    function replayAnim() {
        var at = setupController.animationType

        // Disable Behaviors so initial position is set INSTANTLY (no animation)
        plateBehaviorX.enabled = false
        plateBehaviorY.enabled = false
        plateBehaviorOp.enabled = false
        plateBehaviorSc.enabled = false

        // Set start position based on animation type
        plate.animX = 0
        plate.animY = 0
        plate.animOpacity = 0
        plate.animScale = 1

        if (at === "slide_left")       plate.animX = -pvBox.width * 0.5
        else if (at === "slide_right") plate.animX = pvBox.width * 0.5
        else if (at === "slide_up")    plate.animY = 30
        else if (at === "fade_scale")  { plate.animScale = 0.92 }
        else if (at === "iris")        { plate.animScale = 0.6 }
        // wipe, fade, glitch: only opacity changes (already 0)

        // Wait for the configured delay, then animate in
        replayDelayTimer.interval = Math.max(100, setupController.singleFaceDelayMs)
        replayDelayTimer.restart()
    }

    // Replay when animation type changes
    Connections {
        target: setupController
        function onAnimationChanged() { replayAnim() }
    }

    // ── Helper functions ──────────────────────────────────
    function getBg(s) {
        // Broadcast preview background per style (these are on-air colors, not UI colors)
        var dark = {"bfm":"#0B0B10","lci":"#1A1A18","france2":"#F8F8FF","france24":"#0B0B10","cnn":"#CC0000","bbc":"#BB1919","skynews":"#004A9E","aljazeera":"#6B0000","sports":"#0B0B10","football":"#1A4A1A","olympics":"#F8F8FC","politique":"#1B2A4A","gouvernement":"#F8F8FC","cinema":"#000","luxury":"#0D0D0D","tech":"#0A0F1A","minimal":"transparent","dual":"#0B0B10","fullscreen":"#0B0B10","breaking":"#0B0B10"}
        return dark[s] || (window.darkMode ? "#0B0B10" : "#E8E8EE")
    }
    function getAccent(s) { return ({"bfm":"#E30613","lci":"#FF6D00","france2":"#003189","france24":"#D4001A","cnn":"#FFF","bbc":"#FFF","skynews":"#00A0DC","aljazeera":"#D4A017","sports":"#00FF88","football":"#FFFF00","olympics":"#0033A0","politique":"#002395","gouvernement":"#002395","cinema":"#D4AF37","luxury":"#C0C0C0","tech":"#00E5FF","minimal":"#FFF","dual":"#5B4FDB","fullscreen":"#5B4FDB","breaking":"#CC0000"})[s]||"#E30613" }
    function getNameColor(s) { if(s==="aljazeera"||s==="cinema")return"#D4A017"; if(s==="luxury")return"#F0F0F0"; if(s==="france2"||s==="gouvernement"||s==="olympics")return"#003189"; return"#FFF" }
    function getRoleColor(s) { if(s==="france2"||s==="gouvernement"||s==="olympics")return"#555"; if(s==="tech")return"#00E5FF"; if(s==="sports"||s==="football")return getAccent(s); if(s==="cinema")return"#C0A060"; if(s==="luxury")return"#888"; return"#CCC" }
    function isWhite(s) { return s==="france2"||s==="gouvernement"||s==="olympics" }
    function getName(s) { return({"bfm":"Marie Dupont","lci":"Sophie Martin","france2":"Jean-Pierre Elkabbach","france24":"Claire Fournier","cnn":"Anderson Cooper","bbc":"Fiona Bruce","skynews":"Kay Burley","aljazeera":"Kamahl Santamaria","sports":"Kylian Mbappé","football":"Antoine Griezmann","olympics":"Léon Marchand","politique":"Marine Le Pen","gouvernement":"Gabriel Attal","cinema":"Léa Seydoux","luxury":"Marion Cotillard","tech":"Sam Altman","minimal":"Thomas Pesquet","dual":"Débat du Soir","fullscreen":"Philippe Labro","breaking":"Alerte Info"})[s]||"Marie Dupont" }
    function getRole(s) { return({"bfm":"Présentatrice","lci":"Journaliste politique","france2":"Présentateur JT","france24":"Correspondante int.","cnn":"Anchor CNN","bbc":"BBC Newsreader","skynews":"Sky News Presenter","aljazeera":"Al Jazeera Correspondent","sports":"Attaquant · France","football":"Milieu · Atlético","olympics":"Natation · 400m","politique":"Présidente RN","gouvernement":"Premier Ministre","cinema":"Festival de Cannes","luxury":"Ambassadrice Dior","tech":"CEO · OpenAI","minimal":"Astronaute · ESA","dual":"En direct du plateau","fullscreen":"Écrivain & Journaliste","breaking":"DERNIÈRE MINUTE"})[s]||"Présentatrice" }
    function getTxtMargin(s) { if(s==="aljazeera"||s==="sports")return plate.width*0.16; if(s==="olympics")return plate.width*0.2; return 12 }
}
