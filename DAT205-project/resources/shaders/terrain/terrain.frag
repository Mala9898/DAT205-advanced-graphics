#version 330 core

out vec4 FragColor;

in float Height;
in vec2 TexCoords;
in vec3 Position;

uniform sampler2D myTexture;
uniform sampler2D normalTexture;
uniform sampler2D textureRockGround;
uniform sampler2D textureRockGrass;
uniform float tMin;
uniform float tMax;
uniform vec3 viewPos;
uniform mat4 model;
uniform mat4 view;
uniform float blendFactor = 1;
uniform float blendFactor2 = 0;

//credit: https://github.com/glslify/glsl-easings
float elasticInOut(float t) {
    return t < 0.5
    ? 0.5 * sin(+13.0 * 1.5707 * 2.0 * t) * pow(2.0, 10.0 * (2.0 * t - 1.0))
    : 0.5 * sin(-13.0 * 1.5707 * ((2.0 * t - 1.0) + 1.0)) * pow(2.0, -10.0 * (2.0 * t - 1.0)) + 1.0;
}

void main() {
    float amplitude = abs(tMax) + abs(tMin);
    float h = (Height*3)/(amplitude);
//    vec3 bottom = vec3(0.156, 0.109, 0.043); //brown
    vec3 bottom = vec3(0.0, 0.0, 0.0); //brown
    vec3 top = vec3(0.058, 1, 0.266); // green
//    vec3 rock = texture(myTexture, TexCoords*50).xyz; // tiling
    int tilingFactor = 100;

    vec3 rock = texture(textureRockGround, TexCoords*tilingFactor).xyz; // tiling
    vec3 rocGround = texture(textureRockGrass, TexCoords*tilingFactor).xyz;
    vec3 rocClay = texture(myTexture, TexCoords*tilingFactor).xyz;


    vec3 n = texture(myTexture, TexCoords).xyz; // TODO works
//    vec3 n  = normalize(view* vec4(texture(myTexture, TexCoords).xyz, 1)).xyz;
//    FragColor = vec4(h, h, h, 1.0);
//    vec3 albedo = mix(rock, rocGround, h);
    vec3 temp = mix(rock, rocGround, elasticInOut(h));
    vec3 albedo = mix (temp, rocClay, elasticInOut((Height*blendFactor - blendFactor2)/amplitude));


    vec3 ambient = vec3(0.2,0.2,0.2) *albedo;

    vec3 lightDir = normalize(vec3(30, 100,0)-Position);
    vec3 diffuse = vec3(1,1,1)* max(dot(lightDir, n),0) * albedo;

//    vec3 viewDir = normalize(viewPos-Position);
//    vec3 reflectDir = reflect(-lightDir, n); // reflect about normal
//    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32); // last value is "shininess of highlight"
//    //  vec3 specular = vec3(texture(material.specular,TexCoords)); // emissive
//    vec3 specular = min(spec,1)*vec3(0.1,0.1,0.1);//vec3(texture(material.specular, TexCoords)); //*material.specular;

//    FragColor = vec4(diffuse+ambient+specular, 1);
    vec3 slope = (n.y > 0.95) ? vec3(1,0,0) : vec3(0,0,0);
//    FragColor = vec4(ambient+diffuse, 1); // WORKS
    FragColor = vec4(ambient+diffuse + slope, 1);
//    if (Height>10)
//        FragColor = vec4(1,0,0, 1);
}