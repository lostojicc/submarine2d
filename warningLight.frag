#version 330 core
in vec3 fragPos;

out vec4 FragColor;

uniform int isLightOn;    // Light on/off state

void main() {
    // Calculate the distance of the current fragment from the light center
    float dist = distance(fragPos, vec3(0.125, -0.625, 0.0));

    // Check if we are inside the circle
    if (dist < 0.06125) {
        // The inner circle (red or black depending on state)
        FragColor = isLightOn == 0 ? vec4(0.0, 0.0, 0.0, 1.0) : vec4(1.0, 0.0, 0.0, 1.0);
    } else if (isLightOn) {
        // Calculate fading glow (smoothstep for better transition)
        float glow = smoothstep(0.1, 0.5, dist);
        FragColor = vec4(1 - glow, 0.0, 0.0, (1 - glow) * 0.5);
    } else {
        // If light is off, no glow
        FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    }
    
}
