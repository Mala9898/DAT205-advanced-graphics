#version 410 core

layout (vertices=4) out;

uniform int NumSegments;
uniform int NumStrips;

void main() {
    // unmodified vert position passthrough
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    // tess levels
    gl_TessLevelOuter[0] = float(NumStrips);
    gl_TessLevelOuter[1] = float(NumSegments);
}