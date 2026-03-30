import QtQuick 2.15

Item {
    id: root
    property string talentName: "Nom"
    property string talentRole: "Rôle"
    property color accentColor: "#0033A0"
    property bool overlayVisible: false
    property real bgOpacity: 0.90
    width: 400; height: 72
    opacity: 0; visible: opacity > 0

    states: [ State { name: "shown"; when: overlayVisible; PropertyChanges { target: root; opacity: 1 } },
              State { name: "hidden"; when: !overlayVisible; PropertyChanges { target: root; opacity: 0 } } ]
    transitions: [ Transition { to: "shown"; NumberAnimation { property: "opacity"; duration: 830 } },
                   Transition { to: "hidden"; NumberAnimation { property: "opacity"; duration: 500 } } ]
    y: root.parent ? root.parent.height * 0.78 : 400
    x: root.parent ? root.parent.width * 0.05 : 40

    Rectangle {
        anchors.fill: parent; color: Qt.rgba(1,1,1,root.bgOpacity); radius: 0
        // Olympic rings placeholder
        Row { anchors.left: parent.left; anchors.leftMargin: 12; anchors.verticalCenter: parent.verticalCenter; spacing: 3
            Repeater { model: ["#0085C7","#000000","#DF0024","#F4C300","#009F3D"]
                Rectangle { width: 14; height: 14; radius: 7; color: "transparent"; border.color: modelData; border.width: 2 } } }
        Column {
            anchors.verticalCenter: parent.verticalCenter; anchors.left: parent.left; anchors.leftMargin: 100; spacing: 2
            Text { text: root.talentName; color: "#0033A0"; font.pixelSize: 22; font.bold: true }
            Text { text: root.talentRole; color: "#666666"; font.pixelSize: 14 }
        }
    }
}
