import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: sb
    property string teamA: setupController.scoreboardTeamA
    property string teamB: setupController.scoreboardTeamB
    property int scoreA: setupController.scoreboardScoreA
    property int scoreB: setupController.scoreboardScoreB
    property string colorA: setupController.scoreboardColorA
    property string colorB: setupController.scoreboardColorB
    property bool timerRunning: false
    property int elapsedSec: 0

    Timer { interval: 1000; running: sb.timerRunning; repeat: true; onTriggered: { sb.elapsedSec++; setupController.scoreboardMatchTime = fmt(sb.elapsedSec) } }
    function fmt(s) { var m=Math.floor(s/60); var sec=s%60; return(m<10?"0":"")+m+":"+(sec<10?"0":"")+sec }

    // Goal animation auto-reset
    Timer { id: goalResetTimer; interval: setupController.goalAnimDuration * 1000; repeat: false; onTriggered: setupController.goalAnimActive = false }
    Timer { id: sportEventResetTimer; interval: 4500; repeat: false; onTriggered: setupController.sportEvent = "" }

    Flickable {
        anchors.fill: parent; contentHeight: col.implicitHeight; clip: true
        flickableDirection: Flickable.VerticalFlick
        ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
    ColumnLayout {
        id: col; width: parent.width; spacing: 10
        Item { implicitHeight: 4 }

        // ── Header ──────────────────────────────────
        RowLayout { spacing: 8; Layout.leftMargin: 12
            Rectangle {
                Layout.preferredWidth: 28; Layout.preferredHeight: 28; radius: 7
                color: Qt.rgba(108/255,92/255,231/255,0.12)
                Label { anchors.centerIn: parent; text: "\u26BD"; font.pixelSize: 13 }
            }
            ColumnLayout { spacing: 0
                Label { text: "Scoreboard"; font.pixelSize: 15; font.weight: Font.Bold; color: window.darkMode ? "#F0F0F5" : "#0F0F14" }
                Label { text: sb.timerRunning ? "LIVE — " + setupController.scoreboardMatchTime : "Ready"; font.pixelSize: 10; color: sb.timerRunning ? "#00D68F" : (window.darkMode ? "#505060" : "#999") }
            }
        }

        // ── ON/OFF ──────────────────────────────────
        Switch { text: "Show scoreboard"; checked: setupController.scoreboardVisible; onToggled: setupController.scoreboardVisible = checked; leftPadding: 12 }

        // ── Teams ───────────────────────────────────
        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#1A1A22" : "#E0E0E8"; Layout.leftMargin: 12; Layout.rightMargin: 12 }

        // Team A
        RowLayout { spacing: 8; Layout.leftMargin: 12; Layout.rightMargin: 12
            Rectangle { Layout.preferredWidth: 6; Layout.preferredHeight: 36; radius: 3; color: sb.colorA }
            ColumnLayout { Layout.fillWidth: true; spacing: 2
                Label { text: "TEAM A"; font.pixelSize: 9; font.letterSpacing: 1; color: window.darkMode ? "#505060" : "#999" }
                TextField {
                    Layout.fillWidth: true; text: sb.teamA
                    onTextChanged: { sb.teamA = text; setupController.scoreboardTeamA = text }
                    placeholderText: "Team name"; font.pixelSize: 12; font.weight: Font.Bold; color: window.darkMode ? "#FFF" : "#1A1A1A"
                    background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 6; border.color: window.darkMode ? "#333" : "#CCC" }
                }
            }
            ColorPickerButton { Layout.preferredWidth: 28; Layout.preferredHeight: 28; currentColor: sb.colorA; onColorSelected: function(c) { sb.colorA = c; setupController.scoreboardColorA = c } }
        }

        // Team B
        RowLayout { spacing: 8; Layout.leftMargin: 12; Layout.rightMargin: 12
            Rectangle { Layout.preferredWidth: 6; Layout.preferredHeight: 36; radius: 3; color: sb.colorB }
            ColumnLayout { Layout.fillWidth: true; spacing: 2
                Label { text: "TEAM B"; font.pixelSize: 9; font.letterSpacing: 1; color: window.darkMode ? "#505060" : "#999" }
                TextField {
                    Layout.fillWidth: true; text: sb.teamB
                    onTextChanged: { sb.teamB = text; setupController.scoreboardTeamB = text }
                    placeholderText: "Team name"; font.pixelSize: 12; font.weight: Font.Bold; color: window.darkMode ? "#FFF" : "#1A1A1A"
                    background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 6; border.color: window.darkMode ? "#333" : "#CCC" }
                }
            }
            ColorPickerButton { Layout.preferredWidth: 28; Layout.preferredHeight: 28; currentColor: sb.colorB; onColorSelected: function(c) { sb.colorB = c; setupController.scoreboardColorB = c } }
        }

        // ── Score ───────────────────────────────────
        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#1A1A22" : "#E0E0E8"; Layout.leftMargin: 12; Layout.rightMargin: 12 }

        RowLayout { spacing: 0; Layout.alignment: Qt.AlignHCenter

            // Team A score
            ColumnLayout { spacing: 4; Layout.alignment: Qt.AlignHCenter
                Label { text: sb.teamA || "A"; font.pixelSize: 10; font.weight: Font.Bold; color: sb.colorA; Layout.alignment: Qt.AlignHCenter; elide: Text.ElideRight; Layout.maximumWidth: 80 }
                RowLayout { spacing: 8; Layout.alignment: Qt.AlignHCenter
                    Rectangle {
                        width: 32; height: 32; radius: 8; color: "#FF3D71"
                        Label { anchors.centerIn: parent; text: "-"; color: "white"; font.pixelSize: 18; font.bold: true }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: if(sb.scoreA > 0) { sb.scoreA--; setupController.scoreboardScoreA = sb.scoreA } }
                    }
                    Label { text: sb.scoreA.toString(); font.pixelSize: 36; font.weight: Font.Bold; font.family: "SF Mono, Menlo, monospace"; color: window.darkMode ? "#F0F0F5" : "#0F0F14"; Layout.preferredWidth: 40; horizontalAlignment: Text.AlignHCenter }
                    Rectangle {
                        width: 32; height: 32; radius: 8; color: "#00D68F"
                        Label { anchors.centerIn: parent; text: "+"; color: "white"; font.pixelSize: 18; font.bold: true }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { sb.scoreA++; setupController.scoreboardScoreA = sb.scoreA } }
                    }
                }
            }

            // Separator
            ColumnLayout { spacing: 4; Layout.leftMargin: 16; Layout.rightMargin: 16
                Label { text: "VS"; font.pixelSize: 10; font.letterSpacing: 2; color: window.darkMode ? "#333" : "#CCC"; Layout.alignment: Qt.AlignHCenter }
                Label { text: "—"; font.pixelSize: 24; color: window.darkMode ? "#333" : "#CCC"; Layout.alignment: Qt.AlignHCenter }
            }

            // Team B score
            ColumnLayout { spacing: 4; Layout.alignment: Qt.AlignHCenter
                Label { text: sb.teamB || "B"; font.pixelSize: 10; font.weight: Font.Bold; color: sb.colorB; Layout.alignment: Qt.AlignHCenter; elide: Text.ElideRight; Layout.maximumWidth: 80 }
                RowLayout { spacing: 8; Layout.alignment: Qt.AlignHCenter
                    Rectangle {
                        width: 32; height: 32; radius: 8; color: "#FF3D71"
                        Label { anchors.centerIn: parent; text: "-"; color: "white"; font.pixelSize: 18; font.bold: true }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: if(sb.scoreB > 0) { sb.scoreB--; setupController.scoreboardScoreB = sb.scoreB } }
                    }
                    Label { text: sb.scoreB.toString(); font.pixelSize: 36; font.weight: Font.Bold; font.family: "SF Mono, Menlo, monospace"; color: window.darkMode ? "#F0F0F5" : "#0F0F14"; Layout.preferredWidth: 40; horizontalAlignment: Text.AlignHCenter }
                    Rectangle {
                        width: 32; height: 32; radius: 8; color: "#00D68F"
                        Label { anchors.centerIn: parent; text: "+"; color: "white"; font.pixelSize: 18; font.bold: true }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { sb.scoreB++; setupController.scoreboardScoreB = sb.scoreB } }
                    }
                }
            }
        }

        // ── Timer ───────────────────────────────────
        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#1A1A22" : "#E0E0E8"; Layout.leftMargin: 12; Layout.rightMargin: 12 }

        RowLayout { spacing: 10; Layout.alignment: Qt.AlignHCenter
            Label { text: setupController.scoreboardMatchTime; font.pixelSize: 28; font.family: "SF Mono, Menlo, monospace"; font.weight: Font.Bold; color: "#00E5FF" }
            Rectangle {
                width: 60; height: 32; radius: 8; color: sb.timerRunning ? "#FF3D71" : "#00D68F"
                Label { anchors.centerIn: parent; text: sb.timerRunning ? "STOP" : "START"; color: "white"; font.pixelSize: 10; font.weight: Font.Bold; font.letterSpacing: 0.5 }
                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: sb.timerRunning = !sb.timerRunning }
            }
            Rectangle {
                width: 50; height: 32; radius: 8; color: window.darkMode ? Qt.rgba(255,255,255,0.05) : Qt.rgba(0,0,0,0.05)
                Label { anchors.centerIn: parent; text: "RESET"; color: window.darkMode ? "#666" : "#999"; font.pixelSize: 9; font.weight: Font.Bold }
                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { sb.elapsedSec = 0; setupController.scoreboardMatchTime = "00:00"; sb.timerRunning = false } }
            }
        }

        // Period
        RowLayout { spacing: 6; Layout.alignment: Qt.AlignHCenter
            Label { text: "Period:"; color: window.darkMode ? "#666" : "#999"; font.pixelSize: 10 }
            Repeater {
                model: [{ label: "1st", val: 1 }, { label: "2nd", val: 2 }, { label: "3rd", val: 3 }, { label: "ET", val: 4 }]
                Rectangle {
                    width: 36; height: 26; radius: 6
                    color: setupController.scoreboardPeriod === modelData.val ? "#6C5CE7" : (window.darkMode ? Qt.rgba(255,255,255,0.03) : Qt.rgba(0,0,0,0.03))
                    Label { anchors.centerIn: parent; text: modelData.label; color: setupController.scoreboardPeriod === modelData.val ? "white" : (window.darkMode ? "#666" : "#999"); font.pixelSize: 9; font.weight: Font.Bold }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: setupController.scoreboardPeriod = modelData.val }
                }
            }
        }

        // ── Cards ───────────────────────────────────
        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#1A1A22" : "#E0E0E8"; Layout.leftMargin: 12; Layout.rightMargin: 12 }

        RowLayout { spacing: 8; Layout.leftMargin: 12; Layout.rightMargin: 12
            Label { text: sb.teamA || "A"; font.pixelSize: 10; color: sb.colorA; font.weight: Font.DemiBold; Layout.preferredWidth: 60; elide: Text.ElideRight }
            Rectangle { width: 20; height: 24; radius: 3; color: "#FFCC00"; Label { anchors.centerIn: parent; text: setupController.scoreboardYellowA.toString(); color: "#000"; font.pixelSize: 10; font.bold: true }
                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: setupController.scoreboardYellowA++ } }
            Rectangle { width: 20; height: 24; radius: 3; color: "#FF3D71"; Label { anchors.centerIn: parent; text: setupController.scoreboardRedA.toString(); color: "white"; font.pixelSize: 10; font.bold: true }
                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: setupController.scoreboardRedA++ } }
            Item { Layout.fillWidth: true }
            Rectangle { width: 20; height: 24; radius: 3; color: "#FFCC00"; Label { anchors.centerIn: parent; text: setupController.scoreboardYellowB.toString(); color: "#000"; font.pixelSize: 10; font.bold: true }
                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: setupController.scoreboardYellowB++ } }
            Rectangle { width: 20; height: 24; radius: 3; color: "#FF3D71"; Label { anchors.centerIn: parent; text: setupController.scoreboardRedB.toString(); color: "white"; font.pixelSize: 10; font.bold: true }
                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: setupController.scoreboardRedB++ } }
            Label { text: sb.teamB || "B"; font.pixelSize: 10; color: sb.colorB; font.weight: Font.DemiBold; Layout.preferredWidth: 60; elide: Text.ElideRight; horizontalAlignment: Text.AlignRight }
        }

        // ── GOAL Buttons ────────────────────────────
        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#1A1A22" : "#E0E0E8"; Layout.leftMargin: 12; Layout.rightMargin: 12 }
        Label { text: "Quick actions"; font.pixelSize: 12; font.weight: Font.DemiBold; color: window.darkMode ? "#F0F0F5" : "#0F0F14"; leftPadding: 12 }

        RowLayout { spacing: 8; Layout.alignment: Qt.AlignHCenter
            Rectangle {
                width: 130; height: 44; radius: 10; color: sb.colorA
                RowLayout { anchors.centerIn: parent; spacing: 5
                    Label { text: "\u26BD"; font.pixelSize: 16 }
                    Label { text: "GOAL " + (sb.teamA || "A"); color: "white"; font.pixelSize: 11; font.weight: Font.Bold }
                }
                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { sb.scoreA++; setupController.scoreboardScoreA = sb.scoreA; setupController.goalAnimTeam = "a"; setupController.goalAnimActive = true; goalResetTimer.restart() } }
            }
            Rectangle {
                width: 130; height: 44; radius: 10; color: sb.colorB
                RowLayout { anchors.centerIn: parent; spacing: 5
                    Label { text: "\u26BD"; font.pixelSize: 16 }
                    Label { text: "GOAL " + (sb.teamB || "B"); color: "white"; font.pixelSize: 11; font.weight: Font.Bold }
                }
                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { sb.scoreB++; setupController.scoreboardScoreB = sb.scoreB; setupController.goalAnimTeam = "b"; setupController.goalAnimActive = true; goalResetTimer.restart() } }
            }
        }

        // Sport events
        Flow { spacing: 4; Layout.leftMargin: 12; Layout.rightMargin: 12; Layout.fillWidth: true
            Repeater {
                model: [
                    { label: "\uD83D\uDFE8 Yellow", evt: "yellow_card", c: "#CCAA00" },
                    { label: "\uD83D\uDFE5 Red", evt: "red_card", c: "#CC0000" },
                    { label: "\u23F8 Half-time", evt: "halftime", c: "#555" },
                    { label: "\u2194 Sub", evt: "substitution", c: "#0088CC" },
                    { label: "PEN", evt: "penalty", c: "#CC0000" },
                    { label: "VAR", evt: "var", c: "#0055AA" },
                    { label: "\u23F9 Full-time", evt: "full_time", c: "#888" }
                ]
                Rectangle {
                    width: evtLbl.implicitWidth + 14; height: 28; radius: 6; color: modelData.c
                    Label { id: evtLbl; anchors.centerIn: parent; text: modelData.label; color: "white"; font.pixelSize: 9; font.weight: Font.Bold }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { setupController.sportEvent = modelData.evt; sportEventResetTimer.restart() } }
                }
            }
        }

        // ── Design & Position ───────────────────────
        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#1A1A22" : "#E0E0E8"; Layout.leftMargin: 12; Layout.rightMargin: 12 }

        RowLayout { spacing: 6; Layout.leftMargin: 12
            Label { text: "Design:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11; Layout.preferredWidth: 55 }
            ComboBox {
                model: ["Glass Dark","Solid Dark","Sport Green","FIFA Style","UEFA Modern","Neon Border","White Clean","Red Accent","Blue Accent","Minimal"]
                property var vals: ["sb_glass_dark","sb_solid_dark","sb_sport_green","sb_fifa_style","sb_uefa_modern","sb_neon_border","sb_white_clean","sb_red_accent","sb_blue_accent","sb_minimal"]
                currentIndex: Math.max(0, vals.indexOf(setupController.scoreboardDesign)); Layout.fillWidth: true
                onActivated: setupController.scoreboardDesign = vals[currentIndex]
                background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 6; border.color: window.darkMode ? "#333" : "#CCC" }
            }
        }

        RowLayout { spacing: 6; Layout.leftMargin: 12
            Label { text: "Position:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11; Layout.preferredWidth: 55 }
            Repeater {
                model: [{ label: "Top Left", val: "top_left" }, { label: "Top Right", val: "top_right" }]
                Rectangle {
                    Layout.preferredWidth: 75; Layout.preferredHeight: 28; radius: 6
                    color: setupController.scoreboardPosition === modelData.val ? Qt.rgba(108/255,92/255,231/255,0.18) : (window.darkMode ? Qt.rgba(255,255,255,0.03) : Qt.rgba(0,0,0,0.03))
                    border.color: setupController.scoreboardPosition === modelData.val ? Qt.rgba(108/255,92/255,231/255,0.4) : "transparent"
                    Label { anchors.centerIn: parent; text: modelData.label; font.pixelSize: 10; color: setupController.scoreboardPosition === modelData.val ? "#6C5CE7" : (window.darkMode ? "#888" : "#666") }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: setupController.scoreboardPosition = modelData.val }
                }
            }
        }

        // ── Team Logos ──────────────────────────────
        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#1A1A22" : "#E0E0E8"; Layout.leftMargin: 12; Layout.rightMargin: 12 }
        Label { text: "Team logos"; font.pixelSize: 12; font.weight: Font.DemiBold; color: window.darkMode ? "#F0F0F5" : "#0F0F14"; leftPadding: 12 }
        RowLayout { spacing: 6; Layout.leftMargin: 12; Layout.rightMargin: 12
            Label { text: "A:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11 }
            TextField {
                Layout.fillWidth: true; text: setupController.teamLogoA; placeholderText: "logo_a.png"
                onTextEdited: setupController.teamLogoA = text; font.pixelSize: 11; color: window.darkMode ? "#CCC" : "#333"
                background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
            }
        }
        RowLayout { spacing: 6; Layout.leftMargin: 12; Layout.rightMargin: 12
            Label { text: "B:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11 }
            TextField {
                Layout.fillWidth: true; text: setupController.teamLogoB; placeholderText: "logo_b.png"
                onTextEdited: setupController.teamLogoB = text; font.pixelSize: 11; color: window.darkMode ? "#CCC" : "#333"
                background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 4; border.color: window.darkMode ? "#333" : "#CCC" }
            }
        }

        Item { implicitHeight: 12 }
    }
    }
}
