#version 410 core

layout (location=0) in vec3 aPos;
layout (location=1) in vec3 aNormal;
layout (location=2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main () {
    FragPos = vec3(view*model*vec4(aPos, 1.0));
    // this transpose-inverse compensates for non-uniform scale transformations
    Normal = normalize(transpose(inverse(mat3(view * model))) * aNormal);

    TexCoords = aTexCoords;

    gl_Position = projection*view*model * vec4(aPos, 1.0);
}