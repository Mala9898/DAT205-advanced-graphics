#pragma once

#include <GLFW/glfw3.h>
#include <algorithm>
#include "../external/glm/glm.hpp"

class FreeCamera {
private:
    GLFWwindow * window;
    bool enabled = true;

    float prevX = 400, prevY = 300; // TODO

public:
    float yaw = -90.0f;
    float pitch = 0.0f;
    bool first = true;
    glm::vec3 camY = glm::vec3(0.0f,1.0f,0.0f);
    glm::vec3 camFront = glm::vec3(0.0f,0.0f,-1.0f);
    glm::vec3 camPos = glm::vec3(0.0f,2.0f,3.0f);
    float sens = 0.1f;
    float speed = 0.05f;
    float multiplier = 4.0f;

    FreeCamera(GLFWwindow *window) : window(window) {}
    FreeCamera(GLFWwindow *window, glm::vec3 pos) : camPos(pos), window(window) {}
    ~FreeCamera(){}
    void setStatus(bool b) {
        if (b) {
            enabled = true;
        } else {
            if (enabled) {
                first = true;
                enabled = false;
            } else {
                enabled = false;
            }
        }

    }
    void handleMouse(double x, double y) {
        if (!enabled)
            return;
        if (first) {
            prevX  = x;
            prevY = y;
            first = false;
            return;
        }
        float deltax = (x-prevX) * sens;
        float deltay = (prevY-y) * sens;
        prevX = x;
        prevY = y;
        yaw += deltax;
        pitch = std::clamp(pitch+deltay, -89.0f, 89.0f);
        glm::vec3 dir(cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
                            sin(glm::radians(pitch)),
                            sin(glm::radians(yaw)) * cos(glm::radians(pitch)));
        camFront = glm::normalize(dir);
    }
    void handleKeyboard() {
        float s = speed;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            s = speed*multiplier;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camPos += s * camFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camPos -= s * camFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camPos -= glm::normalize(glm::cross(camFront, camY)) * s;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camPos += glm::normalize(glm::cross(camFront, camY)) * s;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            camPos += camY * s;
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            camPos += -camY * s;
    }


};