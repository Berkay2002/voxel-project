#version 460 core

// Fullscreen quad vertex shader for post-processing passes
// Uses a single triangle that covers the entire screen (more efficient than quad)

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
