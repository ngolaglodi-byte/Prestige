import QtQuick 2.15

Item {
    id: root
    property string talentName: "Nom"
    property string talentRole: "Rôle"
    property color accentColor: "#00E5FF"
    property bool overlayVisible: false
    property real bgOpacity: 0.90
    width: 400; height: 68
    opacity: 0; visible: opacity > 0

    states: [ State { name: "shown"; when: overlayVisible; PropertyChanges { target: root; opacity: 1 } },
              State { name: "hidden"; when: !overlayVisible; PropertyChanges { target: root; opacity: 0 } } ]
    // Glitch-in: 3 fast flickers then stable
    transitions: [ Transition { to: "shown"; SequentialAnimation {
        NumberAnimation { target: root; property: "opacity"; to: 0.7; duration: 50 }
        NumberAnimation { target: root; property: "opacity"; to: 0.2; duration: 40 }
        NumberAnimation { target: root; property: "opacity"; to: 0.8; duration: 50 }
        NumberAnimation { target: root; property: "opacity"; to: 0.1; duration: 30 }
        NumberAnimation { target: root; property: "opacity"; to: 1.0; duration: 330; easing.type: Easing.OutCubic } } },
        Transition { to: "hidden"; NumberAnimation { property: "opacity"; duration: 250 } } ]
    y: root.parent ? root.parent.height * 0.78 : 400
    x: root.parent ? root.parent.width * 0.05 : 40

    Rectangle {
        anchors.fill: parent; color: Qt.rgba(0.04,0.06,0.1,root.bgOpacity)
        Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 2; color: root.accentColor }
        // Scan lines
        Column { anchors.fill: parent; spacing: 2; opacity: 0.05; Repeater { model: 34; Rectangle { width: parent.width; height: 1; color: "white" } } }
        Column {
            anchors.verticalCenter: parent.verticalCenter; anchors.left: parent.left; anchors.leftMargin: 16; spacing: 2
            Text { text: root.talentName; color: "#FFFFFF"; font.pixelSize: 22; font.bold: true; font.family: "Menlo" }
            Text { text: root.talentRole; color: root.accentColor; font.pixelSize: 13; font.family: "Menlo" }
        }
        // LIVE badge
        Rectangle { anchors.top: parent.top; anchors.right: parent.right; anchors.margins: 6; width: 44; height: 18; radius: 3; color: Qt.rgba(0,0.9,1,0.15)
            Text { anchors.centerIn: parent; text: "LIVE"; color: root.accentColor; font.pixelSize: 9; font.bold: true }
            SequentialAnimation on opacity { loops: Animation.Infinite; NumberAnimation { to: 0.4; duration: 1000 } NumberAnimation { to: 1; duration: 1000 } } }
    }
}
