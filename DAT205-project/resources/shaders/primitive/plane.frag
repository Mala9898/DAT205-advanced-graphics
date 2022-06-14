#version 410 core
in vec2 TexCoords;
// write to FrameBuffer targets
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
uniform sampler2D myTexture;

in vec3 FragPos;

void main() {
    gPosition = FragPos; // frag position in world coords
    gNormal = vec3(0,0,1);
    gAlbedoSpec = vec4(1,0,0,1);

}