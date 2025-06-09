#version 450

layout (location = 0) out vec2 outCoord;

vec2 positions[3] = vec2[](
    vec2(3.0f, 1.0f),
    vec2(-1.0f, 1.0f),
    vec2(-1.0f, -3.0f)
);

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0f, 1.0f);
    outCoord = positions[gl_VertexIndex];
}