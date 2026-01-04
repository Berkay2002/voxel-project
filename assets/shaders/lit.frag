#version 460 core

in vec2 TexCoord;
in vec3 Normal;
in float AO;
in float TexIndex;
in vec3 TintColor;  // Biome tint for grass/foliage (white = no tint)

out vec4 FragColor;

uniform sampler2DArray u_TextureArray;  // Texture array for block textures
uniform vec3 u_LightDir;                // Normalized direction TO the light (sun)
uniform float u_AmbientStrength;        // 0.0 - 1.0 (recommend 0.3-0.4)

void main() {
    // Sample from texture array using layer index
    vec4 texColor = texture(u_TextureArray, vec3(TexCoord, TexIndex));
    
    // Apply biome tint (multiply RGB by tint color)
    // TintColor is (1,1,1) for non-tinted blocks, so this is a no-op for them
    vec3 tintedColor = texColor.rgb * TintColor;
    
    // Diffuse lighting (Lambertian reflection)
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, u_LightDir), 0.0);
    
    // Combine ambient + diffuse, modulated by ambient occlusion
    float lighting = (u_AmbientStrength + (1.0 - u_AmbientStrength) * diff) * AO;
    
    FragColor = vec4(tintedColor * lighting, texColor.a);
}
