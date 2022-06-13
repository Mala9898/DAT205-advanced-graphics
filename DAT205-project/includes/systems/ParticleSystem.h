#pragma once

#include <random>
#include "utils/shader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

class ParticleSystem {
private:
    std::random_device randomDevice;
    std::mt19937 generator;
    std::uniform_real_distribution<float> unifDist;

    Shader particleShader;
    const char * vertShaderPath = "shaders/particle/particle.vert";
    const char * fragShaderPath = "shaders/particle/particle.frag";

    unsigned int particlesVAO;
    unsigned int velocityVBO;
    unsigned int timeStartVBO;

    double startTime;


public:
    bool continuous = true;
    int numParticles;
    float particleSize = 0.3f;
    float lifetime = 1.0f;
    glm::vec3 position = glm::vec3(0.0f,0.0f,0.0f);
    glm::vec3 gravity = glm::vec3(0.0f,0.5f,0.0f);
    float thetaStart = 0.0f;
    float thetaEnd = 3.14f;
    float phiStart = 0.0f;
    float phiEnd = 6.28f;
    float velocityStart = 0.05f;
    float velocityEnd = 0.5f;
    bool growSize = false;
    float sizeStart = 0.3f;
    float sizeEnd = 1.0f;

    ParticleSystem(int numParticles, bool continuous) :
            numParticles(numParticles),
            continuous(continuous)
    {
        particleShader = Shader(vertShaderPath, fragShaderPath);
        // setup random device so we can sample from normal distribution
        generator.seed(randomDevice());
        unifDist = std::uniform_real_distribution<float>(0.0f, 1.0f);
    }
    void generate(const char * texturePath){
        // --- build VAO
        glGenVertexArrays(1, &particlesVAO);
        glBindVertexArray(particlesVAO);

        // --- initial velocity
        glGenBuffers(1, &velocityVBO);
        glBindBuffer(GL_ARRAY_BUFFER, velocityVBO);
        std::vector<float> sampledVelocities = sampleSphericalDirs(numParticles);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*sampledVelocities.size(), &sampledVelocities[0],GL_STATIC_DRAW);
        glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE, 3*sizeof(float),0);
        glEnableVertexAttribArray(0);
        glVertexAttribDivisor(0,1);

        // --- start time
        glGenBuffers(1, &timeStartVBO);
        glBindBuffer(GL_ARRAY_BUFFER, timeStartVBO);
        std::vector times = linspace(0, lifetime);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*times.size(), &times[0], GL_STATIC_DRAW);
        glVertexAttribPointer(1,1, GL_FLOAT, GL_FALSE, sizeof(float),0);
        glEnableVertexAttribArray(1);
        glVertexAttribDivisor(1,1);

        // --- texture
        glActiveTexture(GL_TEXTURE0);
        int width, height, nChannels;
        unsigned char *imgdata = stbi_load(texturePath,&width, &height, &nChannels, 0);
        GLuint tex = 0;
        if( imgdata != nullptr ) {
            glGenTextures(1, &tex);
            glBindTexture(GL_TEXTURE_2D, tex);
            glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, imgdata);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

            stbi_image_free(imgdata);
        }
        particleShader.use();
        particleShader.setInt("tex", 0);
        particleShader.setFloat("lifetime", lifetime);
        particleShader.setVec3("size", glm::vec3(particleSize, sizeStart, sizeEnd));
        particleShader.setVec3("gravity", gravity);
        particleShader.setVec3("position", position);
        particleShader.setBool("continuous", continuous);
        particleShader.setBool("growSize", growSize);
        startTime = glfwGetTime();
    }
    void draw(const glm::mat4& MV, const glm::mat4& projection){
        glDepthMask(GL_FALSE);
        particleShader.use();
        particleShader.setMat4("MV", &MV);
        particleShader.setMat4("projection", &projection);
        particleShader.setFloat("time", glfwGetTime()-startTime);
        glBindVertexArray(particlesVAO);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, numParticles);
        glBindVertexArray(0);
        glDepthMask(GL_TRUE);
    }

    inline float unif () {
        return unifDist(randomDevice);
    }
    std::vector<float> linspace(float s, float e) {
        float step = (e-s)/((float)(numParticles-1));
        std::vector<float> toReturn;
        for(auto i = 0; i < numParticles; i++) {
            toReturn.emplace_back(s + (float)(i*step));
        }
        return toReturn;
    }
    std::vector<float> sampleSphericalDirs(int n) {
        // theta : angle "tilted pisa tower makes with y-axis"
        // phi   : spin around
        std::vector<float> toReturn;
        for (auto i = 0; i < n; i++) {
            // randomize direction and velocity
            float u1 = unif();
            float u2 = unif();
            float u3 = unif();
            float theta = thetaStart * u1 + (1-u1)*thetaEnd;
            float phi = phiStart*u2 + (1-u2)*phiEnd;
            float vel = velocityStart*u3 + (1-u3)*velocityEnd;

            glm::vec3 dir(sinf(theta) * cosf(phi), cosf(theta), sinf(theta) * sinf(phi));
            dir *= vel;
            toReturn.emplace_back(dir.x);
            toReturn.emplace_back(dir.y);
            toReturn.emplace_back(dir.z);
        }
        return toReturn;
    }

    ~ParticleSystem(){

    }
    void setTheta(float start, float end) {
        thetaStart = start;
        thetaEnd = end;
    }
    void setPhi(float start, float end) {
        phiStart = start;
        phiEnd = end;
    }
    void setVelocity(float start, float end) {
        velocityStart = start;
        velocityEnd = end;
    }
    void setSize(float start, float end) {
        sizeStart = start;
        sizeEnd = end;
    }
};