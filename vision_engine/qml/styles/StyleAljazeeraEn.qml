import QtQuick 2.15

Item {
    id: root
    property string talentName: "Nom"
    property string talentRole: "Rôle"
    property color accentColor: "#D4A017"
    property bool overlayVisible: false
    property real bgOpacity: 0.90
    width: 400; height: 68
    opacity: 0; visible: opacity > 0

    states: [ State { name: "shown"; when: overlayVisible; PropertyChanges { target: root; opacity: 1 } },
              State { name: "hidden"; when: !overlayVisible; PropertyChanges { target: root; opacity: 0 } } ]
    transitions: [ Transition { to: "shown"; NumberAnimation { property: "opacity"; duration: 660; easing.type: Easing.InOutQuad } },
                   Transition { to: "hidden"; NumberAnimation { property: "opacity"; duration: 400; easing.type: Easing.InQuad } } ]
    y: root.parent ? root.parent.height * 0.78 : 400
    x: root.parent ? root.parent.width * 0.05 : 40

    Rectangle {
        anchors.fill: parent; color: Qt.rgba(0.55,0,0,root.bgOpacity)
        Rectangle { anchors.left: parent.left; width: 52; height: parent.height; color: Qt.rgba(0.83,0.63,0.09,0.3)
            Text { anchors.centerIn: parent; text: "AJ"; color: "#D4A017"; font.pixelSize: 16; font.bold: true } }
        Column {
            anchors.verticalCenter: parent.verticalCenter; anchors.left: parent.left; anchors.leftMargin: 64; spacing: 2
            Text { text: root.talentName; color: "#D4A017"; font.pixelSize: 22; font.bold: true }
            Text { text: root.talentRole; color: "#FFFFFF"; font.pixelSize: 13 }
        }
    }
}
