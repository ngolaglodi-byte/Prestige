import QtQuick 2.15

Item {
    id: root
    property string talentName: "Nom"
    property string talentRole: "Rôle"
    property color accentColor: "#5B4FDB"
    property bool overlayVisible: false
    property real bgOpacity: 0.85
    property string talentName2: "Invité"
    property string talentRole2: "Expert"
    property color accentColor2: "#E30613"
    width: parent ? parent.width : 800; height: 68
    opacity: 0; visible: opacity > 0

    states: [ State { name: "shown"; when: overlayVisible; PropertyChanges { target: root; opacity: 1 } },
              State { name: "hidden"; when: !overlayVisible; PropertyChanges { target: root; opacity: 0 } } ]
    transitions: [ Transition { to: "shown"; NumberAnimation { property: "opacity"; duration: 500 } },
                   Transition { to: "hidden"; NumberAnimation { property: "opacity"; duration: 300 } } ]
    y: root.parent ? root.parent.height * 0.78 : 400

    Row {
        anchors.bottom: parent.bottom; anchors.left: parent.left; anchors.right: parent.right; anchors.margins: 20; spacing: parent.width - 600
        // Person 1 — left
        Rectangle { width: 280; height: 68; color: Qt.rgba(0,0,0,root.bgOpacity)
            Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 3; color: root.accentColor }
            Column { anchors.verticalCenter: parent.verticalCenter; anchors.left: parent.left; anchors.leftMargin: 12; spacing: 2
                Text { text: root.talentName; color: "white"; font.pixelSize: 20; font.bold: true }
                Text { text: root.talentRole; color: "#CCC"; font.pixelSize: 12 } } }
        // Person 2 — right
        Rectangle { width: 280; height: 68; color: Qt.rgba(0,0,0,root.bgOpacity)
            Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 3; color: root.accentColor2 }
            Column { anchors.verticalCenter: parent.verticalCenter; anchors.left: parent.left; anchors.leftMargin: 12; spacing: 2
                Text { text: root.talentName2; color: "white"; font.pixelSize: 20; font.bold: true }
                Text { text: root.talentRole2; color: "#CCC"; font.pixelSize: 12 } } }
    }
}
