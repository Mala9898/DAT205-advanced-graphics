#version 330 core

out vec4 FragColor;

in float Height;
in vec2 TexCoords;
in vec3 Position;

uniform sampler2D myTexture;
uniform sampler2D normalTexture;
uniform sampler2D textureRockGround;
uniform float tMin;
uniform float tMax;
uniform vec3 viewPos;
uniform mat4 model;
uniform mat4 view;

void main() {
    float amplitude = tMax - tMin;
    float h = Height/(amplitude/2);
//    vec3 bottom = vec3(0.156, 0.109, 0.043); //brown
    vec3 bottom = vec3(0.0, 0.0, 0.0); //brown
    vec3 top = vec3(0.058, 1, 0.266); // green
//    vec3 rock = texture(myTexture, TexCoords*50).xyz; // tiling
    vec3 rock = texture(myTexture, TexCoords*20).xyz; // tiling
    vec3 rocGround = texture(textureRockGround, TexCoords*20).xyz;


    vec3 n = texture(myTexture, TexCoords).xyz;
//    FragColor = vec4(h, h, h, 1.0);
    FragColor = vec4(mix(rock, rocGround, h),1);

//    vec3 ambient = vec3(0.2,0.2,0.2) *rock;
//
//    vec3 lightDir = normalize(vec3(30, 100,0)-Position);
//    vec3 diffuse = vec3(1,1,1)* max(dot(lightDir, n),0) * rock;
//
//    vec3 viewDir = normalize(viewPos-Position);
//    vec3 reflectDir = reflect(-lightDir, n); // reflect about normal
//    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32); // last value is "shininess of highlight"
//    //  vec3 specular = vec3(texture(material.specular,TexCoords)); // emissive
//    vec3 specular = min(spec,1)*vec3(0.1,0.1,0.1);//vec3(texture(material.specular, TexCoords)); //*material.specular;

//    FragColor = vec4(diffuse+ambient+specular, 1);
}