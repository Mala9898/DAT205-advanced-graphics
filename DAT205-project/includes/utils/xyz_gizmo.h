#pragma once


//
// Created by Stanislaw Malec on 2022-05-12.
//

#ifndef SHARD_GIZMO_H
#define SHARD_GIZMO_H

// #include "includes/glm/glm.hpp"
#include "external/glm/glm.hpp"
#include "external/glm/gtc/type_ptr.hpp"
#include "shader.h"

class Gizmo {
private:
    unsigned int shaderID;
    unsigned int gizVAO1,gizVAO2,gizVAO3;
    unsigned int gizVBO1,gizVBO2,gizVBO3;
    glm::vec3 start {0.0f,0.0f,0.0f};
    // glm::vec3 end;
    glm::mat4 MVP;

    glm::vec3 red {1.0f,0.0f,0.0f};
    glm::vec3 green {0.0f,1.0f,0.0f};
    glm::vec3 blue {0.0f,0.0f,1.0f};

public:
    Gizmo() {
        Shader gizShader = Shader("shaders/line_vertex.shader", "shaders/forward/gizmo.frag");
        shaderID=gizShader.ID;

        glUseProgram(shaderID);

        // X
        glGenVertexArrays(1, &gizVAO1);
        glGenBuffers(1, &gizVBO1);
        glBindVertexArray(gizVAO1);
        glBindBuffer(GL_ARRAY_BUFFER, gizVBO1);
        float gizX[] = {start.x, start.y,start.z, red.x,red.y,red.z};
        glBufferData(GL_ARRAY_BUFFER, sizeof(gizX), gizX, GL_STATIC_DRAW);
        glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Y
        glGenVertexArrays(1, &gizVAO2);
        glGenBuffers(1, &gizVBO2);
        glBindVertexArray(gizVAO2);
        glBindBuffer(GL_ARRAY_BUFFER, gizVBO2);
        float gizY[] = {start.x, start.y,start.z, green.x,green.y,green.z};
        glBufferData(GL_ARRAY_BUFFER, sizeof(gizY), gizY, GL_STATIC_DRAW);
        glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Z
        glGenVertexArrays(1, &gizVAO3);
        glGenBuffers(1, &gizVBO3);
        glBindVertexArray(gizVAO3);
        glBindBuffer(GL_ARRAY_BUFFER, gizVBO3);
        float gizZ[] = {start.x, start.y,start.z, blue.x,blue.y,blue.z};
        glBufferData(GL_ARRAY_BUFFER, sizeof(gizZ), gizZ, GL_STATIC_DRAW);
        glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // unbind
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    void draw(const glm::mat4& MVP){

        glUseProgram(shaderID);

        // draw line for each axis
        unsigned int loc = glGetUniformLocation(shaderID, "MVP");
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(MVP));
        unsigned int loc2 = glGetUniformLocation(shaderID, "color");
        glUniform3fv(loc2, 1, glm::value_ptr(red));
        glBindVertexArray(gizVAO1);
        glDrawArrays(GL_LINES, 0, 2);

        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(MVP));
        glUniform3fv(loc2, 1, glm::value_ptr(green));
        glBindVertexArray(gizVAO2);
        glDrawArrays(GL_LINES, 0, 2);

        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(MVP));
        glUniform3fv(loc2, 1, glm::value_ptr(blue));
        glBindVertexArray(gizVAO3);
        glDrawArrays(GL_LINES, 0, 2);

    }
    ~Gizmo(){

    }
};

#endif //SHARD_GIZMO_H
