import QtQuick
import QtQuick3D

// An X made of two crossed bars. Drops in when `shown`, glows when part of the winning line.
Node {
    id: piece

    property bool shown: false
    property bool winning: false
    property real grow: shown ? 1 : 0
    property real glow: 0

    visible: grow > 0.001
    scale: Qt.vector3d(grow, grow, grow)
    y: 22 + (1 - grow) * 80

    Behavior on grow { NumberAnimation { duration: 280; easing.type: Easing.OutBack } }

    SequentialAnimation on glow {
        running: piece.winning
        loops: Animation.Infinite
        onStopped: piece.glow = 0
        NumberAnimation { from: 0; to: 0.8; duration: 500; easing.type: Easing.InOutSine }
        NumberAnimation { from: 0.8; to: 0; duration: 500; easing.type: Easing.InOutSine }
    }

    PrincipledMaterial {
        id: material
        baseColor: "#e5484d"
        roughness: 0.3
        emissiveFactor: Qt.vector3d(piece.glow, piece.glow * 0.3, piece.glow * 0.3)
    }

    Model {
        source: "#Cube"
        eulerRotation.y: 45
        scale: Qt.vector3d(0.78, 0.18, 0.18)
        castsShadows: true
        materials: material
    }
    Model {
        source: "#Cube"
        eulerRotation.y: -45
        scale: Qt.vector3d(0.78, 0.18, 0.18)
        castsShadows: true
        materials: material
    }
}
