import QtQuick
import QtQuick3D

// A bench power supply: a metal box with two glowing readouts, knobs and banana jacks on the
// front. Sits on its origin and faces +z; 30 x 14 x 22 cm.
Node {
    id: supply

    property color voltageColor: "#ff3b1f"
    property color currentColor: "#3cff5a"
    readonly property vector3d redJack: Qt.vector3d(9, 2.5, 11.4)   // where the leads plug in
    readonly property vector3d blackJack: Qt.vector3d(12, 2.5, 11.4)

    Model { // case
        source: "#Cube"
        y: 7
        scale: Qt.vector3d(0.30, 0.14, 0.22)
        materials: PrincipledMaterial { baseColor: "#9aa0a8"; metalness: 0.6; roughness: 0.45 }
    }
    Model { // front panel
        source: "#Cube"
        position: Qt.vector3d(0, 7, 11.05)
        scale: Qt.vector3d(0.29, 0.13, 0.002)
        materials: PrincipledMaterial { baseColor: "#1d1f24"; roughness: 0.6 }
    }

    Repeater3D { // readouts: volts above, amps below
        model: 2
        Model {
            required property int index
            source: "#Cube"
            position: Qt.vector3d(-6, 10 - index * 4.5, 11.2)
            scale: Qt.vector3d(0.12, 0.03, 0.002)
            materials: PrincipledMaterial {
                readonly property color glow: index === 0 ? supply.voltageColor : supply.currentColor
                baseColor: glow
                emissiveFactor: Qt.vector3d(glow.r, glow.g, glow.b)
                lighting: PrincipledMaterial.NoLighting
            }
        }
    }

    Repeater3D { // knobs
        model: 2
        Model {
            required property int index
            source: "#Cylinder"
            position: Qt.vector3d(4 + index * 6, 9.5, 11.8)
            eulerRotation.x: 90
            scale: Qt.vector3d(0.035, 0.02, 0.035)
            materials: PrincipledMaterial { baseColor: "#2a2c31"; roughness: 0.3 }
        }
    }

    Model { // red jack
        source: "#Cylinder"
        position: supply.redJack
        eulerRotation.x: 90
        scale: Qt.vector3d(0.016, 0.02, 0.016)
        materials: PrincipledMaterial { baseColor: "#c8261c"; roughness: 0.3 }
    }
    Model { // black jack
        source: "#Cylinder"
        position: supply.blackJack
        eulerRotation.x: 90
        scale: Qt.vector3d(0.016, 0.02, 0.016)
        materials: PrincipledMaterial { baseColor: "#111111"; roughness: 0.3 }
    }
}
