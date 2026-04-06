import QtQuick 2.15

Item {
    id: root
    property string talentName: "Nom"
    property string talentRole: "Rôle"
    property color accentColor: "#D4AF37"
    property bool overlayVisible: false
    property real bgOpacity: 0.95
    width: 420; height: 72
    opacity: 0; visible: opacity > 0

    states: [ State { name: "shown"; when: overlayVisible; PropertyChanges { target: root; opacity: 1 } },
              State { name: "hidden"; when: !overlayVisible; PropertyChanges { target: root; opacity: 0 } } ]
    transitions: [ Transition { to: "shown"; NumberAnimation { property: "opacity"; duration: 900; easing.type: Easing.OutCubic } },
                   Transition { to: "hidden"; NumberAnimation { property: "opacity"; duration: 400 } } ]
    y: root.parent ? root.parent.height * 0.80 : 420
    x: root.parent ? root.parent.width * 0.05 : 40

    Rectangle {
        anchors.fill: parent; color: Qt.rgba(0,0,0,root.bgOpacity)
        Rectangle { anchors.top: parent.top; width: parent.width; height: 1; color: "#D4AF37" }
        Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: "#D4AF37" }
        Column {
            anchors.verticalCenter: parent.verticalCenter; anchors.left: parent.left; anchors.leftMargin: 20; spacing: 4
            Text { text: root.talentName; color: "#D4AF37"; font.pixelSize: 26; font.bold: true }
            Text { text: root.talentRole; color: "#C0A060"; font.pixelSize: 15; font.italic: true }
        }
    }
}
