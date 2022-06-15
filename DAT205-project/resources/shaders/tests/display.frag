#version 410 core

in vec2 TexCoords;
layout (location = 0) out vec4 FragColor;

uniform sampler2D myTexture;

void main () {
    FragColor = texture(myTexture, TexCoords);
}