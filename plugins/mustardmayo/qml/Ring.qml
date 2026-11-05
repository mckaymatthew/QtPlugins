import QtQuick
import QtQuick3D

// A torus approximated by cylinder segments (Qt 6.4 has no built-in torus), lying in the XZ plane.
Node {
    id: ring

    property real ringRadius: 30
    property real tubeRadius: 4
    property int segments: 32
    property Material material

    Repeater3D {
        model: ring.segments
        Node {
            required property int index
            eulerRotation.y: index * 360 / ring.segments
            Model {
                source: "#Cylinder"
                x: ring.ringRadius
                eulerRotation.x: 90
                scale: Qt.vector3d(ring.tubeRadius / 50,
                                   2 * Math.PI * ring.ringRadius / ring.segments / 100 * 1.15,
                                   ring.tubeRadius / 50)
                materials: ring.material
            }
        }
    }
}
