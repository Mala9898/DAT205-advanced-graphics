#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include "external/glm/glm.hpp"
#include "external/glm/gtc/type_ptr.hpp"
enum class ShaderType {
    vertex, fragment,
    tesc, tese
};
// credit: learnopengl.com
class Shader {
public:
    unsigned int ID;
    Shader(const char* vertexPath, const char* fragmentPath){
        std::cout << "--vert:" << std::endl;
        unsigned int vertex = compileShader(vertexPath, ShaderType::vertex);
        std::cout << "--frag:" << std::endl;
        unsigned int fragment = compileShader(fragmentPath, ShaderType::fragment);

        // shader Program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        // delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }
    unsigned int compileShader(const char * file, ShaderType shaderType) {
        std::string code;
        std::ifstream fileStream;
        fileStream.exceptions (std::ifstream::failbit | std::ifstream::badbit); //enable exceptions
        try {
            fileStream.open(file);
            std::stringstream stringStream;
            stringStream << fileStream.rdbuf();
            fileStream.close();
            code = stringStream.str();
            std::cout << code << std::endl;
        } catch (std::ifstream::failure& e) {
            std::cout << "[ERROR SHADER]: failed to read file: " << e.what() << std::endl;
        }
        unsigned int shaderUnitID;
        if (shaderType == ShaderType::vertex) {
            shaderUnitID = glCreateShader(GL_VERTEX_SHADER);
        } else if (shaderType == ShaderType::fragment) {
            shaderUnitID = glCreateShader(GL_FRAGMENT_SHADER);
        }
        const char * ccode = code.c_str();
        glShaderSource(shaderUnitID, 1, &ccode, NULL);
        glCompileShader(shaderUnitID);
        checkCompileErrors(shaderUnitID, "NOT-PROGRAM");

        return shaderUnitID;
    }

    void use(){
        glUseProgram(ID);
    }
    void setBool(const std::string &name, bool value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    void setInt(const std::string &name, int value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    void setFloat(const std::string &name, float value) const {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
    void setVec3(const std::string &name, float x, float y, float z) {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1 ,&(glm::vec3(x,y,z)[0]) );
    }
    void setVec3(const std::string &name, const glm::vec3& vec) {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &(vec)[0]);
    }
    void setMat4(const std::string &name, const glm::mat4* mat) {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1,GL_FALSE, glm::value_ptr(*mat));
    }
    void setVec2(const std::string &name, const glm::vec2 &value) const {
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }

private:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(unsigned int shader, std::string type)
    {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success){
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- -- " << std::endl;
            }
        }
        else{
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success){
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- -- " << std::endl;
            }
        }
    }
};