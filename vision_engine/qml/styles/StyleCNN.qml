import QtQuick 2.15

Item {
    id: root
    property string talentName: "Nom"
    property string talentRole: "Rôle"
    property color accentColor: "#FFFFFF"
    property bool overlayVisible: false
    property real bgOpacity: 0.95
    width: 380; height: 68
    opacity: 0; visible: opacity > 0

    states: [ State { name: "shown"; when: overlayVisible; PropertyChanges { target: root; opacity: 1; x: root.parent ? root.parent.width * 0.05 : 40 } },
              State { name: "hidden"; when: !overlayVisible; PropertyChanges { target: root; opacity: 0; x: -380 } } ]
    transitions: [ Transition { to: "shown"; NumberAnimation { properties: "opacity,x"; duration: 270; easing.type: Easing.OutQuad } },
                   Transition { to: "hidden"; NumberAnimation { properties: "opacity,x"; duration: 200 } } ]
    y: root.parent ? root.parent.height * 0.78 : 400

    Rectangle {
        anchors.fill: parent; color: Qt.rgba(0.8,0,0,root.bgOpacity); radius: 0
        // Diagonal cut bottom-right
        Canvas { anchors.bottom: parent.bottom; anchors.right: parent.right; width: 12; height: parent.height
            onPaint: { var ctx = getContext("2d"); ctx.fillStyle = "#0A0A0E"; ctx.beginPath(); ctx.moveTo(12,0); ctx.lineTo(12,height); ctx.lineTo(0,height); ctx.fill() } }
        Column {
            anchors.verticalCenter: parent.verticalCenter; anchors.left: parent.left; anchors.leftMargin: 16; spacing: 2
            Item { width: childrenRect.width; height: 30
                Text { text: root.talentName; color: "#000000"; font.pixelSize: 26; font.bold: true; x: 1; y: 1; opacity: 0.7 }
                Text { text: root.talentName; color: "#FFFFFF"; font.pixelSize: 26; font.bold: true }
            }
            Item { width: childrenRect.width; height: 18
                Text { text: root.talentRole; color: "#000000"; font.pixelSize: 14; x: 1; y: 1; opacity: 0.6 }
                Text { text: root.talentRole; color: "#E0E0E0"; font.pixelSize: 14 }
            }
        }
    }
}
