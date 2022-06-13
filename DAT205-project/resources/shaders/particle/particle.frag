#version 410 core

uniform sampler2D tex;
in float transparency;
in vec2 texCoord;
layout ( location = 0 ) out vec4 FragColor;

void main() {
    FragColor = texture(tex, texCoord);
    FragColor.a *= transparency;
}