#version 330 core
layout (location = 0) in vec3 aPos;

out float currentFragment;
  
void main()
{
    currentFragment = aPos.y + 0.2;
    gl_Position = vec4(aPos, 1.0);
}    