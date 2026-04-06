import QtQuick 2.15

Item {
    id: root
    property string talentName: "Nom"
    property string talentRole: "Rôle"
    property color accentColor: "#E30613"
    property bool overlayVisible: false
    property real bgOpacity: 0.82
    width: 380; height: 68
    opacity: 0; visible: opacity > 0

    states: [ State { name: "shown"; when: overlayVisible; PropertyChanges { target: root; opacity: 1; x: root.parent ? root.parent.width * 0.05 : 40 } },
              State { name: "hidden"; when: !overlayVisible; PropertyChanges { target: root; opacity: 0; x: -380 } } ]
    transitions: [ Transition { to: "shown"; NumberAnimation { properties: "opacity,x"; duration: 600; easing.type: Easing.OutQuart } },
                   Transition { to: "hidden"; NumberAnimation { properties: "opacity,x"; duration: 300; easing.type: Easing.InQuad } } ]
    y: root.parent ? root.parent.height * 0.78 : 400

    Rectangle {
        anchors.fill: parent; color: Qt.rgba(0,0,0,root.bgOpacity); radius: 0
        Rectangle { anchors.left: parent.left; anchors.top: parent.top; anchors.bottom: parent.bottom; width: 4; color: root.accentColor }
        Column {
            anchors.verticalCenter: parent.verticalCenter; anchors.left: parent.left; anchors.leftMargin: 24; spacing: 2
            Text { text: root.talentName; color: "#FFFFFF"; font.pixelSize: 24; font.bold: true; font.family: "Helvetica Neue"; style: Text.Raised; styleColor: Qt.rgba(0, 0, 0, 0.5) }
            Text { text: root.talentRole; color: "#CCCCCC"; font.pixelSize: 14; font.family: "Helvetica Neue"; style: Text.Raised; styleColor: Qt.rgba(0, 0, 0, 0.5) }
        }
    }
}
