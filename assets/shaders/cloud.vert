#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 v_TexCoord;
out float v_FogFactor;

uniform mat4 u_ViewProj;
uniform vec3 u_CameraPos;
uniform float u_CloudOffset;  // Eastward drift offset

void main() {
    // Cloud plane is rendered at fixed world position (centered on camera XZ)
    vec3 worldPos = aPos;
    worldPos.x += u_CameraPos.x;
    worldPos.z += u_CameraPos.z;
    
    gl_Position = u_ViewProj * vec4(worldPos, 1.0);
    
    // Scroll UVs for cloud drift animation (eastward = +X = +U)
    v_TexCoord = aTexCoord + vec2(u_CloudOffset, 0.0);
    
    // Distance-based fog (fade clouds at edge of render distance)
    float dist = distance(worldPos.xz, u_CameraPos.xz);
    float fogStart = 80.0;
    float fogEnd = 200.0;
    v_FogFactor = clamp((dist - fogStart) / (fogEnd - fogStart), 0.0, 1.0);
}
