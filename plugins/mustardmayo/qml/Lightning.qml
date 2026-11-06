import QtQuick

// A jagged lightning bolt in the sky. strike() draws a fresh bolt and runs the flash; `flash`
// (0..1) follows it so the rest of the scene can light up with it.
Canvas {
    id: bolt

    property real flash: 0
    property real horizon: height * 0.5 // where the bolt meets the ground

    // Starts a strike: a new bolt, then a bright flash with a flicker as it dies away.
    function strike() {
        seed = Math.random()
        requestPaint()
        flashAnimation.restart()
    }

    property real seed: 0
    opacity: flash

    onPaint: {
        const ctx = getContext("2d")
        ctx.reset()
        let s = seed * 233280
        const random = () => { s = (s * 9301 + 49297) % 233280; return s / 233280 }

        // A zigzag from (x, y) down to endY, drifting sideways by `drift` per pixel of drop.
        function branch(x, y, endY, drift, width) {
            ctx.lineWidth = width
            ctx.beginPath()
            ctx.moveTo(x, y)
            while (y < endY) {
                const step = 8 + random() * 22
                y = Math.min(endY, y + step)
                x += (drift + (random() - 0.5) * 1.6) * step
                ctx.lineTo(x, y)
                if (width > 1.2 && random() < 0.18) { // fork off a thinner, shorter branch
                    ctx.stroke()
                    const side = random() < 0.5 ? -1 : 1
                    branch(x, y, y + (endY - y) * (0.25 + random() * 0.35),
                           side * (0.4 + random() * 0.5), width * 0.5)
                    ctx.lineWidth = width
                    ctx.beginPath()
                    ctx.moveTo(x, y)
                }
            }
            ctx.stroke()
        }

        ctx.lineCap = "round"
        ctx.lineJoin = "round"
        ctx.shadowColor = "#a0b4ff"
        ctx.shadowBlur = 14
        ctx.strokeStyle = "#f4f6ff"
        const startX = width * (0.15 + random() * 0.7)
        branch(startX, 0, horizon, (random() - 0.5) * 0.4, 3.5)
    }

    SequentialAnimation {
        id: flashAnimation
        NumberAnimation { target: bolt; property: "flash"; to: 1; duration: 40 }
        NumberAnimation { target: bolt; property: "flash"; to: 0.25; duration: 110 }
        NumberAnimation { target: bolt; property: "flash"; to: 0.9; duration: 50 }
        PauseAnimation { duration: 90 }
        NumberAnimation { target: bolt; property: "flash"; to: 0; duration: 600; easing.type: Easing.InQuad }
    }
}
