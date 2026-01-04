#version 460 core

in vec2 v_TexCoord;
in float v_Alpha;

out vec4 FragColor;

uniform sampler2D u_Texture;

void main() {
    vec4 color = texture(u_Texture, v_TexCoord);
    
    // Apply distance-based alpha fade
    color.a *= v_Alpha;
    
    // Discard very faint particles
    if (color.a < 0.01) {
        discard;
    }
    
    FragColor = color;
}
