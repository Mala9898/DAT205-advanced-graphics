#version 410 core

layout (location = 2) out vec4 FragColor;
layout (location = 1) out vec4 gNormal;
uniform vec3 color;

void main() {
    FragColor = vec4(color, 1.0f);
    gNormal = vec4(1,0,0,1);
}