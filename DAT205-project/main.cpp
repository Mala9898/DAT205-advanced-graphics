
#include <iostream>
#include <vector>
#include <random>

// ----- image loading -----------
#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"
// -------------------------------

#include "includes/external/glm/glm.hpp"
#include "includes/external/glm/gtc/matrix_transform.hpp"
#include "includes/external/glm/gtc/type_ptr.hpp"

#include "includes/utils/my_gl_init.h"  // opengl context
#include "includes/utils/misc.h"
#include "includes/utils/free_camera.h"
#include "includes/utils/xyz_gizmo.h"
#include "includes/systems/ParticleSystem.h"

void mouse_callback(GLFWwindow* window, double x, double y);
void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void toggle_mouse ();
void toggle_wireframe();

using std::vector, std::string;
using glm::vec3,glm::vec4, glm::mat4, glm::scale, glm::translate;

FreeCamera camera = nullptr;
bool hideMouse = false;
bool wireframe = false;
// --- IMGUI
bool show_demo_window = true;
bool show_another_window = false;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
double startTime = 0;
bool startCounting = false;

GLFWwindow *window = nullptr;

/**
     * Return a rotation matrix that rotates the y-axis to be parallel to dir.
     *
     * @param dir
     * @return
     */
static glm::mat3 makeArbitraryBasis( const glm::vec3 & dir ) {
    glm::mat3 basis;
    glm::vec3 u, v, n;
    v = dir;
    n = glm::cross(glm::vec3(1,0,0), v);
    if( glm::length(n) < 0.00001f ) {
        n = glm::cross(glm::vec3(0,1,0), v);
    }
    u = glm::cross(v,n);
    basis[0] = glm::normalize(u);
    basis[1] = glm::normalize(v);
    basis[2] = glm::normalize(n);
    return basis;
}

int main() {

    window = init_gl();
    print("debug mode activated");

    // --- OPENGL init
    glm::mat4 projection = glm::perspective(glm::radians(60.0f), 800.0f/600.0f, 0.1f, 100.0f);
    Gizmo gizmo = Gizmo();
    camera = FreeCamera(window);
    glfwSetCursorPosCallback(window, mouse_callback);                // get callback for mouse change
    glfwSetKeyCallback(window, keyboard_callback);
    ImGuiIO& io = init_IMGUI(window);
    float offset = 0.0f;

    // enable transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    ParticleSystem ps (500, false);
    {
        ps.gravity = vec3(0.0f,0.1f,0.0f);
        ps.lifetime = 3.0f;
        ps.setVelocity(0.1f, 1.0f);
        // ps.setPhi(0,3.14f);
        ps.setTheta(0.0f, 3.14f/4.0f);
        ps.growSize = true;
        ps.continuous = true;
        ps.generate("textures/fire3.png");
    }


    while(!glfwWindowShouldClose(window)){
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glfwPollEvents();
        camera.setStatus(!io.WantCaptureMouse);

        // --- IMGUI
        {
            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
        // if (show_demo_window)
        //     ImGui::ShowDemoWindow(&show_demo_window);

            ImGui::Begin("MY WINDOW!");                          // Create a window called "Hello, world!" and append into it.
            string tt = "time: "+std::to_string(glfwGetTime()-startTime);
            ImGui::Text(tt.c_str());
            if (ImGui::Button("Hide mouse"))
                toggle_mouse();
            if (ImGui::Button("Wireframe"))
                toggle_wireframe();
            ImGui::SliderFloat("offset",&offset, -1.0f,1.0f);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }
        // --- camera
        GLdouble xPos, yPos;
        glfwGetCursorPos(window, &xPos, &yPos);
        camera.handleMouse(xPos, yPos);
        camera.handleKeyboard();

        // ------------------ <scene> ------------------
        glm::mat4 view = glm::lookAt(camera.camPos, camera.camPos + camera.camFront, camera.camY);
        glm::mat4 model = translate(mat4(1), vec3(0.0f,offset,0.0f));
        mat4 MVP = projection*view*model;
        gizmo.draw(MVP);

        mat4 MV = view*model;
        ps.draw(MV, projection);

        // ------------------ </scene> ------------------
        glBindVertexArray(0);


        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // -- swab buffers
        glfwSwapBuffers(window);
    }
    return 0;
}

void mouse_callback(GLFWwindow* window, double x, double y) {
    camera.handleMouse(x,y);
}
void toggle_mouse () {
    camera.first = true;
    hideMouse = !hideMouse;
    if (hideMouse)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // hide mouse
    else
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // hide mouse
}
void toggle_wireframe() {
    wireframe = !wireframe;
    if(wireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        toggle_mouse();
    }
}