#version 460 core

in vec2 v_TexCoord;
in float v_FogFactor;

out vec4 FragColor;

uniform sampler2D u_CloudTexture;
uniform vec3 u_SkyColor;      // Current sky color for fog blending
uniform float u_CloudBrightness;  // Dim clouds at night (0.3 night, 1.0 day)

void main() {
    vec4 cloudColor = texture(u_CloudTexture, v_TexCoord);
    
    // Minecraft-style alpha test: cloud or not-cloud, no soft edges
    // The clouds.png uses alpha channel for cloud/sky distinction
    if (cloudColor.a < 0.5) {
        discard;
    }
    
    // Brighten/darken clouds based on time of day
    cloudColor.rgb *= u_CloudBrightness;
    
    // Blend toward sky color at distance (fog)
    cloudColor.rgb = mix(cloudColor.rgb, u_SkyColor, v_FogFactor);
    
    // Output with full opacity (already discarded transparent pixels)
    FragColor = vec4(cloudColor.rgb, 1.0);
}
