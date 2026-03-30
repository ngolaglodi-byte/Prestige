import QtQuick 2.15

Item {
    id: root
    property string talentName: "Nom"
    property string talentRole: "Rôle"
    property color accentColor: "#FF6D00"
    property bool overlayVisible: false
    property real bgOpacity: 0.88
    width: 380; height: 68
    opacity: 0; visible: opacity > 0; scale: 0.95

    states: [ State { name: "shown"; when: overlayVisible; PropertyChanges { target: root; opacity: 1; scale: 1.0 } },
              State { name: "hidden"; when: !overlayVisible; PropertyChanges { target: root; opacity: 0; scale: 0.95 } } ]
    transitions: [ Transition { to: "shown"; ParallelAnimation { NumberAnimation { property: "opacity"; duration: 400; easing.type: Easing.OutCubic } NumberAnimation { property: "scale"; duration: 400; easing.type: Easing.OutCubic } } },
                   Transition { to: "hidden"; NumberAnimation { properties: "opacity,scale"; duration: 250 } } ]
    y: root.parent ? root.parent.height * 0.78 : 400
    x: root.parent ? root.parent.width * 0.05 : 40

    Rectangle {
        anchors.fill: parent; radius: 0
        gradient: Gradient { orientation: Gradient.Horizontal; GradientStop { position: 0; color: Qt.rgba(0.1,0.1,0.1,root.bgOpacity) } GradientStop { position: 1; color: Qt.rgba(0,0,0,root.bgOpacity) } }
        Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 3; color: root.accentColor }
        // Orange triangle top-right
        Canvas { anchors.top: parent.top; anchors.right: parent.right; width: 20; height: 20
            onPaint: { var ctx = getContext("2d"); ctx.fillStyle = root.accentColor; ctx.beginPath(); ctx.moveTo(0,0); ctx.lineTo(20,0); ctx.lineTo(20,20); ctx.fill() } }
        Column {
            anchors.verticalCenter: parent.verticalCenter; anchors.left: parent.left; anchors.leftMargin: 16; spacing: 2
            Text { text: root.talentName; color: "#FFFFFF"; font.pixelSize: 22; font.bold: true }
            Text { text: root.talentRole; color: "#E0E0E0"; font.pixelSize: 13 }
        }
    }
}
