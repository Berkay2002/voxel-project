#version 460 core

out vec4 FragColor;

uniform vec4 u_OutlineColor;  // Color of the outline (typically black with some alpha)

void main() {
    FragColor = u_OutlineColor;
}
