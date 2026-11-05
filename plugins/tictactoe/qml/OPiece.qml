import QtQuick
import QtQuick3D

// An O approximated as a torus from short cylinder segments (Qt 6.4 has no built-in torus).
Node {
    id: piece

    property bool shown: false
    property bool winning: false
    property real grow: shown ? 1 : 0
    property real glow: 0

    readonly property int segments: 28
    readonly property real ringRadius: 30
    readonly property real tubeRadius: 9

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
        baseColor: "#3e9bea"
        roughness: 0.3
        emissiveFactor: Qt.vector3d(piece.glow * 0.3, piece.glow * 0.6, piece.glow)
    }

    Repeater3D {
        model: piece.segments
        Node {
            required property int index
            eulerRotation.y: index * 360 / piece.segments
            // The built-in cylinder is 100 units tall along Y with radius 50; lay each segment
            // along the ring's tangent (local Z) at distance ringRadius.
            Model {
                source: "#Cylinder"
                x: piece.ringRadius
                eulerRotation.x: 90
                scale: Qt.vector3d(piece.tubeRadius / 50,
                                   2 * Math.PI * piece.ringRadius / piece.segments / 100 * 1.15,
                                   piece.tubeRadius / 50)
                castsShadows: true
                materials: material
            }
        }
    }
}
