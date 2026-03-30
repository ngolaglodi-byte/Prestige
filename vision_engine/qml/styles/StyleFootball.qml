import QtQuick 2.15

Item {
    id: root
    property string talentName: "Nom"
    property string talentRole: "Rôle"
    property color accentColor: "#FFFF00"
    property bool overlayVisible: false
    property real bgOpacity: 0.88
    width: 400; height: 76
    opacity: 0; visible: opacity > 0

    states: [ State { name: "shown"; when: overlayVisible; PropertyChanges { target: root; opacity: 1; y: root.parent ? root.parent.height * 0.76 : 380 } },
              State { name: "hidden"; when: !overlayVisible; PropertyChanges { target: root; opacity: 0; y: root.parent ? root.parent.height * 0.76 + 40 : 420 } } ]
    transitions: [ Transition { to: "shown"; NumberAnimation { properties: "opacity,y"; duration: 600; easing.type: Easing.OutQuart } },
                   Transition { to: "hidden"; NumberAnimation { properties: "opacity,y"; duration: 300 } } ]
    x: root.parent ? root.parent.width * 0.05 : 40

    Rectangle {
        anchors.fill: parent; color: Qt.rgba(0.1,0.29,0.1,root.bgOpacity)
        // Subtle pitch lines
        Repeater { model: 3; Rectangle { x: 60 + index * 120; y: 0; width: 1; height: parent.height; color: Qt.rgba(1,1,1,0.06) } }
        // Ghost number
        Text { anchors.right: parent.right; anchors.rightMargin: 16; anchors.verticalCenter: parent.verticalCenter; text: "9"; color: Qt.rgba(1,1,1,0.08); font.pixelSize: 72; font.bold: true }
        Column {
            anchors.verticalCenter: parent.verticalCenter; anchors.left: parent.left; anchors.leftMargin: 16; spacing: 2
            Text { text: root.talentName; color: "#FFFFFF"; font.pixelSize: 26; font.bold: true }
            Text { text: root.talentRole; color: root.accentColor; font.pixelSize: 14 }
        }
    }
}
