#pragma once

#include <iostream>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "external/glm/glm.hpp"
#include "external/glm/gtc/matrix_transform.hpp"
#include "external/glm/gtc/type_ptr.hpp"
#include "external/glm/gtx/string_cast.hpp"

#include "shader.h"
using std::string, std::vector;


float plane_datax[] = {
        0.5f,  0.5f, 0.0f,  // top right
        0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f   // top left
};
vector<float> tex_datax = {
        1.0f,  1.0f,  // top right
        1.0f, 0.0f,  // bottom right
        0.0f, 0.0f,  // bottom left
        0.0f,  1.0f   // top left
};
// float tex_data[] = {
//         1.0f,  1.0f,  // top right
//         1.0f, 0.0f,  // bottom right
//         0.0f, 0.0f,  // bottom left
//         0.0f,  1.0f   // top left
// };
// float tex_data[] = {
//         5.0f,  5.0f,  // top right
//         5.0f, 0.0f,  // bottom right
//         0.0f, 0.0f,  // bottom left
//         0.0f,  5.0f   // top left
// };
unsigned int plane_indices[] = {
        0,2,1,
        2,0,3
};
class Plane{
private:
    unsigned int VAO, VBO, EBO, texture, normalTexture, textureBO;
    bool using_texture = false;

    string uniformVar = "myTexture";
    string uniformVarNormal = "normalTexture";
    glm::mat4 &projection;
    float tilingFactor = 1;

    unsigned int loadTexture(char const * path) {
        unsigned int textureID;
        glGenTextures(1, &textureID);
        int width, height, numComponents;
        unsigned char *data = stbi_load(path, &width, &height, &numComponents, 0);
        if(data) {
            glBindTexture(GL_TEXTURE_2D, textureID);
            GLenum format = GL_RGBA;
            glTexImage2D(GL_TEXTURE_2D,0, format, width,height, 0, format,GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            std::cout << "loaded: "<< path << std::endl;

        } else {
            std::cout<< "[error] Failed to load texture: " << path << std::endl;
            stbi_image_free(data);
        }
        return textureID;
    }



public:
    Shader &shader;
    bool using_normal = false;
    void loadNormal(const string& path) {
        normalTexture = loadTexture(path.c_str());
        using_normal = true;
    }
    Plane(Shader & shader, glm::mat4 &projection, string texture_name = "", float tilingFactor=1.0f, bool transparent = false) :
            shader(shader), projection(projection),tilingFactor(tilingFactor)
    {
        shader.use();
        shader.setMat4("projection", &projection);

        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(plane_datax), plane_datax, GL_STATIC_DRAW);
        glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);  // configure our position vertex attribute
        glEnableVertexAttribArray(0);

        if(tilingFactor!=1.0f) {
            std::transform(tex_datax.begin(), tex_datax.end(), tex_datax.begin(),
                           [tilingFactor](float &v) { return v * tilingFactor; });
        }

        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(plane_indices), plane_indices, GL_STATIC_DRAW);

        // texture cords
        glGenBuffers(1, &textureBO);
        glBindBuffer(GL_ARRAY_BUFFER, textureBO);
        // glBufferData(GL_ARRAY_BUFFER, sizeof(tex_data), &tex_data[0], GL_STATIC_DRAW);            // if using arrays[]
        glBufferData(GL_ARRAY_BUFFER, tex_datax.size()*sizeof(float), &tex_datax[0], GL_STATIC_DRAW); // if using vector<>
        glVertexAttribPointer(1,2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);

        // --- texture
        if (texture_name != "") {
            using_texture = true;
            int width, height, nrChannels;

            glGenTextures(1, &texture);        // generate texture (#number of textures, textureID(s))
            glActiveTexture(GL_TEXTURE0);                 // activate the texture unit first before binding texture
            glBindTexture(GL_TEXTURE_2D, texture); // bind texture onto texture unit #0 (GL_TEXTURE0)

            // set texture wrapping
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

            if (transparent) {
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }

            stbi_set_flip_vertically_on_load(true);
            // container2.png
            /*
             * last parameter: desired channels
             * STBI_default = 0, STBI_grey = 1, STBI_grey_alpha = 2, STBI_rgb = 3, STBI_rgb_alpha = 4
             */
            unsigned char *data = stbi_load(texture_name.c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);
            if (data) {
                // load image into texture
                // jpg
                // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                // png
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D); // generate mipmaps
                std::cout << "loaded texture: "+ texture_name << std::endl;
            } else {
                std::cout << "failed to load texture" << std::endl;
            }
            stbi_image_free(data); // free image memory
        }
    }
    void draw(glm::mat4 &view, const glm::mat4 &model) {
        shader.use();
        shader.setMat4("model", &model);
        shader.setMat4("view", &view);
        glBindVertexArray(VAO);
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(shader.ID, uniformVar.c_str()), 0);
        glBindTexture(GL_TEXTURE_2D, texture);
        if (using_normal) {
            shader.use();
            shader.setInt("usingNormal", 1);
            shader.setInt("normalTexture", 1);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, normalTexture);
        } else {
            shader.use();
            shader.setInt("usingNormal", 0);
        }
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
        // glDrawArrays(GL_TRIANGLES, 0, 3);
    }
};