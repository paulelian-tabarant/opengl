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

GLFWwindow *window;
Camera camera;
std::vector<Light *> lights;

int initOpenGlContext();
void initLights();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow *window, double mouseX, double mouseY);
void scroll_callback(GLFWwindow* window, double dx, double dy);
void processInput(GLFWwindow *window);

int main(int argc, char *argv[])
{
    // Init model object
    char path[100];
    sprintf(path, "Models/%s", argv[1]);
    std::ifstream f(path);
    if (!f.good()) {
        std::cout << "Error : could not find specified model file." << std::endl;
        return -1;
    }

    if (initOpenGlContext() == -1) return -1;
    initLights();

    // Init object model
    Model objModel(path);
    // Shader initialization
    Shader objShader("vs.vert", "fs.frag");

    // Set constant uniforms
    objShader.use();
    for (Light *&light : lights) {
        light->writeToShader(objShader);
    }

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Handle user input in a specific function
        processInput(window);

        glClearColor(0.3f, 0.3f, 0.3f, 0.5f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render objects
        objShader.use();
        glm::mat4 model = glm::mat4(1.0f);
        objShader.setMatrix4f("model", model);
        camera.writeToShader(objShader, screenWidth, screenHeight);
        objModel.draw(objShader);

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

int initOpenGlContext()
{
    glfwInit();
    // Set running versions of OpenGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(screenWidth, screenHeight, "Hello world", NULL, NULL);
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
    glViewport(0, 0, screenWidth, screenHeight);
    // Always set callbacks after having created the window
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // Enable cursor capture for input callbacks (& hide hint)
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // Scroll callback
    glfwSetScrollCallback(window, scroll_callback);

    // Enable z-buffer
    glEnable(GL_DEPTH_TEST);

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

void initLights()
{
    lights.push_back(new DirLight("dirLight", -0.2f, -0.2f, 0.1f));
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
