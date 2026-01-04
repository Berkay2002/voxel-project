#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in float aAO;
layout (location = 4) in float aTexIndex;
layout (location = 5) in vec3 aTintColor;  // Biome tint color for grass/foliage

out vec2 TexCoord;
out vec3 Normal;
out float AO;
out float TexIndex;
out vec3 TintColor;
out vec3 FragWorldPos;  // World-space position for fog calculation

uniform mat4 u_MVP;
uniform mat4 u_Model;

void main() {
    vec4 worldPos = u_Model * vec4(aPos, 1.0);
    gl_Position = u_MVP * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
    // Transform normal to world space
    Normal = mat3(u_Model) * aNormal;
    AO = aAO;
    TexIndex = aTexIndex;
    TintColor = aTintColor;
    FragWorldPos = worldPos.xyz;
}

