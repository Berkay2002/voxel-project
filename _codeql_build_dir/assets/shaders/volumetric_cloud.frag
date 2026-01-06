#version 460 core

in float v_Light;
in float v_Fog;
in vec3 v_Normal;

out vec4 FragColor;

uniform vec3 u_SkyColor;
uniform vec3 u_SunDirection;
uniform float u_Brightness;  // Day/night dimming (0.3 night, 1.0 day)

void main() {
    // Base cloud color (bright white)
    vec3 cloudColor = vec3(1.0);
    
    // Apply two-tone pre-baked lighting from vertex
    cloudColor *= v_Light;
    
    // Optional: subtle directional lighting enhancement
    float sunFactor = max(dot(v_Normal, u_SunDirection), 0.0) * 0.05;
    cloudColor += vec3(sunFactor);
    
    // Apply day/night brightness
    cloudColor *= u_Brightness;
    
    // Blend toward sky color at distance (fog)
    cloudColor = mix(cloudColor, u_SkyColor, v_Fog);
    
    // Discard fully fogged fragments for performance
    if (v_Fog > 0.99) discard;
    
    FragColor = vec4(cloudColor, 1.0);
}
