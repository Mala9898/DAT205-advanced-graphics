#version 410 core
//out vec4 FragColor;

// write to FrameBuffer targets
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;

void main(){
    gPosition = FragPos; // frag position in world coords
    gNormal = normalize(Normal);
    //    gAlbedoSpec.rgb = vec3(0.2, 0.2,0.2);//texture(texture_diffuse1, TexCoords).rgb;
    gAlbedoSpec.rgb = vec3(0.9, 0,0); //texture(texture_diffuse1, TexCoords).rgb;
    gAlbedoSpec.a = 0; //texture(texture_specular1, TexCoords).r;

    /*
    // ambient
    float ambient_strength = 0.2;
    float specular_strength = 0.7;
    vec3 ambient = vec3(0.2,0.2,0.2) * vec3(texture(texture_diffuse1, TexCoords));// *material.ambient;

    // diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(vec3(2.0, 2.0,3.0)-FragPos);
    vec3 diffuse = vec3(1,1,1)* max(dot(lightDir, norm),0) * vec3(texture(texture_diffuse1, TexCoords));//*material.diffuse;

    // specular
    //    vec3 viewDir = normalize(viewPos-FragPos);
    //    vec3 reflectDir = reflect(-lightDir, norm); // reflect about normal
    //    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess); // last value is "shininess of highlight"
    //    //  vec3 specular = vec3(texture(material.specular,TexCoords)); // emissive
    //    vec3 specular = light.specular*spec*vec3(texture(material.specular, TexCoords)); //*material.specular;


    //    FragColor = vec4((ambient+diffuse+specular)*objectColor, 1.0); // phong
    FragColor = vec4(ambient+diffuse, 1.0); // point light
    //    FragColor = texture(texture_diffuse1, TexCoords); // just texture
    */
}