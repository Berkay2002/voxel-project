#version 460 core

layout (location = 0) in vec2 a_Position;

void main() {
    // Position is already in NDC (-1 to 1)
    gl_Position = vec4(a_Position, 0.0, 1.0);
}
