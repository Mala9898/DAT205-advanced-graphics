#pragma once

#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "external/glm/glm.hpp"
#include "external/glm/gtc/matrix_transform.hpp"
#include "external/glm/gtc/type_ptr.hpp"
#include "external/glm/gtx/string_cast.hpp"

#include "utils/shader.h"
#include "utils/cube_with_tex.h"

using std::string;

class MyCube {
private:
    bool using_texture = false;
    string uniformVar = "myTexture";
public:
    unsigned int VBO, VAO;
    unsigned int texture;
    MyCube(string texture_name = "") {

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube_verts_tex::vertices), cube_verts_tex::vertices, GL_STATIC_DRAW);
        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // normal attribute
        // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
        // glEnableVertexAttribArray(1);
        // texture coords
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6*sizeof(float)));
        glEnableVertexAttribArray(1);

        // --- texture
        if (texture_name != "") {
            using_texture = true;
            int width, height, nrChannels;

            glGenTextures(1, &texture);        // generate texture (#number of textures, textureID(s))
            glActiveTexture(GL_TEXTURE0);                 // activate the texture unit first before binding texture
            glBindTexture(GL_TEXTURE_2D, texture); // bind texture onto texture unit #0 (GL_TEXTURE0)
            // glActiveTexture(GL_TEXTURE1);              // set additional textures, 0,...,15
            // glBindTexture(GL_TEXTURE_2D, texture2);
            // set texture wrapping
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D); // generate mipmaps
                std::cout << "loaded texture: "+ texture_name << std::endl;
            } else {
                std::cout << "failed to load texture" << std::endl;
            }
            stbi_image_free(data); // free image memory
        }
    }
    void Draw(Shader& shader) {
        glBindVertexArray(VAO);
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(shader.ID, uniformVar.c_str()), 0); // tell sampler to use texture unit 0
        glBindTexture(GL_TEXTURE_2D,texture);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // --- cleanup
        glBindVertexArray(0); // unbind VAO
        glActiveTexture(0);
    }
};
