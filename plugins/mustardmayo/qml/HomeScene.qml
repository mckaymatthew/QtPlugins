import QtQuick
import QtQuick3D

// Phase 3, option 1: home. Go to bed.
Item {
    id: scene
    focus: true

    Keys.onReturnPressed: game.goToBed()

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#03050c" }
            GradientStop { position: 0.7; color: "#141d38" }
        }
    }

    View3D {
        anchors.fill: parent
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Transparent
            antialiasingMode: SceneEnvironment.MSAA
        }

        PerspectiveCamera { position: Qt.vector3d(300, 230, 2300); eulerRotation: Qt.vector3d(4, 6, 0); fieldOfView: 50; clipFar: 20000 }
        DirectionalLight { eulerRotation: Qt.vector3d(-30, -40, 0); brightness: 0.3; color: "#8a9ad0"; castsShadow: true }
        PointLight { position: Qt.vector3d(120, 280, 480); color: "#ffc070"; brightness: 2.5; quadraticFade: 1.2 } // porch light

        Model { // moon
            source: "#Sphere"
            position: Qt.vector3d(-2600, 2600, -6000)
            scale: Qt.vector3d(3, 3, 3)
            materials: PrincipledMaterial { baseColor: "#f4f1dc"; lighting: PrincipledMaterial.NoLighting }
        }
        Model { // lawn
            source: "#Rectangle"
            eulerRotation.x: -90
            scale: Qt.vector3d(80, 60, 1)
            receivesShadows: true
            materials: PrincipledMaterial { baseColor: "#13281a" }
        }
        Model { // driveway
            source: "#Cube"
            position: Qt.vector3d(780, 1, 900)
            scale: Qt.vector3d(3.2, 0.02, 11)
            receivesShadows: true
            materials: PrincipledMaterial { baseColor: "#3a3a3e" }
        }

        Node { // house
            Model { // walls
                source: "#Cube"
                y: 200
                scale: Qt.vector3d(10, 4, 7)
                castsShadows: true
                receivesShadows: true
                materials: PrincipledMaterial { baseColor: "#7d8ea8"; roughness: 0.8 }
            }
            Model { // gable: a square turned 45 degrees, its lower half hidden inside the walls
                source: "#Cube"
                position: Qt.vector3d(0, 400, 0)
                eulerRotation.z: 45
                scale: Qt.vector3d(7.07, 7.07, 6.98)
                materials: PrincipledMaterial { baseColor: "#7d8ea8"; roughness: 0.8 }
            }
            Repeater3D {
                model: 2
                Model { // roof planes
                    required property int index
                    readonly property real side: index === 0 ? -1 : 1
                    source: "#Cube"
                    position: Qt.vector3d(side * 257, 657, 0)
                    eulerRotation.z: -side * 45
                    scale: Qt.vector3d(7.9, 0.2, 7.6)
                    castsShadows: true
                    materials: PrincipledMaterial { baseColor: "#2b2224"; roughness: 0.9 }
                }
            }
            Model { // door
                source: "#Cube"
                position: Qt.vector3d(120, 105, 352)
                scale: Qt.vector3d(1, 2.1, 0.05)
                materials: PrincipledMaterial { baseColor: "#4a2c1a" }
            }
            Repeater3D {
                model: [ -300, 360 ]
                Model { // lit windows
                    required property var modelData
                    source: "#Cube"
                    position: Qt.vector3d(modelData, 220, 352)
                    scale: Qt.vector3d(1.5, 1.2, 0.04)
                    materials: PrincipledMaterial {
                        baseColor: "#ffd27a"
                        emissiveFactor: Qt.vector3d(1, 0.75, 0.35)
                        lighting: PrincipledMaterial.NoLighting
                    }
                }
            }
        }
    }

    Text {
        anchors { top: parent.top; horizontalCenter: parent.horizontalCenter; topMargin: 24 }
        text: qsTr("Home. Finally.")
        color: "white"
        font { pixelSize: 28; bold: true }
        style: Text.Outline; styleColor: "#a0000000"
    }
    OverlayButton {
        anchors { bottom: parent.bottom; horizontalCenter: parent.horizontalCenter; bottomMargin: 24 }
        text: qsTr("Go to bed")
        accent: "#1f4f8f"
        onClicked: game.goToBed()
    }
}
