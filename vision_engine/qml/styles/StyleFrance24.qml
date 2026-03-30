import QtQuick 2.15

Item {
    id: root
    property string talentName: "Nom"
    property string talentRole: "Rôle"
    property color accentColor: "#D4001A"
    property bool overlayVisible: false
    property real bgOpacity: 0.90
    width: 400; height: 70
    opacity: 0; visible: opacity > 0; clip: true

    states: [ State { name: "shown"; when: overlayVisible; PropertyChanges { target: wipe; width: root.width } },
              State { name: "hidden"; when: !overlayVisible; PropertyChanges { target: wipe; width: 0 } } ]
    transitions: [ Transition { to: "shown"; SequentialAnimation { NumberAnimation { target: root; property: "opacity"; to: 1; duration: 50 } NumberAnimation { target: wipe; property: "width"; duration: 660; easing.type: Easing.OutQuart } } },
                   Transition { to: "hidden"; SequentialAnimation { NumberAnimation { target: wipe; property: "width"; duration: 350; easing.type: Easing.InQuad } NumberAnimation { target: root; property: "opacity"; to: 0; duration: 50 } } } ]
    y: root.parent ? root.parent.height * 0.78 : 400
    x: root.parent ? root.parent.width * 0.05 : 40

    Item {
        id: wipe; anchors.left: parent.left; anchors.top: parent.top; height: parent.height; clip: true
        Rectangle {
            width: root.width; height: root.height; color: Qt.rgba(0.04,0.04,0.04,root.bgOpacity)
            Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 3; color: "#D4001A" }
            Rectangle { anchors.bottom: parent.bottom; anchors.bottomMargin: 3; width: parent.width; height: 3; color: "#003580" }
            Column {
                anchors.verticalCenter: parent.verticalCenter; anchors.left: parent.left; anchors.leftMargin: 16; spacing: 2
                Text { text: root.talentName; color: "#FFFFFF"; font.pixelSize: 23; font.bold: true }
                Text { text: root.talentRole; color: "#BBBBBB"; font.pixelSize: 14 }
            }
        }
    }
}
