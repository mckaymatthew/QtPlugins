import QtQuick
import QtQuick3D

// Phase 1: late night at the office. Type the code on screen; every keystroke makes it blurrier.
Item {
    id: scene
    focus: true

    readonly property var typing: game.typing
    readonly property real pixelScale: height / 600
    // Stays readable for most of the snippet, then fades out fast near the end.
    readonly property real fatigueBlur: Math.pow(typing.fatigue, 3) * 22 * pixelScale
    property int pressedKey: -1
    property bool typoFlash: false

    function escapeHtml(s) {
        return s.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;")
                .replace(/ /g, "&nbsp;").replace(/\n/g, "<br>")
    }

    Keys.onPressed: (event) => {
        if (event.text.length === 0)
            return
        typing.typeText(event.text)
        pressedKey = Math.floor(Math.random() * 52)
        keyRelease.restart()
        event.accepted = true
    }
    Timer { id: keyRelease; interval: 90; onTriggered: scene.pressedKey = -1 }

    Connections {
        target: scene.typing
        function onTypo() { scene.typoFlash = true; typoTimer.restart() }
    }
    Timer { id: typoTimer; interval: 160; onTriggered: scene.typoFlash = false }

    // Shared camera and lighting for both layers.
    component OfficeCamera: PerspectiveCamera {
        position: Qt.vector3d(0, 122, 48)
        eulerRotation.x: -10
        fieldOfView: 50
        clipNear: 1
        clipFar: 1000
    }
    component OfficeLights: Node {
        DirectionalLight { eulerRotation: Qt.vector3d(-50, 20, 0); brightness: 0.25; color: "#7f8fbf" }
        PointLight { position: Qt.vector3d(-70, 130, -40); brightness: 1.6; color: "#ffcf8f"; quadraticFade: 2.5 }
        PointLight { position: Qt.vector3d(0, 110, -35); brightness: 0.6; color: "#9fc4ff"; quadraticFade: 4 }
    }

    // Props for the out-of-focus layer.
    component CircuitCard: Node {
        Model { // board
            source: "#Cube"
            scale: Qt.vector3d(0.20, 0.004, 0.13)
            materials: PrincipledMaterial { baseColor: "#1f7a3a"; roughness: 0.5 }
        }
        Repeater3D {
            model: 6
            Model { // chips
                required property int index
                source: "#Cube"
                position: Qt.vector3d(-6 + (index % 3) * 6, 0.6, -2.5 + Math.floor(index / 3) * 5)
                scale: Qt.vector3d(0.035 + (index % 2) * 0.015, 0.008, 0.03)
                materials: PrincipledMaterial { baseColor: "#111111"; roughness: 0.3 }
            }
        }
        Model { // edge connector
            source: "#Cube"
            position: Qt.vector3d(0, 0.3, 6.6)
            scale: Qt.vector3d(0.16, 0.004, 0.012)
            materials: PrincipledMaterial { baseColor: "#d4a531"; metalness: 1; roughness: 0.3 }
        }
    }
    component Wire: Node {
        id: wire
        property real length: 60
        property color color: "red"
        Model {
            source: "#Cylinder"
            eulerRotation.z: 90
            scale: Qt.vector3d(0.008, wire.length / 100, 0.008)
            materials: PrincipledMaterial { baseColor: wire.color; roughness: 0.4 }
        }
    }

    Item {
        id: world
        anchors.fill: parent
        layer.enabled: scene.fatigueBlur > 0.3
        layer.effect: Blur { radius: scene.fatigueBlur }

        // Background layer: desk, wall and clutter, permanently out of focus.
        Item {
            anchors.fill: parent
            layer.enabled: true
            layer.effect: Blur { radius: 6 * scene.pixelScale }

            View3D {
                anchors.fill: parent
                camera: backCamera
                environment: SceneEnvironment {
                    clearColor: "#0c0f16"
                    backgroundMode: SceneEnvironment.Color
                    antialiasingMode: SceneEnvironment.MSAA
                }
                OfficeCamera { id: backCamera }
                OfficeLights {}

                Model { // wall
                    source: "#Rectangle"
                    position: Qt.vector3d(0, 120, -160)
                    scale: Qt.vector3d(6, 3, 1)
                    materials: PrincipledMaterial { baseColor: "#2a3040" }
                }
                Model { // desk
                    source: "#Cube"
                    position: Qt.vector3d(0, 72, -40)
                    scale: Qt.vector3d(1.8, 0.04, 0.95)
                    materials: PrincipledMaterial { baseColor: "#5c5f66"; roughness: 0.7 }
                }

                CircuitCard { position: Qt.vector3d(-48, 74.5, -42); eulerRotation.y: 20 }
                CircuitCard { position: Qt.vector3d(-40, 74.8, -58); eulerRotation.y: -10 }
                CircuitCard { position: Qt.vector3d(50, 74.5, -48); eulerRotation.y: -28 }
                CircuitCard { position: Qt.vector3d(30, 74.5, -72); eulerRotation.y: 8 }

                Wire { position: Qt.vector3d(-30, 74.8, -30); eulerRotation.y: 25; length: 50; color: "#d63c3c" }
                Wire { position: Qt.vector3d(-36, 74.8, -50); eulerRotation.y: -60; length: 40; color: "#2f6ad6" }
                Wire { position: Qt.vector3d(40, 74.8, -35); eulerRotation.y: -35; length: 55; color: "#e0c020" }
                Wire { position: Qt.vector3d(28, 74.8, -60); eulerRotation.y: 70; length: 45; color: "#202020" }
                Wire { position: Qt.vector3d(-5, 74.8, -75); eulerRotation.y: 5; length: 70; color: "#d63c3c" }

                Model { // coffee mug, long cold
                    source: "#Cylinder"
                    position: Qt.vector3d(38, 79, -18)
                    scale: Qt.vector3d(0.08, 0.1, 0.08)
                    materials: PrincipledMaterial { baseColor: "#e8e2d4"; roughness: 0.3 }
                }
            }
        }

        // Foreground layer: monitor and keyboard, in focus (until fatigue sets in).
        View3D {
            anchors.fill: parent
            camera: frontCamera
            environment: SceneEnvironment {
                backgroundMode: SceneEnvironment.Transparent
                antialiasingMode: SceneEnvironment.MSAA
            }
            OfficeCamera { id: frontCamera }
            OfficeLights {}

            // Monitor
            Model { // stand foot
                source: "#Cube"
                position: Qt.vector3d(0, 75, -52)
                scale: Qt.vector3d(0.22, 0.012, 0.16)
                materials: PrincipledMaterial { baseColor: "#1a1a1d"; roughness: 0.4 }
            }
            Model { // neck
                source: "#Cube"
                position: Qt.vector3d(0, 88, -56)
                scale: Qt.vector3d(0.05, 0.26, 0.03)
                materials: PrincipledMaterial { baseColor: "#1a1a1d"; roughness: 0.4 }
            }
            Model { // bezel; flashes red on a typo (a material change, which always re-renders,
                    // unlike edits to the screen's 2D texture item)
                source: "#Cube"
                position: Qt.vector3d(0, 108, -54)
                scale: Qt.vector3d(0.66, 0.40, 0.025)
                materials: PrincipledMaterial {
                    baseColor: scene.typoFlash ? "#ff3b30" : "#111114"
                    emissiveFactor: scene.typoFlash ? Qt.vector3d(0.8, 0.1, 0.05) : Qt.vector3d(0, 0, 0)
                    roughness: 0.35
                }
            }
            Model { // screen
                source: "#Rectangle"
                position: Qt.vector3d(0, 109, -52.7)
                scale: Qt.vector3d(0.62, 0.36, 1)
                materials: PrincipledMaterial {
                    lighting: PrincipledMaterial.NoLighting
                    baseColorMap: Texture {
                        sourceItem: Rectangle {
                            width: 1240
                            height: 720
                            color: "#1b1f27"

                            Rectangle { // title bar
                                width: parent.width
                                height: 56
                                color: "#2b303b"
                                Text {
                                    anchors { left: parent.left; leftMargin: 24; verticalCenter: parent.verticalCenter }
                                    text: "flight_test.cpp"
                                    color: "#c0c6d4"
                                    font { family: fixedFont; pixelSize: 28 }
                                }
                            }
                            Text {
                                x: 40
                                y: 90
                                textFormat: Text.StyledText
                                color: "#5a6272"
                                font { family: fixedFont; pixelSize: 40 }
                                lineHeight: 1.25
                                text: {
                                    const code = scene.typing.code
                                    const n = scene.typing.typedLength
                                    return "<font color='#e8ecf4'>" + scene.escapeHtml(code.substring(0, n))
                                        + "</font><font color='#ffcc00'>▌</font>"
                                        + scene.escapeHtml(code.substring(n))
                                }
                            }
                        }
                    }
                }
            }

            // Keyboard
            Model {
                source: "#Cube"
                position: Qt.vector3d(0, 75.2, -14)
                eulerRotation.x: 4
                scale: Qt.vector3d(0.46, 0.018, 0.16)
                materials: PrincipledMaterial { baseColor: "#232428"; roughness: 0.5 }
            }
            Repeater3D {
                model: 52
                Model {
                    required property int index
                    readonly property int row: Math.floor(index / 13)
                    readonly property int col: index % 13
                    source: "#Cube"
                    position: Qt.vector3d(-19.8 + col * 3.3 + row * 0.6,
                                          scene.pressedKey === index ? 76.4 : 77.0,
                                          -19 + row * 3.4)
                    eulerRotation.x: 4
                    scale: Qt.vector3d(0.028, 0.012, 0.028)
                    materials: PrincipledMaterial { baseColor: "#3a3c42"; roughness: 0.6 }
                }
            }
        }
    }

    // 2D overlay
    Column {
        anchors { top: parent.top; left: parent.left; margins: 18 }
        spacing: 6
        Text {
            text: game.night === 1 ? qsTr("Night 1 — the flight test is at 06:00")
                                   : qsTr("Night %1 — back at work after burgers").arg(game.night)
            color: "white"
            font { pixelSize: 20; bold: true }
            style: Text.Outline; styleColor: "#a0000000"
        }
        Text {
            text: scene.typing.unreadable ? qsTr("You can't read a thing anymore.")
                                          : qsTr("Type the code on the screen.")
            color: "#c8d0e0"
            font.pixelSize: 16
            style: Text.Outline; styleColor: "#a0000000"
        }
    }
    ProgressBar {
        anchors { top: parent.top; right: parent.right; margins: 18 }
        width: Math.min(300, scene.width * 0.35)
        label: qsTr("Alertness")
        value: 1 - scene.typing.fatigue
        fill: value > 0.3 ? "#4cd964" : "#ff3b30"
    }
    OverlayButton {
        anchors.centerIn: parent
        visible: scene.typing.unreadable
        text: qsTr("Go home")
        accent: "#2f7d32"
        onClicked: game.goHome()
    }
}
