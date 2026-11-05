import QtQuick

// Disc blur for use as `layer.effect`; the layer supplies `source`.
ShaderEffect {
    property variant source
    property real radius: 0
    readonly property vector2d pixelStep: Qt.vector2d(1 / Math.max(1, width), 1 / Math.max(1, height))
    fragmentShader: "qrc:/qtplugins/mustardmayo/shaders/blur.frag.qsb"
}
