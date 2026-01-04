#version 460 core

// Depth pre-pass fragment shader
// Outputs view-space normals (depth is written automatically by hardware)

in vec3 ViewNormal;

layout (location = 0) out vec3 gNormal;

void main() {
    // Normalize and output view-space normal
    // Range [-1, 1] stored in RGB16F texture
    gNormal = normalize(ViewNormal);
}
