import QtQuick

// Minimal button so the plugin needs no Qt Quick Controls dependency.
Rectangle {
    id: button

    property alias text: label.text
    signal clicked()

    implicitWidth: label.implicitWidth + 32
    implicitHeight: 38
    radius: 8
    color: mouse.pressed ? "#4a5670" : mouse.containsMouse ? "#3c4760" : "#2e374b"
    border.color: "#59657f"

    Text {
        id: label
        anchors.centerIn: parent
        color: "white"
        font.pixelSize: 15
    }

    MouseArea {
        id: mouse
        anchors.fill: parent
        hoverEnabled: true
        onClicked: button.clicked()
    }
}
