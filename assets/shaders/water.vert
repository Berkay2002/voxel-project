#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in float aAO;
layout (location = 4) in float aTexIndex;
layout (location = 5) in vec3 aTintColor;  // Tint color (for consistency with lit shader)

out vec2 TexCoord;
out vec3 Normal;
out float AO;
out float TexIndex;
out vec3 TintColor;

uniform mat4 u_MVP;
uniform mat4 u_Model;
uniform float u_Time;  // For optional animated UVs

void main() {
    gl_Position = u_MVP * vec4(aPos, 1.0);
    
    // Simple UV animation (optional - subtle wave effect)
    // vec2 animOffset = vec2(sin(u_Time * 0.5), cos(u_Time * 0.3)) * 0.1;
    // TexCoord = aTexCoord + animOffset;
    TexCoord = aTexCoord;  // Static UVs for now
    
    // Transform normal to world space
    Normal = mat3(u_Model) * aNormal;
    AO = aAO;
    TexIndex = aTexIndex;
    TintColor = aTintColor;
}
