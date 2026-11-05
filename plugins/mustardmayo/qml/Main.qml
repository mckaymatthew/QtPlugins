import QtQuick

// `game` (MustardMayoGame) and `fixedFont` (monospace family name) are context properties set by
// MustardMayoExtension. Each phase has its own scene; switching fades through black.
Item {
    id: root

    readonly property var scenes: ({
        work: "WorkScene.qml",
        drive: "DriveScene.qml",
        destination: "DestinationScene.qml",
        home: "HomeScene.qml",
        asleep: "AsleepScene.qml",
        burger: "BurgerScene.qml"
    })

    Loader {
        id: loader
        anchors.fill: parent
        focus: true
        source: root.scenes[game.phase]
        onLoaded: fadeIn.restart()
    }

    Rectangle {
        id: curtain
        anchors.fill: parent
        color: "black"
        opacity: 0
        NumberAnimation on opacity { id: fadeIn; from: 1; to: 0; duration: 700; running: false }
    }
}
