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
#include "includes/utils/Model.h"
#include "utils/Plane.h"
#include "utils/Cube.h"


void mouse_callback(GLFWwindow* window, double x, double y);
void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void toggle_mouse ();
void toggle_wireframe();

using std::vector, std::string;
using glm::vec3,glm::vec4, glm::mat4, glm::scale, glm::translate;

int screenWidth = 800;
int screenHeight = 600;
FreeCamera camera = nullptr;
bool hideMouse = false;
bool wireframe = false;
double startTime = 0;
// --- IMGUI
bool show_demo_window = true;
bool show_another_window = false;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
GLFWwindow *window = nullptr;

float surfacePlaneHeight = 1.0f;
float floorPlaneHeight = 0.0f;
unsigned int refractFrameBuffer, reflectFrameBuffer;
unsigned int refractTextureId, reflectTextureID; // ATTACHMENTS
GLuint renderingProgramSURFACE, renderingProgramFLOOR, renderingProgramCubeMap;
Shader waterFloorShader;
Shader waterShader;

void createReflectRefractBuffers() { // called once from init()
    GLuint bufferId[1];
    glGenBuffers(1, bufferId);
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
    // initialize refraction framebuffer
    glGenFramebuffers(1, bufferId);
    refractFrameBuffer = bufferId[0];
    glBindFramebuffer(GL_FRAMEBUFFER, refractFrameBuffer);
        glGenTextures(1, bufferId);
        refractTextureId = bufferId[0];
        glBindTexture(GL_TEXTURE_2D, refractTextureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, refractTextureId, 0);

        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glGenTextures(1, bufferId);
        glBindTexture(GL_TEXTURE_2D, bufferId[0]);
        glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT24, screenWidth, screenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, bufferId[0], 0);

    // initialize reflection framebuffer
    glGenFramebuffers(1, bufferId);
    reflectFrameBuffer = bufferId[0];
    glBindFramebuffer(GL_FRAMEBUFFER, reflectFrameBuffer);
        glGenTextures(1, bufferId);
        reflectTextureID = bufferId[0];
        glBindTexture(GL_TEXTURE_2D, reflectTextureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, reflectTextureID, 0);

        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glGenTextures(1, bufferId);
        glBindTexture(GL_TEXTURE_2D, bufferId[0]);
        glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT24, screenWidth, screenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, bufferId[0], 0);
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

    waterFloorShader = Shader("shaders/water/water.vert", "shaders/water/waterFloor.frag");
    waterShader = Shader("shaders/water/water.vert", "shaders/water/water.frag");
    Plane waterFloor (waterFloorShader, projection);
    Plane water (waterShader, projection);

    // --- cube
    Shader cubeShader ("shaders/forward/cube.vert", "shaders/forward/cube.frag");
    cubeShader.use();
    cubeShader.setMat4("projection", &projection);
    MyCube cube ("textures/wood.jpg");

    createReflectRefractBuffers();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    vec3 lightPos (0.0f, 3.0f,0.0f);

    while(!glfwWindowShouldClose(window)){
        // --- process inputs
        glfwPollEvents();
        camera.setStatus(!io.WantCaptureMouse);
        // --- camera
        GLdouble xPos, yPos;
        glfwGetCursorPos(window, &xPos, &yPos);
        camera.handleMouse(xPos, yPos);
        camera.handleKeyboard();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* --- part 2: reflect --- */
        // mat4 vMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -(surfacePlaneHeight - camera.camPos.y), 0.0f))
        //         * glm::rotate(glm::mat4(1.0f), glm::radians(-camera.pitch), glm::vec3(1.0f, 0.0f, 0.0f));
        // glBindBuffer(GL_FRAMEBUFFER, reflectFrameBuffer);
        // glClear(GL_DEPTH_BUFFER_BIT);
        // glClear(GL_COLOR_BUFFER_BIT);
        // // prepForSkyBoxRender();
        // glEnable(GL_CULL_FACE);
        // glFrontFace(GL_CCW);
        // glDisable(GL_DEPTH_TEST);
        // glDrawArrays(GL_TRIANGLES, 0, 36);
        // glEnable(GL_DEPTH_TEST);
        // ------------------ <scene> ------------------
        mat4 view = glm::lookAt(camera.camPos, camera.camPos + camera.camFront, camera.camY);
        mat4 model = translate(mat4(1), vec3(0.0f,offset,0.0f));
        mat4 MVP = projection*view*model;

        gizmo.draw(MVP);

        waterFloorShader.use();
        mat4 floorModelMatrix = glm::rotate(mat4(1), 3.14f/2.0f, vec3(1.0f,0.0f,0.0f));
        float floorScaleFactor = 5.0f;
        floorModelMatrix = scale(floorModelMatrix, vec3(floorScaleFactor, floorScaleFactor,floorScaleFactor));
        waterFloor.draw(view, floorModelMatrix);

        waterShader.use();
        waterShader.setVec3("lightPos", lightPos);
        floorModelMatrix = translate(floorModelMatrix, vec3(0.0f, 0.0f, -0.2f));
        water.draw(view, floorModelMatrix);

        cubeShader.use();
        cubeShader.setMat4("view", &view);
        mat4 cubeModelM = translate(mat4(1), vec3(0.0f, 3.0f,0.0f));
        cubeShader.setMat4("model",&cubeModelM);
        cube.Draw(cubeShader);

        // ------------------ </scene> ------------------


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