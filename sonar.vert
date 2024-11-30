#version 330 core

layout(location = 0) in vec2 inPos;
out vec3 chCol;
out vec2 TexCoord;

void main()
{
	TexCoord = inPos * 0.5 + 0.5;
	gl_Position = vec4(inPos, 0.0, 1.0);
}