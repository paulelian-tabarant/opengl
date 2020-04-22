#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <memory>

#include "Shader.h"
#include "Camera.h"
#include "LightTypes.h"
#include "Model.h"

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
std::unique_ptr<Model> roomCube;
std::unique_ptr<Model> cubes[3];
std::unique_ptr<PointLight> pointLight;

// GLFW callback functions
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow *window, double mouseX, double mouseY);
void scroll_callback(GLFWwindow* window, double dx, double dy);

void processInput(GLFWwindow *window);
void renderScene(Shader &shader);

int main(int argc, char *argv[])
{
    glfwInit();
    // Set running versions of OpenGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Hello OpenGL", NULL, NULL);
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
    roomCube = std::make_unique<Model>("Models/cube/cube.obj");
    roomCube->setScale(10.0f, 10.0f, 10.0f);

    for (unsigned int i = 0; i < 3; i++)
        cubes[i] = std::make_unique<Model>("Models/cube/cube.obj");

    cubes[0]->setPosition(0.0f, 2.0f, 1.0f);
    cubes[0]->setRotation(45.0f, glm::vec3(0.5f, 0.5f, 0.0f));
    cubes[1]->setPosition(-3.0f, 0.0f, 2.0f);
    cubes[1]->setRotation(-60.0f, glm::vec3(0.1f, 0.0f, 0.8f));
    cubes[2]->setRotation(30.0f, glm::vec3(0.0f, 0.2f, 1.0f));
    cubes[2]->setPosition(4.0f, 0.0f, 0.0f);

    pointLight = std::make_unique<PointLight>();
    // Call this method to generate a whole cubic depth map for shadow rendering
    pointLight->initCubeMap();

    // Enable z-buffer
    glEnable(GL_DEPTH_TEST);

    // Shader initialization
    Shader objShader("sceneVS.vert", "", "sceneFS.frag");
    // Init light object (also rendered as cube)
    Shader lightShader("lightVS.vert", "", "lightFS.frag");
    // Shadow mapping
    Shader depthShader("depthShaderVS.vert", "depthShaderGS.geom", "depthShaderFS.frag");

    // Init camera object to navigate in the scene
    camera = std::make_unique<Camera>();

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Handle user input in a specific function
        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float x = sin(glfwGetTime() * 0.5) * 5.0;
        pointLight->setPosition(glm::vec3(x, 0.0f, -1.0f));

        // Generate the shadow map : 1st render pass
        glViewport(0, 0, pointLight->SHADOW_WIDTH, pointLight->SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, pointLight->depthMapFBO);
            glClear(GL_DEPTH_BUFFER_BIT); // check if this line is necessary
            depthShader.use();
            pointLight->writeToDepthShader(depthShader);
            renderScene(depthShader);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // Note: GL_DEPTH_BUFFER_BIT automatically discards hidden fragments
        // from calculations under the hood (as it is also done with classic rendering
        // when fragments are behind others from the camera point of view)

        // Render scene with shadow map results
        glViewport(0, 0, screenWidth, screenHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        objShader.use();
        // view, projection and cameraPos uniforms
        camera->writeToShader(objShader, screenWidth, screenHeight);
        // Use frame buffer result from the depth shader pass
        objShader.setInt("cubeMap", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, pointLight->getCubeMapTextureId());
        pointLight->writeToShader(objShader);
        renderScene(objShader);

        lightShader.use();
        camera->writeToShader(lightShader, screenWidth, screenHeight);
        pointLight->writeModelMatrixInShader(lightShader, "model");
        pointLight->draw();

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
    // Scene cubes
    for (unsigned int i = 0; i < 3; i++) {
        shader.setMatrix4f("model", cubes[i]->getModelMat());
        cubes[i]->draw(shader);
    }
    // Room walls as a cube
    shader.setMatrix4f("model", roomCube->getModelMat());
    shader.setInt("reverseNormals", 1);
    roomCube->draw(shader);
    shader.setInt("reverseNormals", 0);
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
