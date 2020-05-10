#include "Shader.h"

#include <glm/glm.hpp>
#include <vector>

class ScreenSpaceAO
{
public:
	ScreenSpaceAO(unsigned int screenWidth, unsigned int screenHeight)
		: SCREEN_WIDTH(screenWidth), SCREEN_HEIGHT(screenHeight)
	{
		glGenRenderbuffers(1, &gRenderBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, gRenderBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenWidth, screenHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gRenderBuffer);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete ! " << std::endl;

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glGenFramebuffers(1, &ssaoFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
		glGenTextures(1, &ssaoOutputTex);
		glBindTexture(GL_TEXTURE_2D, ssaoOutputTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoOutputTex, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "SSAO framebuffer not complete !" << std::endl;

		// SAAO blur (smooth AO result)
		glGenFramebuffers(1, &blurFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, blurFBO);
		glGenTextures(1, &ssaoBlurOutputTex);
		glBindTexture(GL_TEXTURE_2D, ssaoBlurOutputTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoBlurOutputTex, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "SSAO blur buffer not complete !" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Generate 64 random samples for ambient occlusion calculations
		for (unsigned int i = 0; i < 64; i++) {
			float x = ((rand() % 100) / 100.0) * 2.0 - 1.0;
			float y = ((rand() % 100) / 100.0) * 2.0 - 1.0;
			float z = ((rand() % 100) / 100.0);
			glm::vec3 sample {x, y, z};
			sample = glm::normalize(sample);
			// Distribute samples in bigger number on small lengths (close to 0.1)
			float t = i / (float)64;
			float scale = 0.1f + (t * t) * 0.9f;
			ssaoKernel[i] = sample * scale;
		}
		// Add random rotations
		for (unsigned int i = 0; i < 16; i++) {
			float dx = ((rand() % 100) / 100.0) * 2.0 - 1.0;
			float dy = ((rand() % 100) / 100.0) * 2.0 - 1.0;
			ssaoNoise[i] = glm::vec3(dx, dy, 0.0f);
		}

		glGenTextures(1, &noiseTex);
		glBindTexture(GL_TEXTURE_2D, noiseTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGBA, GL_FLOAT, &ssaoNoise[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	unsigned int getFbo() { return ssaoFBO; }
	unsigned int getBlurFbo() { return blurFBO; }
	unsigned int getOutputTexId() { return ssaoOutputTex; }
	unsigned int getBlurOutputTexId() { return ssaoBlurOutputTex; }

	void loadMainPass(Shader &shader, unsigned int gPositionTex, unsigned int gNormalTex)
	{
		glActiveTexture(GL_TEXTURE10);
		glBindTexture(GL_TEXTURE_2D, noiseTex);
		glActiveTexture(GL_TEXTURE29);
		glBindTexture(GL_TEXTURE_2D, gPositionTex);
		glActiveTexture(GL_TEXTURE30);
		glBindTexture(GL_TEXTURE_2D, gNormalTex);
		shader.use();
		for (unsigned int i = 0; i < 64; i++) {
			shader.setVec3("kernelSamples[" + std::to_string(i) + "]", ssaoKernel[i]);
		}
	}

	void loadBlurPass(Shader &shader)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ssaoOutputTex);
	}

private:
	unsigned int gPositionTex, gNormalTex, gColorSpecTex;
	unsigned int gRenderBuffer;
	unsigned int ssaoFBO;
	unsigned int ssaoOutputTex;
	unsigned int blurFBO;
	unsigned int ssaoBlurOutputTex;

	glm::vec3 ssaoKernel[64];
	glm::vec3 ssaoNoise[16];
	unsigned int noiseTex;

	const float SCREEN_WIDTH, SCREEN_HEIGHT;
};