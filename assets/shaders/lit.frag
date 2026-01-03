#version 460 core

in vec2 TexCoord;
in vec3 Normal;
in float AO;
in float TexIndex;

out vec4 FragColor;

uniform sampler2DArray u_TextureArray;  // Texture array for block textures
uniform vec3 u_LightDir;                // Normalized direction TO the light (sun)
uniform float u_AmbientStrength;        // 0.0 - 1.0 (recommend 0.3-0.4)

void main() {
    // Sample from texture array using layer index
    vec4 texColor = texture(u_TextureArray, vec3(TexCoord, TexIndex));
    
    // Diffuse lighting (Lambertian reflection)
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, u_LightDir), 0.0);
    
    // Combine ambient + diffuse, modulated by ambient occlusion
    float lighting = (u_AmbientStrength + (1.0 - u_AmbientStrength) * diff) * AO;
    
    FragColor = vec4(texColor.rgb * lighting, texColor.a);
}
