import QtQuick
import QtQuick3D

// `game` is the GameModel set as a context property by TicTacToeExtension.
Item {
    id: root

    readonly property int playerX: 1
    readonly property int playerO: 2
    readonly property real cellSize: 100 // scene units (built-in meshes are 100 units across)
    property int hoveredCell: -1

    function cellPosition(index) {
        return Qt.vector3d((index % 3 - 1) * cellSize, 0, (Math.floor(index / 3) - 1) * cellSize)
    }

    // Maps a point in the view to a board cell by picking the board's top surface.
    function cellAt(x, y) {
        const result = view.pick(x, y)
        if (result.objectHit !== boardBase)
            return -1
        const p = result.scenePosition
        const col = Math.floor(p.x / cellSize + 1.5)
        const row = Math.floor(p.z / cellSize + 1.5)
        return col < 0 || col > 2 || row < 0 || row > 2 ? -1 : row * 3 + col
    }

    View3D {
        id: view
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#1d2330"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
        }

        // Camera rig sways gently around the board's vertical axis.
        Node {
            SequentialAnimation on eulerRotation.y {
                loops: Animation.Infinite
                NumberAnimation { from: -10; to: 10; duration: 7000; easing.type: Easing.InOutSine }
                NumberAnimation { from: 10; to: -10; duration: 7000; easing.type: Easing.InOutSine }
            }
            PerspectiveCamera {
                position: Qt.vector3d(0, 510, 400)
                eulerRotation.x: -52
                fieldOfView: 45
                clipNear: 10
                clipFar: 2000
            }
        }

        DirectionalLight {
            eulerRotation: Qt.vector3d(-60, -35, 0)
            brightness: 1.1
            castsShadow: true
            shadowFactor: 40
            shadowMapQuality: Light.ShadowMapQualityHigh
        }
        DirectionalLight {
            eulerRotation: Qt.vector3d(-25, 145, 0)
            brightness: 0.35
        }

        // Board: a wooden slab with raised grid bars.
        Model {
            id: boardBase
            source: "#Cube"
            y: -10
            scale: Qt.vector3d(3.4, 0.2, 3.4)
            pickable: true
            receivesShadows: true
            materials: PrincipledMaterial { baseColor: "#c79a62"; roughness: 0.55 }
        }
        Repeater3D {
            model: 4
            Model {
                required property int index
                readonly property bool vertical: index < 2
                readonly property real offset: ((index % 2) - 0.5) * root.cellSize
                source: "#Cube"
                position: vertical ? Qt.vector3d(offset, 6, 0) : Qt.vector3d(0, 6, offset)
                scale: vertical ? Qt.vector3d(0.07, 0.12, 3.1) : Qt.vector3d(3.1, 0.12, 0.07)
                castsShadows: true
                receivesShadows: true
                materials: PrincipledMaterial { baseColor: "#5b3a1e"; roughness: 0.4 }
            }
        }

        // Hover highlight on empty cells.
        Model {
            source: "#Rectangle"
            visible: root.hoveredCell >= 0 && !game.gameOver
                     && game.cells[root.hoveredCell] === 0
            position: root.hoveredCell >= 0 ? root.cellPosition(root.hoveredCell).plus(Qt.vector3d(0, 1, 0))
                                            : Qt.vector3d(0, 0, 0)
            eulerRotation.x: -90
            scale: Qt.vector3d(0.86, 0.86, 1)
            materials: PrincipledMaterial {
                baseColor: game.currentPlayer === root.playerX ? "#ff8a80" : "#80c8ff"
                lighting: PrincipledMaterial.NoLighting
                opacity: 0.35
                alphaMode: PrincipledMaterial.Blend
            }
        }

        // Pieces: one X and one O per cell, grown in/out as the board changes.
        Repeater3D {
            model: 9
            Node {
                id: cell
                required property int index
                readonly property int owner: game.cells[index]
                readonly property bool winning: game.winningLine.indexOf(index) >= 0
                position: root.cellPosition(index)

                XPiece { shown: cell.owner === root.playerX; winning: cell.winning }
                OPiece { shown: cell.owner === root.playerO; winning: cell.winning }
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onClicked: (mouse) => {
            const index = root.cellAt(mouse.x, mouse.y)
            if (index >= 0)
                game.play(index)
        }
        onPositionChanged: (mouse) => root.hoveredCell = root.cellAt(mouse.x, mouse.y)
        onExited: root.hoveredCell = -1
    }

    // 2D overlay: status, score and controls.
    Column {
        anchors { top: parent.top; horizontalCenter: parent.horizontalCenter; topMargin: 16 }
        spacing: 4

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            color: "white"
            font { pixelSize: 26; bold: true }
            text: {
                if (game.gameOver)
                    return game.draw ? qsTr("Draw!")
                                     : game.winner === root.playerX ? qsTr("X wins!") : qsTr("O wins!")
                const player = game.currentPlayer === root.playerX ? "X" : "O"
                if (game.vsComputer && game.currentPlayer === root.playerO)
                    return qsTr("Computer is thinking…")
                return qsTr("%1 to move").arg(player)
            }
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            color: "#b8c2d6"
            font.pixelSize: 15
            text: qsTr("X %1   ·   Draws %2   ·   O %3").arg(game.xWins).arg(game.draws).arg(game.oWins)
        }
    }

    Row {
        anchors { bottom: parent.bottom; horizontalCenter: parent.horizontalCenter; bottomMargin: 16 }
        spacing: 12

        OverlayButton {
            text: qsTr("New game")
            onClicked: game.reset()
        }
        OverlayButton {
            text: game.vsComputer ? qsTr("Opponent: Computer") : qsTr("Opponent: Human")
            onClicked: game.vsComputer = !game.vsComputer
        }
    }
}
