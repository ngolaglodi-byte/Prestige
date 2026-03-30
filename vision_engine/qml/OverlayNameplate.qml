import QtQuick 2.15
import QtQuick.Controls 2.15

// ============================================================
// Prestige AI — Overlay Nameplate
// Broadcast-style talent name badge, positioned over detected face
// ============================================================

Item {
    id: root

    // ── Public properties ──────────────────────────────────
    property string talentName: "Nom"
    property string talentRole: "Rôle"
    property color  accentColor: "#E30613"       // Red accent line
    property color  bgColor: Qt.rgba(0, 0, 0, 0.75)
    property bool   overlayVisible: false
    property int    animDuration: 400            // ~12 frames at 30fps

    // Bounding box from detection (pixel coords)
    property real bboxX: 0
    property real bboxY: 0
    property real bboxW: 100
    property real bboxH: 100

    // Position nameplate below the face bbox
    x: bboxX
    y: bboxY + bboxH + 8
    width: Math.max(nameplateContent.implicitWidth + 32, bboxW)
    height: nameplateContent.implicitHeight + 12

    opacity: 0
    visible: opacity > 0

    // ── Animations ─────────────────────────────────────────
    states: [
        State {
            name: "shown"
            when: overlayVisible
            PropertyChanges { target: root; opacity: 1.0; x: bboxX }
        },
        State {
            name: "hidden"
            when: !overlayVisible
            PropertyChanges { target: root; opacity: 0.0; x: bboxX - 40 }
        }
    ]

    transitions: [
        Transition {
            from: "hidden"; to: "shown"
            ParallelAnimation {
                NumberAnimation { property: "opacity"; duration: animDuration; easing.type: Easing.OutCubic }
                NumberAnimation { property: "x"; duration: animDuration; easing.type: Easing.OutCubic }
            }
        },
        Transition {
            from: "shown"; to: "hidden"
            ParallelAnimation {
                NumberAnimation { property: "opacity"; duration: animDuration / 2; easing.type: Easing.InCubic }
                NumberAnimation { property: "x"; duration: animDuration / 2; easing.type: Easing.InCubic }
            }
        }
    ]

    // ── Visual ─────────────────────────────────────────────
    Rectangle {
        id: background
        anchors.fill: parent
        color: root.bgColor
        radius: 2

        // Accent bar at the bottom
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: 3
            color: root.accentColor
        }
    }

    Column {
        id: nameplateContent
        anchors.centerIn: parent
        spacing: 1

        Text {
            id: nameText
            text: root.talentName
            color: "#FFFFFF"
            font.family: "Noto Sans"
            font.pixelSize: 18
            font.weight: Font.Bold
            font.letterSpacing: 0.5
        }

        Text {
            id: roleText
            text: root.talentRole
            color: "#CCCCCC"
            font.family: "Noto Sans"
            font.pixelSize: 13
            font.weight: Font.Normal
            font.letterSpacing: 0.3
            visible: text.length > 0
        }
    }
}
