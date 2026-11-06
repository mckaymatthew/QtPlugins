import QtQuick
import QtQuick3D

// A cable lying on a surface, running from `from` to `to` and snarled into loops on the way.
// Built from short cylinder segments; the loops rise over themselves so they cross in 3D.
// Positions are in the parent's coordinates, with `from.y` / `to.y` the surface height.
Node {
    id: cable

    property vector3d from: Qt.vector3d(0, 0, 0)
    property vector3d to: Qt.vector3d(50, 0, 0)
    property real loops: 5          // how many times it coils on the way
    property real loopRadius: 4     // cm
    property real thickness: 0.6    // cm
    property color color: "#202020"
    property int seed: 1
    property int segments: 90

    readonly property var pieces: {
        let s = seed * 7919 + 13
        const random = () => { s = (s * 16807) % 2147483647; return s / 2147483647 }

        const dx = to.x - from.x, dz = to.z - from.z
        const length = Math.sqrt(dx * dx + dz * dz) || 1
        const ux = dx / length, uz = dz / length // along the cable
        const px = -uz, pz = ux                  // across it

        // Wandering coils: a loop phase that speeds up and slows down, a radius that breathes,
        // and a slow sideways meander, so no two loops are alike.
        const points = []
        let phase = random() * 2 * Math.PI
        let radius = loopRadius
        let meander = 0, meanderRate = 0
        for (let i = 0; i <= segments; ++i) {
            const t = i / segments
            const envelope = Math.sin(Math.PI * t) // ends lie flat where they plug in
            phase += 2 * Math.PI * loops / segments * (0.5 + random())
            radius += (loopRadius * (0.5 + random()) - radius) * 0.2
            meanderRate += (random() - 0.5) * 0.6
            meanderRate *= 0.9
            meander += meanderRate
            const along = Math.cos(phase) * radius * envelope
            const across = Math.sin(phase) * radius * envelope + meander * envelope
            const lift = (0.5 + 0.5 * Math.sin(phase)) * thickness * 2.2 * envelope
            points.push(Qt.vector3d(from.x + ux * (t * length + along) + px * across,
                                    from.y + (to.y - from.y) * t + thickness / 2 + lift,
                                    from.z + uz * (t * length + along) + pz * across))
        }

        const result = []
        for (let i = 0; i < segments; ++i) {
            const a = points[i], b = points[i + 1]
            const d = b.minus(a)
            const len = d.length()
            if (len < 1e-4)
                continue
            result.push({
                position: a.plus(b).times(0.5),
                yaw: Math.atan2(d.x, d.z) * 180 / Math.PI,
                pitch: 90 - Math.asin(d.y / len) * 180 / Math.PI,
                length: len * 1.15 // overlap so bends have no gaps
            })
        }
        return result
    }

    PrincipledMaterial { id: insulation; baseColor: cable.color; roughness: 0.45 }

    Repeater3D {
        model: cable.pieces
        Node {
            required property var modelData
            position: modelData.position
            eulerRotation.y: modelData.yaw
            Model {
                source: "#Cylinder"
                eulerRotation.x: modelData.pitch
                scale: Qt.vector3d(cable.thickness / 100, modelData.length / 100, cable.thickness / 100)
                materials: insulation
            }
        }
    }
}
