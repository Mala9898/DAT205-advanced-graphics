#version 410 core

in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D myTexture;

void main() {
    FragColor = vec4(1);
}