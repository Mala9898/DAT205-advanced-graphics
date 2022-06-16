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
#include "utils/Cube.h"
// #include "utils/Plane.h"
// #include "utils/Cube.h"


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

    glEnable(GL_DEPTH_TEST);
    // glDisable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    vec3 lightPos (0.0f, 3.0f,0.0f);

    // load and create a texture
    // -------------------------
    // load image, create texture and generate mipmaps
    // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrChannels;
    unsigned char *data = stbi_load("heightmap_test.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        std::cout << "Loaded heightmap of size " << height << " x " << width << std::endl;
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    // width =10;
    // height = 10;


    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    std::vector<float> vertices;
    // float yScale = 64.0f / 256.0f, yShift = 16.0f;
    float yScale = 16.0f / 256.0f, yShift = 0.0f;
    int rez = 1;
    unsigned bytePerPixel = nrChannels;
    for(int i = 0; i < height; i++)
    {
        for(int j = 0; j < width; j++)
        {
            unsigned char* pixelOffset = data + (j + width * i) * bytePerPixel;
            unsigned char y = pixelOffset[0];

            // vertex
            vertices.push_back( -height/2.0f + height*i/(float)height );   // vx
            // vertices.push_back( (int) y * yScale - yShift);   // vy
            vertices.push_back( glm::sin((float)i) - yShift);   // vy
            vertices.push_back( -width/2.0f + width*j/(float)width );   // vz
            // vertices.push_back( (float)i );   // vx
            // vertices.push_back( (float) 0);   // vy
            // vertices.push_back( (float) j);   // vz
        }
    }
    std::cout << "Loaded " << vertices.size() / 3 << " vertices" << std::endl;
    stbi_image_free(data);

    std::vector<unsigned> indices;
    for(unsigned i = 0; i < height-1; i += rez)
    {
        for(unsigned j = 0; j < width; j += rez)
        {
            for(unsigned k = 0; k < 2; k++)
            {
                indices.push_back(j + width * (i + k*rez));
            }
        }
    }
    std::cout << "Loaded " << indices.size() << " indices" << std::endl;

    const int numStrips = (height-1)/rez;
    const int numTrisPerStrip = (width/rez)*2-2;
    std::cout << "Created lattice of " << numStrips << " strips with " << numTrisPerStrip << " triangles each" << std::endl;
    std::cout << "Created " << numStrips * numTrisPerStrip << " triangles total" << std::endl;

    // first, configure the cube's VAO (and terrainVBO + terrainIBO)
    unsigned int terrainVAO, terrainVBO, terrainIBO;
    glGenVertexArrays(1, &terrainVAO);
    glBindVertexArray(terrainVAO);

    glGenBuffers(1, &terrainVBO);
    glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &terrainIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), &indices[0], GL_STATIC_DRAW);

    Shader heightMapShader ("shaders/terrain.vert", "shaders/terrain.frag");

    // --- cube
    Shader cubeShader ("shaders/forward/cube.vert", "shaders/forward/cube.frag");
    cubeShader.use();
    cubeShader.setMat4("projection", &projection);
    MyCube cube ("textures/wood.jpg");

    while(!glfwWindowShouldClose(window)){
        // --- process inputs
        glfwPollEvents();
        camera.setStatus(!io.WantCaptureMouse);
        // --- camera
        GLdouble xPos, yPos;
        glfwGetCursorPos(window, &xPos, &yPos);
        camera.handleMouse(xPos, yPos);
        camera.handleKeyboard();

        // ------------------ <scene> ------------------
        glm::mat4 view = glm::lookAt(camera.camPos, camera.camPos + camera.camFront, camera.camY);
        mat4 model = translate(mat4(1), vec3(0.0f,offset,0.0f));
        mat4 MVP = projection*view*model;


        /* --- default framebuffer --- */
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // sets the color above

        gizmo.draw(MVP);
        cubeShader.use();
        cubeShader.setMat4("view", &view);
        // mat4 cubeModelM = translate(mat4(1), vec3(0.0f, 3.0f,0.0f));
        cubeShader.setMat4("model",&model);
        cube.Draw(cubeShader);


        // be sure to activate shader when setting uniforms/drawing objects
        heightMapShader.use();
        heightMapShader.setMat4("MVP", &MVP);
        // heightMapShader.setMat4("projection", &projection);
        // heightMapShader.setMat4("view", &view);
        //
        // world transformation
        // glm::mat4 model = glm::mat4(1.0f);
        heightMapShader.setMat4("model", &model);

        // render the cube
        glBindVertexArray(terrainVAO);
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        for(unsigned strip = 0; strip < numStrips; strip++){
            glDrawElements(GL_TRIANGLE_STRIP,   // primitive type
                           numTrisPerStrip+2,   // number of indices to render
                           GL_UNSIGNED_INT,     // index data type
                           (void*)(sizeof(unsigned) * (numTrisPerStrip+2) * strip)); // offset to starting index
        }


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
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        toggle_wireframe();
    }
}