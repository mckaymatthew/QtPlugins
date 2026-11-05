import QtQuick
import QtQuick3D

// Phase 3 choice: a fork in the road. Home to bed, or burgers.
Item {
    id: scene
    focus: true

    Keys.onPressed: (event) => {
        if (event.key === Qt.Key_Left || event.key === Qt.Key_A) {
            game.chooseHome()
            event.accepted = true
        } else if (event.key === Qt.Key_Right || event.key === Qt.Key_D) {
            game.chooseBurger()
            event.accepted = true
        }
    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#03050c" }
            GradientStop { position: 0.55; color: "#101830" }
        }
    }

    View3D {
        anchors.fill: parent
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Transparent
            antialiasingMode: SceneEnvironment.MSAA
        }

        PerspectiveCamera { position: Qt.vector3d(-35, 128, 0); eulerRotation.x: 3; fieldOfView: 60; clipFar: 20000 }
        DirectionalLight { eulerRotation: Qt.vector3d(-35, 30, 0); brightness: 0.4; color: "#8a9ad0" }
        SpotLight { // headlights
            position: Qt.vector3d(0, 70, -240)
            eulerRotation.x: -2
            color: "#fff2d6"
            brightness: 40
            coneAngle: 55
            innerConeAngle: 30
            quadraticFade: 0.004
            linearFade: 0.15
        }

        Model { // fields
            source: "#Rectangle"
            eulerRotation.x: -90
            position: Qt.vector3d(0, -3, -6000)
            scale: Qt.vector3d(400, 120, 1)
            materials: PrincipledMaterial { baseColor: "#0e1a10" }
        }
        Model { // road ahead
            source: "#Cube"
            position: Qt.vector3d(-90, -1, -3000)
            scale: Qt.vector3d(7.0, 0.02, 60)
            materials: PrincipledMaterial { baseColor: "#26272b" }
        }
        Model { // white edge line
            source: "#Cube"
            position: Qt.vector3d(140, 0.5, -3000)
            scale: Qt.vector3d(0.12, 0.01, 60)
            materials: PrincipledMaterial { baseColor: "#eeeeee"; emissiveFactor: Qt.vector3d(0.2, 0.2, 0.2) }
        }

        Node { // overhead sign
            position: Qt.vector3d(-60, 0, -1800)
            Repeater3D {
                model: 2
                Model {
                    required property int index
                    source: "#Cylinder"
                    position: Qt.vector3d(index === 0 ? -520 : 520, 300, -8)
                    scale: Qt.vector3d(0.2, 6, 0.2)
                    materials: PrincipledMaterial { baseColor: "#8a8f99"; metalness: 0.8; roughness: 0.4 }
                }
            }
            Model {
                source: "#Cube"
                position: Qt.vector3d(0, 560, -6)
                scale: Qt.vector3d(10.6, 2.6, 0.1)
                materials: PrincipledMaterial { baseColor: "#0b5d2a" }
            }
            Model {
                source: "#Rectangle"
                position: Qt.vector3d(0, 560, 0)
                scale: Qt.vector3d(10.4, 2.4, 1)
                materials: PrincipledMaterial {
                    lighting: PrincipledMaterial.NoLighting
                    baseColorMap: Texture {
                        sourceItem: Rectangle {
                            width: 1733
                            height: 400
                            color: "#0b6b30"
                            border { color: "white"; width: 12 }
                            radius: 24
                            Text {
                                anchors { left: parent.left; leftMargin: 70; verticalCenter: parent.verticalCenter }
                                text: "← Home"
                                color: "white"
                                font { pixelSize: 100; bold: true }
                            }
                            Text {
                                anchors { right: parent.right; rightMargin: 70; verticalCenter: parent.verticalCenter }
                                text: "What-A-Burger →"
                                color: "white"
                                font { pixelSize: 100; bold: true }
                            }
                        }
                    }
                }
            }
        }
    }

    Text {
        anchors { top: parent.top; horizontalCenter: parent.horizontalCenter; topMargin: 24 }
        text: qsTr("You made it off the highway. Where to?")
        color: "white"
        font { pixelSize: 24; bold: true }
        style: Text.Outline; styleColor: "#a0000000"
    }
    Row {
        anchors { bottom: parent.bottom; horizontalCenter: parent.horizontalCenter; bottomMargin: 24 }
        spacing: 16
        OverlayButton { text: qsTr("← Home"); accent: "#1f4f8f"; onClicked: game.chooseHome() }
        OverlayButton { text: qsTr("What-A-Burger →"); accent: "#a8321e"; onClicked: game.chooseBurger() }
    }
}
