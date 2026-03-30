import QtQuick 2.15

Item {
    id: root
    property string talentName: "Nom"
    property string talentRole: "Rôle"
    property color accentColor: "#C0C0C0"
    property bool overlayVisible: false
    property real bgOpacity: 0.97
    width: 460; height: 76
    opacity: 0; visible: opacity > 0

    states: [ State { name: "shown"; when: overlayVisible; PropertyChanges { target: root; opacity: 1 } },
              State { name: "hidden"; when: !overlayVisible; PropertyChanges { target: root; opacity: 0 } } ]
    transitions: [ Transition { to: "shown"; NumberAnimation { property: "opacity"; duration: 1160 } },
                   Transition { to: "hidden"; NumberAnimation { property: "opacity"; duration: 700 } } ]
    y: root.parent ? root.parent.height * 0.80 : 420
    x: root.parent ? root.parent.width * 0.05 : 40

    Rectangle {
        anchors.fill: parent; color: Qt.rgba(0.05,0.05,0.05,root.bgOpacity)
        Rectangle { anchors.top: parent.top; width: parent.width; height: 1; color: "#C0C0C0" }
        Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: "#C0C0C0" }
        Column {
            anchors.centerIn: parent; spacing: 4
            Text { text: root.talentName; color: "#F5F5F5"; font.pixelSize: 28; font.weight: Font.Light; font.letterSpacing: 3; anchors.horizontalCenter: parent.horizontalCenter }
            Text { text: root.talentRole.toUpperCase(); color: "#888888"; font.pixelSize: 12; font.weight: Font.ExtraLight; font.letterSpacing: 6; anchors.horizontalCenter: parent.horizontalCenter }
        }
    }
}
