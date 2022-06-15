#version 410 core

in vec3 fragPos;
in vec3 fragNormal;
in vec3 fragLightDir;
in vec2 TexCoords;

out vec4 FragColor;


void main () {
    vec3 albedo = vec3(0,0,0.5); //todo
    vec3 fragLight = normalize(fragLightDir);
    vec3 fragNormal = normalize(fragNormal);

    vec3 ambient = albedo * 0.2;
    vec3 diffuse = albedo*max(dot(fragLight, fragNormal), 0);

    float c = dot(-fragNormal, normalize(reflect(-fragLight, fragNormal)) );
    float c2 = max(c,0.0);
    float spec = pow(c2, 32);
    vec3 specular = vec3(spec,spec,spec) * 0.7;

    FragColor = vec4(ambient+diffuse+specular, 0.8);

//    // ambient
//    float ambient_strength = 0.2;
//    float specular_strength = 0.7;
//    vec3 ambient = vec3(0,0,0.2);//vec3(0.2,0.2,0.2) * vec3(texture(texture_diffuse1, TexCoords));// *material.ambient;
//
//    // diffuse
//    vec3 norm = vec3(0,1,0); //normalize(Normal);
//    vec3 lightDir = normalize(vec3(2.0, 2.0,3.0)-FragPos);
//    vec3 diffuse = vec3(1,1,1)* max(dot(lightDir, norm),0) * vec3(0,0,0.5);//vec3(texture(texture_diffuse1, TexCoords));//*material.diffuse;
//
//    // specular
//    //    vec3 viewDir = normalize(viewPos-FragPos);
//    //    vec3 reflectDir = reflect(-lightDir, norm); // reflect about normal
//    //    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess); // last value is "shininess of highlight"
//    //    //  vec3 specular = vec3(texture(material.specular,TexCoords)); // emissive
//    //    vec3 specular = light.specular*spec*vec3(texture(material.specular, TexCoords)); //*material.specular;
//
//
//    //    FragColor = vec4((ambient+diffuse+specular)*objectColor, 1.0); // phong
//    FragColor = vec4(ambient+diffuse, 1.0); // point light
//    //    FragColor = texture(texture_diffuse1, TexCoords); // just texture
}