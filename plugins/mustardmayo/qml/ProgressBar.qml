import QtQuick

Item {
    id: bar

    property real value: 0 // 0 .. 1
    property alias label: caption.text
    property color fill: "#f2c200"

    implicitWidth: 360
    implicitHeight: caption.implicitHeight + 22

    Text {
        id: caption
        color: "white"
        font.pixelSize: 15
        style: Text.Outline
        styleColor: "#80000000"
    }
    Rectangle {
        anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
        height: 14
        radius: 7
        color: "#60000000"
        border.color: "#a0ffffff"
        Rectangle {
            anchors { left: parent.left; top: parent.top; bottom: parent.bottom; margins: 2 }
            width: Math.max(0, (parent.width - 4) * Math.min(1, bar.value))
            radius: 5
            color: bar.fill
        }
    }
}
