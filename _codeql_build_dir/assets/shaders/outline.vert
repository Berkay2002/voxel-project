#version 460 core

layout (location = 0) in vec3 aPos;

uniform mat4 u_MVP;
uniform vec3 u_BlockPos;  // World position of the block to highlight
uniform float u_Scale;    // Slight scale-up to render outside the block (e.g., 1.005)

void main() {
    // Scale the unit cube and position at block location
    vec3 worldPos = u_BlockPos + aPos * u_Scale;
    gl_Position = u_MVP * vec4(worldPos, 1.0);
}
