#version 330 core
in float currentFragment;

out vec4 FragColor;

uniform float oLevel;

void main() {
	if (currentFragment <= oLevel)
		FragColor = vec4(0.0, 0.0, 1.0, 1.0); // Blue color for the progress bar
	else
		FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}