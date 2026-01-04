#version 460 core

in vec2 TexCoord;
in vec3 Normal;
in float AO;
in float TexIndex;
in vec3 TintColor;  // Tint color (for consistency)

out vec4 FragColor;

uniform sampler2DArray u_TextureArray;  // Texture array for block textures
uniform vec3 u_LightDir;                // Normalized direction TO the light (sun)
uniform float u_AmbientStrength;        // 0.0 - 1.0 (recommend 0.3-0.4)
uniform float u_WaterAlpha;             // Water transparency (recommend 0.6-0.8)

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
    
    FragColor = vec4(waterColor * lighting, u_WaterAlpha);
}
