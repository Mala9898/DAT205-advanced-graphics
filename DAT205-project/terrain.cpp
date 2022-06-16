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
#include "utils/PerlinNoise.h"
#include "utils/Plane.h"
#include "utils/Skybox.h"
// #include "utils/Plane.h"
// #include "utils/Cube.h"


void mouse_callback(GLFWwindow* window, double x, double y);
void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void toggle_mouse ();
void toggle_wireframe();
void createFBOs();

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
unsigned int refractFBO, refLECT_FBO;
unsigned int refractTextureId, reflectTextureID; // ATTACHMENTS
Shader waterFloorShader;
Shader waterShader;

float blendFactor = 0.194f;
float blendFactor2 = -7.5f;
float waterOffset = 2.9f;


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

    /* --- water init --- */
    waterFloorShader = Shader("shaders/water/water.vert", "shaders/water/waterFloor.frag");
    waterShader = Shader("shaders/water/water.vert", "shaders/water/water2.frag");
    // waterShader = Shader("shaders/tests/display.vert", "shaders/tests/display.frag");
    Plane waterFloor (waterFloorShader, projection);
    Plane water (waterShader, projection);
    createFBOs();

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    // glDisable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    vec3 lightPos (0.0f, 40.0f,0.0f);

    const siv::PerlinNoise::seed_type seed = 123456u;
    const siv::PerlinNoise perlin{ seed };

    int width2, height2, nrChannels2;
    unsigned char *dataMask = stbi_load("mask.png", &width2, &height2, &nrChannels2, 0);

    // load and create a texture
    // -------------------------
    // load image, create texture and generate mipmaps
    // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrChannels;
    unsigned char *data = stbi_load("heightmap_test.png", &width, &height, &nrChannels, 0);
    width =100;
    height = 100;
    // width =10;
    // height = 10;


    float tMin=100.0f;
    float tMax = -100.0f;
    std::vector<float> vertices;
    // float yScale = 64.0f / 256.0f, yShift = 16.0f;
    float yScale = 16.0f / 256.0f, yShift = 5.0f;
    int rez = 1;
    unsigned bytePerPixel = nrChannels2;
    // --- terrain verts
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            unsigned char* pixelOffset = dataMask + (j + width * i) * bytePerPixel;
            unsigned char y = pixelOffset[0];
            // vertex
            vertices.push_back( -height/2.0f + height*i/(float)height );   // vx
            float oct1 = 0.05f; //smooth
            float oct2 = 0.2f;
            float oct3 = 0.4f;

            const double noise1 = perlin.noise2D_01((i * oct1), (j * oct1));
            const double noise2 = 0.0f;//perlin.noise2D_01((i * oct2), (j * oct2));
            const double noise3 = 0.1f*(perlin.noise2D_01((i * oct3), (j * oct3)));
            double totalNoise = noise1+noise2+noise3;
            double toAdd = ((30.0f*totalNoise-10.0f)*y)/256.0f;
            if(toAdd>tMax)
                tMax = toAdd;
            if(toAdd <tMin)
                tMin = toAdd;
            vertices.push_back(toAdd );   // vy
            vertices.push_back( -width/2.0f + width*j/(float)width );   // vz

        }
    }
    stbi_image_free(data);

    // --- normal map data
    float* normalMapArr = new float[3*100*100];
    for(auto i = 0; i < 100; i++) {
        for(auto j = 0; j < 100; j++) {
            int x =j;
            int z = i;

            float heightL = vertices[3*((x-1)*100 +z) + 1]; //getHeight(x-1, z, generator);
            float heightR = vertices[3*((x+1)*100 +z) + 1];
            float heightD = vertices[3*((x)*100 +z-1) + 1];
            float heightU = vertices[3*((x)*100 +z+1) + 1];
            // vec3 normal (heightL - heightR, 2.0f, heightD - heightU);
            vec3 normal (heightL - heightR, 2.0f, heightD - heightU);
            normal = glm::normalize(normal);

            normalMapArr[3*(i*100 + j)+0] = normal.x;
            normalMapArr[3*(i*100 + j)+1] = normal.y;
            normalMapArr[3*(i*100 + j)+2] = normal.z;
        }
    }

    // --- texcoords
    std::vector<float> texCoords;
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            texCoords.push_back( (float) j/100.0f );   // vx
            texCoords.push_back( (float) i/100.0f);   // vy
            // texCoords.push_back( 0.0f );   // vx
            // texCoords.push_back( 0.0f);   // vy
        }
    }
    // --- indices
    std::vector<unsigned> indices;
    for(unsigned i = 0; i < height-1; i += rez){
        for(unsigned j = 0; j < width; j += rez){
            for(unsigned k = 0; k < 2; k++){
                indices.push_back(j + width * (i + k*rez));
            }
        }
    }

    const int nStrips = (height - 1) / rez;
    const int numTrisPerStrip = (width/rez)*2-2;

    // first, configure the cube's VAO (and terrainVBO + terrainIBO)
    unsigned int terrainVAO, terrainVBO,texVBO, terrainIBO, textureRock;
    glGenVertexArrays(1, &terrainVAO);
    glBindVertexArray(terrainVAO);

    glGenBuffers(1, &terrainVBO);
    glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // tex attribute
    glGenTextures(1, &textureRock);        // generate texture (#number of textures, textureID(s))
        string texture_name = "textures/rock.png";
        glActiveTexture(GL_TEXTURE0);                 // activate the texture unit first before binding texture
        glBindTexture(GL_TEXTURE_2D, textureRock); // bind texture onto texture unit #0 (GL_TEXTURE0)
        // glActiveTexture(GL_TEXTURE1);              // set additional textures, 0,...,15
        // glBindTexture(GL_TEXTURE_2D, texture2);
        // set texture wrapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        stbi_set_flip_vertically_on_load(true);

        int width3, height3, nrChannels3;
        unsigned char *dataRock = stbi_load(texture_name.c_str(), &width3, &height3, &nrChannels3, STBI_rgb_alpha);

        if (dataRock) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width3, height3, 0, GL_RGBA, GL_UNSIGNED_BYTE, dataRock); // texture WORKS
            // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 100, 100, 0, GL_RGBA, GL_FLOAT, testArr); // 4 array WORKS
            // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 100, 100, 0, GL_RGB, GL_FLOAT, normalMapArr); // normal

            glGenerateMipmap(GL_TEXTURE_2D); // generate mipmaps
            std::cout << "loaded texture: "+ texture_name << std::endl;
        } else {
            std::cout << "failed to load texture" << std::endl;
        }
        stbi_image_free(dataRock); // free image memory

    unsigned int textureRockGround;
        glGenTextures(1, &textureRockGround);        // generate texture (#number of textures, textureID(s))
        texture_name = "textures/rock_ground.png";
        glActiveTexture(GL_TEXTURE0);                 // activate the texture unit first before binding texture
        glBindTexture(GL_TEXTURE_2D, textureRockGround); // bind texture onto texture unit #0 (GL_TEXTURE0)
        // glActiveTexture(GL_TEXTURE1);              // set additional textures, 0,...,15
        // glBindTexture(GL_TEXTURE_2D, texture2);
        // set texture wrapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        stbi_set_flip_vertically_on_load(true);

        int width4, height4, nrChannels4;
        unsigned char *dataRockGround = stbi_load(texture_name.c_str(), &width4, &height4, &nrChannels4, STBI_rgb_alpha);

        if (dataRockGround) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width4, height4, 0, GL_RGBA, GL_UNSIGNED_BYTE, dataRockGround); // texture WORKS
            glGenerateMipmap(GL_TEXTURE_2D); // generate mipmaps
            std::cout << "loaded texture: "+ texture_name << std::endl;
        }
        stbi_image_free(dataRockGround); // free image memory

    unsigned int textureRockGrass;
    glGenTextures(1, &textureRockGrass);        // generate texture (#number of textures, textureID(s))
    texture_name = "textures/rockGrass.png";
    glActiveTexture(GL_TEXTURE0);                 // activate the texture unit first before binding texture
    glBindTexture(GL_TEXTURE_2D, textureRockGrass); // bind texture onto texture unit #0 (GL_TEXTURE0)
    // glActiveTexture(GL_TEXTURE1);              // set additional textures, 0,...,15
    // glBindTexture(GL_TEXTURE_2D, texture2);
    // set texture wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    stbi_set_flip_vertically_on_load(true);

    int width5, height5, nrChannels5;
    unsigned char *dataRockGrass = stbi_load(texture_name.c_str(), &width5, &height5, &nrChannels5, STBI_rgb_alpha);

    if (dataRockGround) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width5, height5, 0, GL_RGBA, GL_UNSIGNED_BYTE, dataRockGrass); // texture WORKS
        glGenerateMipmap(GL_TEXTURE_2D); // generate mipmaps
        std::cout << "loaded texture: "+ texture_name << std::endl;
    }
    stbi_image_free(dataRockGrass); // free image memory

    // ---  create "normal" texture
    unsigned int terrainNormalTexture;
    glGenTextures(1, &terrainNormalTexture);        // generate texture (#number of textures, textureID(s))
        glActiveTexture(GL_TEXTURE0);                 // activate the texture unit first before binding texture
        glBindTexture(GL_TEXTURE_2D, terrainNormalTexture); // bind texture onto texture unit #0 (GL_TEXTURE0)
        // set texture wrapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 100, 100, 0, GL_RGB, GL_FLOAT, normalMapArr); // normal


    glGenBuffers(1, &texVBO);
    glBindBuffer(GL_ARRAY_BUFFER, texVBO);
    glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(float), &texCoords[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &terrainIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), &indices[0], GL_STATIC_DRAW);

    Shader heightMapShader ("shaders/terrain/terrain.vert", "shaders/terrain/terrain.frag");
    heightMapShader.use();
    heightMapShader.setInt("myTexture",2);
    heightMapShader.setFloat("tMin",tMin);
    heightMapShader.setFloat("tMax",tMax);

    // --- cube
    Shader cubeShader ("shaders/forward/cube.vert", "shaders/forward/cube.frag");
    cubeShader.use();
    cubeShader.setMat4("projection", &projection);
    MyCube cube ("textures/wood.jpg");
    // MyCube cube ("textures/rock.jpeg");

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW);

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
        mat4 terrainModelMatrix = translate(mat4(1), vec3(0.0f, waterOffset, 0.0f));
        mat4 MVP = projection*view*model;

        /* --- refraction --- */
        glBindFramebuffer(GL_FRAMEBUFFER, refractFBO);
            // glDisable(GL_DEPTH_TEST);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            gizmo.draw(MVP);
            // --- <render mountain 🏔> ----
            glFrontFace(GL_CW);
            heightMapShader.use();
            heightMapShader.setMat4("MVP", &MVP);
            heightMapShader.setMat4("model", &terrainModelMatrix);
            heightMapShader.setMat4("view", &view);
            heightMapShader.setInt("myTexture", 2);
            heightMapShader.setInt("normalTexture", 3);
            heightMapShader.setInt("textureRockGround", 4);
            heightMapShader.setInt("textureRockGrass", 5);
            heightMapShader.setVec3("viewPos", camera.camPos);
            heightMapShader.setFloat("blendFactor", blendFactor);
            heightMapShader.setFloat("blendFactor2", blendFactor2);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, textureRock);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, terrainNormalTexture);
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, textureRockGround);
            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_2D, textureRockGrass);

            glBindVertexArray(terrainVAO);
            for(unsigned strip = 0; strip < nStrips; strip++){
                glDrawElements(GL_TRIANGLE_STRIP,numTrisPerStrip+2,GL_UNSIGNED_INT,(void*)(sizeof(unsigned) * (numTrisPerStrip+2) * strip));
            }
            // --- </render mountain 🏔> ----

        /* --- 🪞 reflect --- */
        glBindFramebuffer(GL_FRAMEBUFFER, refLECT_FBO);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            vec3 flip(camera.camPos.x, waterOffset-camera.camPos.y, camera.camPos.z);
            mat4 vMat = glm::lookAt(flip, flip+camera.camFrontFlipped, camera.camYflippped);
            // -- render "reflected" scene

            gizmo.draw(projection*vMat);
            cubeShader.use();
            cubeShader.setMat4("view", &vMat);
            mat4 cubeModelM = translate(mat4(1), vec3(0.0f, 1.0f,0.0f));
            cubeShader.setMat4("model",&cubeModelM);
            cube.Draw(cubeShader);

            // --- <render mountain 🏔> ----
            glFrontFace(GL_CW);
            heightMapShader.use();
            // heightMapShader.setMat4("MVP", &MVP);
            heightMapShader.setMat4("projection", &projection);
            heightMapShader.setMat4("model", &terrainModelMatrix);
            heightMapShader.setMat4("view", &vMat);
            heightMapShader.setInt("myTexture", 2);
            heightMapShader.setInt("normalTexture", 3);
            heightMapShader.setInt("textureRockGround", 4);
            heightMapShader.setInt("textureRockGrass", 5);
            heightMapShader.setVec3("viewPos", camera.camPos);
            heightMapShader.setFloat("blendFactor", blendFactor);
            heightMapShader.setFloat("blendFactor2", blendFactor2);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, textureRock);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, terrainNormalTexture);
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, textureRockGround);
            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_2D, textureRockGrass);

            glBindVertexArray(terrainVAO);
            for(unsigned strip = 0; strip < nStrips; strip++){
                glDrawElements(GL_TRIANGLE_STRIP,numTrisPerStrip+2,GL_UNSIGNED_INT,(void*)(sizeof(unsigned) * (numTrisPerStrip+2) * strip));
            }
            // --- </render mountain 🏔> ----

        glBindFramebuffer(GL_FRAMEBUFFER,0);
            glFrontFace(GL_CCW);
            /* --- default framebuffer --- */
            glClearColor(0.721f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // sets the color above

            gizmo.draw(MVP);
            cubeShader.use();
            cubeShader.setMat4("view", &view);
            // mat4 cubeModelM = translate(mat4(1), vec3(0.0f, 3.0f,0.0f));
            cubeShader.setMat4("model",&model);
            cube.Draw(cubeShader);

            // // 1. render water
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, reflectTextureID);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, refractTextureId);

            waterShader.use();
            waterShader.setInt("textureReflect", 0);
            waterShader.setInt("textureRefract", 1);
            // // waterShader.setInt("textureRefract", 1);
            // waterShader.setVec3("lightPos", lightPos);
            // floorModelMatrix = translate(floorModelMatrix, vec3(0.0f, 0.0f, -0.2f));
            float floorScaleFactor = 100.0f;
            // mat4 floorModelMatrix = translate(mat4(1), vec3(0.0f, waterOffset, 0.0f))
            //         *glm::rotate(mat4(1), -3.14f/2.0f, vec3(1.0f,0.0f,0.0f))
            //         *scale(mat4(1), vec3(floorScaleFactor, floorScaleFactor,floorScaleFactor));
            mat4 floorModelMatrix = translate(mat4(1), vec3(0.0f,waterOffset,0.0f))
                                *glm::rotate(mat4(1), -3.14f/2.0f, vec3(1.0f,0.0f,0.0f))
                                *scale(mat4(1), vec3(floorScaleFactor, floorScaleFactor,floorScaleFactor));

            glFrontFace(GL_CCW);
            water.draw(view, translate(mat4(1), vec3(0.0f, 0.0f,0.0f))*floorModelMatrix );

            // --- <render mountain 🏔> ----
            glFrontFace(GL_CW);
            heightMapShader.use();
            heightMapShader.setMat4("MVP", &MVP);
            heightMapShader.setMat4("model", &terrainModelMatrix);
            heightMapShader.setMat4("view", &view);
            heightMapShader.setInt("myTexture", 2);
            heightMapShader.setInt("normalTexture", 3);
            heightMapShader.setInt("textureRockGround", 4);
            heightMapShader.setInt("textureRockGrass", 5);
            heightMapShader.setVec3("viewPos", camera.camPos);
            heightMapShader.setFloat("blendFactor", blendFactor);
            heightMapShader.setFloat("blendFactor2", blendFactor2);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, textureRock);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, terrainNormalTexture);
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, textureRockGround);
            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_2D, textureRockGrass);

            glBindVertexArray(terrainVAO);
            for(unsigned strip = 0; strip < nStrips; strip++){
                glDrawElements(GL_TRIANGLE_STRIP,numTrisPerStrip+2,GL_UNSIGNED_INT,(void*)(sizeof(unsigned) * (numTrisPerStrip+2) * strip));
            }
            // --- </render mountain 🏔> ----




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

            ImGui::SliderFloat("blend factor",&blendFactor, 0.0f,5.0f);
            ImGui::SliderFloat("blend factor2",&blendFactor2, -10.0f,10.0f);
            ImGui::SliderFloat("water offset",&waterOffset, 0.0f,20.0f);
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

void createFBOs() { // called once from init()
    GLuint bufferId[1];

    glGenFramebuffers(1, &refractFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, refractFBO);
    // create a color attachment texture
    // unsigned int textureColorbuffer;
    glGenTextures(1, &refractTextureId);
    glBindTexture(GL_TEXTURE_2D, refractTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, refractTextureId, 0);

    // create & attach depth buffer
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenWidth, screenHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR: refract FrameBuffer incomplete" << std::endl;

    // initialize reflection framebuffer
    glGenFramebuffers(1, &refLECT_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, refLECT_FBO);
    glGenTextures(1, &reflectTextureID);
    glBindTexture(GL_TEXTURE_2D, reflectTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, reflectTextureID, 0);

    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    // create & attach depth buffer
    unsigned int rboDepth2;
    glGenRenderbuffers(1, &rboDepth2);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth2);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenWidth, screenHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth2);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR: reflect FrameBuffer incomplete" << std::endl;
}