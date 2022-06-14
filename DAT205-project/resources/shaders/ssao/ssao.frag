#version 410 core

out float FragColor;
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform int kernelSize = 64;
uniform mat4 projection;
uniform vec3 samples[64];
uniform sampler2D noise; // random directions

float radius = 0.5;

void main () {
    vec3 Position = texture(gPosition, TexCoords).xyz;
    vec3 Normal = normalize(texture(gNormal, TexCoords).rgb);

//    vec3 tangent = normalize(vec3(1,0,0) - Normal *dot(vec3(1,0,0), Normal));
    vec3 r = normalize(texture(noise, TexCoords*vec2(800.0/4.0, 600.0/4.0)).xyz);
    vec3 tangent = normalize(r - Normal *dot(r, Normal));
    vec3 biTangent = cross(Normal, tangent);

    mat3 camSpaceMatrix = mat3(tangent, biTangent, Normal);

    float occlusion = 0;

    for(int i = 0; i < kernelSize; i++) {
        // from fragment position, add random vector and a controllable radius
        vec3 samplePos = Position + radius *(camSpaceMatrix * samples[i]);

        // project sample to get its position
        vec4 p = projection*vec4(samplePos, 1); // clip space
        p*= 1.0/p.w;
        p.xyz = p.xyz*0.5 +0.5; // into range [0,1]

        // get depth of sample
        float depth = texture(gPosition, p.xy).z;

        // comparison
        float diff = depth - Position.z;
        if (diff >= 0.0 && diff <= radius && depth > Position.z)
            occlusion += 1.0;
    }
    float ao = 1 - (occlusion/kernelSize);
    FragColor = ao;

}