#version 410 core

layout (location=0) in vec3 aPos;
layout (location=1) in vec2 aTexCoords;
layout (location=2) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 lightPos;

out vec3 fragPos;
out vec3 fragNormal;
out vec3 fragLightDir;
out vec2 TexCoords;
void main() {
    // convert to view space
    fragPos = vec3(view*model*vec4(aPos, 1.0));
    fragNormal =normalize(transpose(inverse(mat3(view * model))) * aNormal);
    fragLightDir = lightPos - fragPos;

    gl_Position = projection*view*model*vec4(aPos, 1.0);

    TexCoords = aTexCoords;
}