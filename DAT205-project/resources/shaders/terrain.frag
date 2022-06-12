#version 410 core

in float height;

out vec4 FragColor;

void main () {
    float h = (height + 16)/32.0f;
    FragColor = vec4(0,h, 0, 1);
}