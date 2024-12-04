#version 330 core
out vec4 FragColor;

in vec2 TexCoord; 
uniform sampler2D texturre;
uniform int showText;

void main()
{
    vec4 black = vec4(0.0, 0.0, 0.0, 1.0);

    if (showText)
        FragColor = mix(black, texture(texturre, TexCoord), 1.0);    
    else
        FragColor = black;
}   