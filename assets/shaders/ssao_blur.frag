#version 460 core

// SSAO blur shader (5x5 box blur)
// Smooths the noisy SSAO result for a cleaner look

in vec2 TexCoord;
out float FragColor;

uniform sampler2D u_SSAOTex;

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(u_SSAOTex, 0));
    float result = 0.0;
    
    // 5x5 box blur
    for (int x = -2; x <= 2; ++x) {
        for (int y = -2; y <= 2; ++y) {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(u_SSAOTex, TexCoord + offset).r;
        }
    }
    
    FragColor = result / 25.0;  // Average of 25 samples
}
