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
                Rectangle {
                    id: pvBox; Layout.fillWidth: true; Layout.preferredHeight: width * 9 / 16
                color: window.darkMode ? "#0A0A0E" : "#E8E8EE"; radius: 10; clip: true

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

                // Badges
                Rectangle { anchors.top: parent.top; anchors.right: parent.right; anchors.margins: 8; width: pvLbl.implicitWidth+12; height: 18; radius: 4; color: Qt.rgba(0,0,0,0.4); Label { id: pvLbl; anchors.centerIn: parent; text: "PREVIEW"; font.pixelSize: 10; color: "#666" } }
                Rectangle { anchors.top: parent.top; anchors.left: parent.left; anchors.margins: 8; width: stLbl.implicitWidth+12; height: 18; radius: 4; color: Qt.rgba(0,0,0,0.4); Label { id: stLbl; anchors.centerIn: parent; text: root.previewStyleId.toUpperCase(); font.pixelSize: 10; font.bold: true; color: "#999" } }
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
                    Row { spacing: 5; Repeater { model: ["#E30613","#5B4FDB","#0066CC","#1DB954","#FF6B00","#C8A84E","#00E5FF","#FFF"]; Rectangle { width: 22; height: 22; radius: 11; color: modelData; border.color: modelData===setupController.accentColor.toString()?"white":"transparent"; border.width: 2; MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: setupController.accentColor=modelData } } } }

                    Label { text: window.t("opacity"); font.pixelSize: 13; color: window.darkMode ? "#AAA" : "#444" }
                    RowLayout { Slider { from:0.4;to:1;value:setupController.backgroundOpacity;Layout.preferredWidth:140;onMoved:setupController.backgroundOpacity=value } Label { text: Math.round(setupController.backgroundOpacity*100)+"%";color:window.darkMode?"#888":"#555";font.pixelSize:11 } }

                    Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#222" : "#CCC" }
                    Label { text: window.t("animation"); font.pixelSize: 13; color: window.darkMode ? "#AAA" : "#444" }
                    ComboBox {
                        id: animCombo
                        Layout.fillWidth: true
                        model: [
                            "— Classiques —", "Glisser gauche", "Glisser droite", "Monter", "Fondu+zoom", "Balayage", "Fondu", "Iris", "Glitch",
                            "— Text —", "Typewriter", "Bounce In", "Wave", "Tracking Expand", "Fade Up Letter", "Scale Up Letter", "Rotate In Letter", "Blur In", "Slide Per Letter", "Kinetic Pop",
                            "— Lower Third —", "Line Draw", "Bar Slide", "Shape Morph", "Split Reveal", "Bracket Expand", "Underline Grow", "Box Wipe", "Corner Build",
                            "— Logo —", "Fade Glow", "Light Streak", "Particle Form", "Scale Bounce", "Shatter In", "Blur Zoom", "Rotate 3D", "Pulse Reveal",
                            "— Glow —", "Neon Glow", "Bloom", "Shimmer", "Edge Glow",
                            "— Distortion —", "Glitch RGB", "Chromatic Aberration", "VHS Effect"
                        ]
                        property var values: [
                            "", "slide_left", "slide_right", "slide_up", "fade_scale", "wipe", "fade", "iris", "glitch",
                            "", "typewriter", "bounce_in", "wave_text", "tracking_expand", "fade_up_letter", "scale_up_letter", "rotate_in_letter", "blur_in", "slide_per_letter", "kinetic_pop",
                            "", "line_draw", "bar_slide", "shape_morph", "split_reveal", "bracket_expand", "underline_grow", "box_wipe", "corner_build",
                            "", "fade_glow", "light_streak", "particle_form", "scale_bounce", "shatter_in", "blur_zoom", "rotate_3d", "pulse_reveal",
                            "", "neon_glow", "bloom", "shimmer", "edge_glow",
                            "", "glitch_rgb", "chromatic_aberration", "vhs_effect"
                        ]
                        currentIndex: Math.max(0, values.indexOf(setupController.animationType))
                        onActivated: { if (values[currentIndex] !== "") setupController.animationType = values[currentIndex] }
                        background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
                        contentItem: Label { text: animCombo.displayText; color: window.darkMode ? "#CCC" : "#333"; font.pixelSize: 12; leftPadding: 8; verticalAlignment: Text.AlignVCenter }
                    }

                    RowLayout { Label{text:"Entrée:";color:window.darkMode?"#999":"#666";font.pixelSize:12} Slider{from:3;to:45;stepSize:1;value:setupController.animEnterFrames;Layout.preferredWidth:100;onMoved:setupController.animEnterFrames=value} Label{text:setupController.animEnterFrames+"f";color:window.darkMode?"#888":"#555";font.pixelSize:11} }
                    RowLayout { Label{text:"Sortie:";color:window.darkMode?"#999":"#666";font.pixelSize:12} Slider{from:2;to:20;stepSize:1;value:setupController.animExitFrames;Layout.preferredWidth:100;onMoved:setupController.animExitFrames=value} Label{text:setupController.animExitFrames+"f";color:window.darkMode?"#888":"#555";font.pixelSize:11} }

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
