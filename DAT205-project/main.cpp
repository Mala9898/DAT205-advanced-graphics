
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
// --- IMGUI
bool show_demo_window = true;
bool show_another_window = false;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
double startTime = 0;
bool enableSSAO = true;

unsigned int quadVAO = 0;
unsigned int quadVBO;
float plane_data[] = {
        1.0f,  1.0f, 0.0f,  // top right
        1.0f, -1.0f, 0.0f,  // bottom right
        -1.0f, -1.0f, 0.0f,  // bottom left
        -1.0f,  1.0f, 0.0f   // top left
};
vector<float> tex_data = {
        1.0f,  1.0f,  // top right
        1.0f, 0.0f,  // bottom right
        0.0f, 0.0f,  // bottom left
        0.0f,  1.0f   // top left
};
unsigned int quad_buffer_indices[] = {
        0,2,1,
        2,0,3
};
unsigned int EBO, textureBO;
void renderQuad();

GLFWwindow *window = nullptr;

unsigned int geometryFrameBuffer;
unsigned int geometryFB_position, geometryFB_normal, geometryFB_Albedo_and_Spec;
// TODO
float lerp(float a, float b, float f)
{
    return a + f * (b - a);
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
    // glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    ParticleSystem ps (500, false);
    {
        ps.gravity = vec3(0.0f,0.1f,0.0f);
        ps.lifetime = 3.0f;
        ps.setVelocity(0.1f, 1.0f);
        // ps.setPhi(0,3.14f);
        ps.setTheta(0.0f, 3.14f/4.0f);
        ps.growSize = true;
        ps.sizeEnd = 3.0f;
        ps.continuous = true;
        ps.generate("textures/fire3.png");
    }

    // ------------------ <Screen Quad> ------------------

    // screen quad VAO
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);

    // load indices
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_buffer_indices), quad_buffer_indices, GL_STATIC_DRAW);
    // load verts
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(plane_data), &plane_data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    //load tex coords
    glGenBuffers(1, &textureBO);
    glBindBuffer(GL_ARRAY_BUFFER, textureBO);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(tex_data), &tex_data[0], GL_STATIC_DRAW);            // if using arrays[]
    glBufferData(GL_ARRAY_BUFFER, tex_data.size()*sizeof(float), &tex_data[0], GL_STATIC_DRAW); // if using vector<>
    glVertexAttribPointer(1,2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    Shader quadDisplayShader ("shaders/deferred/quad_display.vert","shaders/deferred/quad_display.frag");
    quadDisplayShader.use();
    quadDisplayShader.setInt("myTexture", 0); // texture unit 0
    // ------------------ </Screen Quad> ------------------

    // ⚠️ shader BROKEN
    // Shader modelShader ("shaders/model/model.vert","shaders/model/model_primitive.frag");
    // string modelName = "models/model_plane.obj";
    // Model backpack(&modelName[0]);

    Shader modelShader ("shaders/model/model.vert","shaders/model/model.frag");
    string modelName = "models/backpack.obj";
    Model backpack(&modelName[0]);

    // ---- Pipeline ----
    // 1. geometry pass
    //     - each fragment needs to store: (vector) 1. position, 2. normal, 3. color , (float) 4. specular
    //     - render each component to different render targets (textures/renderbuffer)
    // 2. SSAO pass
    // 3. SSAO blur pass
    // 4. lighting pass

    // --- create FrameBuffer
    glGenFramebuffers(1, &geometryFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, geometryFrameBuffer);

    // - position (color buffer) vec4[x,y,z, null]
    glGenTextures(1, &geometryFB_position);
    glBindTexture(GL_TEXTURE_2D, geometryFB_position);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, geometryFB_position, 0);

    // - normal (color buffer) vec4[x,y,z, null]
    glGenTextures(1, &geometryFB_normal);
    glBindTexture(GL_TEXTURE_2D, geometryFB_normal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, geometryFB_normal, 0);

    // - color + specular, combined (color buffer) vec4[r,g,b, specular]
    glGenTextures(1, &geometryFB_Albedo_and_Spec);
    glBindTexture(GL_TEXTURE_2D, geometryFB_Albedo_and_Spec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, geometryFB_Albedo_and_Spec, 0);

    // specify order of attachments of Geometry Framebuffer
    // order will correspond to fragment shader output variables
    //                             layout (location = 0) layout (location = 1) layout (location = 2)
    unsigned int attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, attachments);

    // create & attach depth buffer
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenWidth, screenHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR: deferred FrameBuffer incomplete" << std::endl;

    // --- SSAO FrameBuffer
    unsigned int ssaoFrameBuffer;
    glGenFramebuffers(1, &ssaoFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFrameBuffer);
    unsigned int ssaoColorBuffer;
    // SSAO color buffer
    glGenTextures(1, &ssaoColorBuffer);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, screenWidth, screenHeight, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR: ssao FrameBuffer incomplete" << std::endl;

    // -- SSAO blur FrameBuffer
    unsigned int ssaoBlurFrameBuffer;
    glGenFramebuffers(1, &ssaoBlurFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFrameBuffer);
    unsigned int ssaoBlurBuffer;
    glGenTextures(1, &ssaoBlurBuffer);
    glBindTexture(GL_TEXTURE_2D, ssaoBlurBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, screenWidth, screenHeight, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoBlurBuffer, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR: ssao blur FrameBuffer incomplete" << std::endl;

    // bind default FrameBuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // ----- <sampling kernel> ------
    // --- points that will test if they're occluded or not
    int kernelSize = 64;
    // usage: unifDist(randomDevice)
    std::random_device randomDevice;
    std::mt19937 generator;
    std::uniform_real_distribution<float> unifDist;
    generator.seed(randomDevice());
    unifDist = std::uniform_real_distribution<float>(-1.0f, 1.0f);
    // --- MINE
    vector<vec3> kernel;
    for (auto i = 0; i < kernelSize; i++) {
        // shift distribution towards the center
        float scale = glm::mix(0.1f, 1.0f, ((float)i*i)/(float(kernelSize*kernelSize)));
        vec3 sample (unifDist(randomDevice), unifDist(randomDevice), abs(unifDist(randomDevice)));
        sample = sample*scale;

        kernel.emplace_back(sample);
    }
    // ----- </sampling kernel> ------
    // ----- <random noise> -------
    vector<vec3> rotationVecs;
    int noiseSize = 4;
    for(auto i = 0; i < noiseSize*noiseSize; i++) {
        float x =  unifDist(randomDevice)*2.0f -1.0f;
        float y =  unifDist(randomDevice)*2.0f -1.0f;
        rotationVecs.emplace_back(vec3(x,y,0.0f));
    }

    unsigned int rotationVecTexture;
    glGenTextures(1, &rotationVecTexture);
    glBindTexture(GL_TEXTURE_2D, rotationVecTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, noiseSize, noiseSize, 0, GL_RGB, GL_FLOAT, &rotationVecs[0]);
    // enable tiling
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // ----- </random noise> ------

    Shader ssaoShader ("shaders/deferred/quad_display.vert","shaders/ssao/ssao.frag");
    ssaoShader.use();
    ssaoShader.setMat4("projection", &projection);
    ssaoShader.setInt("kernelSize", kernelSize);
    for (auto i = 0; i < kernelSize; i++)
        ssaoShader.setVec3("samples[" + std::to_string(i) + "]", kernel[i]);

    Shader ssaoBlurShader ("shaders/deferred/quad_display.vert","shaders/ssao/blur.frag");

    Shader planeShader("shaders/primitive/plane.vert","shaders/primitive/plane.frag");
    Plane plane1(planeShader, projection);

    while(!glfwWindowShouldClose(window)){
        // --- process inputs
        glfwPollEvents();
        camera.setStatus(!io.WantCaptureMouse);
        // --- camera
        GLdouble xPos, yPos;
        glfwGetCursorPos(window, &xPos, &yPos);
        camera.handleMouse(xPos, yPos);
        camera.handleKeyboard();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // [~[~[~[~[~[~[~[~ Setup Geometry Buffer [~[~[~[~[~[~[~[~
        glBindFramebuffer(GL_FRAMEBUFFER, geometryFrameBuffer);
            glEnable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // ------------------ <scene> ------------------
            glm::mat4 view = glm::lookAt(camera.camPos, camera.camPos + camera.camFront, camera.camY);
            glm::mat4 model = translate(mat4(1), vec3(0.0f,offset,0.0f));
            mat4 MVP = projection*view*model;

            gizmo.draw(MVP);
            // --- render model
            modelShader.use();
            modelShader.setMat4("model", &model);
            modelShader.setMat4("view", &view);
            modelShader.setMat4("projection", &projection);
            backpack.Draw(modelShader);

            // glm::mat4 model2 = glm::rotate(mat4(1), 3.14f/2.0f, vec3(0.0f,1.0f,0.0f));
            // modelShader.setMat4("model", &model2);
            // backpack.Draw(modelShader);
            //
            // glm::mat4 model3 = glm::rotate(mat4(1), -3.14f/2.0f, vec3(1.0f,0.0f,0.0f));
            // modelShader.setMat4("model", &model3);
            // backpack.Draw(modelShader);

            plane1.draw(view, translate(mat4(1), vec3(0.0f,2.0f,-0.2f)));
            plane1.draw(view, translate(mat4(1), vec3(2.0f,2.0f,-0.2f)));

        glBindVertexArray(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // ------------------ </scene> ------------------

        // // [~[~[~[~[~[~[~[~ SSAO FrameBuffer [~[~[~[~[~[~[~[~
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoFrameBuffer);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glBindVertexArray(quadVAO);
            ssaoShader.use();
            ssaoShader.setInt("gPosition", 0);
            ssaoShader.setInt("gNormal", 1);
            ssaoShader.setInt("noise", 2);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, geometryFB_position);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, geometryFB_normal);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, rotationVecTexture);

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
            glBindVertexArray(0);

        // ---- BLUR BUFFER
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFrameBuffer);
            glClear(GL_COLOR_BUFFER_BIT);
            glBindVertexArray(quadVAO);
            ssaoBlurShader.use();
            ssaoBlurShader.setInt("ssao", 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
            glBindVertexArray(0);
            // glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // --- ⚠️ WARNING: ONLY FOR TESTING ---
        // glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //     // glDisable(GL_DEPTH_TEST);
        //     glClear(GL_COLOR_BUFFER_BIT);
        //     glBindVertexArray(quadVAO);
        //     quadDisplayShader.use();
        //     quadDisplayShader.setInt("gPosition", 0);
        //     glActiveTexture(GL_TEXTURE0);
        //     // glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
        //     glBindTexture(GL_TEXTURE_2D, ssaoBlurBuffer);
        //     glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
        //     glBindVertexArray(0);
        //     glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // [~[~[~[~[~[~[~[~ Bind default FrameBuffer [~[~[~[~[~[~[~[~
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
            // glDisable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT);
            // (1). bind VAO, (2). activate Texture Unit 0, (3) bind texture
            glBindVertexArray(quadVAO);
            quadDisplayShader.use();
            quadDisplayShader.setBool("enableSSAO", enableSSAO);
            quadDisplayShader.setInt("gPosition", 0);
            quadDisplayShader.setInt("gNormal", 1);
            quadDisplayShader.setInt("gAlbedoSpec", 2);
            quadDisplayShader.setInt("gAO", 3);
            quadDisplayShader.setVec3("viewPos", camera.camPos);
            quadDisplayShader.setVec3("lightPos", vec3(0.0f, 3.0f, 2.0f));

            // any texture we previously attached to Geometry FrameBuffer
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, geometryFB_position);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, geometryFB_normal);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, geometryFB_Albedo_and_Spec);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, ssaoBlurBuffer);

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
            glBindVertexArray(0);



        // --- copy depth buffer from Geometry Framebuffer into default FrameBuffer
        glBindFramebuffer(GL_READ_FRAMEBUFFER, geometryFrameBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
        glBlitFramebuffer(
                0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST
        );
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // --- render particle system
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        mat4 MV = view*model;
        ps.draw(MV, projection);
        glDisable(GL_BLEND);


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
            if (ImGui::Button("Toggle SSAO"))
                enableSSAO = !enableSSAO;
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
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        enableSSAO = !enableSSAO;
    }
}

void renderQuad(){
    if (quadVAO == 0){
        float quadVertices[] = {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}