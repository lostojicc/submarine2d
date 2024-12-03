#version 330 core
out vec4 FragColor;

in vec2 TexCoord; 

uniform sampler2D numberTexture;
uniform int positionIndicator, h, t, o;

void main()
{
    vec4 black = vec4(0.0, 0.0, 0.0, 1.0);

    switch (positionIndicator) {
        case 0:
            if (h)
                FragColor = mix(black, texture(numberTexture, TexCoord), 1.0);
            else
                FragColor = black;
            break;
        case 1:
            if (t == 0 && h == 0)
                FragColor = black;
            else
                FragColor = mix(black, texture(numberTexture, TexCoord), 1.0);
            break;
        case 2:
            FragColor = mix(black, texture(numberTexture, TexCoord), 1.0);
            break;
    }

    
}   