#version 460 core

// SSAO calculation shader (half-resolution)
// Based on LearnOpenGL SSAO tutorial with hemisphere sampling

in vec2 TexCoord;
out float FragColor;

uniform sampler2D u_DepthTex;   // Full-res depth buffer
uniform sampler2D u_NormalTex;  // Full-res view-space normals
uniform sampler2D u_NoiseTex;   // 4x4 rotation noise

// Hemisphere sample kernel (UBO for efficiency - uploaded once at setup)
layout(std140, binding = 0) uniform KernelBlock {
    vec4 u_Samples[64];  // Padded to vec4 for std140 alignment
};
uniform mat4 u_Projection;      // Camera projection matrix
uniform mat4 u_InvProjection;   // Inverse projection for depth reconstruction

uniform vec2 u_NoiseScale;      // screenSize / 4.0 (for noise tiling)
uniform float u_Radius;         // Sampling radius
uniform float u_Bias;           // Depth bias to prevent self-occlusion
uniform float u_Power;          // AO intensity/contrast

/**
 * Reconstruct view-space position from depth
 */
vec3 ViewPosFromDepth(vec2 uv, float depth) {
    // NDC position (z in [0,1] from depth texture)
    vec4 clipPos = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    
    // Transform to view space
    vec4 viewPos = u_InvProjection * clipPos;
    return viewPos.xyz / viewPos.w;
}

void main() {
    // Sample depth at current fragment
    float depth = texture(u_DepthTex, TexCoord).r;
    
    // Skip sky (depth = 1.0)
    if (depth >= 1.0) {
        FragColor = 1.0;
        return;
    }
    
    // Reconstruct view-space position
    vec3 fragPos = ViewPosFromDepth(TexCoord, depth);
    
    // Get view-space normal
    vec3 normal = normalize(texture(u_NormalTex, TexCoord).xyz);
    
    // Get random rotation vector from noise texture
    vec3 randomVec = normalize(texture(u_NoiseTex, TexCoord * u_NoiseScale).xyz);
    
    // Create TBN matrix (Gram-Schmidt process)
    // This orients the hemisphere to the surface normal
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    
    // Accumulate occlusion
    float occlusion = 0.0;
    int sampleCount = 64;
    
    for (int i = 0; i < sampleCount; ++i) {
        // Get sample position in view space
        // TBN transforms sphere sample to normal-oriented hemisphere
        vec3 samplePos = fragPos + TBN * u_Samples[i].xyz * u_Radius;
        
        // Project sample to screen space
        vec4 offset = u_Projection * vec4(samplePos, 1.0);
        offset.xyz /= offset.w;
        offset.xy = offset.xy * 0.5 + 0.5;  // NDC to [0,1]
        
        // Clamp to screen bounds
        offset.xy = clamp(offset.xy, 0.0, 1.0);
        
        // Sample depth at offset position
        float sampleDepth = texture(u_DepthTex, offset.xy).r;
        vec3 sampleViewPos = ViewPosFromDepth(offset.xy, sampleDepth);
        
        // Range check: only occlude if sample is within radius
        // This prevents far surfaces from occluding close ones
        float rangeCheck = smoothstep(0.0, 1.0, u_Radius / abs(fragPos.z - sampleViewPos.z));
        
        // Occlusion test: if sample is in front of geometry at that point
        occlusion += (sampleViewPos.z >= samplePos.z + u_Bias ? 1.0 : 0.0) * rangeCheck;
    }
    
    // Average and invert (high occlusion = dark)
    occlusion = 1.0 - (occlusion / float(sampleCount));
    
    // Apply power for contrast control
    FragColor = pow(occlusion, u_Power);
}
