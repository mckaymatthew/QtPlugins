#version 440
// Single-pass disc blur: samples along a golden-angle spiral out to `radius` pixels. Used for the
// tired-eyes blur and for the out-of-focus background at the office.

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    float radius;    // pixels
    vec2 pixelStep;  // 1 / item size
};

layout(binding = 1) uniform sampler2D source;

const int Samples = 48;
const float GoldenAngle = 2.39996323;

void main()
{
    vec4 sum = texture(source, qt_TexCoord0);
    float total = 1.0;
    for (int i = 1; i <= Samples; ++i) {
        float t = float(i) / float(Samples);
        float r = sqrt(t) * radius;
        float a = float(i) * GoldenAngle;
        float w = 1.0 - 0.5 * t;
        sum += texture(source, qt_TexCoord0 + vec2(cos(a), sin(a)) * r * pixelStep) * w;
        total += w;
    }
    fragColor = sum / total * qt_Opacity;
}
