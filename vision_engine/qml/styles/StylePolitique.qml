import QtQuick 2.15

Item {
    id: root
    property string talentName: "Nom"
    property string talentRole: "Rôle"
    property color accentColor: "#1B2A4A"
    property bool overlayVisible: false
    property real bgOpacity: 0.88
    width: 420; height: 72
    opacity: 0; visible: opacity > 0; scale: 0.6

    states: [ State { name: "shown"; when: overlayVisible; PropertyChanges { target: root; opacity: 1; scale: 1.0 } },
              State { name: "hidden"; when: !overlayVisible; PropertyChanges { target: root; opacity: 0; scale: 0.6 } } ]
    transitions: [ Transition { to: "shown"; ParallelAnimation { NumberAnimation { property: "opacity"; duration: 660 } NumberAnimation { property: "scale"; duration: 660; easing.type: Easing.OutQuart } } },
                   Transition { to: "hidden"; NumberAnimation { properties: "opacity,scale"; duration: 350 } } ]
    transformOrigin: Item.Center
    y: root.parent ? root.parent.height * 0.78 : 400
    x: root.parent ? root.parent.width * 0.05 : 40

    Rectangle {
        anchors.fill: parent; color: Qt.rgba(0.11,0.16,0.29,root.bgOpacity)
        // Flag placeholder left
        Column { anchors.left: parent.left; width: 8; height: parent.height; spacing: 0
            Rectangle { width: 8; height: parent.parent.height/3; color: "#002395" }
            Rectangle { width: 8; height: parent.parent.height/3; color: "#FFFFFF" }
            Rectangle { width: 8; height: parent.parent.height/3; color: "#ED2939" } }
        Column {
            anchors.verticalCenter: parent.verticalCenter; anchors.left: parent.left; anchors.leftMargin: 20; spacing: 2
            Text { text: root.talentName; color: "#FFFFFF"; font.pixelSize: 22; font.bold: true }
            Text { text: root.talentRole; color: "#A8C4E0"; font.pixelSize: 13; font.italic: true }
        }
    }
}
