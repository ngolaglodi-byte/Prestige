import QtQuick 2.15

Item {
    id: root
    property string talentName: "Nom"
    property string talentRole: "Rôle"
    property color accentColor: "#CC0000"
    property bool overlayVisible: false
    property real bgOpacity: 0.95
    width: parent ? parent.width : 800; height: parent ? parent.height : 450
    opacity: 0; visible: opacity > 0

    states: [ State { name: "shown"; when: overlayVisible; PropertyChanges { target: root; opacity: 1 } },
              State { name: "hidden"; when: !overlayVisible; PropertyChanges { target: root; opacity: 0 } } ]
    transitions: [ Transition { to: "shown"; SequentialAnimation {
        NumberAnimation { target: root; property: "opacity"; to: 1; duration: 100 }
        NumberAnimation { target: breakingBanner; property: "opacity"; to: 1; duration: 0 }
        NumberAnimation { target: breakingBanner; property: "opacity"; to: 0.6; duration: 150 }
        NumberAnimation { target: breakingBanner; property: "opacity"; to: 1; duration: 150 } } },
        Transition { to: "hidden"; NumberAnimation { property: "opacity"; duration: 300 } } ]

    // Breaking News banner top
    Rectangle {
        id: breakingBanner; anchors.top: parent.top; anchors.topMargin: parent.height * 0.06
        width: parent.width; height: 48; color: root.accentColor
        Text { anchors.centerIn: parent; text: "BREAKING NEWS"; color: "white"; font.pixelSize: 22; font.bold: true; font.letterSpacing: 4 }
        SequentialAnimation on opacity { loops: Animation.Infinite; running: root.overlayVisible
            NumberAnimation { to: 0.85; duration: 1500 } NumberAnimation { to: 1; duration: 1500 } }
    }

    // Nameplate bottom
    Rectangle {
        anchors.bottom: parent.bottom; anchors.bottomMargin: parent.height * 0.12
        x: parent.width * 0.05; width: 380; height: 68
        color: Qt.rgba(0,0,0,root.bgOpacity)
        Rectangle { anchors.left: parent.left; width: 4; height: parent.height; color: root.accentColor }
        Column {
            anchors.verticalCenter: parent.verticalCenter; anchors.left: parent.left; anchors.leftMargin: 16; spacing: 2
            Text { text: root.talentName; color: "white"; font.pixelSize: 24; font.bold: true }
            Text { text: root.talentRole; color: "#CCC"; font.pixelSize: 14 }
        }
    }

    // Ticker bar
    Rectangle {
        anchors.bottom: parent.bottom; anchors.bottomMargin: parent.height * 0.04
        width: parent.width; height: 36; color: root.accentColor
        clip: true
        Text {
            id: ticker; y: 8; text: "DERNIÈRE MINUTE \u2014 " + root.talentName + " \u2014 " + root.talentRole + "     \u2022     DERNIÈRE MINUTE \u2014 " + root.talentName + " \u2014 " + root.talentRole
            color: "white"; font.pixelSize: 14; font.bold: true
            NumberAnimation on x { from: parent.width; to: -ticker.implicitWidth; duration: 15000; loops: Animation.Infinite; running: root.overlayVisible }
        }
    }
}
