import QtQuick 2.15

Item {
    id: root
    property string talentName: "Nom"
    property string talentRole: "Rôle"
    property color accentColor: "#6C5CE7"
    property bool overlayVisible: false
    property real bgOpacity: 0.70
    width: parent ? parent.width : 800; height: 140
    opacity: 0; visible: opacity > 0

    states: [ State { name: "shown"; when: overlayVisible; PropertyChanges { target: root; opacity: 1 } },
              State { name: "hidden"; when: !overlayVisible; PropertyChanges { target: root; opacity: 0 } } ]
    transitions: [ Transition { to: "shown"; NumberAnimation { property: "opacity"; duration: 800; easing.type: Easing.OutCubic } },
                   Transition { to: "hidden"; NumberAnimation { property: "opacity"; duration: 500 } } ]
    anchors.bottom: parent ? parent.bottom : undefined

    // Gradient fade from transparent to dark
    Rectangle {
        anchors.fill: parent
        gradient: Gradient { GradientStop { position: 0.0; color: "transparent" } GradientStop { position: 1.0; color: Qt.rgba(0,0,0,root.bgOpacity) } }

        Row {
            anchors.bottom: parent.bottom; anchors.bottomMargin: 16; anchors.horizontalCenter: parent.horizontalCenter; spacing: 16
            // Photo medallion placeholder
            Rectangle { width: 80; height: 80; radius: 40; color: "#333"; border.color: root.accentColor; border.width: 2
                Text { anchors.centerIn: parent; text: root.talentName.charAt(0); color: "white"; font.pixelSize: 32; font.bold: true } }
            Column { spacing: 4; anchors.verticalCenter: parent.verticalCenter
                Text { text: root.talentName; color: "white"; font.pixelSize: 32; font.bold: true }
                Text { text: root.talentRole; color: "#CCC"; font.pixelSize: 14; width: 400; wrapMode: Text.WordWrap } }
        }
    }
}
