#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in float aLightLevel;  // Two-tone: 0.65 bottom, 0.85 side, 1.0 top

out float v_Light;
out float v_Fog;
out vec3 v_Normal;

uniform mat4 u_ViewProj;
uniform vec3 u_CameraPos;
uniform float u_FogStart;
uniform float u_FogEnd;

void main() {
    gl_Position = u_ViewProj * vec4(aPos, 1.0);
    
    v_Light = aLightLevel;
    v_Normal = aNormal;
    
    // Distance-based fog
    float dist = distance(aPos.xz, u_CameraPos.xz);
    v_Fog = clamp((dist - u_FogStart) / (u_FogEnd - u_FogStart), 0.0, 1.0);
}
