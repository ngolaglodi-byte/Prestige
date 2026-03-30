import QtQuick 2.15

Item {
    id: root
    property string talentName: "Nom"
    property string talentRole: "Rôle"
    property color accentColor: "#FFFFFF"
    property bool overlayVisible: false
    property real bgOpacity: 0.0
    width: 380; height: 60
    opacity: 0; visible: opacity > 0

    states: [ State { name: "shown"; when: overlayVisible; PropertyChanges { target: root; opacity: 1 } },
              State { name: "hidden"; when: !overlayVisible; PropertyChanges { target: root; opacity: 0 } } ]
    transitions: [ Transition { to: "shown"; NumberAnimation { property: "opacity"; duration: 660 } },
                   Transition { to: "hidden"; NumberAnimation { property: "opacity"; duration: 400 } } ]
    y: root.parent ? root.parent.height * 0.80 : 420
    x: root.parent ? root.parent.width * 0.05 : 40

    Column {
        anchors.left: parent.left; anchors.bottom: parent.bottom; spacing: 4
        Text { text: root.talentName; color: "white"; font.pixelSize: 24; font.bold: true
            layer.enabled: true; layer.effect: Item {} }
        Rectangle { width: parent.children[0].implicitWidth; height: 1; color: "white" }
        Text { text: root.talentRole; color: "white"; opacity: 0.7; font.pixelSize: 14 }
    }
}
