import QtQuick

// The end: lights out.
Rectangle {
    id: scene
    focus: true
    color: "#020306"

    Keys.onReturnPressed: game.restart()

    Column {
        anchors.centerIn: parent
        spacing: 18

        Text {
            id: zzz
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Z z z …"
            color: "#9fb4ff"
            font { pixelSize: 64; bold: true }
            SequentialAnimation on opacity {
                loops: Animation.Infinite
                NumberAnimation { from: 0.35; to: 1; duration: 1400; easing.type: Easing.InOutSine }
                NumberAnimation { from: 1; to: 0.35; duration: 1400; easing.type: Easing.InOutSine }
            }
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("You made it home. The flight test is in a few hours.")
            color: "white"
            font.pixelSize: 20
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: game.night === 1 ? qsTr("Typos tonight: %1").arg(game.typing.typos)
                                   : qsTr("Nights at work: %1 · typos on the last one: %2")
                                         .arg(game.night).arg(game.typing.typos)
            color: "#8a93a6"
            font.pixelSize: 16
        }
        Item { width: 1; height: 12 }
        OverlayButton {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Play again")
            onClicked: game.restart()
        }
    }
}
