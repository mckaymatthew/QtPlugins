import QtQuick

// Minimal button so the plugin needs no Qt Quick Controls dependency.
Rectangle {
    id: button

    property alias text: label.text
    property color accent: "#2e374b"
    signal clicked()

    implicitWidth: label.implicitWidth + 40
    implicitHeight: 46
    radius: 10
    color: mouse.pressed ? Qt.lighter(accent, 1.5) : mouse.containsMouse ? Qt.lighter(accent, 1.25) : accent
    border.color: Qt.lighter(accent, 1.6)

    Text {
        id: label
        anchors.centerIn: parent
        color: "white"
        font { pixelSize: 18; bold: true }
    }

    MouseArea {
        id: mouse
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: button.clicked()
    }
}
