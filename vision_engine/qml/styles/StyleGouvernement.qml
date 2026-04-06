import QtQuick 2.15

Item {
    id: root
    property string talentName: "Nom"
    property string talentRole: "Rôle"
    property color accentColor: "#002395"
    property bool overlayVisible: false
    property real bgOpacity: 0.95
    width: 440; height: 72
    opacity: 0; visible: opacity > 0

    states: [ State { name: "shown"; when: overlayVisible; PropertyChanges { target: root; opacity: 1 } },
              State { name: "hidden"; when: !overlayVisible; PropertyChanges { target: root; opacity: 0 } } ]
    transitions: [ Transition { to: "shown"; NumberAnimation { property: "opacity"; duration: 750; easing.type: Easing.OutQuart } },
                   Transition { to: "hidden"; NumberAnimation { property: "opacity"; duration: 400 } } ]
    y: root.parent ? root.parent.height * 0.78 : 400
    x: root.parent ? root.parent.width * 0.05 : 40

    Rectangle {
        anchors.fill: parent; color: Qt.rgba(1,1,1,root.bgOpacity)
        // Tricolore left border
        Column { anchors.left: parent.left; width: 6; height: parent.height; spacing: 0
            Rectangle { width: 6; height: parent.parent.height/3; color: "#002395" }
            Rectangle { width: 6; height: parent.parent.height/3; color: "#FFFFFF" }
            Rectangle { width: 6; height: parent.parent.height/3; color: "#ED2939" } }
        Column {
            anchors.verticalCenter: parent.verticalCenter; anchors.left: parent.left; anchors.leftMargin: 18; spacing: 2
            Text { text: root.talentName; color: "#1A1A1A"; font.pixelSize: 22; font.bold: true }
            Text { text: root.talentRole; color: "#333333"; font.pixelSize: 13; elide: Text.ElideRight; width: 340 }
        }
        // Seal placeholder
        Rectangle { anchors.right: parent.right; anchors.rightMargin: 10; anchors.verticalCenter: parent.verticalCenter; width: 48; height: 48; radius: 24; color: "#F0F0F0"
            Text { anchors.centerIn: parent; text: "RF"; color: "#002395"; font.pixelSize: 14; font.bold: true } }
    }
}
