#version 460 core

in vec2 TexCoord;
in vec3 Normal;
in float AO;

out vec4 FragColor;

uniform sampler2D u_Texture;
uniform vec3 u_LightDir;          // Normalized direction TO the light (sun)
uniform float u_AmbientStrength;  // 0.0 - 1.0 (recommend 0.3-0.4)
uniform float u_WaterAlpha;       // Water transparency (recommend 0.6-0.8)

void main() {
    vec4 texColor = texture(u_Texture, TexCoord);
    
    // Water tint - blend with blue
    vec3 waterTint = vec3(0.2, 0.4, 0.8);
    vec3 tintedColor = mix(texColor.rgb, waterTint, 0.6);
    
    // Diffuse lighting (Lambertian reflection)
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, u_LightDir), 0.0);
    
    // Combine ambient + diffuse, modulated by ambient occlusion
    float lighting = (u_AmbientStrength + (1.0 - u_AmbientStrength) * diff) * AO;
    
    // Add slight brightness boost to water
    lighting = min(lighting * 1.1, 1.0);
    
    FragColor = vec4(tintedColor * lighting, u_WaterAlpha);
}
