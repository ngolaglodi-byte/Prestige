import QtQuick 2.15

Item {
    id: styleEngine

    property string currentStyle: "bfm"
    property string talentName: ""
    property string talentRole: ""
    property color accentColor: "#E30613"
    property real bgOpacity: 0.82
    property bool overlayVisible: false

    readonly property var styleMap: ({
        "bfm":          "styles/StyleBFM.qml",
        "lci":          "styles/StyleLCI.qml",
        "france2":      "styles/StyleFrance2.qml",
        "france24":     "styles/StyleFrance24.qml",
        "cnn":          "styles/StyleCNN.qml",
        "bbc":          "styles/StyleBBC.qml",
        "skynews":      "styles/StyleSkyNews.qml",
        "aljazeera":    "styles/StyleAljazeeraEn.qml",
        "sports":       "styles/StyleSports.qml",
        "football":     "styles/StyleFootball.qml",
        "olympics":     "styles/StyleOlympics.qml",
        "politique":    "styles/StylePolitique.qml",
        "gouvernement": "styles/StyleGouvernement.qml",
        "cinema":       "styles/StyleCinema.qml",
        "luxury":       "styles/StyleLuxury.qml",
        "tech":         "styles/StyleTechModern.qml",
        "minimal":      "styles/StyleMinimal.qml",
        "dual":         "styles/StyleDualPerson.qml",
        "fullscreen":   "styles/StyleFullscreen.qml",
        "breaking":     "styles/StyleBreaking.qml"
    })

    Loader {
        id: overlayLoader
        anchors.fill: parent
        source: styleEngine.styleMap[styleEngine.currentStyle] || ""
        onLoaded: applyProps()
    }

    onTalentNameChanged: applyProps()
    onTalentRoleChanged: applyProps()
    onOverlayVisibleChanged: applyProps()
    onAccentColorChanged: applyProps()
    onBgOpacityChanged: applyProps()

    function applyProps() {
        if (!overlayLoader.item) return
        overlayLoader.item.talentName = styleEngine.talentName
        overlayLoader.item.talentRole = styleEngine.talentRole
        overlayLoader.item.overlayVisible = styleEngine.overlayVisible
        if (overlayLoader.item.accentColor !== undefined)
            overlayLoader.item.accentColor = styleEngine.accentColor
        if (overlayLoader.item.bgOpacity !== undefined)
            overlayLoader.item.bgOpacity = styleEngine.bgOpacity
    }

    function switchStyle(newStyle) {
        if (overlayLoader.item)
            overlayLoader.item.overlayVisible = false
        styleTransitionTimer.newStyle = newStyle
        styleTransitionTimer.start()
    }

    Timer {
        id: styleTransitionTimer
        property string newStyle: ""
        interval: 500
        onTriggered: {
            styleEngine.currentStyle = newStyle
            Qt.callLater(function() { styleEngine.applyProps() })
        }
    }
}
