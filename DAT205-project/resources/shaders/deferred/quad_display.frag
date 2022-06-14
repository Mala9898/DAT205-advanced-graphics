#version 410 core

in vec2 TexCoords;
layout (location = 0) out vec4 FragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D gAO;

uniform vec3 viewPos;
uniform vec3 lightPos;

void main () {
//    FragColor = vec4(1.0, 0.0,0.0, 1.0);
    // --- lighting pass
    // - get data from geometry buffers
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;
    float ssao = texture(gAO, TexCoords).r;

    // - calculate lighting
    vec3 ambient = Albedo * 0.2;

    vec3 viewDir = normalize(viewPos-FragPos);

    vec3 lightDir = normalize(lightPos-FragPos);
    vec3 diffuse = max (dot(Normal, lightDir), 0.0) * Albedo;

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(Normal,halfwayDir), 0), 16);
    vec3 specular = vec3(0);//vec3(Specular * spec);
//    FragColor = vec4(Albedo, 1.0);
    FragColor = vec4(ambient+diffuse + specular, 1.0);
//    if (enableSSAO)
//        FragColor = vec4((ambient+diffuse)*ssao + specular, 1.0);
//    else
//        FragColor = vec4(ambient+diffuse + specular, 1.0);


}
