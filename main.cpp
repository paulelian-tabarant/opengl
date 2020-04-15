#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

#include "Shader.h"
#include "Camera.h"
#include "LightTypes.h"
#include "Model.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>

unsigned int screenWidth = 1920, screenHeight = 1080;

// Mouse interaction
float lastX, lastY;
bool firstMouse = true;
// Timing
float deltaTime = 0.0f;
float lastFrameTime = 0.0f;

Camera camera;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow *window, double mouseX, double mouseY);
void scroll_callback(GLFWwindow* window, double dx, double dy);
void processInput(GLFWwindow *window);

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

    // Init different types of light
    DirLight dirLight(-1.5f, -3.0f, -1.5f);
    dirLight.setUniformName("dirLight");

    // Init object models
    Model *cubes[3];
    for (unsigned int i = 0; i < 3; i++) {
        cubes[i] = new Model("Models/cube/cube.obj");
    }
    cubes[0]->setRotation(45.0f, glm::vec3(0.5f, 0.5f, 0.0f));
    cubes[1]->setRotation(-60.0f, glm::vec3(0.1f, 0.0f, 0.8f));
    cubes[1]->setPosition(-1.5f, 0.0f, 1.0f);
    cubes[2]->setRotation(30.0f, glm::vec3(0.0f, 0.2f, 1.0f));
    cubes[2]->setPosition(2.0f, 0.0f, 0.0f);
    Model floorModel("Models/floor/floor.obj");
    floorModel.setScale(0.3f, 0.3f, 0.3f);
    floorModel.setRotation(-90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
    floorModel.setPosition(0.0f, 0.0f, -3.0f);

    // Enable z-buffer
    glEnable(GL_DEPTH_TEST);

    // Shader initialization
    Shader objShader("vs.vert", "fs.frag");
    // Init light object (also rendered as cube)
    Shader lightShader("light_vs.vert", "light_fs.frag");
    // Shadow mapping
    Shader depthShader("lightDepthShaderVS.vert", "lightDepthShaderFS.frag");

    // Init camera object to navigate in the scene
    camera = Camera();

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Handle user input in a specific function
        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        depthShader.use();
        depthShader.setMatrix4f("lightSpaceMatrix", dirLight.getLightSpaceMatrix());

        glViewport(0, 0, dirLight.SHADOW_WIDTH, dirLight.SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, dirLight.depthMapFBO);
            glClear(GL_DEPTH_BUFFER_BIT);
            for (unsigned int i = 0; i < 3; i++) {
                depthShader.setMatrix4f("model", cubes[i]->getModelMat());
                cubes[i]->draw(depthShader);
            }
            depthShader.setMatrix4f("model", floorModel.getModelMat());
            floorModel.draw(depthShader);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, screenWidth, screenHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        objShader.use();
        // view, projection and cameraPos uniforms
        camera.writeToShader(objShader, screenWidth, screenHeight);
        dirLight.writeToShader(objShader);
        // TODO: embed this instruction in above writeToShader() method
        objShader.setMatrix4f("lightSpaceMatrix", dirLight.getLightSpaceMatrix());
        // Use frame buffer result from the depth shader pass
        objShader.setInt("shadowMap", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, dirLight.depthMap);
        // Render objects
        for (unsigned int i = 0; i < 3; i++) {
            objShader.setMatrix4f("model", cubes[i]->getModelMat());
            cubes[i]->draw(objShader);
        }
        objShader.setMatrix4f("model", floorModel.getModelMat());
        floorModel.draw(objShader);

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
    camera.rotateFromInput(dx, dy);

    lastX = mouseX;
    lastY = mouseY;
}

void scroll_callback(GLFWwindow* window, double dx, double dy)
{
    camera.zoomFromScroll(dy);
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    const float camSpeed = 2.5f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.moveFromInput(deltaTime, 0.0f);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.moveFromInput(-deltaTime, 0.0f);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.moveFromInput(0.0f, -deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.moveFromInput(0.0f, deltaTime);
}
