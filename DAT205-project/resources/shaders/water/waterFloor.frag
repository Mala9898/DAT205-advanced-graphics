#version 410 core

in vec2 TexCoords;
out vec4 FragColor;

vec3 checkerboard(vec2 tc) {
    float tileScale = 16.0;
    float tile = mod(floor(tc.x * tileScale) + floor(tc.y * tileScale), 2.0);
    return tile * vec3(1,1,1);
}

void main () {
    FragColor = vec4(checkerboard(TexCoords), 1);
}