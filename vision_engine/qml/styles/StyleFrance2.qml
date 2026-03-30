import QtQuick 2.15

Item {
    id: root
    property string talentName: "Nom"
    property string talentRole: "Rôle"
    property color accentColor: "#003189"
    property bool overlayVisible: false
    property real bgOpacity: 0.90
    width: 380; height: 68
    opacity: 0; visible: opacity > 0

    states: [ State { name: "shown"; when: overlayVisible; PropertyChanges { target: root; opacity: 1; y: root.parent ? root.parent.height * 0.78 : 400 } },
              State { name: "hidden"; when: !overlayVisible; PropertyChanges { target: root; opacity: 0; y: root.parent ? root.parent.height * 0.78 + 30 : 430 } } ]
    transitions: [ Transition { to: "shown"; NumberAnimation { properties: "opacity,y"; duration: 500; easing.type: Easing.OutQuart } },
                   Transition { to: "hidden"; NumberAnimation { properties: "opacity,y"; duration: 280 } } ]
    x: root.parent ? root.parent.width * 0.05 : 40

    Rectangle {
        anchors.fill: parent; color: Qt.rgba(1,1,1,root.bgOpacity); radius: 0
        layer.enabled: true; layer.effect: Item {} // shadow placeholder
        Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 4; color: root.accentColor }
        Column {
            anchors.verticalCenter: parent.verticalCenter; anchors.left: parent.left; anchors.leftMargin: 16; spacing: 2
            Text { text: root.talentName; color: "#003189"; font.pixelSize: 22; font.bold: true }
            Text { text: root.talentRole; color: "#555555"; font.pixelSize: 13 }
        }
    }
}
