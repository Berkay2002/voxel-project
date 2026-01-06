#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in float aAO;
layout (location = 4) in float aTexIndex;
layout (location = 5) in vec3 aTintColor;  // Biome tint color for grass/foliage
layout (location = 6) in float aSkyLight;  // Sky light exposure (0=underground, 1=open sky)

out vec2 TexCoord;
out vec3 Normal;
out float AO;
out float TexIndex;
out vec3 TintColor;
out vec3 FragWorldPos;        // World-space position for fog calculation
out vec4 FragPosLightSpace;   // Light-space position for shadow mapping
out float SkyLight;           // Sky light exposure for directional lighting

uniform mat4 u_MVP;
uniform mat4 u_Model;
uniform mat4 u_LightSpaceMatrix;  // For shadow mapping

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
    FragPosLightSpace = u_LightSpaceMatrix * worldPos;
    SkyLight = aSkyLight;
}
