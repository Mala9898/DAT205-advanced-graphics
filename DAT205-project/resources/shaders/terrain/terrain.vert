#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out float Height;
out vec2 TexCoords;
out vec3 Position;

uniform mat4 model;
uniform mat4 view;
//uniform mat4 projection;

uniform mat4 MVP;

void main(){
    Height = aPos.y;
    TexCoords = aTexCoords;
    Position = vec3(model*vec4(aPos, 1.0));//(view * model * vec4(aPos, 1.0)).xyz;
//    gl_Position = projection * view * model * vec4(aPos, 1.0);
    gl_Position = MVP * vec4(aPos, 1.0);
}