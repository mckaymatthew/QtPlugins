import QtQuick
import QtQuick3D

// Phase 3, option 2: a late-night burger, then... back to work?
Item {
    id: scene
    focus: true

    readonly property real progress: game.eatingProgress

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#03050c" }
            GradientStop { position: 0.7; color: "#1a1830" }
        }
    }

    View3D {
        anchors.fill: parent
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Transparent
            antialiasingMode: SceneEnvironment.MSAA
        }

        PerspectiveCamera { position: Qt.vector3d(-150, 220, 2200); eulerRotation.x: 4; fieldOfView: 50; clipFar: 20000 }
        DirectionalLight { eulerRotation: Qt.vector3d(-35, 30, 0); brightness: 0.25; color: "#8a9ad0" }
        PointLight { position: Qt.vector3d(0, 600, 900); color: "#fff0d0"; brightness: 3; quadraticFade: 0.6 } // lot light

        Model { // parking lot
            source: "#Rectangle"
            eulerRotation.x: -90
            scale: Qt.vector3d(80, 60, 1)
            materials: PrincipledMaterial { baseColor: "#2a2a2e"; roughness: 0.95 }
        }
        Repeater3D {
            model: 6
            Model { // bay stripes
                required property int index
                source: "#Cube"
                position: Qt.vector3d(-750 + index * 300, 1, 900)
                scale: Qt.vector3d(0.12, 0.01, 4.5)
                materials: PrincipledMaterial { baseColor: "#e8e8e8"; emissiveFactor: Qt.vector3d(0.15, 0.15, 0.15) }
            }
        }

        Node { // restaurant
            Model { // building
                source: "#Cube"
                y: 210
                scale: Qt.vector3d(14, 4.2, 8)
                materials: PrincipledMaterial { baseColor: "#f2ede4"; roughness: 0.7 }
            }
            Model { // roof band
                source: "#Cube"
                y: 450
                scale: Qt.vector3d(14.4, 0.8, 8.4)
                materials: PrincipledMaterial { baseColor: "#c8261c"; emissiveFactor: Qt.vector3d(0.35, 0.05, 0.03) }
            }
            Model { // windows, lit inside
                source: "#Cube"
                position: Qt.vector3d(-150, 200, 402)
                scale: Qt.vector3d(9, 2, 0.04)
                materials: PrincipledMaterial {
                    baseColor: "#ffe4a8"
                    emissiveFactor: Qt.vector3d(1, 0.85, 0.55)
                    lighting: PrincipledMaterial.NoLighting
                }
            }
            Model { // door
                source: "#Cube"
                position: Qt.vector3d(470, 125, 402)
                scale: Qt.vector3d(1.4, 2.5, 0.04)
                materials: PrincipledMaterial { baseColor: "#6b7d8c"; emissiveFactor: Qt.vector3d(0.2, 0.25, 0.3) }
            }
        }

        Node { // pole sign
            position: Qt.vector3d(-850, 0, 800)
            Model {
                source: "#Cylinder"
                y: 300
                scale: Qt.vector3d(0.25, 6, 0.25)
                materials: PrincipledMaterial { baseColor: "#777b82"; metalness: 0.8; roughness: 0.4 }
            }
            Model {
                source: "#Rectangle"
                position: Qt.vector3d(0, 650, 0)
                eulerRotation.y: 12
                scale: Qt.vector3d(5.6, 2.2, 1)
                materials: PrincipledMaterial {
                    lighting: PrincipledMaterial.NoLighting
                    baseColorMap: Texture {
                        sourceItem: Rectangle {
                            width: 1120
                            height: 440
                            radius: 40
                            color: "#ffd23f"
                            border { color: "#c8261c"; width: 24 }
                            Column {
                                anchors.centerIn: parent
                                Text {
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    text: "WHAT-A-BURGER"
                                    color: "#c8261c"
                                    font { pixelSize: 108; bold: true }
                                }
                                Text {
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    text: "OPEN 24 HOURS"
                                    color: "#3a2a10"
                                    font { pixelSize: 60; bold: true }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // The cheeseburger arrives: a photo that slowly zooms in on the patty while you eat.
    Item {
        id: closeUp
        anchors.fill: parent
        visible: scene.progress > 0
        opacity: visible ? 1 : 0
        Behavior on opacity { NumberAnimation { duration: 400 } }

        Rectangle {
            anchors.fill: parent
            color: "#e8000000"
        }

        Rectangle { // photo frame
            id: frame
            anchors.centerIn: parent
            anchors.verticalCenterOffset: -10
            width: Math.min(parent.width * 0.78, (parent.height * 0.66) * photo.aspect)
            height: width / photo.aspect
            color: "black"
            radius: 12
            clip: true
            border { color: "#40ffffff"; width: 2 }

            Image {
                id: photo
                readonly property real aspect: implicitHeight > 0 ? implicitWidth / implicitHeight : 16 / 9
                anchors.fill: parent
                anchors.margins: 2
                source: "assets/burger.png"
                fillMode: Image.PreserveAspectCrop
                smooth: true
                mipmap: true
                transform: Scale { // zoom towards the patty's holes as the burger disappears
                    origin { x: photo.width * 0.42; y: photo.height * 0.58 }
                    xScale: 1 + 1.4 * scene.progress
                    yScale: xScale
                }
            }
        }
    }

    Text {
        anchors { top: parent.top; horizontalCenter: parent.horizontalCenter; topMargin: 24 }
        text: scene.progress > 0 && scene.progress < 1 ? qsTr("One cheeseburger. Look closer.")
            : scene.progress >= 1 ? qsTr("That hit the spot. It's 2 AM…")
                                  : qsTr("What-A-Burger. Open 24 hours.")
        color: "white"
        font { pixelSize: 26; bold: true }
        style: Text.Outline; styleColor: "#a0000000"
    }

    Column {
        anchors { bottom: parent.bottom; horizontalCenter: parent.horizontalCenter; bottomMargin: 24 }
        spacing: 12

        ProgressBar {
            anchors.horizontalCenter: parent.horizontalCenter
            visible: scene.progress > 0 && scene.progress < 1
            label: qsTr("Eating…")
            value: scene.progress
            fill: "#ff9f1c"
        }
        OverlayButton {
            anchors.horizontalCenter: parent.horizontalCenter
            visible: scene.progress === 0
            text: qsTr("Eat")
            accent: "#a8321e"
            onClicked: game.eat()
        }
        OverlayButton {
            anchors.horizontalCenter: parent.horizontalCenter
            visible: scene.progress >= 1
            text: qsTr("Go back to work")
            accent: "#2f5d8f"
            onClicked: game.backToWork()
        }
    }
}
