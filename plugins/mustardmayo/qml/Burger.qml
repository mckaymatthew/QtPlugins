import QtQuick
import QtQuick3D

// A cheeseburger whose bun, cheese and patty are riddled with tight clusters of pits
// (textures from scripts/generate_burger_textures.py). About 140 units tall including the plate.
Node {
    id: burger

    component PittedMaterial: PrincipledMaterial {
        id: pitted
        property string surface
        property real tiling: 2
        roughness: 0.55
        normalStrength: 1.2
        baseColorMap: Texture {
            source: "assets/" + pitted.surface + "_color.png"
            scaleU: pitted.tiling
            scaleV: pitted.tiling
            tilingModeHorizontal: Texture.Repeat
            tilingModeVertical: Texture.Repeat
        }
        normalMap: Texture {
            source: "assets/" + pitted.surface + "_normal.png"
            scaleU: pitted.tiling
            scaleV: pitted.tiling
            tilingModeHorizontal: Texture.Repeat
            tilingModeVertical: Texture.Repeat
        }
    }

    PittedMaterial { id: bunMaterial; surface: "bun"; tiling: 2 }
    PittedMaterial { id: bunSideMaterial; surface: "bun"; tiling: 3 }
    PittedMaterial { id: pattyMaterial; surface: "patty"; tiling: 3; roughness: 0.8 }
    PittedMaterial { id: cheeseMaterial; surface: "cheese"; tiling: 1.5; roughness: 0.35 }

    Model { // plate
        source: "#Cylinder"
        y: 2
        scale: Qt.vector3d(1.4, 0.04, 1.4)
        materials: PrincipledMaterial { baseColor: "#f4f4f0"; roughness: 0.15 }
    }
    Model { // bottom bun
        source: "#Cylinder"
        y: 12
        scale: Qt.vector3d(1.0, 0.16, 1.0)
        materials: bunSideMaterial
    }
    Model { // patty
        source: "#Cylinder"
        y: 27
        scale: Qt.vector3d(1.08, 0.14, 1.08)
        materials: pattyMaterial
    }

    Model { // cheese slice, corners hanging out past the patty
        source: "#Cube"
        y: 35.5
        eulerRotation.y: 45
        scale: Qt.vector3d(1.02, 0.025, 1.02)
        materials: cheeseMaterial
    }

    Model { // tomato
        source: "#Cylinder"
        y: 40
        scale: Qt.vector3d(0.94, 0.05, 0.94)
        materials: PrincipledMaterial { baseColor: "#c8261c"; roughness: 0.25 }
    }
    Model { // top bun: a flattened sphere whose lower half hides inside the stack
        source: "#Sphere"
        y: 45
        scale: Qt.vector3d(1.05, 0.52, 1.05)
        materials: bunMaterial
    }
}
