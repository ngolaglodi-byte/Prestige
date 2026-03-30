import QtQuick 2.15

// ============================================================
// Prestige AI — Transition Effect
// Reusable transition wrapper for overlay elements
// ============================================================

Item {
    id: root

    property bool   active: false
    property string transitionType: "fade"  // "fade", "slide", "wipe"
    property int    duration: 400

    opacity: 0
    visible: opacity > 0

    states: [
        State {
            name: "active"
            when: root.active
            PropertyChanges {
                target: root
                opacity: 1.0
                scale: 1.0
                x: root.transitionType === "slide" ? 0 : root.x
            }
        },
        State {
            name: "inactive"
            when: !root.active
            PropertyChanges {
                target: root
                opacity: 0.0
                scale: root.transitionType === "wipe" ? 0.95 : 1.0
            }
        }
    ]

    transitions: [
        Transition {
            to: "active"
            ParallelAnimation {
                NumberAnimation { property: "opacity"; duration: root.duration; easing.type: Easing.OutCubic }
                NumberAnimation { property: "scale"; duration: root.duration; easing.type: Easing.OutCubic }
                NumberAnimation { property: "x"; duration: root.duration; easing.type: Easing.OutCubic }
            }
        },
        Transition {
            to: "inactive"
            ParallelAnimation {
                NumberAnimation { property: "opacity"; duration: root.duration / 2; easing.type: Easing.InCubic }
                NumberAnimation { property: "scale"; duration: root.duration / 2; easing.type: Easing.InCubic }
            }
        }
    ]
}
