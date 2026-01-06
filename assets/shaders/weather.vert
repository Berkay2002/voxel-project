#version 460 core

layout(location = 0) in vec3 aPos;       // Particle world position
layout(location = 1) in vec2 aTexCoord;  // UV coordinates
layout(location = 2) in float aOffset;   // Random vertical offset for staggered fall

out vec2 v_TexCoord;
out float v_Alpha;

uniform mat4 u_ViewProj;
uniform vec3 u_CameraPos;
uniform vec3 u_CameraRight;  // For billboarding
uniform float u_Time;
uniform float u_FallSpeed;
uniform float u_ParticleHeight;  // Height of the particle (for rain streaks)
uniform float u_ParticleWidth;   // Width of the particle
uniform float u_TerrainHeight;   // Terrain height at camera position (baseline)

void main() {
    vec3 pos = aPos;
    
    // Animate fall: particles fall from spawn height and loop
    // Start particles from terrain height + buffer zone (spawn above terrain)
    float spawnHeight = max(u_TerrainHeight + 64.0, 128.0);  // At least 64 blocks above terrain
    float fallCycle = 128.0;  // Total fall distance
    float fallDistance = mod(u_Time * u_FallSpeed + aOffset, fallCycle);
    pos.y = spawnHeight - fallDistance;
    
    // Center particles around camera
    pos.x += u_CameraPos.x;
    pos.z += u_CameraPos.z;
    
    // Fade out particles as they approach terrain (prevent popping through ground)
    float distanceAboveTerrain = pos.y - u_TerrainHeight;
    float fadeStart = 3.0;  // Start fading 3 blocks above terrain
    float fadeEnd = 0.5;    // Fully transparent 0.5 blocks above terrain
    float terrainFade = smoothstep(fadeEnd, fadeStart, distanceAboveTerrain);
    
    // Discard particles below terrain (underground/in caves)
    if (distanceAboveTerrain < 0.0) {
        // Move particle far away to effectively cull it
        pos = vec3(99999.0);
    }
    
    // Create vertical billboard (only rotate around Y axis)
    // Rain/snow should stay vertical, not face camera directly
    vec3 rightOffset = u_CameraRight * aTexCoord.x * u_ParticleWidth;
    vec3 upOffset = vec3(0.0, aTexCoord.y * u_ParticleHeight, 0.0);
    
    vec3 worldPos = pos + rightOffset + upOffset;
    
    gl_Position = u_ViewProj * vec4(worldPos, 1.0);
    v_TexCoord = aTexCoord;
    
    // Fade particles at distance (avoid sharp cutoff at spawn cylinder edge)
    float dist = distance(pos.xz, u_CameraPos.xz);
    float maxDist = 32.0;
    float distanceFade = 1.0 - smoothstep(maxDist * 0.7, maxDist, dist);
    
    // Combine terrain fade and distance fade
    v_Alpha = distanceFade * terrainFade;
}
