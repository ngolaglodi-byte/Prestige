import QtQuick 2.15

Item {
    id: root
    property string talentName: "Nom"
    property string talentRole: "Rôle"
    property color accentColor: "#00FF88"
    property bool overlayVisible: false
    property real bgOpacity: 0.85
    width: 400; height: 76
    opacity: 0; visible: opacity > 0; scale: 0.8

    states: [ State { name: "shown"; when: overlayVisible; PropertyChanges { target: root; opacity: 1; scale: 1.0 } },
              State { name: "hidden"; when: !overlayVisible; PropertyChanges { target: root; opacity: 0; scale: 0.8 } } ]
    transitions: [ Transition { to: "shown"; ParallelAnimation { NumberAnimation { property: "opacity"; duration: 330; easing.type: Easing.OutBack } NumberAnimation { property: "scale"; duration: 330; easing.type: Easing.OutBack } } },
                   Transition { to: "hidden"; NumberAnimation { properties: "opacity,scale"; duration: 200 } } ]
    y: root.parent ? root.parent.height * 0.78 : 400
    x: root.parent ? root.parent.width * 0.05 : 40

    Rectangle {
        anchors.fill: parent; color: Qt.rgba(0,0,0,root.bgOpacity); radius: 0
        // Number badge
        Rectangle { anchors.left: parent.left; width: 60; height: parent.height; color: root.accentColor
            Text { anchors.centerIn: parent; text: "10"; color: "white"; font.pixelSize: 28; font.bold: true } }
        Column {
            anchors.verticalCenter: parent.verticalCenter; anchors.left: parent.left; anchors.leftMargin: 72; spacing: 2
            Text { text: root.talentName; color: "#FFFFFF"; font.pixelSize: 28; font.weight: Font.ExtraBold }
            Text { text: root.talentRole; color: root.accentColor; font.pixelSize: 16 }
        }
    }
}
