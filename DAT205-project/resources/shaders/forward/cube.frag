#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D myTexture;

void main() {
    FragColor = vec4(vec3(texture(myTexture, TexCoords)), 1);
//        FragColor = vec4(1, 0,0,1);
}