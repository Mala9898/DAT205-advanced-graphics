#pragma once

#include <string>
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

// --- IMGUI
#include "../external/imgui/imgui.h"
#include "../external/imgui/imgui_impl_glfw.h"
#include "../external/imgui/imgui_impl_opengl3.h"

GLFWwindow* init_gl(const std::string & window_title="Hello world", int width=800, int height=600);
ImGuiIO& init_IMGUI(GLFWwindow * window);