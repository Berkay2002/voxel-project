#version 460 core

in vec2 v_TexCoord;

out vec4 FragColor;

uniform sampler2D u_Texture;
uniform vec4 u_Tint = vec4(1.0, 1.0, 1.0, 1.0);

void main() {
    vec4 texColor = texture(u_Texture, v_TexCoord);
    FragColor = texColor * u_Tint;
    
    // Discard fully transparent pixels
    if (FragColor.a < 0.01) {
        discard;
    }
}
