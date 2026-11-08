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
    component PowerBrick: Node { // a laptop-style power brick
        Model {
            source: "#Cube"
            y: 1.6
            scale: Qt.vector3d(0.065, 0.032, 0.12)
            materials: PrincipledMaterial { baseColor: "#141416"; roughness: 0.5 }
        }
        Model { // status LED
            source: "#Cube"
            position: Qt.vector3d(0, 3.25, 4)
            scale: Qt.vector3d(0.006, 0.002, 0.006)
            materials: PrincipledMaterial {
                baseColor: "#5cff6a"; emissiveFactor: Qt.vector3d(0.4, 1, 0.45)
                lighting: PrincipledMaterial.NoLighting
            }
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

                // Bench supplies, feeding the cards through snarled test leads.
                PowerSupply { position: Qt.vector3d(-68, 74, -74); eulerRotation.y: 15 }
                TangledCable { from: Qt.vector3d(-56.4, 75.5, -65.3); to: Qt.vector3d(-48, 74.8, -44)
                               loops: 4; loopRadius: 2.5; thickness: 0.5; color: "#d63c3c"; seed: 3 }
                TangledCable { from: Qt.vector3d(-53.5, 75.5, -66.1); to: Qt.vector3d(-38, 75, -56)
                               loops: 3; loopRadius: 2.2; thickness: 0.5; color: "#1a1a1a"; seed: 4 }
                PowerSupply {
                    position: Qt.vector3d(64, 74, -80); eulerRotation.y: -22
                    voltageColor: "#ff6a1f"; currentColor: "#ff3b1f"
                }
                TangledCable { from: Qt.vector3d(70, 75.5, -70); to: Qt.vector3d(50, 74.8, -50)
                               loops: 5; loopRadius: 3; thickness: 0.5; color: "#d63c3c"; seed: 5 }
                TangledCable { from: Qt.vector3d(73, 75.5, -71); to: Qt.vector3d(30, 74.8, -70)
                               loops: 6; loopRadius: 3; thickness: 0.5; color: "#1a1a1a"; seed: 6 }

                Model { // power strip along the back, every socket taken
                    source: "#Cube"
                    position: Qt.vector3d(-5, 75.5, -84)
                    scale: Qt.vector3d(0.5, 0.03, 0.05)
                    materials: PrincipledMaterial { baseColor: "#e4e2dc"; roughness: 0.6 }
                }
                Repeater3D {
                    model: 5
                    Model { // plugs
                        required property int index
                        source: "#Cube"
                        position: Qt.vector3d(-22 + index * 8, 78, -84)
                        scale: Qt.vector3d(0.04, 0.035, 0.035)
                        materials: PrincipledMaterial { baseColor: "#18181a"; roughness: 0.5 }
                    }
                }
                Model { // its glowing switch
                    source: "#Cube"
                    position: Qt.vector3d(19, 77, -84)
                    scale: Qt.vector3d(0.03, 0.01, 0.025)
                    materials: PrincipledMaterial {
                        baseColor: "#ff3020"; emissiveFactor: Qt.vector3d(1, 0.2, 0.1)
                        lighting: PrincipledMaterial.NoLighting
                    }
                }
                PowerBrick { position: Qt.vector3d(-74, 74, -20); eulerRotation.y: 35 }
                PowerBrick { position: Qt.vector3d(76, 74, -32); eulerRotation.y: -50 }

                // The cable nest: power cords, USB and network, all wound round each other.
                TangledCable { from: Qt.vector3d(-22, 74, -84); to: Qt.vector3d(-74, 74, -20)
                               loops: 9; loopRadius: 5; thickness: 0.8; color: "#141414"; seed: 11 }
                TangledCable { from: Qt.vector3d(-14, 74, -84); to: Qt.vector3d(-60, 74, -64)
                               loops: 6; loopRadius: 4; thickness: 0.9; color: "#141414"; seed: 12 }
                TangledCable { from: Qt.vector3d(2, 74, -84); to: Qt.vector3d(76, 74, -32)
                               loops: 10; loopRadius: 5; thickness: 0.8; color: "#141414"; seed: 13 }
                TangledCable { from: Qt.vector3d(10, 74, -84); to: Qt.vector3d(58, 74, -76)
                               loops: 4; loopRadius: 3; thickness: 0.9; color: "#141414"; seed: 14 }
                TangledCable { from: Qt.vector3d(-80, 74, -50); to: Qt.vector3d(-28, 74, -28)
                               loops: 8; loopRadius: 5; thickness: 0.6; color: "#2f6ad6"; seed: 21 }
                TangledCable { from: Qt.vector3d(-62, 74, -10); to: Qt.vector3d(-30, 74, -70)
                               loops: 7; loopRadius: 4; thickness: 0.5; color: "#9a9ca2"; seed: 22 }
                TangledCable { from: Qt.vector3d(24, 74, -14); to: Qt.vector3d(86, 74, -58)
                               loops: 8; loopRadius: 4.5; thickness: 0.6; color: "#e0c020"; seed: 23 }
                TangledCable { from: Qt.vector3d(30, 74, -60); to: Qt.vector3d(82, 74, -12)
                               loops: 7; loopRadius: 4; thickness: 0.5; color: "#e07020"; seed: 24 }

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
        visible: scene.typing.unreadable && !scene.typing.emailOpen
        text: qsTr("Go home")
        accent: "#2f7d32"
        onClicked: game.goHome()
    }

    // An email from the manager, demanding attention until Enter is pressed. Crisp, because it's
    // outside the blurred world: you can always read the email, just not your code.
    Rectangle {
        id: emailPopup
        objectName: "emailPopup"
        readonly property bool shown: scene.typing.emailOpen
        anchors.centerIn: parent
        anchors.verticalCenterOffset: shown ? 0 : 30
        Behavior on anchors.verticalCenterOffset { NumberAnimation { duration: 180; easing.type: Easing.OutCubic } }
        opacity: shown ? 1 : 0
        visible: opacity > 0
        Behavior on opacity { NumberAnimation { duration: 150 } }

        width: Math.min(560, scene.width * 0.8)
        height: emailColumn.implicitHeight
        radius: 8
        color: "#f7f8fa"
        border.color: "#9aa3b2"

        Column {
            id: emailColumn
            width: parent.width

            Rectangle { // window title bar
                width: parent.width
                height: 34
                radius: 8
                color: "#0a64d8"
                Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 8; color: parent.color }
                Text {
                    anchors { left: parent.left; leftMargin: 14; verticalCenter: parent.verticalCenter }
                    text: qsTr("New message — Inbox (%1 unread)").arg(212 + scene.typing.emailCount * 7)
                    color: "white"
                    font { pixelSize: 14; bold: true }
                }
                Rectangle { // importance flag
                    anchors { right: parent.right; rightMargin: 12; verticalCenter: parent.verticalCenter }
                    width: 22; height: 22; radius: 11
                    color: "#e8322a"
                    Text { anchors.centerIn: parent; text: "!"; color: "white"; font { pixelSize: 15; bold: true } }
                }
            }
            Column {
                x: 18
                width: parent.width - 36
                topPadding: 14
                bottomPadding: 16
                spacing: 6
                Text {
                    text: qsTr("From: Program Manager")
                    color: "#5a6372"
                    font.pixelSize: 14
                }
                Text {
                    width: parent.width
                    text: scene.typing.emailSubject
                    color: "#1b1f27"
                    font { pixelSize: 20; bold: true }
                    wrapMode: Text.Wrap
                }
                Rectangle { width: parent.width; height: 1; color: "#d5dae2" }
                Text {
                    width: parent.width
                    topPadding: 4
                    text: scene.typing.emailBody
                    color: "#1b1f27"
                    font.pixelSize: 16
                    wrapMode: Text.Wrap
                    lineHeight: 1.15
                }
                Item { width: 1; height: 8 }
                Rectangle { // the only way out
                    anchors.right: parent.right
                    width: okText.implicitWidth + 32
                    height: 34
                    radius: 5
                    color: "#0a64d8"
                    Text {
                        id: okText
                        anchors.centerIn: parent
                        text: qsTr("Acknowledge  ⏎ Enter")
                        color: "white"
                        font { pixelSize: 15; bold: true }
                    }
                }
            }
        }
    }
}
