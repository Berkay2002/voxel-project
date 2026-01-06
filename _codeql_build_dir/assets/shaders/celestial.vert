#version 460 core

layout(location = 0) in vec3 aPos;      // Billboard corner offsets (-0.5 to 0.5)
layout(location = 1) in vec2 aTexCoord;

out vec2 v_TexCoord;

uniform mat4 u_ViewProj;
uniform vec3 u_WorldPos;      // Center position of the billboard
uniform float u_Size;         // Size of the billboard in world units
uniform vec2 u_UVOffset;      // For moon phase selection (0,0 for sun)
uniform vec2 u_UVScale;       // For moon phase selection (1,1 for sun)
uniform vec3 u_CameraRight;   // Camera right vector (for billboarding)
uniform vec3 u_CameraUp;      // Camera up vector (for billboarding)

void main() {
    // Billboard always faces camera by using camera's right/up vectors
    vec3 worldPos = u_WorldPos
        + u_CameraRight * aPos.x * u_Size
        + u_CameraUp * aPos.y * u_Size;
    
    gl_Position = u_ViewProj * vec4(worldPos, 1.0);
    
    // Apply UV offset and scale for moon phases
    // Sun: offset=(0,0), scale=(1,1) - uses full texture
    // Moon: offset=(phase%4 * 0.25, phase/4 * 0.5), scale=(0.25, 0.5)
    v_TexCoord = aTexCoord * u_UVScale + u_UVOffset;
}
