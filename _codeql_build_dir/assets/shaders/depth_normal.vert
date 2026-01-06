#version 460 core

// Depth pre-pass vertex shader
// Outputs view-space normals for SSAO hemisphere sampling

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec3 aNormal;

out vec3 ViewNormal;

uniform mat4 u_MVP;        // Model * View * Projection
uniform mat4 u_ModelView;  // Model * View (for normal transformation)

void main() {
    gl_Position = u_MVP * vec4(aPos, 1.0);
    
    // Transform normal to view space (use inverse transpose for non-uniform scaling)
    // For voxels with uniform scaling, mat3(u_ModelView) is sufficient
    ViewNormal = normalize(mat3(u_ModelView) * aNormal);
}
