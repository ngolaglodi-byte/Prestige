import QtQuick 2.15
import QtQuick.Controls 2.15

// ============================================================
// Prestige AI — Lower Third
// Classic broadcast lower-third with title + subtitle
// ============================================================

Item {
    id: root

    // ── Public properties ──────────────────────────────────
    property string title: "Titre"
    property string subtitle: "Sous-titre"
    property color  accentColor: "#E30613"
    property color  bgPrimary: Qt.rgba(0.1, 0.1, 0.12, 0.9)
    property color  bgSecondary: Qt.rgba(0.15, 0.15, 0.18, 0.85)
    property bool   overlayVisible: false
    property string logoSource: ""
    property int    animDuration: 500

    // Anchored to the lower third of the screen
    anchors.bottom: parent ? parent.bottom : undefined
    anchors.bottomMargin: parent ? parent.height * 0.05 : 30
    anchors.left: parent ? parent.left : undefined
    anchors.leftMargin: parent ? parent.width * 0.05 : 50
    width: parent ? parent.width * 0.45 : 600
    height: 80

    opacity: 0
    visible: opacity > 0
    clip: true

    // ── States & Transitions ───────────────────────────────
    states: [
        State {
            name: "shown"
            when: overlayVisible
            PropertyChanges { target: root; opacity: 1.0 }
            PropertyChanges { target: contentRow; x: 0 }
        },
        State {
            name: "hidden"
            when: !overlayVisible
            PropertyChanges { target: root; opacity: 0.0 }
            PropertyChanges { target: contentRow; x: -root.width }
        }
    ]

    transitions: [
        Transition {
            from: "hidden"; to: "shown"
            SequentialAnimation {
                NumberAnimation { target: root; property: "opacity"; to: 1.0; duration: 100 }
                NumberAnimation { target: contentRow; property: "x"; duration: animDuration; easing.type: Easing.OutExpo }
            }
        },
        Transition {
            from: "shown"; to: "hidden"
            SequentialAnimation {
                NumberAnimation { target: contentRow; property: "x"; duration: animDuration / 2; easing.type: Easing.InExpo }
                NumberAnimation { target: root; property: "opacity"; to: 0.0; duration: 100 }
            }
        }
    ]

    // ── Visual ─────────────────────────────────────────────

    // Accent bar on the left
    Rectangle {
        id: accentBar
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 5
        color: root.accentColor
        z: 2
    }

    Row {
        id: contentRow
        anchors.left: accentBar.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        x: -root.width

        // Optional logo
        Image {
            id: logoImg
            source: root.logoSource
            visible: root.logoSource.length > 0
            width: 60
            height: parent.height
            fillMode: Image.PreserveAspectFit
            anchors.verticalCenter: parent.verticalCenter
        }

        // Text block
        Column {
            anchors.verticalCenter: parent.verticalCenter
            leftPadding: 16
            spacing: 4

            // Title bar
            Rectangle {
                width: titleText.implicitWidth + 24
                height: titleText.implicitHeight + 10
                color: root.bgPrimary

                Text {
                    id: titleText
                    anchors.centerIn: parent
                    text: root.title
                    color: "#FFFFFF"
                    font.family: "Noto Sans"
                    font.pixelSize: 22
                    font.weight: Font.Bold
                    font.letterSpacing: 0.5
                }
            }

            // Subtitle bar
            Rectangle {
                width: subtitleText.implicitWidth + 24
                height: subtitleText.implicitHeight + 8
                color: root.bgSecondary
                visible: root.subtitle.length > 0

                Text {
                    id: subtitleText
                    anchors.centerIn: parent
                    text: root.subtitle
                    color: "#DDDDDD"
                    font.family: "Noto Sans"
                    font.pixelSize: 15
                    font.weight: Font.Normal
                    font.letterSpacing: 0.3
                }
            }
        }
    }
}
