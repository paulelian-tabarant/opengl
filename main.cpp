#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <memory>

#include "Shader.h"
#include "Camera.h"
#include "LightTypes.h"
#include "Model.h"
#include "ScreenSpaceAO.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>

const unsigned int screenWidth {1920}, screenHeight {1080};

// Mouse interaction
float lastX, lastY;
bool firstMouse {true};

// Timing
float deltaTime     {0.0f},
      lastFrameTime {0.0f};

std::unique_ptr<Camera> camera;
std::unique_ptr<Model> roomBox;
std::unique_ptr<Model> stitch;
const unsigned int lightsNb {3};
std::unique_ptr<PointLight> pointLights[lightsNb];

// Deferred shading geometry pass
unsigned int gFrameBuffer;
unsigned int gPositionTex, gNormalTex, gColorSpecTex;

// GLFW callback functions
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow *window, double mouseX, double mouseY);
void scroll_callback(GLFWwindow* window, double dx, double dy);

void processInput(GLFWwindow *window);
void renderQuad();
void renderScene(Shader &shader);
void initGeometryPass();

int main(int argc, char *argv[])
{
    glfwInit();
    // Set running versions of OpenGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Hello OpenGL", glfwGetPrimaryMonitor(), NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Tell GLFW to make the context of our window the main context of current thread
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    // Always set callbacks after having created the window
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // Enable cursor capture for input callbacks (& hide hint)
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // Scroll callback
    glfwSetScrollCallback(window, scroll_callback);

    // Init object models
    roomBox = std::make_unique<Model>("Models/cube/cube.obj");
    roomBox->setPosition(0.0f, 0.0f, -0.2f);
    roomBox->setScale(10.0f, 10.0f, 10.0f);

    stitch = std::make_unique<Model>("Models/stitch/stitch.obj");
    stitch->setPosition(0.0f, -2.0f, -4.0f);

    srand(time(NULL));
    for (unsigned int i = 0; i < lightsNb; i++) {
        float x = ((rand() % 100) / 100.0) * 8.0 - 4.0,
              y = ((rand() % 100) / 100.0) * 8.0 - 4.0,
              z = ((rand() % 100) / 100.0) * 8.0 - 6.0;
        float r = ((rand() % 100) / 100.0) * 5.0;
        float g = ((rand() % 100) / 100.0) * 5.0;
        float b = ((rand() % 100) / 100.0) * 5.0;
        pointLights[i] = std::make_unique<PointLight>(x, y, z);
        glm::vec3 diffuse(r, g, b);
        pointLights[i]->setColor(diffuse * 0.8f, diffuse * 1.2f, glm::vec3(20.0f));
        pointLights[i]->setUniformName("lights[" + std::to_string(i) + "]");
    }

    // Enable z-buffer
    glEnable(GL_DEPTH_TEST);

    // CAUTION: always init buffers AFTER enabling GL_DEPTH_TEST

	// Initialization of geometry + SSAO passes
	initGeometryPass();
	ScreenSpaceAO ssao(screenWidth, screenHeight);

    // Shaders initialization
    Shader geomShader("geomVS.vert", "", "geomFS.frag");
    // Screen space ambient occlusion
    // CAUTION: verify texture names !!!
    Shader ssaoShader("ssaoVS.vert", "", "ssaoFS.frag");
    ssaoShader.use();
    ssaoShader.setInt("positionTex", 29);
    ssaoShader.setInt("normalTex", 30);
    ssaoShader.setInt("noiseTex", 10);
    // Init light object (also rendered as cube)
    Shader lightShader("lightVS.vert", "", "lightFS.frag");
    // HDR rendering
    Shader illumShader("illumVS.vert", "", "illumFS.frag");
    illumShader.use();
    illumShader.setInt("positionTex", 29);
    illumShader.setInt("normalTex", 30);
    illumShader.setInt("colorSpecTex", 31);
    illumShader.setInt("ssaoTex", 10);

    // Init camera object to navigate in the scene
    camera = std::make_unique<Camera>();

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Handle user input in a specific function
        processInput(window);

        glBindFramebuffer(GL_FRAMEBUFFER, gFrameBuffer);
            glClearColor(0.0, 0.0, 0.0, 1.0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            geomShader.use();
            camera->writeToShader(geomShader, screenWidth, screenHeight);
            renderScene(geomShader);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // SSAO pass
        glBindFramebuffer(GL_FRAMEBUFFER, ssao.getFbo());
            glClear(GL_COLOR_BUFFER_BIT);
            ssaoShader.use();
			ssao.loadMainPass(ssaoShader, gPositionTex, gNormalTex);
            camera->writeToShader(ssaoShader, screenWidth, screenHeight);
            renderQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Final illumination pass
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        illumShader.use();
        glActiveTexture(GL_TEXTURE29);
        glBindTexture(GL_TEXTURE_2D, gPositionTex);
        glActiveTexture(GL_TEXTURE30);
        glBindTexture(GL_TEXTURE_2D, gNormalTex);
        glActiveTexture(GL_TEXTURE31);
        glBindTexture(GL_TEXTURE_2D, gColorSpecTex);
        glActiveTexture(GL_TEXTURE10);
        glBindTexture(GL_TEXTURE_2D, ssao.getOutputTexId());
        camera->writeToShader(illumShader, screenWidth, screenHeight);
        illumShader.setFloat("attenuation.kc", PointLight::attenuation.constant);
        illumShader.setFloat("attenuation.kl", PointLight::attenuation.linear);
        illumShader.setFloat("attenuation.kq", PointLight::attenuation.quadratic);
        for (unsigned int i = 0; i < lightsNb; i++)
            pointLights[i]->writeToShaderFromView(illumShader, camera->getViewMatrix());
        renderQuad();

        glfwSwapBuffers(window);

        // Per-frame timing
        float curTime = glfwGetTime();
        deltaTime = curTime - lastFrameTime;
        lastFrameTime = curTime;

        // Look for new interaction events
        glfwPollEvents();
    }

    // Clean GLFW variables data
    glfwTerminate();
    return 0;
}

void renderScene(Shader &shader)
{
    shader.setMatrix4f("model", stitch->getModelMat());
    stitch->draw(shader);
    // Room walls as a cube
    shader.setMatrix4f("model", roomBox->getModelMat());
    shader.setInt("reverseNormals", 1);
    roomBox->draw(shader);
    shader.setInt("reverseNormals", 0);
}

unsigned int quadVAO = 0;
unsigned int quadVBO;

void renderQuad()
{
    if (quadVAO == 0) {
        float quadVertices[] {
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // initialize vertex attributs layout
        glGenVertexArrays(1, &quadVAO);
        glBindVertexArray(quadVAO);
        // Prepare GPU buffer for receiving 3D positions
        glGenBuffers(1, &quadVBO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        // Vertex attributes
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void initGeometryPass()
{
    // Init frame buffer for deferred shading
    glGenFramebuffers(1, &gFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gFrameBuffer);

    glGenTextures(1, &gPositionTex);
    glBindTexture(GL_TEXTURE_2D, gPositionTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, gPositionTex, 0);

    glGenTextures(1, &gNormalTex);
    glBindTexture(GL_TEXTURE_2D, gNormalTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, gNormalTex, 0);

    glGenTextures(1, &gColorSpecTex);
    glBindTexture(GL_TEXTURE_2D, gColorSpecTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, gColorSpecTex, 0);

    GLenum frameBufferTextures[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, frameBufferTextures);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *window, double mouseX, double mouseY)
{
    if (firstMouse) {
	lastX = mouseX;
	lastY = mouseY;
	firstMouse = false;
    }

    float dx = mouseX - lastX, dy = mouseY - lastY;
    camera->rotateFromInput(dx, dy);

    lastX = mouseX;
    lastY = mouseY;
}

void scroll_callback(GLFWwindow* window, double dx, double dy)
{
    camera->zoomFromScroll(dy);
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    const float camSpeed = 2.5f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera->moveFromInput(deltaTime, 0.0f);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera->moveFromInput(-deltaTime, 0.0f);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera->moveFromInput(0.0f, -deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera->moveFromInput(0.0f, deltaTime);
}
