#include "my_gl_init.h"


GLFWwindow* init_gl(const std::string & window_title, int width, int height) {
    // want OpenGL 4.1 the last version that Apple supports
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow *window = glfwCreateWindow(800, 600,window_title.c_str(), NULL, NULL);
    glfwMakeContextCurrent(window);
    if(!window) {
        std::cout << "[ERROR] failed to create glfw window" << std::endl;
        exit(33);
    }
    glewExperimental = true;
    glewInit();

    glEnable(GL_DEPTH_TEST); // enable depth buffer (so fragments don't overwrite each other)

    return window;
}
ImGuiIO& init_IMGUI(GLFWwindow * window) {
    // ---------- Setup Dear ImGui context ----------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    const char* glsl_version = "#version 410";
    ImGui_ImplOpenGL3_Init(glsl_version);

    return io;
}