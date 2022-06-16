#version 330 core

out vec4 FragColor;

in float Height;
in vec2 TexCoords;

uniform sampler2D myTexture;
uniform float tMin;
uniform float tMax;
void main() {
    float amplitude = tMax - tMin;
    float h = Height/(amplitude/2);
    vec3 bottom = vec3(0.156, 0.109, 0.043); //brown
    vec3 top = vec3(0.058, 1, 0.266); // green


//    FragColor = vec4(h, h, h, 1.0);
    FragColor = vec4(mix(bottom, top, h),1);
}