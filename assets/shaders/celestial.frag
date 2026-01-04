#version 460 core

in vec2 v_TexCoord;

out vec4 FragColor;

uniform sampler2D u_Texture;

void main() {
    vec4 color = texture(u_Texture, v_TexCoord);
    
    // Alpha test for transparent parts of sun/moon textures
    if (color.a < 0.1) {
        discard;
    }
    
    FragColor = color;
}
