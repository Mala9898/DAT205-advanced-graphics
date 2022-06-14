#version 410 core

in vec2 TexCoords;
layout (location = 0) out vec4 FragColor;

uniform sampler2D myTexture;

void main () {
    FragColor = vec4(vec3(texture(myTexture, TexCoords)), 1);
}
