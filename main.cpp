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
#include "DrawUtils.h"
#include "ImageBasedLighting.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>

const unsigned int screenWidth{1920}, screenHeight{1080};

// Mouse interaction
float lastX, lastY;
bool firstMouse{true};

// Timing
float deltaTime{0.0f},
		lastFrameTime{0.0f};

std::unique_ptr<Camera> camera;
std::unique_ptr<Model> objectModel;

// Deferred shading geometry pass
unsigned int gFrameBuffer, gRenderBuffer;
unsigned int gPositionTex, gNormalTex, gColorSpecTex;

// Buffers for simple geometry to be rendered
unsigned int quadVAO{0};
unsigned int quadVBO;
unsigned int cubeVAO{0};
unsigned int cubeVBO;

// GLFW callback functions
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double mouseX, double mouseY);
void scroll_callback(GLFWwindow *window, double dx, double dy);

void processInput(GLFWwindow *window);
void renderScene(Shader &shader);
void initGeometryPass();

int main(int argc, char *argv[])
{
	glfwInit();
	// Set running versions of OpenGL
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow *window = glfwCreateWindow(screenWidth, screenHeight, "Hello OpenGL", glfwGetPrimaryMonitor(), NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	// Tell GLFW to make the context of our window the main context of current thread
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
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

	// Enable z-buffer
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// Avoid non-linear effects when sampling low resolution cube maps
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	std::string modelPath = argv[1];
	std::string iblImagePath = argv[2];

	// CAUTION: always init buffers AFTER enabling GL_DEPTH_TEST

	// Initialization of geometry + SSAO passes
	initGeometryPass();
	ScreenSpaceAO ssao(screenWidth, screenHeight);

	// Image-based lighting object
	std::string iblImagesDir = "Images/";
	ImageBasedLighting ibl(iblImagesDir + iblImagePath, 5);

	// Shaders initialization
	Shader geomShader("geomVS.vert", "", "geomFS.frag");
	// Skybox
	Shader environmentShader("environmentVS.vert", "", "environmentFS.frag");
	environmentShader.use();
	ibl.setEnvMapTextures(environmentShader);
	// Screen space ambient occlusion
	// CAUTION: verify texture names !!!
	Shader ssaoShader("ssaoVS.vert", "", "ssaoFS.frag");
	ssaoShader.use();
	ssaoShader.setInt("positionTex", 29);
	ssaoShader.setInt("normalTex", 30);
	ssaoShader.setInt("noiseTex", 10);
	Shader ssaoBlurShader("ssaoVS.vert", "", "ssaoBlurFS.frag");
	ssaoBlurShader.use();
	ssaoBlurShader.setInt("ssaoTex", 0);
	// Init light object (also rendered as cube)
	Shader lightShader("lightVS.vert", "", "lightFS.frag");
	// HDR rendering
	Shader illumShader("illumVS.vert", "", "illumFS.frag");
	illumShader.use();
	illumShader.setInt("positionTex", 29);
	illumShader.setInt("normalTex", 30);
	illumShader.setInt("colorSpecTex", 31);
	illumShader.setInt("ssaoTex", 10);
	ibl.setTextures(illumShader);

	// Init camera object to navigate in the scene
	camera = std::make_unique<Camera>();

	objectModel = std::make_unique<Model>("Models/" + modelPath);
	objectModel->setPosition(0.0f, 0.0f, 0.0f);

	// Render loop
	while (!glfwWindowShouldClose(window))
	{
		// Handle user input in a specific function
		processInput(window);

		glBindFramebuffer(GL_FRAMEBUFFER, gFrameBuffer);
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		geomShader.use();
		camera->writeToShader(geomShader, screenWidth, screenHeight);
		renderScene(geomShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// SSAO passes (computation + blur)
		glBindFramebuffer(GL_FRAMEBUFFER, ssao.getFbo());
		glClear(GL_COLOR_BUFFER_BIT);
		ssaoShader.use();
		ssao.setUniforms(ssaoShader, gPositionTex, gNormalTex);
		camera->writeToShader(ssaoShader, screenWidth, screenHeight);
		DrawUtils::renderQuad(quadVAO, quadVBO);
		glBindFramebuffer(GL_FRAMEBUFFER, ssao.getBlurFbo());
		glClear(GL_COLOR_BUFFER_BIT);
		ssaoBlurShader.use();
		ssao.setBlurUniforms(ssaoBlurShader);
		DrawUtils::renderQuad(quadVAO, quadVBO);
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
		glBindTexture(GL_TEXTURE_2D, ssao.getBlurOutputTexId());
		glActiveTexture(GL_TEXTURE11);
		ibl.setUniforms(illumShader);
		camera->writeToShader(illumShader, screenWidth, screenHeight);
		illumShader.setFloat("attenuation.kc", PointLight::attenuation.constant);
		illumShader.setFloat("attenuation.kl", PointLight::attenuation.linear);
		illumShader.setFloat("attenuation.kq", PointLight::attenuation.quadratic);
		DrawUtils::renderQuad(quadVAO, quadVBO);

		// Draw envmap image in the background
		environmentShader.use();
		camera->writeToShader(environmentShader, screenWidth, screenHeight);
		ibl.setEnvMapUniforms(environmentShader);
		DrawUtils::renderCube(cubeVAO, cubeVBO);

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
	shader.setMatrix4f("model", objectModel->getModelMat());
	objectModel->draw(shader);
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

	GLenum frameBufferTextures[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
	glDrawBuffers(3, frameBufferTextures);

	glGenRenderbuffers(1, &gRenderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, gRenderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenWidth, screenHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gRenderBuffer);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete ! " << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *window, double mouseX, double mouseY)
{
	if (firstMouse)
	{
		lastX = mouseX;
		lastY = mouseY;
		firstMouse = false;
	}

	float dx = mouseX - lastX, dy = mouseY - lastY;
	camera->rotateFromInput(dx, dy);

	lastX = mouseX;
	lastY = mouseY;
}

void scroll_callback(GLFWwindow *window, double dx, double dy)
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
