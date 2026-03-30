import QtQuick 2.15

Item {
    id: root
    property string talentName: "Nom"
    property string talentRole: "Rôle"
    property color accentColor: "#00A0DC"
    property bool overlayVisible: false
    property real bgOpacity: 0.92
    width: 380; height: 68
    opacity: 0; visible: opacity > 0

    states: [ State { name: "shown"; when: overlayVisible; PropertyChanges { target: root; opacity: 1; x: root.parent ? root.parent.width * 0.55 : 500 } },
              State { name: "hidden"; when: !overlayVisible; PropertyChanges { target: root; opacity: 0; x: root.parent ? root.parent.width : 900 } } ]
    transitions: [ Transition { to: "shown"; NumberAnimation { properties: "opacity,x"; duration: 500; easing.type: Easing.OutQuart } },
                   Transition { to: "hidden"; NumberAnimation { properties: "opacity,x"; duration: 300 } } ]
    y: root.parent ? root.parent.height * 0.78 : 400

    Rectangle {
        anchors.fill: parent; color: Qt.rgba(0,0.34,0.66,root.bgOpacity)
        Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 3; color: root.accentColor }
        Column {
            anchors.verticalCenter: parent.verticalCenter; anchors.left: parent.left; anchors.leftMargin: 16; spacing: 2
            Text { text: root.talentName; color: "#FFFFFF"; font.pixelSize: 22; font.bold: true }
            Text { text: root.talentRole; color: "#A8D4F0"; font.pixelSize: 13 }
        }
    }
}
