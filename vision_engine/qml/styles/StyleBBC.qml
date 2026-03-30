import QtQuick 2.15

Item {
    id: root
    property string talentName: "Nom"
    property string talentRole: "Rôle"
    property color accentColor: "#BB1919"
    property bool overlayVisible: false
    property real bgOpacity: 0.95
    width: 400; height: 68
    opacity: 0; visible: opacity > 0; clip: true

    states: [ State { name: "shown"; when: overlayVisible; PropertyChanges { target: clipRect; width: root.width } },
              State { name: "hidden"; when: !overlayVisible; PropertyChanges { target: clipRect; width: 0 } } ]
    transitions: [ Transition { to: "shown"; SequentialAnimation { NumberAnimation { target: root; property: "opacity"; to: 1; duration: 50 } NumberAnimation { target: clipRect; property: "width"; duration: 400; easing.type: Easing.OutCubic } } },
                   Transition { to: "hidden"; SequentialAnimation { NumberAnimation { target: clipRect; property: "width"; duration: 250 } NumberAnimation { target: root; property: "opacity"; to: 0; duration: 50 } } } ]
    y: root.parent ? root.parent.height * 0.78 : 400
    x: root.parent ? root.parent.width * 0.05 : 40

    Item {
        id: clipRect; anchors.left: parent.left; height: parent.height; clip: true
        Rectangle {
            width: root.width; height: root.height; color: Qt.rgba(0.73,0.1,0.1,root.bgOpacity)
            Rectangle { anchors.left: parent.left; width: 8; height: parent.height; color: "white" }
            Column {
                anchors.verticalCenter: parent.verticalCenter; anchors.left: parent.left; anchors.leftMargin: 20; spacing: 2
                Text { text: root.talentName; color: "#FFFFFF"; font.pixelSize: 22; font.bold: true }
                Text { text: root.talentRole; color: "#FFAAAA"; font.pixelSize: 13 }
            }
            // BBC logo placeholder
            Rectangle { anchors.right: parent.right; anchors.rightMargin: 10; anchors.verticalCenter: parent.verticalCenter; width: 48; height: 48; radius: 4; color: Qt.rgba(1,1,1,0.15)
                Text { anchors.centerIn: parent; text: "BBC"; color: "white"; font.pixelSize: 12; font.bold: true } }
        }
    }
}
