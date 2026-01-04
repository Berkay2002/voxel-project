#version 460 core

in vec2 TexCoord;
in vec3 Normal;
in float AO;
in float TexIndex;
in vec3 TintColor;        // Biome tint for grass/foliage (white = no tint)
in vec3 FragWorldPos;     // World-space position for fog
in vec4 FragPosLightSpace;// Light-space position for shadow mapping

out vec4 FragColor;

uniform sampler2DArray u_TextureArray;  // Texture array for block textures
uniform vec3 u_LightDir;                // Normalized direction TO the light (sun)
uniform float u_AmbientStrength;        // 0.0 - 1.0 (recommend 0.3-0.4)

// Fog uniforms
uniform vec3 u_CameraPos;               // Camera world position
uniform vec3 u_FogColor;                // Fog color (should match sky)
uniform float u_FogStart;               // Distance where fog starts
uniform float u_FogEnd;                 // Distance where fog is fully opaque

// Shadow mapping uniforms
uniform sampler2DShadow u_ShadowMap;    // Shadow depth texture with hardware comparison
uniform bool u_ShadowsEnabled;          // Toggle for shadows (disabled at night)
uniform float u_ShadowStrength;         // Fade factor for smooth dawn/dusk transition (0-1)

// SSAO (Screen-Space Ambient Occlusion) uniforms
uniform sampler2D u_SSAOTex;            // Blurred SSAO texture
uniform bool u_SSAOEnabled;             // Toggle for SSAO (O key)
uniform vec2 u_ScreenSize;              // Screen dimensions for UV calculation

/**
 * Calculate shadow factor using PCF (Percentage Closer Filtering)
 * Returns 1.0 = fully lit, 0.0 = fully in shadow
 */
float CalculateShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    // Perspective divide (light uses ortho so w=1, but good practice)
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // Transform to [0,1] range (NDC is [-1,1])
    projCoords = projCoords * 0.5 + 0.5;
    
    // Outside shadow map = not in shadow
    if (projCoords.z > 1.0) {
        return 1.0;
    }
    
    // Dynamic bias based on surface angle to light
    // Steeper angles need more bias to prevent shadow acne
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.001);
    
    // PCF: Sample 3x3 kernel for soft shadow edges
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(u_ShadowMap, 0);
    
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            vec2 offset = vec2(x, y) * texelSize;
            // sampler2DShadow does depth comparison automatically
            // Returns 1.0 if current depth (with bias) <= shadow map depth
            float sampleDepth = projCoords.z - bias;
            shadow += texture(u_ShadowMap, vec3(projCoords.xy + offset, sampleDepth));
        }
    }
    shadow /= 9.0;  // Average of 9 samples
    
    return shadow;
}

void main() {
    // Sample from texture array using layer index
    vec4 texColor = texture(u_TextureArray, vec3(TexCoord, TexIndex));
    
    // Apply biome tint (multiply RGB by tint color)
    // TintColor is (1,1,1) for non-tinted blocks, so this is a no-op for them
    vec3 tintedColor = texColor.rgb * TintColor;
    
    // Diffuse lighting (Lambertian reflection)
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, u_LightDir), 0.0);
    
    // Shadow calculation (only when enabled - disabled at night)
    float shadow = 1.0;
    if (u_ShadowsEnabled) {
        float rawShadow = CalculateShadow(FragPosLightSpace, norm, u_LightDir);
        // Blend shadow based on strength (smooth fade near dawn/dusk)
        shadow = mix(1.0, rawShadow, u_ShadowStrength);
    }
    
    // Select AO source: SSAO (screen-space) or vertex AO
    // When SSAO is enabled, use the screen-space result; otherwise use per-vertex AO
    float ao = AO;  // Default to vertex AO
    if (u_SSAOEnabled) {
        vec2 screenUV = gl_FragCoord.xy / u_ScreenSize;
        ao = texture(u_SSAOTex, screenUV).r;
    }
    
    // Combine ambient + diffuse * shadow, modulated by ambient occlusion
    // Shadows only affect diffuse lighting, not ambient
    float lighting = (u_AmbientStrength + (1.0 - u_AmbientStrength) * diff * shadow) * ao;
    
    vec3 litColor = tintedColor * lighting;
    
    // Distance fog calculation
    float dist = length(FragWorldPos - u_CameraPos);
    float fogFactor = clamp((u_FogEnd - dist) / (u_FogEnd - u_FogStart), 0.0, 1.0);
    vec3 finalColor = mix(u_FogColor, litColor, fogFactor);
    
    FragColor = vec4(finalColor, texColor.a);
}
