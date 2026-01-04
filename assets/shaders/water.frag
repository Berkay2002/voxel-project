#version 460 core

in vec2 TexCoord;
in vec3 Normal;
in float AO;
in float TexIndex;
in vec3 TintColor;  // Tint color (for consistency)
in vec3 FragWorldPos;  // World-space position for fog

out vec4 FragColor;

uniform sampler2DArray u_TextureArray;  // Texture array for block textures
uniform vec3 u_LightDir;                // Normalized direction TO the light (sun)
uniform float u_AmbientStrength;        // 0.0 - 1.0 (recommend 0.3-0.4)
uniform float u_WaterAlpha;             // Water transparency (recommend 0.6-0.8)

// Fog uniforms
uniform vec3 u_CameraPos;               // Camera world position
uniform vec3 u_FogColor;                // Fog color (should match sky)
uniform float u_FogStart;               // Distance where fog starts
uniform float u_FogEnd;                 // Distance where fog is fully opaque

void main() {
    // Sample from texture array
    vec4 texColor = texture(u_TextureArray, vec3(TexCoord, TexIndex));
    
    // Apply tint color (water tint from TintColor, or use texture directly)
    vec3 waterColor = texColor.rgb * TintColor;
    
    // Diffuse lighting (Lambertian reflection)
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, u_LightDir), 0.0);
    
    // Combine ambient + diffuse, modulated by ambient occlusion
    float lighting = (u_AmbientStrength + (1.0 - u_AmbientStrength) * diff) * AO;
    
    // Add slight brightness boost to water
    lighting = min(lighting * 1.1, 1.0);
    
    vec3 litColor = waterColor * lighting;
    
    // Distance fog calculation
    float dist = length(FragWorldPos - u_CameraPos);
    float fogFactor = clamp((u_FogEnd - dist) / (u_FogEnd - u_FogStart), 0.0, 1.0);
    vec3 finalColor = mix(u_FogColor, litColor, fogFactor);
    
    // Fade alpha with fog as well for smoother blending
    float finalAlpha = u_WaterAlpha * fogFactor;
    
    FragColor = vec4(finalColor, finalAlpha);
}

