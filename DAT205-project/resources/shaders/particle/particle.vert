#version 410 core

layout (location=0) in vec3 velocity;
layout (location=1) in float startTime;

const vec2 texCoords[] = vec2[](vec2(0,0), vec2(1,0), vec2(1,1), vec2(0,0), vec2(1,1), vec2(0,1));
const vec3 offsets[] = vec3[](vec3(-0.5,-0.5,0), vec3(0.5,-0.5,0), vec3(0.5,0.5,0),
    vec3(-0.5,-0.5,0), vec3(0.5,0.5,0), vec3(-0.5,0.5,0) );

// --- particle system properties
uniform bool continuous;
uniform vec3 size;
uniform float lifetime;
uniform vec3 position;
uniform float time;
uniform vec3 gravity;
uniform bool growSize;

uniform mat4 MV;
uniform mat4 projection;

out float transparency;
out vec2 texCoord;

void main () {
    float t = 0;
    if (continuous) {
        t = mod (time - startTime, startTime);
    } else {
        t = time - startTime;
    }
    vec3 camPos = vec3(0);
    if (t > 0 && t < lifetime) {
        vec3 p = position + velocity*t + gravity*t*t;
        if (growSize) {
            camPos = (MV*vec4(p,1)).xyz + (offsets[gl_VertexID] * mix(size.y, size.z, t/lifetime));
        } else {
            camPos = (MV*vec4(p,1)).xyz + (offsets[gl_VertexID] * size.x);
        }
        transparency = mix(1, 0, t/lifetime); // linear fadeout. TODO: nonlinear
    } else {
        transparency = 0;
    }
    texCoord = texCoords[gl_VertexID];
    gl_Position = projection*vec4(camPos, 1);
}