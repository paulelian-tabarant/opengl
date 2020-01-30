#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

#include "Shader.h"
#include "Camera.h"
#include "Light.h"
#include "Object.h"
#define STB_IMAGE_IMPLEMENTATION ;
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

unsigned int screenWidth = 800, screenHeight = 600;

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

int main() 
{
    glfwInit(); 
    // Set running versions of OpenGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Hello world", NULL, NULL);
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

    glm::vec3 cubePositions[] = {
        glm::vec3( 0.0f,  0.0f,   0.0f), 
        glm::vec3( 2.0f,  5.0f, -15.0f), 
        glm::vec3(-1.5f, -2.2f,  -2.5f),  
        glm::vec3(-3.8f, -2.0f, -12.3f),  
        glm::vec3( 2.4f, -0.4f,  -3.5f),  
        glm::vec3(-1.7f,  3.0f,  -7.5f),  
        glm::vec3( 1.3f, -2.0f,  -2.5f),  
        glm::vec3( 1.5f,  2.0f,  -2.5f), 
        glm::vec3( 1.5f,  0.2f,  -1.5f), 
        glm::vec3(-1.3f,  1.0f,  -1.5f)  
    };


    // Enable z-buffer
    glEnable(GL_DEPTH_TEST);

    // Shader initialization
    Shader objShader("vs.vert", "fs.frag");

    // Init light object (also rendered as cube)
    Shader lightShader("light_vs.vert", "light_fs.frag");

    Light light = Light();
    Object object = Object();
    camera = Camera();

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Handle user input in a specific function
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render objects
        objShader.use();
        light.writeToShader(objShader);
        camera.writeToShader(objShader, screenWidth, screenHeight);

        // Draw cube vertices for each cube position previously set
        for (unsigned int i = 0; i < 10; i++) {
            object.setPosition(cubePositions[i]);
            object.setAngle(20.0f * i);
            object.writeToShader(objShader);
            object.draw();
        }

        // Render light
        lightShader.use();
        camera.writeToShader(lightShader, screenWidth, screenHeight);
        light.writeModelMatrixInShader(lightShader, "model");
        light.draw();

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
