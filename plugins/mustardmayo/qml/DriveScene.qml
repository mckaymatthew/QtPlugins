import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D

// Phase 2: the drive home at night, in a thunderstorm. Keep the car between the yellow centre line (mustard) and the
// white edge line (mayo). Units are centimetres; the lane centre is x = 0.
Item {
    id: scene
    focus: true

    readonly property var driving: game.driving
    readonly property real laneHalfWidth: 140
    readonly property real carHalfWidth: 90
    readonly property real carX: driving.lanePosition * (laneHalfWidth - carHalfWidth)
    readonly property real sliceLength: 600 // short slices keep sharp bends smooth
    readonly property int sliceCount: 32
    readonly property int firstSlice: Math.floor(driving.distance * 100 / sliceLength)
    readonly property real flash: lightning.flash // 0..1 while lightning strikes

    // Lightning every few seconds, at random.
    Timer {
        id: lightningTimer
        interval: 2500 + Math.random() * 5000
        running: true
        onTriggered: {
            lightning.strike()
            interval = 4000 + Math.random() * 7000
            restart()
        }
    }

    function setKey(key, held) {
        if (key === Qt.Key_Left || key === Qt.Key_A) {
            driving.setLeftHeld(held)
            return true
        }
        if (key === Qt.Key_Right || key === Qt.Key_D) {
            driving.setRightHeld(held)
            return true
        }
        return false
    }
    Keys.onPressed: (event) => { if (!event.isAutoRepeat) event.accepted = setKey(event.key, true) }
    Keys.onReleased: (event) => { if (!event.isAutoRepeat) event.accepted = setKey(event.key, false) }

    Item {
        id: world
        anchors.fill: parent
        layer.enabled: scene.driving.blur > 0.02
        layer.effect: Blur { radius: scene.driving.blur * 16 * scene.height / 600 }

        // Stormy night sky behind the transparent 3D view.
        Rectangle {
            anchors.fill: parent
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#05060a" }
                GradientStop { position: 0.55; color: "#151a26" }
            }
            Canvas { // low, soft storm clouds
                anchors.fill: parent
                onWidthChanged: requestPaint()
                onHeightChanged: requestPaint()
                onPaint: {
                    const ctx = getContext("2d")
                    ctx.reset()
                    for (let i = 0; i < 70; ++i) { // deterministic scatter of soft puffs
                        const x = ((i * 7919) % 1000) / 1000 * width
                        const y = (((i * 104729) % 1000) / 1000) * height * 0.38
                        const r = height * (0.08 + ((i * 31) % 10) / 70)
                        const puff = ctx.createRadialGradient(x, y, 0, x, y, r)
                        puff.addColorStop(0, "rgba(52, 58, 76, 0.45)")
                        puff.addColorStop(1, "rgba(52, 58, 76, 0)")
                        ctx.fillStyle = puff
                        ctx.fillRect(x - r, y - r, 2 * r, 2 * r)
                    }
                }
            }
            Rectangle { // the whole sky lights up with each strike
                anchors.fill: parent
                opacity: scene.flash * 0.7
                gradient: Gradient {
                    GradientStop { position: 0.0; color: "#c8d0ff" }
                    GradientStop { position: 0.55; color: "#40486a" }
                }
            }
            Lightning {
                id: lightning
                objectName: "lightning"
                anchors.fill: parent
                horizon: scene.height * 0.5
            }
        }

        View3D {
            anchors.fill: parent
            environment: SceneEnvironment {
                backgroundMode: SceneEnvironment.Transparent
                antialiasingMode: SceneEnvironment.MSAA
            }

            DirectionalLight { // what little light gets through the clouds
                eulerRotation: Qt.vector3d(-35, 30, 0)
                brightness: 0.3
                color: "#8a9ad0"
            }
            DirectionalLight { // lightning
                eulerRotation: Qt.vector3d(-60, -20, 0)
                brightness: scene.flash * 2.5
                color: "#d4dcff"
                visible: scene.flash > 0.01
            }

            // Rain, falling around the car and streaming past it.
            ParticleSystem3D {
                LineParticle3D {
                    id: raindrop
                    maxAmount: 2500
                    color: "#c4d2ee"
                    colorVariation: Qt.vector4d(0.05, 0.05, 0.05, 0.3)
                    particleScale: 1.4
                    length: 90
                    segmentCount: 2
                    alphaFade: 0.3
                    billboard: true
                    fadeInDuration: 60
                    fadeOutDuration: 60
                }
                ParticleEmitter3D {
                    particle: raindrop
                    position: Qt.vector3d(scene.carX, 250, -2600)
                    shape: ParticleShape3D {
                        type: ParticleShape3D.Cube
                        extents: Qt.vector3d(1000, 250, 2600)
                    }
                    emitRate: 3500
                    lifeSpan: 650
                    velocity: VectorDirection3D {
                        // Down, and towards the car at its speed.
                        direction: Qt.vector3d(-scene.driving.steeringAngle * 200, -900,
                                               scene.driving.speed * 100)
                        directionVariation: Qt.vector3d(30, 120, 150)
                    }
                }
            }

            Model { // fields either side
                source: "#Rectangle"
                eulerRotation.x: -90
                position: Qt.vector3d(0, -3, -10000)
                scale: Qt.vector3d(800, 220, 1)
                materials: PrincipledMaterial { baseColor: "#0e1a10"; roughness: 1 }
            }

            // The road, as slices that scroll towards the camera and follow the bend.
            Repeater3D {
                model: scene.sliceCount
                Node {
                    required property int index
                    readonly property real zNear: (scene.driving.distance * 100) % scene.sliceLength
                                                  - index * scene.sliceLength
                    readonly property real zCentre: zNear - scene.sliceLength / 2
                    // The model knows the road ahead; `distance` in the binding re-evaluates it
                    // every frame.
                    readonly property real ahead: -zCentre / 100 + 0 * scene.driving.distance
                    position: Qt.vector3d(scene.driving.roadOffsetAt(ahead) * 100, 0, zCentre)
                    eulerRotation.y: -scene.driving.roadHeadingAt(ahead) * 180 / Math.PI
                    readonly property int worldSlice: scene.firstSlice + index

                    Model { // asphalt: oncoming lane, our lane and a shoulder
                        source: "#Cube"
                        position: Qt.vector3d(-90, -1, 0)
                        scale: Qt.vector3d(7.0, 0.02, scene.sliceLength / 100 + 0.6)
                        // wet: smooth enough to catch the headlights and the lightning
                        materials: PrincipledMaterial { baseColor: "#1e1f23"; roughness: 0.35 }
                    }
                    Model { // yellow dash (3 m dash, 9 m gap: every other slice)
                        visible: parent.worldSlice % 2 === 0
                        source: "#Cube"
                        position: Qt.vector3d(-scene.laneHalfWidth, 0.5, scene.sliceLength / 2 - 150)
                        scale: Qt.vector3d(0.12, 0.01, 3)
                        materials: PrincipledMaterial {
                            baseColor: "#f2c200"
                            emissiveFactor: Qt.vector3d(0.25, 0.19, 0)
                        }
                    }
                    Model { // solid white edge line
                        source: "#Cube"
                        position: Qt.vector3d(scene.laneHalfWidth, 0.5, 0)
                        scale: Qt.vector3d(0.12, 0.01, scene.sliceLength / 100 + 0.3)
                        materials: PrincipledMaterial {
                            baseColor: "#eeeeee"
                            emissiveFactor: Qt.vector3d(0.2, 0.2, 0.2)
                        }
                    }
                    Model { // far edge line of the oncoming lane
                        source: "#Cube"
                        position: Qt.vector3d(-3 * scene.laneHalfWidth, 0.5, 0)
                        scale: Qt.vector3d(0.12, 0.01, scene.sliceLength / 100 + 0.3)
                        materials: PrincipledMaterial { baseColor: "#bbbbbb" }
                    }
                    Node { // reflector post, every 24 m
                        visible: parent.worldSlice % 4 === 0
                        position: Qt.vector3d(scene.laneHalfWidth + 190, 0, 0)
                        Model {
                            source: "#Cube"
                            y: 50
                            scale: Qt.vector3d(0.1, 1, 0.1)
                            materials: PrincipledMaterial { baseColor: "#d8d8d8" }
                        }
                        Model {
                            source: "#Cube"
                            position: Qt.vector3d(0, 88, 5.5)
                            scale: Qt.vector3d(0.08, 0.14, 0.01)
                            materials: PrincipledMaterial {
                                baseColor: "#ff8c00"
                                emissiveFactor: Qt.vector3d(1, 0.55, 0)
                                lighting: PrincipledMaterial.NoLighting
                            }
                        }
                    }
                }
            }

            // The car: everything here moves with the lane position.
            Node {
                x: scene.carX

                PerspectiveCamera {
                    position: Qt.vector3d(-35, 128, 0) // driver sits left of centre
                    eulerRotation.x: -5
                    fieldOfView: 60
                    clipNear: 5
                    clipFar: 25000
                }

                SpotLight { // headlights
                    position: Qt.vector3d(0, 70, -240)
                    eulerRotation.x: -7
                    color: "#fff2d6"
                    brightness: 40
                    coneAngle: 55
                    innerConeAngle: 30
                    quadraticFade: 0.004
                    linearFade: 0.15
                }
                PointLight { // dashboard glow on the wheel
                    position: Qt.vector3d(-35, 105, -40)
                    color: "#4f7fff"
                    brightness: 0.3
                    quadraticFade: 6
                }

                Model { // hood
                    source: "#Cube"
                    position: Qt.vector3d(0, 84, -180)
                    eulerRotation.x: 3
                    scale: Qt.vector3d(1.7, 0.06, 1.4)
                    materials: PrincipledMaterial { baseColor: "#8a1c26"; metalness: 0.3; roughness: 0.3 }
                }
                Repeater3D { // instrument glow on the dashboard
                    model: 2
                    Model {
                        required property int index
                        source: "#Cylinder"
                        position: Qt.vector3d(-47 + index * 24, 106.5, -66)
                        scale: Qt.vector3d(0.06, 0.002, 0.06)
                        materials: PrincipledMaterial {
                            baseColor: "#1d3a73"
                            emissiveFactor: Qt.vector3d(0.08, 0.18, 0.45)
                            lighting: PrincipledMaterial.NoLighting
                        }
                    }
                }
                Model { // dashboard
                    source: "#Cube"
                    position: Qt.vector3d(0, 92, -75)
                    scale: Qt.vector3d(1.9, 0.28, 0.45)
                    materials: PrincipledMaterial { baseColor: "#17181b"; roughness: 0.8 }
                }

                Node { // steering wheel, tilted towards the driver; only its top shows
                    position: Qt.vector3d(-35, 93, -50)
                    eulerRotation.x: 65
                    Node {
                        eulerRotation.y: -scene.driving.steeringAngle * 90
                        PrincipledMaterial { id: wheelMaterial; baseColor: "#202124"; roughness: 0.6 }
                        Ring { ringRadius: 19; tubeRadius: 2.4; segments: 28; material: wheelMaterial }
                        Repeater3D {
                            model: 3
                            Node {
                                required property int index
                                eulerRotation.y: 90 + index * 120
                                Model {
                                    source: "#Cube"
                                    x: 9
                                    scale: Qt.vector3d(0.18, 0.025, 0.04)
                                    materials: wheelMaterial
                                }
                            }
                        }
                        Model { // hub
                            source: "#Cylinder"
                            scale: Qt.vector3d(0.1, 0.04, 0.1)
                            materials: wheelMaterial
                        }
                    }
                }
            }
        }

        // Drops landing on the windscreen and running off.
        Repeater {
            id: drops
            model: 36
            Rectangle {
                required property int index
                width: 4 + (index % 4) * 2
                height: width * 1.2
                radius: width / 2
                color: "#305a6a8a"
                border { color: "#60c8d8f0"; width: 1 }
                opacity: 0
                NumberAnimation on opacity {
                    id: fade
                    running: false
                    from: 0.9; to: 0; duration: 1400; easing.type: Easing.InQuad
                }
                function land() {
                    x = Math.random() * scene.width
                    y = Math.random() * scene.height * 0.75
                    fade.restart()
                }
            }
        }
        Timer {
            interval: 70
            repeat: true
            running: true
            property int next: 0
            onTriggered: {
                drops.itemAt(next).land()
                next = (next + 1) % drops.count
            }
        }
    }

    // 2D overlay
    ProgressBar {
        anchors { top: parent.top; horizontalCenter: parent.horizontalCenter; topMargin: 18 }
        width: Math.min(420, scene.width * 0.6)
        label: qsTr("Getting home")
        value: scene.driving.progress
    }

    Text {
        id: warning
        anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter; verticalCenterOffset: -scene.height * 0.12 }
        visible: text.length > 0
        text: scene.driving.warningText
        color: scene.driving.warning === 1 ? "#ffd21f" : "#fffdf0"
        font { pixelSize: Math.max(28, scene.height * 0.075); bold: true }
        style: Text.Outline
        styleColor: "#c0000000"
        SequentialAnimation on scale {
            running: warning.visible
            loops: Animation.Infinite
            NumberAnimation { from: 1; to: 1.12; duration: 250 }
            NumberAnimation { from: 1.12; to: 1; duration: 250 }
        }
    }

    Text {
        anchors { right: parent.right; bottom: parent.bottom; margins: 18 }
        text: qsTr("%1 mph").arg(Math.round(scene.driving.speed * 2.237))
        color: "#9fc4ff"
        font { pixelSize: 26; bold: true }
    }

    Text {
        id: hint
        anchors { left: parent.left; bottom: parent.bottom; margins: 18 }
        text: qsTr("← / → to steer")
        color: "white"
        font.pixelSize: 18
        style: Text.Outline; styleColor: "#a0000000"
        NumberAnimation on opacity { from: 1; to: 0; duration: 1500; running: scene.driving.distance > 150 }
    }
}
