#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform int isSonarTurnedOn;
uniform vec3 greenColor;
uniform sampler2D sonarTexture;
uniform float sweepAngle; 
uniform vec2 objects[100];
uniform int objectCount;

void main()
{
    float dist = distance(TexCoord, vec2(0.5, 0.5));
    vec2 center = vec2(0.5, 0.5);
    vec2 dir = TexCoord - center;

    // Discard fragments outside the circle
    if (dist > 0.5) discard;

    // Default to sonar texture for the entire circle
    vec4 sonarColor = texture(sonarTexture, TexCoord);
    FragColor = sonarColor * vec4(greenColor, 0.5);

    if (isSonarTurnedOn) {
        // Normalize the direction vector
        dir = normalize(dir);

        // Compute the angle of the current fragment
        float angle = atan(dir.y, dir.x);
        if (angle < 0.0) angle += 6.28318530718;

        // Compute the difference between the fragment angle and the sweep angle
        float angleDiff = angle - sweepAngle;
        if (angleDiff < -3.14159265359) angleDiff += 6.28318530718;
        if (angleDiff > 3.14159265359) angleDiff -= 6.28318530718;

        // Draw trail only for fragments behind the pointer
        if (angleDiff <= 0.0) {

            float trailIntensity = smoothstep(-2.5, 0.5, angleDiff);
            vec4 trailColor = vec4(trailIntensity, 0.0, 0.0, trailIntensity);
            FragColor += trailColor;
        }

        // Draw the pointer as a solid red line
        float pointerThreshold = 0.01; // Adjust for pointer thickness
        if (abs(angleDiff) < pointerThreshold) {
            FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        }

        // Draw red dots for detected objects
        for (int i = 0; i < objectCount; i++) {
            float redDotDist = distance(TexCoord - center, objects[i].xy);
            if (redDotDist <= 0.01) {
                FragColor = mix(FragColor, vec4(1.0, 0.0, 0.0, 1.0), 1.0);
            }
        }
    }else 
        FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);

}
