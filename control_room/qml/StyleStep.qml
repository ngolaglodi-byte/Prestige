import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root

    property string previewStyleId: setupController.selectedStyle
    property string previewAnimType: setupController.animationType

    RowLayout {
        anchors.fill: parent; anchors.margins: 20; spacing: 20

        // ── Left: Style list (scrollable) — 50% ────────────
        ColumnLayout {
            Layout.preferredWidth: (parent.width - 20) / 2
            Layout.fillHeight: true; spacing: 10

            // Section header
            RowLayout {
                spacing: 8
                Rectangle {
                    Layout.preferredWidth: 28; Layout.preferredHeight: 28; radius: 7
                    color: Qt.rgba(108/255,92/255,231/255,0.12)
                    Label { anchors.centerIn: parent; text: "\u25A0"; font.pixelSize: 12; color: "#6C5CE7" }
                }
                ColumnLayout {
                    spacing: 0
                    Label { text: window.t("overlay_style"); font.pixelSize: 15; font.weight: Font.Bold; color: window.darkMode ? "#F0F0F5" : "#0F0F14" }
                    Label { text: window.t("hover_preview"); font.pixelSize: 11; color: window.darkMode ? "#505060" : "#999" }
                }
            }

            ListView {
                id: styleList; Layout.fillWidth: true; Layout.fillHeight: true
                spacing: 3; clip: true
                model: ListModel {
                    ListElement { sid: "title_01"; nm: "Simple Tag"; ref: "Animation epuree, tag simple" }
                    ListElement { sid: "title_02"; nm: "Stacked Blocks"; ref: "Blocs empiles, moderne" }
                    ListElement { sid: "title_03"; nm: "Modern Design"; ref: "Design contemporain" }
                    ListElement { sid: "title_04"; nm: "Angled Bars"; ref: "Barres obliques, dynamique" }
                    ListElement { sid: "title_05"; nm: "Clean Bar"; ref: "Barre propre, classique" }
                    ListElement { sid: "title_06"; nm: "Text Block"; ref: "Bloc texte, pro" }
                    ListElement { sid: "title_07"; nm: "Stylish Line"; ref: "Ligne stylisee, elegant" }
                    ListElement { sid: "title_08"; nm: "Motion Block"; ref: "Bloc en mouvement" }
                    ListElement { sid: "title_09"; nm: "Wide Bar"; ref: "Barre large, broadcast" }
                }
                delegate: Rectangle {
                    id: styleDelegate
                    property bool isSelected: sid === setupController.lottiePreset
                    width: styleList.width; height: 54; radius: 10
                    color: isSelected
                        ? (window.darkMode ? Qt.rgba(108/255,92/255,231/255,0.14) : Qt.rgba(108/255,92/255,231/255,0.10))
                        : (ma.containsMouse ? (window.darkMode ? Qt.rgba(255,255,255,0.04) : Qt.rgba(0,0,0,0.04)) : (window.darkMode ? "#0D0D12" : "#E4E4EA"))
                    border.color: isSelected ? Qt.rgba(108/255,92/255,231/255,0.35) : "transparent"
                    border.width: isSelected ? 1 : 0
                    Behavior on color { ColorAnimation { duration: 150 } }
                    Behavior on border.color { ColorAnimation { duration: 150 } }

                    RowLayout {
                        anchors.fill: parent; anchors.leftMargin: 10; anchors.rightMargin: 12; anchors.topMargin: 6; anchors.bottomMargin: 6; spacing: 10

                        // Accent bar with gradient
                        Rectangle {
                            Layout.preferredWidth: 3; Layout.preferredHeight: 34; radius: 2
                            gradient: Gradient {
                                GradientStop { position: 0.0; color: isSelected ? "#8B7AFF" : "#6C5CE7" }
                                GradientStop { position: 1.0; color: isSelected ? "#6C5CE7" : "#5A4BD4" }
                            }
                            opacity: isSelected ? 1.0 : 0.4
                            Behavior on opacity { NumberAnimation { duration: 200 } }
                        }

                        ColumnLayout { Layout.fillWidth: true; spacing: 2
                            Label {
                                text: nm; font.pixelSize: 13; font.weight: Font.DemiBold
                                color: isSelected ? (window.darkMode ? "#F0F0F5" : "#0F0F14") : (window.darkMode ? "#B0B0BC" : "#333")
                            }
                            Label {
                                text: ref; font.pixelSize: 10
                                color: window.darkMode ? "#505060" : "#999"
                            }
                        }

                        // Checkmark
                        Rectangle {
                            Layout.preferredWidth: 22; Layout.preferredHeight: 22; radius: 11
                            color: isSelected ? "#6C5CE7" : "transparent"
                            border.color: isSelected ? "#6C5CE7" : (window.darkMode ? Qt.rgba(255,255,255,0.08) : Qt.rgba(0,0,0,0.10))
                            border.width: 1
                            Behavior on color { ColorAnimation { duration: 200 } }
                            Label {
                                anchors.centerIn: parent; text: "\u2713"
                                color: "white"; font.pixelSize: 11; font.weight: Font.Bold
                                opacity: isSelected ? 1 : 0
                                Behavior on opacity { NumberAnimation { duration: 200 } }
                            }
                        }
                    }
                    MouseArea {
                        id: ma; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                        onClicked: { setupController.lottiePreset = sid }
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

                // Section header
                RowLayout {
                    spacing: 8
                    Rectangle {
                        Layout.preferredWidth: 28; Layout.preferredHeight: 28; radius: 7
                        color: Qt.rgba(108/255,92/255,231/255,0.12)
                        Label { anchors.centerIn: parent; text: "\u25B6"; font.pixelSize: 12; color: "#6C5CE7" }
                    }
                    ColumnLayout {
                        spacing: 0
                        Label { text: window.t("preview"); font.pixelSize: 15; font.weight: Font.Bold; color: window.darkMode ? "#F0F0F5" : "#0F0F14" }
                        Label { text: setupController.lottiePreset.replace("_"," ").toUpperCase(); font.pixelSize: 10; color: window.darkMode ? "#505060" : "#999" }
                    }
                }

                // ── PREVIEW BOX ────────────────────────────────
                Rectangle {
                    id: pvBox; Layout.fillWidth: true; Layout.preferredHeight: width * 9 / 16
                    color: "#000000"; radius: 8; clip: true

                    // Simulated camera background (dark studio gradient)
                    Rectangle {
                        anchors.fill: parent; radius: 8
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: "#141820" }
                            GradientStop { position: 0.5; color: "#0C1018" }
                            GradientStop { position: 1.0; color: "#080A10" }
                        }
                    }

                    // Subtle simulated person silhouette (gives broadcast context)
                    Rectangle {
                        anchors.horizontalCenter: parent.horizontalCenter
                        y: parent.height * 0.08; width: parent.width * 0.09; height: width
                        radius: width / 2; color: Qt.rgba(1,1,1,0.05)
                    }
                    Rectangle {
                        anchors.horizontalCenter: parent.horizontalCenter
                        y: parent.height * 0.18; width: parent.width * 0.18; height: parent.height * 0.5
                        radius: width * 0.1; color: Qt.rgba(1,1,1,0.03)
                    }

                    // Lottie animation preview — rendered by C++ LottiePreviewProvider
                    Image {
                        id: lottiePreview
                        anchors.fill: parent
                        fillMode: Image.PreserveAspectFit
                        cache: false
                        source: "image://lottie/" + setupController.lottiePreset + "?" + lottieFrameCounter
                        property int lottieFrameCounter: 0
                    }
                    Timer {
                        interval: 50; running: true; repeat: true  // ~20fps for smoother preview
                        onTriggered: lottiePreview.lottieFrameCounter++
                    }

                    // Top-left badge showing current preset
                    Rectangle {
                        anchors.top: parent.top; anchors.left: parent.left; anchors.margins: 8
                        width: stLbl.implicitWidth + 12; height: 20; radius: 4
                        color: Qt.rgba(0,0,0,0.5); z: 10
                        Label { id: stLbl; anchors.centerIn: parent; text: setupController.lottiePreset.replace("_"," ").toUpperCase(); font.pixelSize: 9; font.bold: true; color: "#999" }
                    }

                    // LIVE badge (if VE connected)
                    Rectangle {
                        anchors.top: parent.top; anchors.right: parent.right; anchors.margins: 8
                        width: pvLbl2.implicitWidth + 12; height: 20; radius: 4
                        color: Qt.rgba(0,0,0,0.5); z: 10
                        Label { id: pvLbl2; anchors.centerIn: parent; text: previewMonitor.active ? "LIVE" : "PREVIEW"; font.pixelSize: 9; color: previewMonitor.active ? "#00D68F" : "#666" }
                    }

                // Hidden fallback item (kept for replay animation code compatibility)
                Item {
                    anchors.fill: parent; visible: false

                Rectangle { anchors.fill: parent }
                Rectangle { id: plate; property real animX: 0; property real animY: 0; property real animOpacity: 1; property real animScale: 1; visible: false
                    Behavior on animX { id: plateBehaviorX; NumberAnimation { duration: 500; easing.type: Easing.OutCubic } }
                    Behavior on animY { id: plateBehaviorY; NumberAnimation { duration: 500; easing.type: Easing.OutCubic } }
                    Behavior on animOpacity { id: plateBehaviorOp; NumberAnimation { duration: 500; easing.type: Easing.OutCubic } }
                    Behavior on animScale { id: plateBehaviorSc; NumberAnimation { duration: 500; easing.type: Easing.OutCubic } }

                    // ── TALENT PAR DEFAUT ──────────────────
                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left; anchors.leftMargin: 12
                        spacing: 2
                        Text {
                            text: "Marie Dupont"
                            color: "white"
                            font.pixelSize: Math.max(10, pvBox.height * 0.035)
                            font.bold: true
                            font.family: "Helvetica Neue"
                        }
                        Text {
                            text: "Journaliste - Prestige TV"
                            color: "#AAA"
                            font.pixelSize: Math.max(7, pvBox.height * 0.022)
                            font.family: "Helvetica Neue"
                        }
                    }

                    // Animation name badge
                    Rectangle {
                        anchors.top: parent.top; anchors.right: parent.right; anchors.margins: 4
                        width: animLbl.implicitWidth + 8; height: 14; radius: 3
                        color: Qt.rgba(108/255, 92/255, 231/255, 0.4)
                        Label { id: animLbl; anchors.centerIn: parent; text: setupController.lottiePreset.replace("_", " ").toUpperCase(); font.pixelSize: 6; font.bold: true; color: "#DDD" }
                    }
                }

                // ── Lottie animation indicator ──────────

                // (Effect previews removed — Lottie handles all visual styling)
                Repeater {
                    model: 0
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

                // Animation timer for preview
                Timer { id: glowTimer; interval: 50; running: true; repeat: true; property real phase: 0; onTriggered: phase += 0.1 }

                } // End QML fallback Item

            } // End pvBox

            // Replay animation button
            Row {
                spacing: 12
                Rectangle {
                    width: rpLbl.implicitWidth + 16; height: 28; radius: 6
                    color: rpMa.containsMouse ? Qt.rgba(108/255,92/255,231/255,0.15) : (window.darkMode ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.06))
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
                    RowLayout { spacing: 6
                        Rectangle { Layout.preferredWidth: 30; Layout.preferredHeight: 30; radius: 6; color: setupController.accentColor; border.color: window.darkMode ? "#444" : "#BBB"; border.width: 1 }
                        TextField {
                            Layout.preferredWidth: 90; text: setupController.accentColor.toString().toUpperCase(); font.pixelSize: 11; font.family: "SF Mono, Menlo, monospace"; color: window.darkMode ? "white" : "#1A1A1A"
                            onTextEdited: { if (text.match(/^#[0-9A-Fa-f]{6}$/i)) setupController.accentColor = text }
                            background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
                        }
                        ColorPickerButton { currentColor: setupController.accentColor; onColorSelected: function(c) { setupController.accentColor = c } }
                        // Quick presets (compact)
                        Repeater { model: ["#E30613","#6C5CE7","#0066CC","#00D68F","#FF6B00","#D4AF37","#00E5FF","#FFFFFF"]; Rectangle { width: 18; height: 18; radius: 9; color: modelData; border.color: modelData===setupController.accentColor.toString()?"white":"transparent"; border.width: 1.5; MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: setupController.accentColor=modelData } } }
                    }

                    Label { text: window.t("opacity"); font.pixelSize: 13; color: window.darkMode ? "#AAA" : "#444" }
                    RowLayout { Slider { from:0.4;to:1;value:setupController.backgroundOpacity;Layout.preferredWidth:140;onMoved:setupController.backgroundOpacity=value } Label { text: Math.round(setupController.backgroundOpacity*100)+"%";color:window.darkMode?"#888":"#555";font.pixelSize:11 } }

                    // ══════════════════════════════════════════════
                    // AE EASING CURVE (Graph Editor)
                    // ══════════════════════════════════════════════
                    Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#222" : "#CCC" }
                    Label { text: "Courbe d'animation (AE)"; font.pixelSize: 13; font.bold: true; color: "#6C5CE7" }
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
                        background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: "#6C5CE7"; border.width: 1 }
                        contentItem: Label { text: parent.displayText; color: window.darkMode ? "#CCC" : "#333"; font.pixelSize: 12; leftPadding: 8; verticalAlignment: Text.AlignVCenter }
                    }

                    // ══════════════════════════════════════════════
                    // AE BLEND MODE (27 modes)
                    // ══════════════════════════════════════════════
                    Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#222" : "#CCC" }
                    Label { text: "Mode de fusion (AE)"; font.pixelSize: 13; font.bold: true; color: "#6C5CE7" }
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
                        background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: "#6C5CE7"; border.width: 1 }
                        contentItem: Label { text: parent.displayText; color: window.darkMode ? "#CCC" : "#333"; font.pixelSize: 12; leftPadding: 8; verticalAlignment: Text.AlignVCenter }
                    }

                    // ══════════════════════════════════════════════
                    // AE POST-EFFECTS on Overlay
                    // ══════════════════════════════════════════════
                    Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#222" : "#CCC" }
                    Label { text: "Effet AE sur overlay"; font.pixelSize: 13; font.bold: true; color: "#6C5CE7" }
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
                        background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: "#6C5CE7"; border.width: 1 }
                        contentItem: Label { text: parent.displayText; color: window.darkMode ? "#CCC" : "#333"; font.pixelSize: 12; leftPadding: 8; verticalAlignment: Text.AlignVCenter }
                    }

                    // AE Effect parameters (visible when an effect is selected)
                    ColumnLayout {
                        visible: setupController.aeEffectId !== ""
                        Layout.fillWidth: true; spacing: 6

                        RowLayout { Label{text:"Intensité:";color:window.darkMode?"#999":"#666";font.pixelSize:12} Slider{id:slIntensity;from:0;to:1;value:0.5;Layout.preferredWidth:120;onMoved:setupController.aeEffectIntensity=value} Label{text:Math.round(slIntensity.value*100)+"%";color:window.darkMode?"#888":"#555";font.pixelSize:11} }
                        RowLayout { Label{text:"Param 1:";color:window.darkMode?"#999":"#666";font.pixelSize:12} Slider{id:slParam1;from:0;to:1;value:0.5;Layout.preferredWidth:120;onMoved:setupController.aeEffectParam1=value} Label{text:Math.round(slParam1.value*100)+"%";color:window.darkMode?"#888":"#555";font.pixelSize:11} }
                        RowLayout { Label{text:"Param 2:";color:window.darkMode?"#999":"#666";font.pixelSize:12} Slider{id:slParam2;from:0;to:1;value:0.5;Layout.preferredWidth:120;onMoved:setupController.aeEffectParam2=value} Label{text:Math.round(slParam2.value*100)+"%";color:window.darkMode?"#888":"#555";font.pixelSize:11} }

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
                        Label { text: "Expression: Wiggle"; font.pixelSize: 13; font.bold: true; color: "#6C5CE7" }
                        Switch { checked: false; onCheckedChanged: setupController.wiggleEnabled = checked }
                    }
                    RowLayout {
                        visible: setupController.wiggleEnabled
                        Label{text:"Freq:";color:window.darkMode?"#999":"#666";font.pixelSize:12} Slider{id:slFreq;from:0.5;to:15;value:3;Layout.preferredWidth:80;onMoved:setupController.wiggleFreq=value} Label{text:slFreq.value.toFixed(1);color:window.darkMode?"#888":"#555";font.pixelSize:11}
                        Label{text:"Amp:";color:window.darkMode?"#999":"#666";font.pixelSize:12} Slider{id:slAmp;from:1;to:30;value:5;Layout.preferredWidth:80;onMoved:setupController.wiggleAmp=value} Label{text:Math.round(slAmp.value)+"px";color:window.darkMode?"#888":"#555";font.pixelSize:11}
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
    function getAccent(s) { return ({"bfm":"#E30613","lci":"#FF6D00","france2":"#003189","france24":"#D4001A","cnn":"#FFF","bbc":"#FFF","skynews":"#00A0DC","aljazeera":"#D4A017","sports":"#00FF88","football":"#FFFF00","olympics":"#0033A0","politique":"#002395","gouvernement":"#002395","cinema":"#D4AF37","luxury":"#C0C0C0","tech":"#00E5FF","minimal":"#FFF","dual":"#6C5CE7","fullscreen":"#6C5CE7","breaking":"#CC0000"})[s]||"#E30613" }
    function getNameColor(s) { if(s==="aljazeera"||s==="cinema")return"#D4A017"; if(s==="luxury")return"#F0F0F0"; if(s==="france2"||s==="gouvernement"||s==="olympics")return"#003189"; return"#FFF" }
    function getRoleColor(s) { if(s==="france2"||s==="gouvernement"||s==="olympics")return"#555"; if(s==="tech")return"#00E5FF"; if(s==="sports"||s==="football")return getAccent(s); if(s==="cinema")return"#C0A060"; if(s==="luxury")return"#888"; return"#CCC" }
    function isWhite(s) { return s==="france2"||s==="gouvernement"||s==="olympics" }
    function getName(s) { return({"bfm":"Marie Dupont","lci":"Sophie Martin","france2":"Jean-Pierre Elkabbach","france24":"Claire Fournier","cnn":"Anderson Cooper","bbc":"Fiona Bruce","skynews":"Kay Burley","aljazeera":"Kamahl Santamaria","sports":"Kylian Mbappé","football":"Antoine Griezmann","olympics":"Léon Marchand","politique":"Marine Le Pen","gouvernement":"Gabriel Attal","cinema":"Léa Seydoux","luxury":"Marion Cotillard","tech":"Sam Altman","minimal":"Thomas Pesquet","dual":"Débat du Soir","fullscreen":"Philippe Labro","breaking":"Alerte Info"})[s]||"Marie Dupont" }
    function getRole(s) { return({"bfm":"Présentatrice","lci":"Journaliste politique","france2":"Présentateur JT","france24":"Correspondante int.","cnn":"Anchor CNN","bbc":"BBC Newsreader","skynews":"Sky News Presenter","aljazeera":"Al Jazeera Correspondent","sports":"Attaquant · France","football":"Milieu · Atlético","olympics":"Natation · 400m","politique":"Présidente RN","gouvernement":"Premier Ministre","cinema":"Festival de Cannes","luxury":"Ambassadrice Dior","tech":"CEO · OpenAI","minimal":"Astronaute · ESA","dual":"En direct du plateau","fullscreen":"Écrivain & Journaliste","breaking":"DERNIÈRE MINUTE"})[s]||"Présentatrice" }
    function getTxtMargin(s) { if(s==="aljazeera"||s==="sports")return plate.width*0.16; if(s==="olympics")return plate.width*0.2; return 12 }
}
